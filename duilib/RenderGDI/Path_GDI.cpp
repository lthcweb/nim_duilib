#include "duilib/RenderGDI/Path_GDI.h"

#include <algorithm>

namespace ui {

void Path_GDI::SetFillType(FillType mode) { m_fillType = mode; }
IPath::FillType Path_GDI::GetFillType() { return m_fillType; }
void Path_GDI::AddLine(int x1, int y1, int x2, int y2) { m_bounds.Union(UiRect(x1, y1, x2, y2)); }

void Path_GDI::AddLines(const UiPoint* points, int count)
{
    if ((points == nullptr) || (count <= 1)) {
        return;
    }
    for (int i = 1; i < count; ++i) {
        AddLine(points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
    }
}

void Path_GDI::AddBezier(int, int, int, int, int, int, int, int) {}
void Path_GDI::AddBeziers(const UiPoint*, int) {}
void Path_GDI::AddRect(const UiRect& rect) { m_bounds.Union(rect); }
void Path_GDI::AddEllipse(const UiRect& rect) { m_bounds.Union(rect); }
void Path_GDI::AddArc(const UiRect& rect, float, float) { m_bounds.Union(rect); }

void Path_GDI::AddPolygon(const UiPoint* points, int count)
{
    if ((points == nullptr) || (count <= 0)) {
        return;
    }
    UiRect rect(points[0].x, points[0].y, points[0].x, points[0].y);
    for (int i = 1; i < count; ++i) {
        rect.left = std::min(rect.left, points[i].x);
        rect.top = std::min(rect.top, points[i].y);
        rect.right = std::max(rect.right, points[i].x);
        rect.bottom = std::max(rect.bottom, points[i].y);
    }
    m_bounds.Union(rect);
}

void Path_GDI::AddPolygon(const UiPointF* points, int count)
{
    if ((points == nullptr) || (count <= 0)) {
        return;
    }
    UiRect rect(static_cast<int>(points[0].x), static_cast<int>(points[0].y), static_cast<int>(points[0].x), static_cast<int>(points[0].y));
    for (int i = 1; i < count; ++i) {
        rect.left = std::min(rect.left, static_cast<int>(points[i].x));
        rect.top = std::min(rect.top, static_cast<int>(points[i].y));
        rect.right = std::max(rect.right, static_cast<int>(points[i].x));
        rect.bottom = std::max(rect.bottom, static_cast<int>(points[i].y));
    }
    m_bounds.Union(rect);
}

void Path_GDI::Transform(IMatrix* pMatrix) { UNUSED_VARIABLE(pMatrix); }
UiRect Path_GDI::GetBounds(const IPen* pen) { UNUSED_VARIABLE(pen); return m_bounds; }
void Path_GDI::Close() {}
void Path_GDI::Reset() { m_bounds.Clear(); }

IPath* Path_GDI::Clone()
{
    auto* pPath = new Path_GDI();
    pPath->m_fillType = m_fillType;
    pPath->m_bounds = m_bounds;
    return pPath;
}

} // namespace ui
