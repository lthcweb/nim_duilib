#ifndef UI_RENDER_MATRIX_GDI_H_
#define UI_RENDER_MATRIX_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

/** GDI+ 矩阵实现类
*/
class UILIB_API Matrix_GDI : public IMatrix
{
public:
    Matrix_GDI();
    virtual ~Matrix_GDI();

    Matrix_GDI(const Matrix_GDI&) = delete;
    Matrix_GDI& operator=(const Matrix_GDI&) = delete;

public:
    virtual void Translate(int offsetX, int offsetY) override;
    virtual void Scale(float scaleX, float scaleY) override;
    virtual void Rotate(float angle) override;
    virtual void RotateAt(float angle, const UiPoint& center) override;

public:
    /** 获取 GDI+ Matrix 对象
    */
    Gdiplus::Matrix* GetMatrix() const { return m_pMatrix; }

private:
    Gdiplus::Matrix* m_pMatrix;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_MATRIX_GDI_H_
