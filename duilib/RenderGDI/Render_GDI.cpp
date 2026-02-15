#include "Render_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "Font_GDI.h"
#include "Pen_GDI.h"
#include "Brush_GDI.h"
#include "Path_GDI.h"
#include "Matrix_GDI.h"
#include "Bitmap_GDI.h"
#include "duilib/Render/BitmapAlpha.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")

namespace ui {

Render_GDI::Render_GDI()
    : m_pBitmap(nullptr)
    , m_pGraphics(nullptr)
    , m_pPointOrg(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_backendType(RenderBackendType::kRaster_BackendType)
    , m_hWnd(nullptr)
    , m_hDC(nullptr)
{
    m_pPointOrg = new Gdiplus::PointF(0, 0);
}

Render_GDI::~Render_GDI()
{
    if (m_pGraphics != nullptr) {
        delete m_pGraphics;
        m_pGraphics = nullptr;
    }
    if (m_pBitmap != nullptr) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }
    if (m_pPointOrg != nullptr) {
        delete m_pPointOrg;
        m_pPointOrg = nullptr;
    }
}

RenderType Render_GDI::GetRenderType() const
{
    return RenderType::kRenderType_GDI;
}

RenderBackendType Render_GDI::GetRenderBackendType() const
{
    return m_backendType;
}

int32_t Render_GDI::GetWidth() const
{
    return m_nWidth;
}

int32_t Render_GDI::GetHeight() const
{
    return m_nHeight;
}

bool Render_GDI::Resize(int32_t width, int32_t height)
{
    if ((width <= 0) || (height <= 0)) {
        return false;
    }

    if ((m_nWidth == width) && (m_nHeight == height)) {
        return true;
    }

    // 清理旧对象
    if (m_pGraphics != nullptr) {
        delete m_pGraphics;
        m_pGraphics = nullptr;
    }
    if (m_pBitmap != nullptr) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }

    // 创建新位图
    m_pBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
    if (m_pBitmap == nullptr) {
        return false;
    }

    // 创建 Graphics 对象
    m_pGraphics = Gdiplus::Graphics::FromImage(m_pBitmap);
    if (m_pGraphics == nullptr) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
        return false;
    }

    // 设置渲染质量
    m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    m_pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    m_nWidth = width;
    m_nHeight = height;

    return true;
}

void* Render_GDI::GetPixelBits() const
{
    if (m_pBitmap == nullptr) {
        return nullptr;
    }

    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, m_nWidth, m_nHeight);

    if (m_pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead,
                           PixelFormat32bppARGB, &bitmapData) == Gdiplus::Ok) {
        void* pBits = bitmapData.Scan0;
        m_pBitmap->UnlockBits(&bitmapData);
        return pBits;
    }

    return nullptr;
}

UiPoint Render_GDI::OffsetWindowOrg(UiPoint ptOffset)
{
    UiPoint ptOldOrg = { static_cast<int32_t>(m_pPointOrg->X), 
                         static_cast<int32_t>(m_pPointOrg->Y) };
    
    m_pPointOrg->X -= static_cast<Gdiplus::REAL>(ptOffset.x);
    m_pPointOrg->Y -= static_cast<Gdiplus::REAL>(ptOffset.y);
    
    return ptOldOrg;
}

UiPoint Render_GDI::SetWindowOrg(UiPoint pt)
{
    UiPoint ptOldOrg = { static_cast<int32_t>(m_pPointOrg->X),
                         static_cast<int32_t>(m_pPointOrg->Y) };
    
    m_pPointOrg->X = static_cast<Gdiplus::REAL>(pt.x);
    m_pPointOrg->Y = static_cast<Gdiplus::REAL>(pt.y);
    
    return ptOldOrg;
}

UiPoint Render_GDI::GetWindowOrg() const
{
    return UiPoint { static_cast<int32_t>(m_pPointOrg->X),
                     static_cast<int32_t>(m_pPointOrg->Y) };
}

void Render_GDI::SaveClip(int32_t& nState)
{
    if (m_pGraphics != nullptr) {
        nState = m_pGraphics->Save();
        m_stateStack.push_back(static_cast<Gdiplus::GraphicsState>(nState));
    }
}

void Render_GDI::RestoreClip(int32_t nState)
{
    if (m_pGraphics != nullptr && !m_stateStack.empty()) {
        if (m_stateStack.back() == static_cast<Gdiplus::GraphicsState>(nState)) {
            m_pGraphics->Restore(static_cast<Gdiplus::GraphicsState>(nState));
            m_stateStack.pop_back();
        }
    }
}

void Render_GDI::SetClip(const UiRect& rc, bool bIntersect)
{
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Rect gdipRect(rc.left + static_cast<int>(m_pPointOrg->X),
                          rc.top + static_cast<int>(m_pPointOrg->Y),
                          rc.Width(), rc.Height());

    m_pGraphics->Save();
    
    if (bIntersect) {
        m_pGraphics->SetClip(gdipRect, Gdiplus::CombineModeIntersect);
    }
    else {
        m_pGraphics->SetClip(gdipRect, Gdiplus::CombineModeExclude);
    }
}

void Render_GDI::SetRoundClip(const UiRect& rcItem, float rx, float ry, bool bIntersect)
{
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::GraphicsPath path;
    Gdiplus::RectF rect(static_cast<Gdiplus::REAL>(rcItem.left + m_pPointOrg->X),
                       static_cast<Gdiplus::REAL>(rcItem.top + m_pPointOrg->Y),
                       static_cast<Gdiplus::REAL>(rcItem.Width()),
                       static_cast<Gdiplus::REAL>(rcItem.Height()));

    // 创建圆角矩形路径
    Gdiplus::REAL diameter = rx * 2;
    Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);
    
    path.AddArc(arc, 180, 90);
    arc.X = rect.GetRight() - diameter;
    path.AddArc(arc, 270, 90);
    arc.Y = rect.GetBottom() - diameter;
    path.AddArc(arc, 0, 90);
    arc.X = rect.GetLeft();
    path.AddArc(arc, 90, 90);
    path.CloseFigure();

    m_pGraphics->Save();
    
    if (bIntersect) {
        m_pGraphics->SetClip(&path, Gdiplus::CombineModeIntersect);
    }
    else {
        m_pGraphics->SetClip(&path, Gdiplus::CombineModeExclude);
    }
}

void Render_GDI::ClearClip()
{
    if (m_pGraphics != nullptr && !m_stateStack.empty()) {
        m_pGraphics->Restore(m_stateStack.back());
        m_stateStack.pop_back();
    }
}

Gdiplus::CompositingMode Render_GDI::GetCompositingMode(RopMode rop) const
{
    switch (rop) {
    case RopMode::kSrcCopy:
        return Gdiplus::CompositingModeSourceCopy;
    case RopMode::kSrcAnd:
    case RopMode::kSrcInvert:
    case RopMode::kDstInvert:
    default:
        return Gdiplus::CompositingModeSourceOver;
    }
}

bool Render_GDI::BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy,
                        IRender* pSrcRender, int32_t xSrc, int32_t ySrc,
                        RopMode rop)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr || m_pGraphics == nullptr) {
        return false;
    }

    Render_GDI* pGdiRender = dynamic_cast<Render_GDI*>(pSrcRender);
    ASSERT(pGdiRender != nullptr);
    if (pGdiRender == nullptr || pGdiRender->m_pBitmap == nullptr) {
        return false;
    }

    Gdiplus::Rect destRect(x + static_cast<int>(m_pPointOrg->X),
                          y + static_cast<int>(m_pPointOrg->Y),
                          cx, cy);

    Gdiplus::CompositingMode oldMode = m_pGraphics->GetCompositingMode();
    m_pGraphics->SetCompositingMode(GetCompositingMode(rop));

    Gdiplus::Status status = m_pGraphics->DrawImage(pGdiRender->m_pBitmap,
                                                    destRect,
                                                    xSrc, ySrc, cx, cy,
                                                    Gdiplus::UnitPixel);

    m_pGraphics->SetCompositingMode(oldMode);

    return (status == Gdiplus::Ok);
}

bool Render_GDI::StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
                            RopMode rop)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr || m_pGraphics == nullptr) {
        return false;
    }

    Render_GDI* pGdiRender = dynamic_cast<Render_GDI*>(pSrcRender);
    ASSERT(pGdiRender != nullptr);
    if (pGdiRender == nullptr || pGdiRender->m_pBitmap == nullptr) {
        return false;
    }

    Gdiplus::Rect destRect(xDest + static_cast<int>(m_pPointOrg->X),
                          yDest + static_cast<int>(m_pPointOrg->Y),
                          widthDest, heightDest);

    Gdiplus::CompositingMode oldMode = m_pGraphics->GetCompositingMode();
    m_pGraphics->SetCompositingMode(GetCompositingMode(rop));

    Gdiplus::Status status = m_pGraphics->DrawImage(pGdiRender->m_pBitmap,
                                                    destRect,
                                                    xSrc, ySrc, widthSrc, heightSrc,
                                                    Gdiplus::UnitPixel);

    m_pGraphics->SetCompositingMode(oldMode);

    return (status == Gdiplus::Ok);
}

bool Render_GDI::AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
                            uint8_t alpha)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr || m_pGraphics == nullptr) {
        return false;
    }

    Render_GDI* pGdiRender = dynamic_cast<Render_GDI*>(pSrcRender);
    ASSERT(pGdiRender != nullptr);
    if (pGdiRender == nullptr || pGdiRender->m_pBitmap == nullptr) {
        return false;
    }

    Gdiplus::Rect destRect(xDest + static_cast<int>(m_pPointOrg->X),
                          yDest + static_cast<int>(m_pPointOrg->Y),
                          widthDest, heightDest);

    // 设置透明度
    Gdiplus::ImageAttributes imageAttr;
    if (alpha != 255) {
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, alpha / 255.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
    }

    Gdiplus::Status status = m_pGraphics->DrawImage(pGdiRender->m_pBitmap,
                                                    destRect,
                                                    xSrc, ySrc, widthSrc, heightSrc,
                                                    Gdiplus::UnitPixel,
                                                    alpha != 255 ? &imageAttr : nullptr);

    return (status == Gdiplus::Ok);
}

Gdiplus::Color Render_GDI::UiColorToGdiplusColor(UiColor color, uint8_t alpha) const
{
    if (alpha != 255) {
        return Gdiplus::Color(alpha, color.GetR(), color.GetG(), color.GetB());
    }
    return Gdiplus::Color(color.GetA(), color.GetR(), color.GetG(), color.GetB());
}

void Render_GDI::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth)
{
    DrawLine(pt1, pt2, penColor, static_cast<float>(nWidth));
}

void Render_GDI::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Pen pen(UiColorToGdiplusColor(penColor), fWidth);
    
    Gdiplus::PointF p1(pt1.x + m_pPointOrg->X, pt1.y + m_pPointOrg->Y);
    Gdiplus::PointF p2(pt2.x + m_pPointOrg->X, pt2.y + m_pPointOrg->Y);
    
    m_pGraphics->DrawLine(&pen, p1, p2);
}

void Render_GDI::DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Pen pen(UiColorToGdiplusColor(penColor), fWidth);
    
    Gdiplus::PointF p1(pt1.x + m_pPointOrg->X, pt1.y + m_pPointOrg->Y);
    Gdiplus::PointF p2(pt2.x + m_pPointOrg->X, pt2.y + m_pPointOrg->Y);
    
    m_pGraphics->DrawLine(&pen, p1, p2);
}

void Render_GDI::SetPenByIPen(Gdiplus::Pen& pen, const IPen* pPen)
{
    if (pPen == nullptr) {
        return;
    }

    pen.SetColor(UiColorToGdiplusColor(pPen->GetColor()));
    pen.SetWidth(pPen->GetWidth());

    // 设置线帽样式
    auto ConvertLineCap = [](IPen::LineCap cap) -> Gdiplus::LineCap {
        switch (cap) {
        case IPen::kButt_Cap:
        return Gdiplus::LineCapFlat;
        case IPen::kRound_Cap:
        return Gdiplus::LineCapRound;
        case IPen::kSquare_Cap:
        return Gdiplus::LineCapSquare;
        default:
        return Gdiplus::LineCapFlat;
        }
    };

    // 转换 DashCap（注意：DashCap 是独立的枚举类型）
    auto ConvertDashCap = [](IPen::LineCap cap) -> Gdiplus::DashCap {
        switch (cap) {
        case IPen::kButt_Cap:
        return Gdiplus::DashCapFlat;
        case IPen::kRound_Cap:
        return Gdiplus::DashCapRound;
        case IPen::kSquare_Cap:
        return Gdiplus::DashCapTriangle;  // GDI+ 没有 DashCapSquare
        default:
        return Gdiplus::DashCapFlat;
        }
    };

    pen.SetStartCap(ConvertLineCap(pPen->GetStartCap()));
    pen.SetEndCap(ConvertLineCap(pPen->GetEndCap()));
    pen.SetDashCap(ConvertDashCap(pPen->GetDashCap()));  // 使用 DashCap 转换

    // 设置线条连接样式
    switch (pPen->GetLineJoin()) {
    case IPen::kMiter_Join:
    pen.SetLineJoin(Gdiplus::LineJoinMiter);
    break;
    case IPen::kBevel_Join:
    pen.SetLineJoin(Gdiplus::LineJoinBevel);
    break;
    case IPen::kRound_Join:
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    break;
    default:
    pen.SetLineJoin(Gdiplus::LineJoinMiter);
    break;
    }

    // 设置虚线样式
    switch (pPen->GetDashStyle()) {
    case IPen::kDashStyleSolid:
    pen.SetDashStyle(Gdiplus::DashStyleSolid);
    break;
    case IPen::kDashStyleDash:
    pen.SetDashStyle(Gdiplus::DashStyleDash);
    break;
    case IPen::kDashStyleDot:
    pen.SetDashStyle(Gdiplus::DashStyleDot);
    break;
    case IPen::kDashStyleDashDot:
    pen.SetDashStyle(Gdiplus::DashStyleDashDot);
    break;
    case IPen::kDashStyleDashDotDot:
    pen.SetDashStyle(Gdiplus::DashStyleDashDotDot);
    break;
    default:
    pen.SetDashStyle(Gdiplus::DashStyleSolid);
    break;
    }
}

void Render_GDI::DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Pen_GDI* pGdiPen = dynamic_cast<Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::PointF p1(pt1.x + m_pPointOrg->X, pt1.y + m_pPointOrg->Y);
    Gdiplus::PointF p2(pt2.x + m_pPointOrg->X, pt2.y + m_pPointOrg->Y);
    
    m_pGraphics->DrawLine(pGdiPen->GetPen(), p1, p2);
}

void Render_GDI::DrawLine(const UiPointF& pt1, const UiPointF& pt2, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Pen_GDI* pGdiPen = dynamic_cast<Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::PointF p1(pt1.x + m_pPointOrg->X, pt1.y + m_pPointOrg->Y);
    Gdiplus::PointF p2(pt2.x + m_pPointOrg->X, pt2.y + m_pPointOrg->Y);
    
    m_pGraphics->DrawLine(pGdiPen->GetPen(), p1, p2);
}

// 继续下一部分...

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
