#ifndef UI_RENDER_PEN_GDI_H_
#define UI_RENDER_PEN_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

/** GDI+ 画笔实现类
*/
class UILIB_API Pen_GDI : public IPen
{
public:
    Pen_GDI(UiColor color, float fWidth = 1.0f);
    virtual ~Pen_GDI();

    Pen_GDI(const Pen_GDI&) = delete;
    Pen_GDI& operator=(const Pen_GDI&) = delete;

public:
    virtual void SetWidth(float fWidth) override;
    virtual float GetWidth() const override;
    virtual void SetColor(UiColor color) override;
    virtual UiColor GetColor() const override;

    virtual void SetStartCap(LineCap cap) override;
    virtual LineCap GetStartCap() const override;
    virtual void SetEndCap(LineCap cap) override;
    virtual LineCap GetEndCap() const override;
    virtual void SetDashCap(LineCap cap) override;
    virtual LineCap GetDashCap() const override;

    virtual void SetLineJoin(LineJoin join) override;
    virtual LineJoin GetLineJoin() const override;

    virtual void SetDashStyle(DashStyle style) override;
    virtual DashStyle GetDashStyle() const override;

    virtual IPen* Clone() const override;

public:
    /** 获取 GDI+ Pen 对象
    */
    Gdiplus::Pen* GetPen() const { return m_pPen; }

private:
    Gdiplus::Pen* m_pPen;
    float m_fWidth;
    UiColor m_color;
    LineCap m_startCap;
    LineCap m_endCap;
    LineCap m_dashCap;
    LineJoin m_lineJoin;
    DashStyle m_dashStyle;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_PEN_GDI_H_
