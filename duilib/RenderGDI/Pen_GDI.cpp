#include "Pen_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

Pen_GDI::Pen_GDI(UiColor color, float fWidth)
    : m_pPen(nullptr)
    , m_fWidth(fWidth)
    , m_color(color)
    , m_startCap(kButt_Cap)
    , m_endCap(kButt_Cap)
    , m_dashCap(kButt_Cap)
    , m_lineJoin(kMiter_Join)
    , m_dashStyle(kDashStyleSolid)
{
    Gdiplus::Color gdipColor(color.GetA(), color.GetR(), color.GetG(), color.GetB());
    m_pPen = new Gdiplus::Pen(gdipColor, fWidth);
}

Pen_GDI::~Pen_GDI()
{
    if (m_pPen != nullptr) {
        delete m_pPen;
        m_pPen = nullptr;
    }
}

void Pen_GDI::SetWidth(float fWidth)
{
    m_fWidth = fWidth;
    if (m_pPen != nullptr) {
        m_pPen->SetWidth(fWidth);
    }
}

float Pen_GDI::GetWidth() const
{
    return m_fWidth;
}

void Pen_GDI::SetColor(UiColor color)
{
    m_color = color;
    if (m_pPen != nullptr) {
        Gdiplus::Color gdipColor(color.GetA(), color.GetR(), color.GetG(), color.GetB());
        m_pPen->SetColor(gdipColor);
    }
}

UiColor Pen_GDI::GetColor() const
{
    return m_color;
}

void Pen_GDI::SetStartCap(LineCap cap)
{
    m_startCap = cap;
    if (m_pPen != nullptr) {
        Gdiplus::LineCap gdipCap;
        switch (cap) {
        case kButt_Cap:
            gdipCap = Gdiplus::LineCapFlat;
            break;
        case kRound_Cap:
            gdipCap = Gdiplus::LineCapRound;
            break;
        case kSquare_Cap:
            gdipCap = Gdiplus::LineCapSquare;
            break;
        default:
            gdipCap = Gdiplus::LineCapFlat;
            break;
        }
        m_pPen->SetStartCap(gdipCap);
    }
}

IPen::LineCap Pen_GDI::GetStartCap() const
{
    return m_startCap;
}

void Pen_GDI::SetEndCap(LineCap cap)
{
    m_endCap = cap;
    if (m_pPen != nullptr) {
        Gdiplus::LineCap gdipCap;
        switch (cap) {
        case kButt_Cap:
            gdipCap = Gdiplus::LineCapFlat;
            break;
        case kRound_Cap:
            gdipCap = Gdiplus::LineCapRound;
            break;
        case kSquare_Cap:
            gdipCap = Gdiplus::LineCapSquare;
            break;
        default:
            gdipCap = Gdiplus::LineCapFlat;
            break;
        }
        m_pPen->SetEndCap(gdipCap);
    }
}

IPen::LineCap Pen_GDI::GetEndCap() const
{
    return m_endCap;
}

void Pen_GDI::SetDashCap(LineCap cap)
{
    m_dashCap = cap;
    if (m_pPen != nullptr) {
        Gdiplus::DashCap gdipCap;
        switch (cap) {
        case kButt_Cap:
            gdipCap = Gdiplus::DashCapFlat;
            break;
        case kRound_Cap:
            gdipCap = Gdiplus::DashCapRound;
            break;
        case kSquare_Cap:
            gdipCap = Gdiplus::DashCapTriangle;
            break;
        default:
            gdipCap = Gdiplus::DashCapFlat;
            break;
        }
        m_pPen->SetDashCap(gdipCap);
    }
}

IPen::LineCap Pen_GDI::GetDashCap() const
{
    return m_dashCap;
}

void Pen_GDI::SetLineJoin(LineJoin join)
{
    m_lineJoin = join;
    if (m_pPen != nullptr) {
        Gdiplus::LineJoin gdipJoin;
        switch (join) {
        case kMiter_Join:
            gdipJoin = Gdiplus::LineJoinMiter;
            break;
        case kBevel_Join:
            gdipJoin = Gdiplus::LineJoinBevel;
            break;
        case kRound_Join:
            gdipJoin = Gdiplus::LineJoinRound;
            break;
        default:
            gdipJoin = Gdiplus::LineJoinMiter;
            break;
        }
        m_pPen->SetLineJoin(gdipJoin);
    }
}

IPen::LineJoin Pen_GDI::GetLineJoin() const
{
    return m_lineJoin;
}

void Pen_GDI::SetDashStyle(DashStyle style)
{
    m_dashStyle = style;
    if (m_pPen != nullptr) {
        Gdiplus::DashStyle gdipStyle;
        switch (style) {
        case kDashStyleSolid:
            gdipStyle = Gdiplus::DashStyleSolid;
            break;
        case kDashStyleDash:
            gdipStyle = Gdiplus::DashStyleDash;
            break;
        case kDashStyleDot:
            gdipStyle = Gdiplus::DashStyleDot;
            break;
        case kDashStyleDashDot:
            gdipStyle = Gdiplus::DashStyleDashDot;
            break;
        case kDashStyleDashDotDot:
            gdipStyle = Gdiplus::DashStyleDashDotDot;
            break;
        default:
            gdipStyle = Gdiplus::DashStyleSolid;
            break;
        }
        m_pPen->SetDashStyle(gdipStyle);
    }
}

IPen::DashStyle Pen_GDI::GetDashStyle() const
{
    return m_dashStyle;
}

IPen* Pen_GDI::Clone() const
{
    Pen_GDI* pNewPen = new Pen_GDI(m_color, m_fWidth);
    pNewPen->SetStartCap(m_startCap);
    pNewPen->SetEndCap(m_endCap);
    pNewPen->SetDashCap(m_dashCap);
    pNewPen->SetLineJoin(m_lineJoin);
    pNewPen->SetDashStyle(m_dashStyle);
    return pNewPen;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
