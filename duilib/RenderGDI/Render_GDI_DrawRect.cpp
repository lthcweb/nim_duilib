// Render_GDI.cpp 的补充部分 - 矩形和圆形绘制
#include "Render_GDI.h"
#include "Pen_GDI.h"
#include "Brush_GDI.h"

namespace ui {

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

} // namespace ui

