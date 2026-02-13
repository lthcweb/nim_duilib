#ifndef UI_RENDER_GDI_DRAW_GDI_IMAGE_H_
#define UI_RENDER_GDI_DRAW_GDI_IMAGE_H_

#include "duilib/RenderGDI/Bitmap_GDI.h"

namespace ui {

class DrawGDIImage {
public:
    static bool DrawImageRect(Bitmap_GDI& destBitmap,
                              const UiRect& rcDest,
                              const Bitmap_GDI& srcBitmap,
                              const UiRect& rcSource,
                              uint8_t uFade);
};

} // namespace ui

#endif
