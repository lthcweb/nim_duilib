#ifndef UI_RENDER_GDI_MATRIX_GDI_H_
#define UI_RENDER_GDI_MATRIX_GDI_H_

#include "duilib/Render/IRender.h"

namespace ui {

class Matrix_GDI : public IMatrix {
public:
    void Translate(int offsetX, int offsetY) override;
    void Scale(float scaleX, float scaleY) override;
    void Rotate(float angle) override;
    void RotateAt(float angle, const UiPoint& center) override;
};

} // namespace ui

#endif
