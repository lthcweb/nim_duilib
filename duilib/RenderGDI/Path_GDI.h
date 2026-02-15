#ifndef UI_RENDER_PATH_GDI_H_
#define UI_RENDER_PATH_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

/** GDI+ 路径实现类
*/
class UILIB_API Path_GDI : public IPath
{
public:
    Path_GDI();
    virtual ~Path_GDI();

    Path_GDI(const Path_GDI&) = delete;
    Path_GDI& operator=(const Path_GDI&) = delete;

public:
    virtual void SetFillType(FillType mode) override;
    virtual FillType GetFillType() override;

    virtual void AddLine(int x1, int y1, int x2, int y2) override;
    virtual void AddLines(const UiPoint* points, int count) override;
    virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
    virtual void AddBeziers(const UiPoint* points, int count) override;
    virtual void AddRect(const UiRect& rect) override;
    virtual void AddEllipse(const UiRect& rect) override;
    virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) override;
    virtual void AddPolygon(const UiPoint* points, int count) override;
    virtual void AddPolygon(const UiPointF* points, int count) override;
    virtual void Transform(IMatrix* pMatrix) override;
    virtual UiRect GetBounds(const IPen* pen) override;
    virtual void Close() override;
    virtual void Reset() override;
    virtual IPath* Clone() override;

public:
    /** 获取 GDI+ GraphicsPath 对象
    */
    Gdiplus::GraphicsPath* GetPath() const { return m_pPath; }

private:
    Gdiplus::GraphicsPath* m_pPath;
    FillType m_fillType;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_PATH_GDI_H_
