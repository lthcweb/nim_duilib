// Render_GDI.cpp 的补充部分 - 路径和弧形绘制


#include "Render_GDI.h"
#include "Path_GDI.h"
#include "Pen_GDI.h"
#include "Brush_GDI.h"

namespace ui {

void Render_GDI::DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter,
    const IPen* pen,
    UiColor* gradientColor,
    const UiRect* gradientRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Pen_GDI* pGdiPen = dynamic_cast<const Pen_GDI*>(pen);
    if (pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
        rc.top + m_pPointOrg->Y,
        static_cast<Gdiplus::REAL>(rc.Width()),
        static_cast<Gdiplus::REAL>(rc.Height()));

    if (useCenter) {
        // 使用扇形（包含中心点）
        Gdiplus::GraphicsPath path;
        Gdiplus::PointF center(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
        path.AddArc(rect, startAngle, sweepAngle);

        // 获取路径的最后一个点
        Gdiplus::PointF lastPoint;
        path.GetLastPoint(&lastPoint);  // 注意：需要传入指针
        path.AddLine(lastPoint, center);
        path.CloseFigure();

        if (gradientColor != nullptr && gradientRect != nullptr) {
            // 使用渐变画刷
            Gdiplus::RectF gradRect(gradientRect->left + m_pPointOrg->X,
                gradientRect->top + m_pPointOrg->Y,
                static_cast<Gdiplus::REAL>(gradientRect->Width()),
                static_cast<Gdiplus::REAL>(gradientRect->Height()));

            Gdiplus::Color color1 = UiColorToGdiplusColor(pen->GetColor());
            Gdiplus::Color color2 = UiColorToGdiplusColor(*gradientColor);

            Gdiplus::PointF pt1(gradRect.X, gradRect.Y);
            Gdiplus::PointF pt2(gradRect.GetRight(), gradRect.GetBottom());

            Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
            Gdiplus::Pen gradPen(&brush, pen->GetWidth());
            m_pGraphics->DrawPath(&gradPen, &path);
        }
        else {
            m_pGraphics->DrawPath(pGdiPen->GetPen(), &path);
        }
    }
    else {
        // 只绘制弧线（这部分不需要修改）
        if (gradientColor != nullptr && gradientRect != nullptr) {
            Gdiplus::RectF gradRect(gradientRect->left + m_pPointOrg->X,
                gradientRect->top + m_pPointOrg->Y,
                static_cast<Gdiplus::REAL>(gradientRect->Width()),
                static_cast<Gdiplus::REAL>(gradientRect->Height()));

            Gdiplus::Color color1 = UiColorToGdiplusColor(pen->GetColor());
            Gdiplus::Color color2 = UiColorToGdiplusColor(*gradientColor);

            Gdiplus::PointF pt1(gradRect.X, gradRect.Y);
            Gdiplus::PointF pt2(gradRect.GetRight(), gradRect.GetBottom());

            Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
            Gdiplus::Pen gradPen(&brush, pen->GetWidth());
            m_pGraphics->DrawArc(&gradPen, rect, startAngle, sweepAngle);
        }
        else {
            m_pGraphics->DrawArc(pGdiPen->GetPen(), rect, startAngle, sweepAngle);
        }
    }
}

void Render_GDI::DrawPath(const IPath* path, const IPen* pen)
{
    ASSERT(path != nullptr);
    ASSERT(pen != nullptr);
    if (path == nullptr || pen == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    const Pen_GDI* pGdiPen = dynamic_cast<const Pen_GDI*>(pen);

    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr ||
        pGdiPen == nullptr || pGdiPen->GetPen() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        m_pGraphics->DrawPath(pGdiPen->GetPen(), pPath);

        delete pPath;
    }
}

void Render_GDI::FillPath(const IPath* path, const IBrush* brush)
{
    ASSERT(path != nullptr);
    ASSERT(brush != nullptr);
    if (path == nullptr || brush == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    const Brush_GDI* pGdiBrush = dynamic_cast<const Brush_GDI*>(brush);

    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr ||
        pGdiBrush == nullptr || pGdiBrush->GetBrush() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        m_pGraphics->FillPath(pGdiBrush->GetBrush(), pPath);

        delete pPath;
    }
}

void Render_GDI::FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction)
{
    ASSERT(path != nullptr);
    if (path == nullptr || m_pGraphics == nullptr) {
        return;
    }

    const Path_GDI* pGdiPath = dynamic_cast<const Path_GDI*>(path);
    if (pGdiPath == nullptr || pGdiPath->GetPath() == nullptr) {
        return;
    }

    // 克隆路径并应用偏移
    Gdiplus::GraphicsPath* pPath = pGdiPath->GetPath()->Clone();
    if (pPath != nullptr) {
        Gdiplus::Matrix matrix;
        matrix.Translate(m_pPointOrg->X, m_pPointOrg->Y);
        pPath->Transform(&matrix);

        // 创建渐变画刷
        Gdiplus::RectF rect(rc.left + m_pPointOrg->X,
                           rc.top + m_pPointOrg->Y,
                           static_cast<Gdiplus::REAL>(rc.Width()),
                           static_cast<Gdiplus::REAL>(rc.Height()));

        // 标准化方向
        if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
            nColor2Direction = 1;
        }

        Gdiplus::PointF pt1(rect.X, rect.Y);
        Gdiplus::PointF pt2;

        if (nColor2Direction == 2) {
            pt2.X = rect.X;
            pt2.Y = rect.GetBottom();
        }
        else if (nColor2Direction == 3) {
            pt2.X = rect.GetRight();
            pt2.Y = rect.GetBottom();
        }
        else if (nColor2Direction == 4) {
            pt1.X = rect.GetRight();
            pt2.X = rect.X;
            pt2.Y = rect.GetBottom();
        }
        else {
            pt2.X = rect.GetRight();
            pt2.Y = rect.Y;
        }

        Gdiplus::Color color1 = UiColorToGdiplusColor(dwColor);
        Gdiplus::Color color2 = UiColorToGdiplusColor(dwColor2);

        Gdiplus::LinearGradientBrush brush(pt1, pt2, color1, color2);
        m_pGraphics->FillPath(&brush, pPath);

        delete pPath;
    }
}

} // namespace ui

