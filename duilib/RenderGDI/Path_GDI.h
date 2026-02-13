#ifndef UI_RENDER_GDI_PATH_GDI_H_
#define UI_RENDER_GDI_PATH_GDI_H_

#include "duilib/Render/IRender.h"

namespace ui {

class Path_GDI : public IPath {
public:
    void SetFillType(FillType mode) override;
    FillType GetFillType() override;
    void AddLine(int x1, int y1, int x2, int y2) override;
    void AddLines(const UiPoint* points, int count) override;
    void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
    void AddBeziers(const UiPoint* points, int count) override;
    void AddRect(const UiRect& rect) override;
    void AddEllipse(const UiRect& rect) override;
    void AddArc(const UiRect& rect, float startAngle, float sweepAngle) override;
    void AddPolygon(const UiPoint* points, int count) override;
    void AddPolygon(const UiPointF* points, int count) override;
    void Transform(IMatrix* pMatrix) override;
    UiRect GetBounds(const IPen* pen) override;
    void Close() override;
    void Reset() override;
    IPath* Clone() override;

private:
    FillType m_fillType = FillType::kWinding;
    UiRect m_bounds;
};

} // namespace ui

#endif
