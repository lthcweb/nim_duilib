#ifndef UI_RENDER_GDI_FONT_GDI_H_
#define UI_RENDER_GDI_FONT_GDI_H_

#include "duilib/Render/IRender.h"

namespace ui {

class Font_GDI : public IFont {
public:
    bool InitFont(const UiFont& fontInfo) override;
    DString FontName() const override;
    int FontSize() const override;
    bool IsBold() const override;
    bool IsUnderline() const override;
    bool IsItalic() const override;
    bool IsStrikeOut() const override;

private:
    UiFont m_fontInfo;
};

} // namespace ui

#endif
