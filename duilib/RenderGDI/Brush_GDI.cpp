#include "Brush_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

Brush_GDI::Brush_GDI(UiColor color)
    : m_pBrush(nullptr)
    , m_color(color)
{
    Gdiplus::Color gdipColor(color.GetA(), color.GetR(), color.GetG(), color.GetB());
    m_pBrush = new Gdiplus::SolidBrush(gdipColor);
}

Brush_GDI::~Brush_GDI()
{
    if (m_pBrush != nullptr) {
        delete m_pBrush;
        m_pBrush = nullptr;
    }
}

IBrush* Brush_GDI::Clone()
{
    return new Brush_GDI(m_color);
}

UiColor Brush_GDI::GetColor() const
{
    return m_color;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
