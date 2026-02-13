#include "duilib/RenderGDI/Brush_GDI.h"

namespace ui {

Brush_GDI::Brush_GDI(UiColor color) : m_color(color) {}
IBrush* Brush_GDI::Clone() { return new Brush_GDI(m_color); }
UiColor Brush_GDI::GetColor() const { return m_color; }

} // namespace ui
