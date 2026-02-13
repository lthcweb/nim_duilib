#include "duilib/RenderGDI/Pen_GDI.h"

namespace ui {

Pen_GDI::Pen_GDI(UiColor color, float width) : m_color(color), m_width(width) {}
void Pen_GDI::SetWidth(float fWidth) { m_width = fWidth; }
float Pen_GDI::GetWidth() const { return m_width; }
void Pen_GDI::SetColor(UiColor color) { m_color = color; }
UiColor Pen_GDI::GetColor() const { return m_color; }
void Pen_GDI::SetStartCap(LineCap cap) { m_startCap = cap; }
IPen::LineCap Pen_GDI::GetStartCap() const { return m_startCap; }
void Pen_GDI::SetEndCap(LineCap cap) { m_endCap = cap; }
IPen::LineCap Pen_GDI::GetEndCap() const { return m_endCap; }
void Pen_GDI::SetDashCap(LineCap cap) { m_dashCap = cap; }
IPen::LineCap Pen_GDI::GetDashCap() const { return m_dashCap; }
void Pen_GDI::SetLineJoin(LineJoin join) { m_lineJoin = join; }
IPen::LineJoin Pen_GDI::GetLineJoin() const { return m_lineJoin; }
void Pen_GDI::SetDashStyle(DashStyle style) { m_dashStyle = style; }
IPen::DashStyle Pen_GDI::GetDashStyle() const { return m_dashStyle; }

IPen* Pen_GDI::Clone() const
{
    auto* pPen = new Pen_GDI(m_color, m_width);
    pPen->m_startCap = m_startCap;
    pPen->m_endCap = m_endCap;
    pPen->m_dashCap = m_dashCap;
    pPen->m_lineJoin = m_lineJoin;
    pPen->m_dashStyle = m_dashStyle;
    return pPen;
}

} // namespace ui
