// Render_GDI.cpp 的补充部分 - 其他辅助函数


#include "Render_GDI.h"
#include "Bitmap_GDI.h"
#include "duilib/Render/BitmapAlpha.h"

namespace ui {

void Render_GDI::DrawBoxShadow(const UiRect& rc,
                              const UiSize& roundSize,
                              const UiPoint& cpOffset,
                              int32_t nBlurRadius,
                              int32_t nSpreadRadius,
                              UiColor dwColor)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(dwColor.GetARGB() != 0);
    if (nBlurRadius < 0) {
        nBlurRadius = 0;
    }

    if (m_pGraphics == nullptr) {
        return;
    }

    // GDI+ 实现阴影需要手动实现模糊效果
    // 简化实现：使用半透明矩形近似
    // 完整实现需要高斯模糊算法

    UiRect destRc = rc;
    destRc.left -= nSpreadRadius;
    destRc.top -= nSpreadRadius;
    destRc.right += nSpreadRadius;
    destRc.bottom += nSpreadRadius;

    // 应用偏移
    destRc.Offset(cpOffset.x, cpOffset.y);

    // 创建阴影路径
    Gdiplus::GraphicsPath shadowPath;
    Gdiplus::RectF rect(destRc.left + m_pPointOrg->X,
                       destRc.top + m_pPointOrg->Y,
                       static_cast<Gdiplus::REAL>(destRc.Width()),
                       static_cast<Gdiplus::REAL>(destRc.Height()));

    if (roundSize.cx > 0 && roundSize.cy > 0) {
        // 圆角矩形
        float diameter = roundSize.cx * 2.0f;
        Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);

        shadowPath.AddArc(arc, 180, 90);
        arc.X = rect.GetRight() - diameter;
        shadowPath.AddArc(arc, 270, 90);
        arc.Y = rect.GetBottom() - diameter;
        shadowPath.AddArc(arc, 0, 90);
        arc.X = rect.GetLeft();
        shadowPath.AddArc(arc, 90, 90);
        shadowPath.CloseFigure();
    }
    else {
        shadowPath.AddRectangle(rect);
    }

    // 创建排除原始区域的路径
    Gdiplus::GraphicsPath excludePath;
    Gdiplus::RectF excludeRect(rc.left + m_pPointOrg->X,
                              rc.top + m_pPointOrg->Y,
                              static_cast<Gdiplus::REAL>(rc.Width()),
                              static_cast<Gdiplus::REAL>(rc.Height()));

    if (roundSize.cx > 0 && roundSize.cy > 0) {
        float diameter = roundSize.cx * 2.0f;
        Gdiplus::RectF arc(excludeRect.X, excludeRect.Y, diameter, diameter);

        excludePath.AddArc(arc, 180, 90);
        arc.X = excludeRect.GetRight() - diameter;
        excludePath.AddArc(arc, 270, 90);
        arc.Y = excludeRect.GetBottom() - diameter;
        excludePath.AddArc(arc, 0, 90);
        arc.X = excludeRect.GetLeft();
        excludePath.AddArc(arc, 90, 90);
        excludePath.CloseFigure();
    }
    else {
        excludePath.AddRectangle(excludeRect);
    }

    // 保存状态
    Gdiplus::GraphicsState state = m_pGraphics->Save();

    // 裁剪中间区域
    m_pGraphics->SetClip(&excludePath, Gdiplus::CombineModeExclude);

    // 简化的阴影绘制（多层半透明）
    int layers = std::min(nBlurRadius / 2, 10);
    if (layers == 0) layers = 1;

    for (int i = 0; i < layers; ++i) {
        float ratio = 1.0f - (float)i / layers;
        int alpha = static_cast<int>(dwColor.GetA() * ratio / layers);

        Gdiplus::Color shadowColor(alpha, dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
        Gdiplus::SolidBrush brush(shadowColor);

        m_pGraphics->FillPath(&brush, &shadowPath);

        // 扩展路径
        Gdiplus::Matrix matrix;
        matrix.Translate(0.5f, 0.5f);
        shadowPath.Transform(&matrix);
    }

    // 恢复状态
    m_pGraphics->Restore(state);
}

void Render_GDI::Clear(const UiColor& uiColor)
{
    if (m_pGraphics != nullptr) {
        Gdiplus::Color gdipColor = UiColorToGdiplusColor(uiColor);
        m_pGraphics->Clear(gdipColor);
    }
}

void Render_GDI::ClearRect(const UiRect& rcDirty, const UiColor& uiColor)
{
    FillRect(rcDirty, uiColor, 255);
}

std::unique_ptr<IRender> Render_GDI::Clone()
{
    std::unique_ptr<Render_GDI> pNewRender(new Render_GDI());
    if (pNewRender != nullptr) {
        pNewRender->Resize(m_nWidth, m_nHeight);
        pNewRender->SetRenderDpi(m_spRenderDpi);

        // 复制位图数据
        if (m_pBitmap != nullptr && pNewRender->m_pBitmap != nullptr) {
            Gdiplus::Graphics* pGraphics = Gdiplus::Graphics::FromImage(pNewRender->m_pBitmap);
            if (pGraphics != nullptr) {
                pGraphics->DrawImage(m_pBitmap, 0, 0);
                delete pGraphics;
            }
        }
    }
    return pNewRender;
}

bool Render_GDI::ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen)
{
    ASSERT(dstPixels != nullptr);
    if (dstPixels == nullptr || m_pBitmap == nullptr) {
        return false;
    }

    ASSERT(!rc.IsEmpty());
    if (rc.IsEmpty()) {
        return false;
    }

    ASSERT(dstPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (dstPixelsLen < (rc.Width() * rc.Height() * sizeof(uint32_t))) {
        return false;
    }

    Gdiplus::Rect gdipRect(rc.left + static_cast<int>(m_pPointOrg->X),
                          rc.top + static_cast<int>(m_pPointOrg->Y),
                          rc.Width(), rc.Height());

    Gdiplus::BitmapData bitmapData;
    Gdiplus::Status status = m_pBitmap->LockBits(&gdipRect,
                                                 Gdiplus::ImageLockModeRead,
                                                 PixelFormat32bppARGB,
                                                 &bitmapData);

    if (status != Gdiplus::Ok) {
        return false;
    }

    // 复制数据
    memcpy(dstPixels, bitmapData.Scan0, rc.Width() * rc.Height() * 4);

    m_pBitmap->UnlockBits(&bitmapData);

    return true;
}

bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc)
{
    ASSERT(srcPixels != nullptr);
    if (srcPixels == nullptr || m_pBitmap == nullptr) {
        return false;
    }

    ASSERT(!rc.IsEmpty());
    if (rc.IsEmpty()) {
        return false;
    }

    ASSERT(srcPixelsLen == (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (srcPixelsLen != (rc.Width() * rc.Height() * sizeof(uint32_t))) {
        return false;
    }

    Gdiplus::Rect gdipRect(rc.left + static_cast<int>(m_pPointOrg->X),
                          rc.top + static_cast<int>(m_pPointOrg->Y),
                          rc.Width(), rc.Height());

    Gdiplus::BitmapData bitmapData;
    Gdiplus::Status status = m_pBitmap->LockBits(&gdipRect,
                                                 Gdiplus::ImageLockModeWrite,
                                                 PixelFormat32bppARGB,
                                                 &bitmapData);

    if (status != Gdiplus::Ok) {
        return false;
    }

    // 复制数据
    memcpy(bitmapData.Scan0, srcPixels, rc.Width() * rc.Height() * 4);

    m_pBitmap->UnlockBits(&bitmapData);

    return true;
}

bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint)
{
    if (rc == rcPaint) {
        return WritePixels(srcPixels, srcPixelsLen, rc);
    }

    // 计算交集区域
    UiRect updateRect = rc;
    updateRect.Intersect(rcPaint);

    if (updateRect.IsEmpty()) {
        return false;
    }

    // 计算源数据中的偏移
    int32_t srcOffsetX = updateRect.left - rc.left;
    int32_t srcOffsetY = updateRect.top - rc.top;

    // 锁定目标区域
    Gdiplus::Rect gdipRect(updateRect.left + static_cast<int>(m_pPointOrg->X),
                          updateRect.top + static_cast<int>(m_pPointOrg->Y),
                          updateRect.Width(), updateRect.Height());

    Gdiplus::BitmapData bitmapData;
    Gdiplus::Status status = m_pBitmap->LockBits(&gdipRect,
                                                 Gdiplus::ImageLockModeWrite,
                                                 PixelFormat32bppARGB,
                                                 &bitmapData);

    if (status != Gdiplus::Ok) {
        return false;
    }

    // 逐行复制数据
    uint8_t* pSrc = static_cast<uint8_t*>(srcPixels) + (srcOffsetY * rc.Width() + srcOffsetX) * 4;
    uint8_t* pDst = static_cast<uint8_t*>(bitmapData.Scan0);

    for (int32_t y = 0; y < updateRect.Height(); ++y) {
        memcpy(pDst, pSrc, updateRect.Width() * 4);
        pSrc += rc.Width() * 4;
        pDst += bitmapData.Stride;
    }

    m_pBitmap->UnlockBits(&bitmapData);

    return true;
}

RenderClipType Render_GDI::GetClipInfo(std::vector<UiRect>& clipRects)
{
    clipRects.clear();

    if (m_pGraphics == nullptr) {
        return RenderClipType::kEmpty;
    }

    Gdiplus::Region region;
    m_pGraphics->GetClip(&region);

    if (region.IsEmpty(m_pGraphics)) {
        return RenderClipType::kEmpty;
    }

    if (region.IsInfinite(m_pGraphics)) {
        // 整个画布
        clipRects.push_back(UiRect(0, 0, m_nWidth, m_nHeight));
        return RenderClipType::kRect;
    }

    // 获取裁剪区域的边界
    Gdiplus::RectF bounds;
    region.GetBounds(&bounds, m_pGraphics);

    UiRect rc(static_cast<int>(bounds.X - m_pPointOrg->X),
             static_cast<int>(bounds.Y - m_pPointOrg->Y),
             static_cast<int>(bounds.GetRight() - m_pPointOrg->X),
             static_cast<int>(bounds.GetBottom() - m_pPointOrg->Y));

    clipRects.push_back(rc);

    // 检查是否为简单矩形
    Gdiplus::Matrix matrix;
    UINT count = region.GetRegionScansCount(&matrix);
    if (count == 1) {
        return RenderClipType::kRect;
    }
    else {
        return RenderClipType::kRegion;
    }
}

bool Render_GDI::IsClipEmpty() const
{
    if (m_pGraphics != nullptr) {
        Gdiplus::Region region;
        m_pGraphics->GetClip(&region);
        return region.IsEmpty(m_pGraphics) ? true : false;
    }
    return false;
}

bool Render_GDI::IsEmpty() const
{
    return (m_pGraphics == nullptr) || (m_nWidth <= 0) || (m_nHeight <= 0);
}

void Render_GDI::SetRenderDpi(const IRenderDpiPtr& spRenderDpi)
{
    m_spRenderDpi = spRenderDpi;
}

bool Render_GDI::PaintAndSwapBuffers(IRenderPaint* pRenderPaint)
{
    if (pRenderPaint == nullptr || m_hWnd == nullptr) {
        return false;
    }

    // 获取更新区域
    UiRect rcUpdate;
    bool bPartialPaint = pRenderPaint->GetUpdateRect(rcUpdate);

    // 执行绘制
    UiRect rcPaint = bPartialPaint ? rcUpdate : UiRect(0, 0, m_nWidth, m_nHeight);
    if (!pRenderPaint->DoPaint(rcPaint)) {
        return false;
    }

    // 获取窗口 DC
    HDC hDC = ::GetDC(m_hWnd);
    if (hDC == nullptr) {
        return false;
    }

    // 创建兼容 DC 和位图
    Gdiplus::Graphics graphics(hDC);
    graphics.DrawImage(m_pBitmap, 0, 0);

    ::ReleaseDC(m_hWnd, hDC);

    return true;
}

bool Render_GDI::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw)
{
    if (m_hWnd == nullptr) {
        return false;
    }

    HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom,
                                     static_cast<int>(rx * 2), static_cast<int>(ry * 2));
    if (hRgn != nullptr) {
        ::SetWindowRgn(m_hWnd, hRgn, bRedraw ? TRUE : FALSE);
        return true;
    }
    return false;
}

bool Render_GDI::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw)
{
    if (m_hWnd == nullptr) {
        return false;
    }

    HRGN hRgn = ::CreateRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
    if (hRgn != nullptr) {
        ::SetWindowRgn(m_hWnd, hRgn, bRedraw ? TRUE : FALSE);
        return true;
    }
    return false;
}

void Render_GDI::ClearWindowRgn(bool bRedraw)
{
    if (m_hWnd != nullptr) {
        ::SetWindowRgn(m_hWnd, nullptr, bRedraw ? TRUE : FALSE);
    }
}

HDC Render_GDI::GetRenderDC(HWND hWnd)
{
    m_hWnd = hWnd;
    if (m_hDC == nullptr && m_pGraphics != nullptr) {
        m_hDC = m_pGraphics->GetHDC();
    }
    return m_hDC;
}

void Render_GDI::ReleaseRenderDC(HDC hdc)
{
    if (m_hDC != nullptr && m_pGraphics != nullptr) {
        m_pGraphics->ReleaseHDC(m_hDC);
        m_hDC = nullptr;
    }
}

} // namespace ui

