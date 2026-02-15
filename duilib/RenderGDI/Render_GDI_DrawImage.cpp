// Render_GDI.cpp 的补充部分 - 图片绘制（九宫格）

#ifdef DUILIB_BUILD_FOR_WIN

#include "Render_GDI.h"
#include "Bitmap_GDI.h"
#include "Matrix_GDI.h"
#include "duilib/Utils/PerformanceUtil.h"

namespace ui {

// 计算平铺绘制次数（与 Skia 实现对齐）
static int32_t CalcDrawImageTimes(int32_t nAvailableSpace, int32_t nImageSize, int32_t nTiledMargin, bool bFullyTiled)
{
    if (nAvailableSpace <= 0 || nImageSize <= 0) {
        return 0;
    }

    int32_t firstImageRequired = bFullyTiled ? nImageSize : 1;
    if (nAvailableSpace < firstImageRequired) {
        return 0;
    }

    int32_t remainingSpace = nAvailableSpace - nImageSize;
    int32_t drawTimes = 1;

    if (remainingSpace <= 0) {
        return drawTimes;
    }

    while (true) {
        if (remainingSpace < nTiledMargin) {
            break;
        }
        remainingSpace -= nTiledMargin;

        if (remainingSpace <= 0) {
            break;
        }

        if ((!bFullyTiled) || (remainingSpace >= nImageSize)) {
            drawTimes++;
            remainingSpace -= nImageSize;
        }
        else {
            break;
        }
    }

    return drawTimes;
}

void Render_GDI::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                           const UiRect& rcDest, const UiRect& rcDestCorners,
                           const UiRect& rcSource, const UiRect& rcSourceCorners,
                           uint8_t uFade,
                           const TiledDrawParam* pTiledDrawParam,
                           bool bWindowShadowMode)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    UiRect rcTestTemp;
    if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
        return;
    }

    PerformanceStat statPerformance(_T("Render_GDI::DrawImage"));

    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Bitmap_GDI* pGdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
    ASSERT(pGdiBitmap != nullptr);
    if (pGdiBitmap == nullptr || pGdiBitmap->GetBitmap() == nullptr) {
        return;
    }

    Gdiplus::Bitmap* pGdipBitmap = pGdiBitmap->GetBitmap();

    // 设置图像属性（透明度）
    Gdiplus::ImageAttributes imageAttr;
    if (uFade != 255) {
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, uFade / 255.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
    }

    Gdiplus::ImageAttributes* pImageAttr = (uFade != 255) ? &imageAttr : nullptr;

    UiRect rcTemp;
    UiRect rcDrawSource;
    UiRect rcDrawDest;

    // 平铺绘制参数
    bool bTiledX = false;
    bool bTiledY = false;
    bool bFullTiledX = false;
    bool bFullTiledY = false;
    int32_t nTiledMarginX = 0;
    int32_t nTiledMarginY = 0;
    UiPadding rcTiledPadding;

    if (pTiledDrawParam != nullptr) {
        bTiledX = pTiledDrawParam->m_bTiledX;
        bTiledY = pTiledDrawParam->m_bTiledY;
        bFullTiledX = pTiledDrawParam->m_bFullTiledX;
        bFullTiledY = pTiledDrawParam->m_bFullTiledY;
        nTiledMarginX = pTiledDrawParam->m_nTiledMarginX;
        nTiledMarginY = pTiledDrawParam->m_nTiledMarginY;
        rcTiledPadding = pTiledDrawParam->m_rcTiledPadding;
    }

    // 目标中间区域
    rcDrawDest.left = rcDest.left + rcDestCorners.left;
    rcDrawDest.top = rcDest.top + rcDestCorners.top;
    rcDrawDest.right = rcDest.right - rcDestCorners.right;
    rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

    if (bTiledX || bTiledY) {
        rcDrawDest.Deflate(rcTiledPadding);
    }

    // 源中间区域
    rcDrawSource.left = rcSource.left + rcSourceCorners.left;
    rcDrawSource.top = rcSource.top + rcSourceCorners.top;
    rcDrawSource.right = rcSource.right - rcSourceCorners.right;
    rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;

    if (rcDestCorners.IsZero()) {
        bWindowShadowMode = false;
    }

    // 绘制中间部分
    if (!bWindowShadowMode && UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
        if (!bTiledX && !bTiledY) {
            // 拉伸绘制
            Gdiplus::RectF destRect(rcDrawDest.left + m_pPointOrg->X,
                                   rcDrawDest.top + m_pPointOrg->Y,
                                   static_cast<Gdiplus::REAL>(rcDrawDest.Width()),
                                   static_cast<Gdiplus::REAL>(rcDrawDest.Height()));

            m_pGraphics->DrawImage(pGdipBitmap, destRect,
                                  static_cast<Gdiplus::REAL>(rcDrawSource.left),
                                  static_cast<Gdiplus::REAL>(rcDrawSource.top),
                                  static_cast<Gdiplus::REAL>(rcDrawSource.Width()),
                                  static_cast<Gdiplus::REAL>(rcDrawSource.Height()),
                                  Gdiplus::UnitPixel, pImageAttr);
        }
        else if (bTiledX && bTiledY) {
            // 横向和纵向平铺
            const int32_t nImageWidth = rcSource.Width() - rcSourceCorners.left - rcSourceCorners.right;
            const int32_t nImageHeight = rcSource.Height() - rcSourceCorners.top - rcSourceCorners.bottom;
            const int32_t iTimesX = CalcDrawImageTimes(rcDrawDest.Width(), nImageWidth, nTiledMarginX, bFullTiledX);
            const int32_t iTimesY = CalcDrawImageTimes(rcDrawDest.Height(), nImageHeight, nTiledMarginY, bFullTiledY);

            int32_t nPosY = rcDrawDest.top;
            for (int32_t j = 0; j < iTimesY; ++j) {
                if (j > 0) nPosY += nTiledMarginY;
                int32_t lDestTop = nPosY;
                int32_t lDestBottom = lDestTop + nImageHeight;
                int32_t lDrawHeight = nImageHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= (lDestBottom - rcDrawDest.bottom);
                    lDestBottom = rcDrawDest.bottom;
                }

                int32_t nPosX = rcDrawDest.left;
                for (int32_t i = 0; i < iTimesX; ++i) {
                    if (i > 0) nPosX += nTiledMarginX;
                    int32_t lDestLeft = nPosX;
                    int32_t lDestRight = lDestLeft + nImageWidth;
                    int32_t lDrawWidth = nImageWidth;
                    if (lDestRight > rcDrawDest.right) {
                        lDrawWidth -= (lDestRight - rcDrawDest.right);
                        lDestRight = rcDrawDest.right;
                    }

                    Gdiplus::RectF destRect(lDestLeft + m_pPointOrg->X,
                                           lDestTop + m_pPointOrg->Y,
                                           static_cast<Gdiplus::REAL>(lDrawWidth),
                                           static_cast<Gdiplus::REAL>(lDrawHeight));

                    m_pGraphics->DrawImage(pGdipBitmap, destRect,
                                          static_cast<Gdiplus::REAL>(rcDrawSource.left),
                                          static_cast<Gdiplus::REAL>(rcDrawSource.top),
                                          static_cast<Gdiplus::REAL>(lDrawWidth),
                                          static_cast<Gdiplus::REAL>(lDrawHeight),
                                          Gdiplus::UnitPixel, pImageAttr);

                    nPosX += lDrawWidth;
                }
                nPosY += lDrawHeight;
            }
        }
        else if (bTiledX) {
            // 横向平铺
            const int32_t nImageWidth = rcSource.Width() - rcSourceCorners.left - rcSourceCorners.right;
            const int32_t iTimesX = CalcDrawImageTimes(rcDrawDest.Width(), nImageWidth, nTiledMarginX, bFullTiledX);

            int32_t nPosX = rcDrawDest.left;
            for (int32_t i = 0; i < iTimesX; ++i) {
                if (i > 0) nPosX += nTiledMarginX;
                int32_t lDestLeft = nPosX;
                int32_t lDestRight = lDestLeft + nImageWidth;
                int32_t lDrawWidth = nImageWidth;
                if (lDestRight > rcDrawDest.right) {
                    lDrawWidth -= (lDestRight - rcDrawDest.right);
                    lDestRight = rcDrawDest.right;
                }

                Gdiplus::RectF destRect(lDestLeft + m_pPointOrg->X,
                                       rcDrawDest.top + m_pPointOrg->Y,
                                       static_cast<Gdiplus::REAL>(lDrawWidth),
                                       static_cast<Gdiplus::REAL>(rcDrawDest.Height()));

                m_pGraphics->DrawImage(pGdipBitmap, destRect,
                                      static_cast<Gdiplus::REAL>(rcDrawSource.left),
                                      static_cast<Gdiplus::REAL>(rcDrawSource.top),
                                      static_cast<Gdiplus::REAL>(lDrawWidth),
                                      static_cast<Gdiplus::REAL>(rcDrawSource.Height()),
                                      Gdiplus::UnitPixel, pImageAttr);

                nPosX += lDrawWidth;
            }
        }
        else { // bTiledY
            // 纵向平铺
            const int32_t nImageHeight = rcSource.Height() - rcSourceCorners.top - rcSourceCorners.bottom;
            const int32_t iTimesY = CalcDrawImageTimes(rcDrawDest.Height(), nImageHeight, nTiledMarginY, bFullTiledY);

            int32_t nPosY = rcDrawDest.top;
            for (int32_t i = 0; i < iTimesY; ++i) {
                if (i > 0) nPosY += nTiledMarginY;
                int32_t lDestTop = nPosY;
                int32_t lDestBottom = lDestTop + nImageHeight;
                int32_t lDrawHeight = nImageHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= (lDestBottom - rcDrawDest.bottom);
                    lDestBottom = rcDrawDest.bottom;
                }

                Gdiplus::RectF destRect(rcDrawDest.left + m_pPointOrg->X,
                                       lDestTop + m_pPointOrg->Y,
                                       static_cast<Gdiplus::REAL>(rcDrawDest.Width()),
                                       static_cast<Gdiplus::REAL>(lDrawHeight));

                m_pGraphics->DrawImage(pGdipBitmap, destRect,
                                      static_cast<Gdiplus::REAL>(rcDrawSource.left),
                                      static_cast<Gdiplus::REAL>(rcDrawSource.top),
                                      static_cast<Gdiplus::REAL>(rcDrawSource.Width()),
                                      static_cast<Gdiplus::REAL>(lDrawHeight),
                                      Gdiplus::UnitPixel, pImageAttr);

                nPosY += lDrawHeight;
            }
        }
    }

    // 绘制九个边角（与 Skia 实现完全对齐）
    auto DrawCorner = [&](const UiRect& dest, const UiRect& src) {
        if (UiRect::Intersect(rcTemp, rcPaint, dest)) {
            Gdiplus::RectF destRect(dest.left + m_pPointOrg->X,
                                   dest.top + m_pPointOrg->Y,
                                   static_cast<Gdiplus::REAL>(dest.Width()),
                                   static_cast<Gdiplus::REAL>(dest.Height()));

            m_pGraphics->DrawImage(pGdipBitmap, destRect,
                                  static_cast<Gdiplus::REAL>(src.left),
                                  static_cast<Gdiplus::REAL>(src.top),
                                  static_cast<Gdiplus::REAL>(src.Width()),
                                  static_cast<Gdiplus::REAL>(src.Height()),
                                  Gdiplus::UnitPixel, pImageAttr);
        }
    };

    // 左上角
    if (rcSourceCorners.left > 0 && rcSourceCorners.top > 0) {
        rcDrawDest = UiRect(rcDest.left, rcDest.top,
                           rcDest.left + rcDestCorners.left,
                           rcDest.top + rcDestCorners.top);
        rcDrawSource = UiRect(rcSource.left, rcSource.top,
                             rcSource.left + rcSourceCorners.left,
                             rcSource.top + rcSourceCorners.top);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 上边
    if (rcSourceCorners.top > 0) {
        rcDrawDest = UiRect(rcDest.left + rcDestCorners.left, rcDest.top,
                           rcDest.right - rcDestCorners.right, rcDest.top + rcDestCorners.top);
        rcDrawSource = UiRect(rcSource.left + rcSourceCorners.left, rcSource.top,
                             rcSource.right - rcSourceCorners.right, rcSource.top + rcSourceCorners.top);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 右上角
    if (rcSourceCorners.right > 0 && rcSourceCorners.top > 0) {
        rcDrawDest = UiRect(rcDest.right - rcDestCorners.right, rcDest.top,
                           rcDest.right, rcDest.top + rcDestCorners.top);
        rcDrawSource = UiRect(rcSource.right - rcSourceCorners.right, rcSource.top,
                             rcSource.right, rcSource.top + rcSourceCorners.top);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 左边
    if (rcSourceCorners.left > 0) {
        rcDrawDest = UiRect(rcDest.left, rcDest.top + rcDestCorners.top,
                           rcDest.left + rcDestCorners.left, rcDest.bottom - rcDestCorners.bottom);
        rcDrawSource = UiRect(rcSource.left, rcSource.top + rcSourceCorners.top,
                             rcSource.left + rcSourceCorners.left, rcSource.bottom - rcSourceCorners.bottom);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 右边
    if (rcSourceCorners.right > 0) {
        rcDrawDest = UiRect(rcDest.right - rcDestCorners.right, rcDest.top + rcDestCorners.top,
                           rcDest.right, rcDest.bottom - rcDestCorners.bottom);
        rcDrawSource = UiRect(rcSource.right - rcSourceCorners.right, rcSource.top + rcSourceCorners.top,
                             rcSource.right, rcSource.bottom - rcSourceCorners.bottom);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 左下角
    if (rcSourceCorners.left > 0 && rcSourceCorners.bottom > 0) {
        rcDrawDest = UiRect(rcDest.left, rcDest.bottom - rcDestCorners.bottom,
                           rcDest.left + rcDestCorners.left, rcDest.bottom);
        rcDrawSource = UiRect(rcSource.left, rcSource.bottom - rcSourceCorners.bottom,
                             rcSource.left + rcSourceCorners.left, rcSource.bottom);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 下边
    if (rcSourceCorners.bottom > 0) {
        rcDrawDest = UiRect(rcDest.left + rcDestCorners.left, rcDest.bottom - rcDestCorners.bottom,
                           rcDest.right - rcDestCorners.right, rcDest.bottom);
        rcDrawSource = UiRect(rcSource.left + rcSourceCorners.left, rcSource.bottom - rcSourceCorners.bottom,
                             rcSource.right - rcSourceCorners.right, rcSource.bottom);
        DrawCorner(rcDrawDest, rcDrawSource);
    }

    // 右下角
    if (rcSourceCorners.right > 0 && rcSourceCorners.bottom > 0) {
        rcDrawDest = UiRect(rcDest.right - rcDestCorners.right, rcDest.bottom - rcDestCorners.bottom,
                           rcDest.right, rcDest.bottom);
        rcDrawSource = UiRect(rcSource.right - rcSourceCorners.right, rcSource.bottom - rcSourceCorners.bottom,
                             rcSource.right, rcSource.bottom);
        DrawCorner(rcDrawDest, rcDrawSource);
    }
}

void Render_GDI::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                           const UiRect& rcDest, const UiRect& rcSource,
                           uint8_t uFade,
                           const TiledDrawParam* pTiledDrawParam,
                           bool bWindowShadowMode)
{
    UiRect rcDestCorners;
    UiRect rcSourceCorners;
    return DrawImage(rcPaint, pBitmap,
                    rcDest, rcDestCorners,
                    rcSource, rcSourceCorners,
                    uFade, pTiledDrawParam, bWindowShadowMode);
}

void Render_GDI::DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
                               const UiRect& rcDest, const UiRect& rcSource,
                               uint8_t uFade, IMatrix* pMatrix)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (pMatrix == nullptr) {
        UiRect rcTestTemp;
        if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
            return;
        }
    }

    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Bitmap_GDI* pGdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
    ASSERT(pGdiBitmap != nullptr);
    if (pGdiBitmap == nullptr || pGdiBitmap->GetBitmap() == nullptr) {
        return;
    }

    // 设置图像属性
    Gdiplus::ImageAttributes imageAttr;
    if (uFade != 255) {
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, uFade / 255.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
    }

    // 应用矩阵变换
    bool isMatrixSet = false;
    if (pMatrix != nullptr) {
        Matrix_GDI* pGdiMatrix = dynamic_cast<Matrix_GDI*>(pMatrix);
        if (pGdiMatrix != nullptr && pGdiMatrix->GetMatrix() != nullptr) {
            m_pGraphics->SetTransform(pGdiMatrix->GetMatrix());
            isMatrixSet = true;
        }
    }

    Gdiplus::RectF destRect(rcDest.left + m_pPointOrg->X,
                           rcDest.top + m_pPointOrg->Y,
                           static_cast<Gdiplus::REAL>(rcDest.Width()),
                           static_cast<Gdiplus::REAL>(rcDest.Height()));

    m_pGraphics->DrawImage(pGdiBitmap->GetBitmap(), destRect,
                          static_cast<Gdiplus::REAL>(rcSource.left),
                          static_cast<Gdiplus::REAL>(rcSource.top),
                          static_cast<Gdiplus::REAL>(rcSource.Width()),
                          static_cast<Gdiplus::REAL>(rcSource.Height()),
                          Gdiplus::UnitPixel,
                          uFade != 255 ? &imageAttr : nullptr);

    if (isMatrixSet) {
        m_pGraphics->ResetTransform();
    }
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
