#include "Font_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/UiString.h"

#ifndef DUILIB_UNICODE
#include "duilib/Utils/StringUtil.h"
#endif

namespace ui {

Font_GDI::Font_GDI()
    : m_pFont(nullptr)
    , m_pFontFamily(nullptr)
    , m_fontSize(12)
    , m_bBold(false)
    , m_bUnderline(false)
    , m_bItalic(false)
    , m_bStrikeOut(false)
{
}

Font_GDI::~Font_GDI()
{
    if (m_pFont != nullptr) {
        delete m_pFont;
        m_pFont = nullptr;
    }
    if (m_pFontFamily != nullptr) {
        delete m_pFontFamily;
        m_pFontFamily = nullptr;
    }
}

bool Font_GDI::InitFont(const UiFont& fontInfo)
{
    // 清理旧字体
    if (m_pFont != nullptr) {
        delete m_pFont;
        m_pFont = nullptr;
    }
    if (m_pFontFamily != nullptr) {
        delete m_pFontFamily;
        m_pFontFamily = nullptr;
    }

    // 保存字体信息
    // UiFont 使用 UiString 类型，需要转换为 DString
    m_fontName = fontInfo.m_fontName.c_str();
    m_fontSize = fontInfo.m_fontSize;
    m_bBold = fontInfo.m_bBold;
    m_bUnderline = fontInfo.m_bUnderline;
    m_bItalic = fontInfo.m_bItalic;
    m_bStrikeOut = fontInfo.m_bStrikeOut;

    // 转换字体名称为宽字符
#ifdef DUILIB_UNICODE
    const wchar_t* szFontName = m_fontName.c_str();
#else
    std::wstring wsFontName = StringUtil::UTF8ToUTF16(m_fontName);
    const wchar_t* szFontName = wsFontName.c_str();
#endif

    // 创建字体族
    m_pFontFamily = new Gdiplus::FontFamily(szFontName);
    if (m_pFontFamily == nullptr || !m_pFontFamily->IsAvailable()) {
        // 如果指定字体不可用，使用默认字体
        if (m_pFontFamily != nullptr) {
            delete m_pFontFamily;
        }
        m_pFontFamily = new Gdiplus::FontFamily(L"Microsoft YaHei");
        if (!m_pFontFamily->IsAvailable()) {
            delete m_pFontFamily;
            m_pFontFamily = new Gdiplus::FontFamily(L"Arial");
        }
    }

    // 设置字体样式
    int style = Gdiplus::FontStyleRegular;
    if (m_bBold) {
        style |= Gdiplus::FontStyleBold;
    }
    if (m_bItalic) {
        style |= Gdiplus::FontStyleItalic;
    }
    if (m_bUnderline) {
        style |= Gdiplus::FontStyleUnderline;
    }
    if (m_bStrikeOut) {
        style |= Gdiplus::FontStyleStrikeout;
    }

    // 创建字体
    m_pFont = new Gdiplus::Font(m_pFontFamily, 
                                static_cast<Gdiplus::REAL>(m_fontSize),
                                style,
                                Gdiplus::UnitPixel);

    return (m_pFont != nullptr && m_pFont->GetLastStatus() == Gdiplus::Ok);
}

DString Font_GDI::FontName() const
{
    return m_fontName;
}

int Font_GDI::FontSize() const
{
    return m_fontSize;
}

bool Font_GDI::IsBold() const
{
    return m_bBold;
}

bool Font_GDI::IsUnderline() const
{
    return m_bUnderline;
}

bool Font_GDI::IsItalic() const
{
    return m_bItalic;
}

bool Font_GDI::IsStrikeOut() const
{
    return m_bStrikeOut;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
