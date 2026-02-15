#include "Matrix_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

Matrix_GDI::Matrix_GDI()
    : m_pMatrix(nullptr)
{
    m_pMatrix = new Gdiplus::Matrix();
}

Matrix_GDI::~Matrix_GDI()
{
    if (m_pMatrix != nullptr) {
        delete m_pMatrix;
        m_pMatrix = nullptr;
    }
}

void Matrix_GDI::Translate(int offsetX, int offsetY)
{
    if (m_pMatrix != nullptr) {
        m_pMatrix->Translate(static_cast<Gdiplus::REAL>(offsetX),
                            static_cast<Gdiplus::REAL>(offsetY),
                            Gdiplus::MatrixOrderAppend);
    }
}

void Matrix_GDI::Scale(float scaleX, float scaleY)
{
    if (m_pMatrix != nullptr) {
        m_pMatrix->Scale(scaleX, scaleY, Gdiplus::MatrixOrderAppend);
    }
}

void Matrix_GDI::Rotate(float angle)
{
    if (m_pMatrix != nullptr) {
        m_pMatrix->Rotate(angle, Gdiplus::MatrixOrderAppend);
    }
}

void Matrix_GDI::RotateAt(float angle, const UiPoint& center)
{
    if (m_pMatrix != nullptr) {
        Gdiplus::PointF centerPoint(static_cast<Gdiplus::REAL>(center.x),
                                    static_cast<Gdiplus::REAL>(center.y));
        m_pMatrix->RotateAt(angle, centerPoint, Gdiplus::MatrixOrderAppend);
    }
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
