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

namespace ui
{

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
    return UiPoint{ static_cast<int32_t>(m_pPointOrg->X),
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
void Render_GDI::DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter,
    const IPen* pen,
    UiColor* gradientColor,
    const UiRect* gradientRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Pen_GDI* pGdiPen = dynamic_cast<const Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        static_cast<Gdiplus::REAL>(rc.Width()),
        static_cast<Gdiplus::REAL>(rc.Height()));

    if (useCenter) {
        // 使用扇形（包含中心点）
        Gdiplus::GraphicsPath path;
        Gdiplus::PointF center(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
        path.AddArc(rect, startAngle, sweepAngle);

        // 获取路径的最后一个点
        Gdiplus::PointF lastPoint;
        path.GetLastPoint(&lastPoint);  // 注意：需要传入指针
        path.AddLine(lastPoint, center);
        path.CloseFigure();

        if (gradientColor != nullptr && gradientRect != nullptr) {
            // 使用渐变画刷
            Gdiplus::RectF gradRect(gradientRect->left + m_pPointOrg->X,
                gradientRect->top + m_pPointOrg->Y,
                static_cast<Gdiplus::REAL>(gradientRect->Width()),
                static_cast<Gdiplus::REAL>(gradientRect->Height()));

            Gdiplus::Color color1 = UiColorToGdiplusColor(pen->GetColor());
            Gdiplus::Color color2 = UiColorToGdiplusColor(*gradientColor);

            Gdiplus::PointF pt1(gradRect.X, gradRect.Y);
            Gdiplus::PointF pt2(gradRect.GetRight(), gradRect.GetBottom());

            Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
            Gdiplus::Pen gradPen(&brush, pen->GetWidth());
            m_pGraphics->DrawPath(&gradPen, &path);
        }
        else {
            m_pGraphics->DrawPath(pGdiPen->GetPen(), &path);
        }
    }
    else {
        // 只绘制弧线（这部分不需要修改）
        if (gradientColor != nullptr && gradientRect != nullptr) {
            Gdiplus::RectF gradRect(gradientRect->left + m_pPointOrg->X,
                gradientRect->top + m_pPointOrg->Y,
                static_cast<Gdiplus::REAL>(gradientRect->Width()),
                static_cast<Gdiplus::REAL>(gradientRect->Height()));

            Gdiplus::Color color1 = UiColorToGdiplusColor(pen->GetColor());
            Gdiplus::Color color2 = UiColorToGdiplusColor(*gradientColor);

            Gdiplus::PointF pt1(gradRect.X, gradRect.Y);
            Gdiplus::PointF pt2(gradRect.GetRight(), gradRect.GetBottom());

            Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
            Gdiplus::Pen gradPen(&brush, pen->GetWidth());
            m_pGraphics->DrawArc(&gradPen, rect, startAngle, sweepAngle);
        }
        else {
            m_pGraphics->DrawArc(pGdiPen->GetPen(), rect, startAngle, sweepAngle);
        }
    }
}

void Render_GDI::DrawPath(const IPath* path, const IPen* pen)
{
    ASSERT(path != nullptr);
    ASSERT(pen != nullptr);
    if (path == nullptr || pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    const Pen_GDI* pGdiPen = dynamic_cast<const Pen_GDI*>(pen);

    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr ||
        pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        m_pGraphics->DrawPath(pGdiPen->GetPen(), pPath);

        delete pPath;
    }
}

void Render_GDI::FillPath(const IPath* path, const IBrush* brush)
{
    ASSERT(path != nullptr);
    ASSERT(brush != nullptr);
    if (path == nullptr || brush == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    const Brush_GDI* pGdiBrush = dynamic_cast<const Brush_GDI*>(brush);

    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr ||
        pGdiBrush == nullptr || pGdiBrush->GetBrush() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        m_pGraphics->FillPath(pGdiBrush->GetBrush(), pPath);

        delete pPath;
    }
}

void Render_GDI::FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction)
{
    ASSERT(path != nullptr);
    if (path == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        // 创建渐变画刷
        Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
            rc.top + m_pPointOrg->Y,
            static_cast<Gdiplus::REAL>(rc.Width()),
            static_cast<Gdiplus::REAL>(rc.Height()));

// 标准化方向
        if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
            nColor2Direction = 1;
        }

        Gdiplus::PointF pt1(rect.X, rect.Y);
        Gdiplus::PointF pt2;

        if (nColor2Direction == 2) {
            pt2.X = rect.X;
            pt2.Y = rect.GetBottom();
        }
        else if (nColor2Direction == 3) {
            pt2.X = rect.GetRight();
            pt2.Y = rect.GetBottom();
        }
        else if (nColor2Direction == 4) {
            pt1.X = rect.GetRight();
            pt2.X = rect.X;
            pt2.Y = rect.GetBottom();
        }
        else {
            pt2.X = rect.GetRight();
            pt2.Y = rect.Y;
        }

        Gdiplus::Color color1 = UiColorToGdiplusColor(dwColor);
        Gdiplus::Color color2 = UiColorToGdiplusColor(dwColor2);

        Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
        m_pGraphics->FillPath(&brush, pPath);

        delete pPath;
    }
}



void Render_GDI::DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect)
{
    DrawRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        penColor, static_cast<float>(nWidth), bLineInRect);
}

void Render_GDI::DrawRect(const UiRectF& rc, UiColor penColor, int32_t nWidth, bool bLineInRect)
{
    DrawRect(rc, penColor, static_cast<float>(nWidth), bLineInRect);
}

void Render_GDI::DrawRect(const UiRect& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    DrawRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        penColor, fWidth, bLineInRect);
}

void Render_GDI::DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Pen pen(UiColorToGdiplusColor(penColor), fWidth);

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    if (bLineInRect) {
        // 确保线条在矩形内部
        float fHalfWidth = fWidth / 2.0f;
        rect.X += fHalfWidth;
        rect.Y += fHalfWidth;
        rect.Width -= fWidth;
        rect.Height -= fWidth;
    }

    m_pGraphics->DrawRectangle(&pen, rect);
}

void Render_GDI::DrawRect(const UiRect& rc, IPen* pen, bool bLineInRect)
{
    DrawRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        pen, bLineInRect);
}

void Render_GDI::DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Pen_GDI* pGdiPen = dynamic_cast<Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    if (bLineInRect) {
        float fHalfWidth = pen->GetWidth() / 2.0f;
        rect.X += fHalfWidth;
        rect.Y += fHalfWidth;
        rect.Width -= pen->GetWidth();
        rect.Height -= pen->GetWidth();
    }

    m_pGraphics->DrawRectangle(pGdiPen->GetPen(), rect);
}

void Render_GDI::FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade)
{
    FillRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        dwColor, uFade);
}

void Render_GDI::FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::SolidBrush brush(UiColorToGdiplusColor(dwColor, uFade));

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    m_pGraphics->FillRectangle(&brush, rect);
}

void Render_GDI::InitGradientBrush(Gdiplus::LinearGradientBrush& brush, const Gdiplus::RectF& rc,
    UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const
{
    // 标准化方向
    if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
        nColor2Direction = 1;
    }

    Gdiplus::PointF pt1(rc.X, rc.Y);
    Gdiplus::PointF pt2;

    if (nColor2Direction == 2) {
        // 上->下
        pt2.X = rc.X;
        pt2.Y = rc.GetBottom();
    }
    else if (nColor2Direction == 3) {
        // 左上->右下
        pt2.X = rc.GetRight();
        pt2.Y = rc.GetBottom();
    }
    else if (nColor2Direction == 4) {
        // 右上->左下
        pt1.X = rc.GetRight();
        pt1.Y = rc.Y;
        pt2.X = rc.X;
        pt2.Y = rc.GetBottom();
    }
    else {
        // 左->右
        pt2.X = rc.GetRight();
        pt2.Y = rc.Y;
    }

    Gdiplus::Color color1 = UiColorToGdiplusColor(dwColor);
    Gdiplus::Color color2 = UiColorToGdiplusColor(dwColor2);

    // 创建渐变画刷（注意：这里需要重新构造）
    // brush 需要在外部用这些参数重新创建
}

void Render_GDI::FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    FillRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        dwColor, dwColor2, nColor2Direction, uFade);
}

void Render_GDI::FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    if (dwColor2.IsEmpty()) {
        return FillRect(rc, dwColor, uFade);
    }

    // 标准化方向
    if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
        nColor2Direction = 1;
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    Gdiplus::PointF pt1(rect.X, rect.Y);
    Gdiplus::PointF pt2;

    if (nColor2Direction == 2) {
        pt2.X = rect.X;
        pt2.Y = rect.GetBottom();
    }
    else if (nColor2Direction == 3) {
        pt2.X = rect.GetRight();
        pt2.Y = rect.GetBottom();
    }
    else if (nColor2Direction == 4) {
        pt1.X = rect.GetRight();
        pt2.X = rect.X;
        pt2.Y = rect.GetBottom();
    }
    else {
        pt2.X = rect.GetRight();
        pt2.Y = rect.Y;
    }

    Gdiplus::Color color1 = UiColorToGdiplusColor(dwColor, uFade);
    Gdiplus::Color color2 = UiColorToGdiplusColor(dwColor2, uFade);

    Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
    m_pGraphics->FillRectangle(&brush, rect);
}

// 圆角矩形绘制
void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, int32_t nWidth)
{
    DrawRoundRect(rc, rx, ry, penColor, static_cast<float>(nWidth));
}

void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, int32_t nWidth)
{
    DrawRoundRect(rc, rx, ry, penColor, static_cast<float>(nWidth));
}

void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, float fWidth)
{
    DrawRoundRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        rx, ry, penColor, fWidth);
}

void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Pen pen(UiColorToGdiplusColor(penColor), fWidth);

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

// 创建圆角矩形路径
    Gdiplus::GraphicsPath path;
    float diameter = rx * 2;
    Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);

    path.AddArc(arc, 180, 90);
    arc.X = rect.GetRight() - diameter;
    path.AddArc(arc, 270, 90);
    arc.Y = rect.GetBottom() - diameter;
    path.AddArc(arc, 0, 90);
    arc.X = rect.GetLeft();
    path.AddArc(arc, 90, 90);
    path.CloseFigure();

    m_pGraphics->DrawPath(&pen, &path);
}

void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, IPen* pen)
{
    DrawRoundRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        rx, ry, pen);
}

void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Pen_GDI* pGdiPen = dynamic_cast<Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    Gdiplus::GraphicsPath path;
    float diameter = rx * 2;
    Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);

    path.AddArc(arc, 180, 90);
    arc.X = rect.GetRight() - diameter;
    path.AddArc(arc, 270, 90);
    arc.Y = rect.GetBottom() - diameter;
    path.AddArc(arc, 0, 90);
    arc.X = rect.GetLeft();
    path.AddArc(arc, 90, 90);
    path.CloseFigure();

    m_pGraphics->DrawPath(pGdiPen->GetPen(), &path);
}

void Render_GDI::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, uint8_t uFade)
{
    FillRoundRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        rx, ry, dwColor, uFade);
}

void Render_GDI::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::SolidBrush brush(UiColorToGdiplusColor(dwColor, uFade));

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

    Gdiplus::GraphicsPath path;
    float diameter = rx * 2;
    Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);

    path.AddArc(arc, 180, 90);
    arc.X = rect.GetRight() - diameter;
    path.AddArc(arc, 270, 90);
    arc.Y = rect.GetBottom() - diameter;
    path.AddArc(arc, 0, 90);
    arc.X = rect.GetLeft();
    path.AddArc(arc, 90, 90);
    path.CloseFigure();

    m_pGraphics->FillPath(&brush, &path);
}

void Render_GDI::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    FillRoundRect(UiRectF(static_cast<float>(rc.left), static_cast<float>(rc.top),
        static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
        rx, ry, dwColor, dwColor2, nColor2Direction, uFade);
}

void Render_GDI::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    if (dwColor2.IsEmpty()) {
        return FillRoundRect(rc, rx, ry, dwColor, uFade);
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        rc.right - rc.left,
        rc.bottom - rc.top);

// 创建渐变画刷
    if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
        nColor2Direction = 1;
    }

    Gdiplus::PointF pt1(rect.X, rect.Y);
    Gdiplus::PointF pt2;

    if (nColor2Direction == 2) {
        pt2.X = rect.X;
        pt2.Y = rect.GetBottom();
    }
    else if (nColor2Direction == 3) {
        pt2.X = rect.GetRight();
        pt2.Y = rect.GetBottom();
    }
    else if (nColor2Direction == 4) {
        pt1.X = rect.GetRight();
        pt2.X = rect.X;
        pt2.Y = rect.GetBottom();
    }
    else {
        pt2.X = rect.GetRight();
        pt2.Y = rect.Y;
    }

    Gdiplus::Color color1 = UiColorToGdiplusColor(dwColor, uFade);
    Gdiplus::Color color2 = UiColorToGdiplusColor(dwColor2, uFade);

    Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);

    // 创建圆角矩形路径
    Gdiplus::GraphicsPath path;
    float diameter = rx * 2;
    Gdiplus::RectF arc(rect.X, rect.Y, diameter, diameter);

    path.AddArc(arc, 180, 90);
    arc.X = rect.GetRight() - diameter;
    path.AddArc(arc, 270, 90);
    arc.Y = rect.GetBottom() - diameter;
    path.AddArc(arc, 0, 90);
    arc.X = rect.GetLeft();
    path.AddArc(arc, 90, 90);
    path.CloseFigure();

    m_pGraphics->FillPath(&brush, &path);
}

// 圆形绘制
void Render_GDI::DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int32_t nWidth)
{
    DrawCircle(centerPt, radius, penColor, static_cast<float>(nWidth));
}

void Render_GDI::DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::Pen pen(UiColorToGdiplusColor(penColor), fWidth);

    Gdiplus::RectF rect(centerPt.x - radius + m_pPointOrg->X,
        centerPt.y - radius + m_pPointOrg->Y,
        radius * 2.0f,
        radius * 2.0f);

    m_pGraphics->DrawEllipse(&pen, rect);
}

void Render_GDI::DrawCircle(const UiPoint& centerPt, int32_t radius, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Pen_GDI* pGdiPen = dynamic_cast<Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::RectF rect(centerPt.x - radius + m_pPointOrg->X,
        centerPt.y - radius + m_pPointOrg->Y,
        radius * 2.0f,
        radius * 2.0f);

    m_pGraphics->DrawEllipse(pGdiPen->GetPen(), rect);
}

void Render_GDI::FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (m_pGraphics == nullptr) {
        return;
    }

    Gdiplus::SolidBrush brush(UiColorToGdiplusColor(dwColor, uFade));

    Gdiplus::RectF rect(centerPt.x - radius + m_pPointOrg->X,
        centerPt.y - radius + m_pPointOrg->Y,
        radius * 2.0f,
        radius * 2.0f);

    m_pGraphics->FillEllipse(&brush, rect);
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

IBitmap* Render_GDI::MakeImageSnapshot()
{
    return nullptr;
}

void Render_GDI::ClearAlpha(const UiRect& rcDirty, uint8_t alpha)
{
}

void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha)
{
}

void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding)
{
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

UiRect Render_GDI::MeasureString(const DString& strText, const MeasureStringParam& measureParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        return UiRect();
    }

    PerformanceStat statPerformance(_T("Render_GDI::MeasureString"));

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return UiRect();
    }

    ASSERT(measureParam.pFont != nullptr);
    if (measureParam.pFont == nullptr || m_pGraphics == nullptr) {
        return UiRect();
    }

    Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(measureParam.pFont);
    ASSERT(pGdiFont != nullptr);
    if (pGdiFont == nullptr || pGdiFont->GetFont() == nullptr) {
        return UiRect();
    }

#ifdef DUILIB_UNICODE
    const wchar_t* szText = strText.c_str();
    int textLen = static_cast<int>(strText.length());
#else
    std::wstring wsText = StringUtil::UTF8ToUTF16(strText);
    const wchar_t* szText = wsText.c_str();
    int textLen = static_cast<int>(wsText.length());
#endif

    // 设置字符串格式
    Gdiplus::StringFormat stringFormat;

    // 单行/多行模式
    if (measureParam.uFormat & TEXT_SINGLELINE) {
        stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    }

    // 水平对齐
    if (measureParam.uFormat & TEXT_HCENTER) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (measureParam.uFormat & TEXT_RIGHT) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    }

    // 垂直对齐
    if (measureParam.uFormat & TEXT_VCENTER) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (measureParam.uFormat & TEXT_BOTTOM) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    }

    // 测量文本
    Gdiplus::RectF layoutRect;
    if (measureParam.rectSize != DUI_NOSET_VALUE) {
        if (measureParam.uFormat & TEXT_VERTICAL) {
            layoutRect.Width = 10000.0f;
            layoutRect.Height = static_cast<Gdiplus::REAL>(measureParam.rectSize);
        }
        else {
            layoutRect.Width = static_cast<Gdiplus::REAL>(measureParam.rectSize);
            layoutRect.Height = 10000.0f;
        }
    }
    else {
        layoutRect.Width = 10000.0f;
        layoutRect.Height = 10000.0f;
    }

    Gdiplus::RectF boundingBox;
    m_pGraphics->MeasureString(szText, textLen, pGdiFont->GetFont(),
        layoutRect, &stringFormat, &boundingBox);

    UiRect rc;
    rc.left = 0;
    rc.top = 0;
    rc.right = static_cast<int>(boundingBox.Width + 0.5f);
    rc.bottom = static_cast<int>(boundingBox.Height + 0.5f);

    return rc;
}

void Render_GDI::DrawString(const DString& strText, const DrawStringParam& drawParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        return;
    }

    PerformanceStat statPerformance(_T("Render_GDI::DrawString"));

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }

    ASSERT(!drawParam.textRect.IsEmpty());
    if (drawParam.textRect.IsEmpty()) {
        return;
    }

    ASSERT(drawParam.pFont != nullptr);
    if (drawParam.pFont == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(drawParam.pFont);
    ASSERT(pGdiFont != nullptr);
    if (pGdiFont == nullptr || pGdiFont->GetFont() == nullptr) {
        return;
    }

#ifdef DUILIB_UNICODE
    const wchar_t* szText = strText.c_str();
    int textLen = static_cast<int>(strText.length());
#else
    std::wstring wsText = StringUtil::UTF8ToUTF16(strText);
    const wchar_t* szText = wsText.c_str();
    int textLen = static_cast<int>(wsText.length());
#endif

    // 设置文本颜色
    Gdiplus::Color textColor = UiColorToGdiplusColor(drawParam.dwTextColor, drawParam.uFade);
    Gdiplus::SolidBrush brush(textColor);

    // 设置绘制区域
    Gdiplus::RectF rect(drawParam.textRect.left + m_pPointOrg->X,
        drawParam.textRect.top + m_pPointOrg->Y,
        static_cast<Gdiplus::REAL>(drawParam.textRect.Width()),
        static_cast<Gdiplus::REAL>(drawParam.textRect.Height()));

// 设置字符串格式
    Gdiplus::StringFormat stringFormat;

    // 单行/多行模式
    if (drawParam.uFormat & TEXT_SINGLELINE) {
        stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    }

    // 自动换行
    if (drawParam.uFormat & TEXT_WORD_WRAP) {
        // GDI+ 默认支持自动换行
    }

    // 水平对齐
    if (drawParam.uFormat & TEXT_HCENTER) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (drawParam.uFormat & TEXT_RIGHT) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    }

    // 垂直对齐
    if (drawParam.uFormat & TEXT_VCENTER) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (drawParam.uFormat & TEXT_BOTTOM) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    }

    // 省略号
    if (drawParam.uFormat & TEXT_END_ELLIPSIS) {
        stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    }
    else if (drawParam.uFormat & TEXT_PATH_ELLIPSIS) {
        stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
    }

    // 不裁剪
    if (drawParam.uFormat & TEXT_NOCLIP) {
        stringFormat.SetFormatFlags(stringFormat.GetFormatFlags() | Gdiplus::StringFormatFlagsNoClip);
    }

    // 绘制文本
    m_pGraphics->DrawString(szText, textLen, pGdiFont->GetFont(),
        rect, &stringFormat, &brush);
}

// 富文本绘制相关函数 - 这些需要更复杂的实现
void Render_GDI::MeasureRichText(const UiRect& textRect,
    const UiSize& szScrollOffset,
    IRenderFactory* pRenderFactory,
    const std::vector<RichTextData>& richTextData,
    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText"));
    // TODO: 实现富文本测量
    // 这需要参考 Skia 的 DrawRichText 实现
}

void Render_GDI::MeasureRichText2(const UiRect& textRect,
    const UiSize& szScrollOffset,
    IRenderFactory* pRenderFactory,
    const std::vector<RichTextData>& richTextData,
    RichTextLineInfoParam* pLineInfoParam,
    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText2"));
    // TODO: 实现富文本测量2
}

void Render_GDI::MeasureRichText3(const UiRect& textRect,
    const UiSize& szScrollOffset,
    IRenderFactory* pRenderFactory,
    const std::vector<RichTextData>& richTextData,
    RichTextLineInfoParam* pLineInfoParam,
    std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText3"));
    // TODO: 实现富文本测量3
}

void Render_GDI::DrawRichText(const UiRect& textRect,
    const UiSize& szScrollOffset,
    IRenderFactory* pRenderFactory,
    const std::vector<RichTextData>& richTextData,
    uint8_t uFade,
    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::DrawRichText"));
    // TODO: 实现富文本绘制
}

bool Render_GDI::CreateDrawRichTextCache(const UiRect& textRect,
    const UiSize& szScrollOffset,
    IRenderFactory* pRenderFactory,
    const std::vector<RichTextData>& richTextData,
    std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    PerformanceStat statPerformance(_T("Render_GDI::CreateDrawRichTextCache"));
    // TODO: 实现富文本缓存创建
    return false;
}

bool Render_GDI::IsValidDrawRichTextCache(const UiRect& textRect,
    const std::vector<RichTextData>& richTextData,
    const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    // TODO: 实现富文本缓存验证
    return false;
}

bool Render_GDI::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
    const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
    std::vector<RichTextData>& richTextDataNew,
    size_t nStartLine,
    const std::vector<size_t>& modifiedLines,
    size_t nModifiedRows,
    const std::vector<size_t>& deletedLines,
    size_t nDeletedRows,
    const std::vector<int32_t>& rowRectTopList)
{
    PerformanceStat statPerformance(_T("Render_GDI::UpdateDrawRichTextCache"));
    // TODO: 实现富文本缓存更新
    return false;
}

bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    // TODO: 实现富文本缓存比较
    return false;
}

void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
    const UiRect& textRect,
    const UiSize& szNewScrollOffset,
    const std::vector<int32_t>& rowXOffset,
    uint8_t uFade,
    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    // TODO: 实现富文本缓存绘制
}

size_t Render_GDI::GetUTF16CharCount(const DStringW::value_type* srcPtr, size_t textStartIndex) const
{
    if (srcPtr != nullptr) {
        ASSERT(sizeof(uint16_t) == sizeof(DStringW::value_type));
        const uint16_t* src = (const uint16_t*)(srcPtr + textStartIndex);
        // 检查是否为高代理字符（UTF-16 代理对）
        if ((*src >= 0xD800) && (*src <= 0xDBFF)) {
            // 高代理字符，需要两个字符
            return 2;
        }
    }
    return 1;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
