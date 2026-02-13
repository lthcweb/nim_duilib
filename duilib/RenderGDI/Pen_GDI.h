#ifndef UI_RENDER_GDI_PEN_GDI_H_
#define UI_RENDER_GDI_PEN_GDI_H_

#include "duilib/Render/IRender.h"

namespace ui {

class Pen_GDI : public IPen {
public:
    Pen_GDI(UiColor color, float width);

    void SetWidth(float fWidth) override;
    float GetWidth() const override;
    void SetColor(UiColor color) override;
    UiColor GetColor() const override;
    void SetStartCap(LineCap cap) override;
    LineCap GetStartCap() const override;
    void SetEndCap(LineCap cap) override;
    LineCap GetEndCap() const override;
    void SetDashCap(LineCap cap) override;
    LineCap GetDashCap() const override;
    void SetLineJoin(LineJoin join) override;
    LineJoin GetLineJoin() const override;
    void SetDashStyle(DashStyle style) override;
    DashStyle GetDashStyle() const override;
    IPen* Clone() const override;

private:
    UiColor m_color;
    float m_width;
    LineCap m_startCap = kButt_Cap;
    LineCap m_endCap = kButt_Cap;
    LineCap m_dashCap = kButt_Cap;
    LineJoin m_lineJoin = kMiter_Join;
    DashStyle m_dashStyle = kDashStyleSolid;
};

} // namespace ui

#endif
