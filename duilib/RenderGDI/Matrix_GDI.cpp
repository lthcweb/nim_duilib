#include "duilib/RenderGDI/Matrix_GDI.h"

namespace ui {

void Matrix_GDI::Translate(int offsetX, int offsetY)
{
    UNUSED_VARIABLE(offsetX);
    UNUSED_VARIABLE(offsetY);
}

void Matrix_GDI::Scale(float scaleX, float scaleY)
{
    UNUSED_VARIABLE(scaleX);
    UNUSED_VARIABLE(scaleY);
}

void Matrix_GDI::Rotate(float angle)
{
    UNUSED_VARIABLE(angle);
}

void Matrix_GDI::RotateAt(float angle, const UiPoint& center)
{
    UNUSED_VARIABLE(angle);
    UNUSED_VARIABLE(center);
}

} // namespace ui
