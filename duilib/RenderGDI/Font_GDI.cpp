#include "duilib/RenderGDI/Font_GDI.h"

namespace ui {

bool Font_GDI::InitFont(const UiFont& fontInfo)
{
    m_fontInfo = fontInfo;
    return true;
}

DString Font_GDI::FontName() const { return m_fontInfo.m_fontName.c_str(); }
int Font_GDI::FontSize() const { return m_fontInfo.m_fontSize; }
bool Font_GDI::IsBold() const { return m_fontInfo.m_bBold; }
bool Font_GDI::IsUnderline() const { return m_fontInfo.m_bUnderline; }
bool Font_GDI::IsItalic() const { return m_fontInfo.m_bItalic; }
bool Font_GDI::IsStrikeOut() const { return m_fontInfo.m_bStrikeOut; }

} // namespace ui
