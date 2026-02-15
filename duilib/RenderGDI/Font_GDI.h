#ifndef UI_RENDER_FONT_GDI_H_
#define UI_RENDER_FONT_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

/** GDI+ 字体实现类
*/
class UILIB_API Font_GDI : public IFont
{
public:
    Font_GDI();
    virtual ~Font_GDI();

    Font_GDI(const Font_GDI&) = delete;
    Font_GDI& operator=(const Font_GDI&) = delete;

public:
    virtual bool InitFont(const UiFont& fontInfo) override;
    virtual DString FontName() const override;
    virtual int FontSize() const override;
    virtual bool IsBold() const override;
    virtual bool IsUnderline() const override;
    virtual bool IsItalic() const override;
    virtual bool IsStrikeOut() const override;

public:
    /** 获取 GDI+ Font 对象
    */
    Gdiplus::Font* GetFont() const { return m_pFont; }

    /** 获取 GDI+ FontFamily 对象
    */
    const Gdiplus::FontFamily* GetFontFamily() const { return m_pFontFamily; }

private:
    Gdiplus::Font* m_pFont;
    Gdiplus::FontFamily* m_pFontFamily;
    DString m_fontName;
    int m_fontSize;
    bool m_bBold;
    bool m_bUnderline;
    bool m_bItalic;
    bool m_bStrikeOut;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_FONT_GDI_H_
