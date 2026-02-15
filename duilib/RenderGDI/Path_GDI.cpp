#include "Path_GDI.h"
#include "Matrix_GDI.h"
#include "Pen_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

Path_GDI::Path_GDI()
    : m_pPath(nullptr)
    , m_fillType(FillType::kEvenOdd)
{
    m_pPath = new Gdiplus::GraphicsPath();
}

Path_GDI::~Path_GDI()
{
    if (m_pPath != nullptr) {
        delete m_pPath;
        m_pPath = nullptr;
    }
}

void Path_GDI::SetFillType(FillType mode)
{
    m_fillType = mode;
    if (m_pPath != nullptr) {
        Gdiplus::FillMode gdipMode;
        switch (mode) {
        case FillType::kEvenOdd:
        case FillType::kInverseEvenOdd:
            gdipMode = Gdiplus::FillModeAlternate;
            break;
        case FillType::kWinding:
        case FillType::kInverseWinding:
            gdipMode = Gdiplus::FillModeWinding;
            break;
        default:
            gdipMode = Gdiplus::FillModeAlternate;
            break;
        }
        m_pPath->SetFillMode(gdipMode);
    }
}

IPath::FillType Path_GDI::GetFillType()
{
    return m_fillType;
}

void Path_GDI::AddLine(int x1, int y1, int x2, int y2)
{
    if (m_pPath != nullptr) {
        m_pPath->AddLine(x1, y1, x2, y2);
    }
}

void Path_GDI::AddLines(const UiPoint* points, int count)
{
    if (m_pPath == nullptr || points == nullptr || count < 2) {
        return;
    }

    std::vector<Gdiplus::Point> gdipPoints;
    gdipPoints.reserve(count);
    for (int i = 0; i < count; ++i) {
        gdipPoints.push_back(Gdiplus::Point(points[i].x, points[i].y));
    }

    m_pPath->AddLines(gdipPoints.data(), count);
}

void Path_GDI::AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    if (m_pPath != nullptr) {
        m_pPath->AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
    }
}

void Path_GDI::AddBeziers(const UiPoint* points, int count)
{
    if (m_pPath == nullptr || points == nullptr || count < 4) {
        return;
    }

    std::vector<Gdiplus::Point> gdipPoints;
    gdipPoints.reserve(count);
    for (int i = 0; i < count; ++i) {
        gdipPoints.push_back(Gdiplus::Point(points[i].x, points[i].y));
    }

    m_pPath->AddBeziers(gdipPoints.data(), count);
}

void Path_GDI::AddRect(const UiRect& rect)
{
    if (m_pPath != nullptr) {
        Gdiplus::Rect gdipRect(rect.left, rect.top, rect.Width(), rect.Height());
        m_pPath->AddRectangle(gdipRect);
    }
}

void Path_GDI::AddEllipse(const UiRect& rect)
{
    if (m_pPath != nullptr) {
        Gdiplus::Rect gdipRect(rect.left, rect.top, rect.Width(), rect.Height());
        m_pPath->AddEllipse(gdipRect);
    }
}

void Path_GDI::AddArc(const UiRect& rect, float startAngle, float sweepAngle)
{
    if (m_pPath != nullptr) {
        Gdiplus::Rect gdipRect(rect.left, rect.top, rect.Width(), rect.Height());
        m_pPath->AddArc(gdipRect, startAngle, sweepAngle);
    }
}

void Path_GDI::AddPolygon(const UiPoint* points, int count)
{
    if (m_pPath == nullptr || points == nullptr || count < 3) {
        return;
    }

    std::vector<Gdiplus::Point> gdipPoints;
    gdipPoints.reserve(count);
    for (int i = 0; i < count; ++i) {
        gdipPoints.push_back(Gdiplus::Point(points[i].x, points[i].y));
    }

    m_pPath->AddPolygon(gdipPoints.data(), count);
}

void Path_GDI::AddPolygon(const UiPointF* points, int count)
{
    if (m_pPath == nullptr || points == nullptr || count < 3) {
        return;
    }

    std::vector<Gdiplus::PointF> gdipPoints;
    gdipPoints.reserve(count);
    for (int i = 0; i < count; ++i) {
        gdipPoints.push_back(Gdiplus::PointF(points[i].x, points[i].y));
    }

    m_pPath->AddPolygon(gdipPoints.data(), count);
}

void Path_GDI::Transform(IMatrix* pMatrix)
{
    if (m_pPath == nullptr || pMatrix == nullptr) {
        return;
    }

    Matrix_GDI* pGdiMatrix = dynamic_cast<Matrix_GDI*>(pMatrix);
    if (pGdiMatrix != nullptr && pGdiMatrix->GetMatrix() != nullptr) {
        m_pPath->Transform(pGdiMatrix->GetMatrix());
    }
}

UiRect Path_GDI::GetBounds(const IPen* pen)
{
    if (m_pPath == nullptr) {
        return UiRect();
    }

    Gdiplus::RectF bounds;
    if (pen != nullptr) {
        const Pen_GDI* pGdiPen = dynamic_cast<const Pen_GDI*>(pen);
        if (pGdiPen != nullptr && pGdiPen->GetPen() != nullptr) {
            m_pPath->GetBounds(&bounds, nullptr, pGdiPen->GetPen());
        }
        else {
            m_pPath->GetBounds(&bounds);
        }
    }
    else {
        m_pPath->GetBounds(&bounds);
    }

    return UiRect(
        static_cast<int>(bounds.X),
        static_cast<int>(bounds.Y),
        static_cast<int>(bounds.GetRight()),
        static_cast<int>(bounds.GetBottom())
    );
}

void Path_GDI::Close()
{
    if (m_pPath != nullptr) {
        m_pPath->CloseFigure();
    }
}

void Path_GDI::Reset()
{
    if (m_pPath != nullptr) {
        m_pPath->Reset();
    }
}

IPath* Path_GDI::Clone()
{
    Path_GDI* pNewPath = new Path_GDI();
    if (pNewPath != nullptr && m_pPath != nullptr) {
        delete pNewPath->m_pPath;
        pNewPath->m_pPath = m_pPath->Clone();
        pNewPath->m_fillType = m_fillType;
    }
    return pNewPath;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
