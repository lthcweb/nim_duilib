#include "duilib/RenderGDI/FontMgr_GDI.h"

#include <algorithm>

namespace ui {

uint32_t FontMgr_GDI::GetFontCount() const
{
    return static_cast<uint32_t>(m_fontNames.size());
}

bool FontMgr_GDI::GetFontName(uint32_t nIndex, DString& fontName) const
{
    if (nIndex >= m_fontNames.size()) {
        return false;
    }
    fontName = m_fontNames[nIndex];
    return true;
}

bool FontMgr_GDI::HasFontName(const DString& fontName) const
{
    return std::find(m_fontNames.begin(), m_fontNames.end(), fontName) != m_fontNames.end();
}

void FontMgr_GDI::SetDefaultFontName(const DString& fontName)
{
    m_defaultFontName = fontName;
    if (!fontName.empty() && !HasFontName(fontName)) {
        m_fontNames.push_back(fontName);
    }
}

bool FontMgr_GDI::LoadFontFile(const DString& fontFilePath)
{
    if (fontFilePath.empty()) {
        return false;
    }
    if (!HasFontName(fontFilePath)) {
        m_fontNames.push_back(fontFilePath);
    }
    return true;
}

bool FontMgr_GDI::LoadFontFileData(const void* data, size_t length)
{
    UNUSED_VARIABLE(data);
    return length > 0;
}

void FontMgr_GDI::ClearFontFiles()
{
    m_fontNames.clear();
    if (!m_defaultFontName.empty()) {
        m_fontNames.push_back(m_defaultFontName);
    }
}

void FontMgr_GDI::ClearFontCache()
{
}

} // namespace ui
