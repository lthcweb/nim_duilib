#ifndef UI_RENDER_GDI_BRUSH_GDI_H_
#define UI_RENDER_GDI_BRUSH_GDI_H_

#include "duilib/Render/IRender.h"

namespace ui {

class Brush_GDI : public IBrush {
public:
    explicit Brush_GDI(UiColor color);
    IBrush* Clone() override;
    UiColor GetColor() const override;

private:
    UiColor m_color;
};

} // namespace ui

#endif
