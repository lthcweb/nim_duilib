#ifndef UI_RENDER_BRUSH_GDI_H_
#define UI_RENDER_BRUSH_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

/** GDI+ 画刷实现类
*/
class UILIB_API Brush_GDI : public IBrush
{
public:
    Brush_GDI(UiColor color);
    virtual ~Brush_GDI();

    Brush_GDI(const Brush_GDI&) = delete;
    Brush_GDI& operator=(const Brush_GDI&) = delete;

public:
    virtual IBrush* Clone() override;
    virtual UiColor GetColor() const override;

public:
    /** 获取 GDI+ Brush 对象
    */
    Gdiplus::SolidBrush* GetBrush() const { return m_pBrush; }

private:
    Gdiplus::SolidBrush* m_pBrush;
    UiColor m_color;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_BRUSH_GDI_H_
