#ifndef UI_RENDER_GDI_RENDER_HELPER_H_
#define UI_RENDER_GDI_RENDER_HELPER_H_

#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiString.h"

#if defined(DUILIB_BUILD_FOR_WIN)
#include "duilib/duilib_config_windows.h"
#endif
#include <duilib/Core/UiColor.h>

namespace ui
{

/** 将RGB颜色转换成HSL */
UILIB_API void RGBtoHSL(UiColor rgb, float* h, float* s, float* l);

/** 将HSL颜色转换成RGB */
UILIB_API UiColor HSLtoRGB(float h, float s, float l, uint8_t alpha = 255);

/** 计算图片目标区域（功能来源于旧版 UIRender.cpp） */
UILIB_API bool MakeImageDest(const UiRect& rcControl,
    const UiSize& szImage,
    const DString& sAlign,
    const UiPadding& rcPadding,
    UiRect& rcDest);

#if defined(DUILIB_BUILD_FOR_WIN)
/** 基于像素alpha的BitBlt合成（功能来源于旧版 UIRender.cpp） */
UILIB_API bool AlphaBitBlt(HDC hDC,
    int32_t nDestX,
    int32_t nDestY,
    int32_t dwWidth,
    int32_t dwHeight,
    HDC hSrcDC,
    int32_t nSrcX,
    int32_t nSrcY,
    int32_t wSrc,
    int32_t hSrc,
    BLENDFUNCTION ftn);
#endif

} // namespace ui

#endif // UI_RENDER_GDI_RENDER_HELPER_H_
