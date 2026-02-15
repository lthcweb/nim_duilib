#include "RenderFactory_GDI.h"
#include "Render_GDI.h"
#include "Font_GDI.h"
#include "Pen_GDI.h"
#include "Brush_GDI.h"
#include "Path_GDI.h"
#include "Matrix_GDI.h"
#include "Bitmap_GDI.h"
#include "FontMgr_GDI.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

RenderFactory_GDI::RenderFactory_GDI()
    : m_pFontMgr(nullptr)
    , m_gdiplusToken(0)
{
    // 初始化 GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    // 创建字体管理器
    m_pFontMgr = new FontMgr_GDI();
}

RenderFactory_GDI::~RenderFactory_GDI()
{
    // 删除字体管理器
    if (m_pFontMgr != nullptr) {
        delete m_pFontMgr;
        m_pFontMgr = nullptr;
    }

    // 关闭 GDI+
    if (m_gdiplusToken != 0) {
        Gdiplus::GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
    }
}

IFont* RenderFactory_GDI::CreateIFont()
{
    return new Font_GDI();
}

IPen* RenderFactory_GDI::CreatePen(UiColor color, float fWidth)
{
    return new Pen_GDI(color, fWidth);
}

IBrush* RenderFactory_GDI::CreateBrush(UiColor color)
{
    return new Brush_GDI(color);
}

IPath* RenderFactory_GDI::CreatePath()
{
    return new Path_GDI();
}

IMatrix* RenderFactory_GDI::CreateMatrix()
{
    return new Matrix_GDI();
}

IBitmap* RenderFactory_GDI::CreateBitmap()
{
    return new Bitmap_GDI();
}

IRender* RenderFactory_GDI::CreateRender(const IRenderDpiPtr& spRenderDpi,
                                        void* platformData,
                                        RenderBackendType backendType)
{
    Render_GDI* pRender = new Render_GDI();
    if (pRender != nullptr) {
        pRender->SetRenderDpi(spRenderDpi);
        // platformData 在 Windows 平台是 HWND
        // 在 Render_GDI 中可以根据需要使用
    }
    return pRender;
}

IFontMgr* RenderFactory_GDI::GetFontMgr() const
{
    return m_pFontMgr;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
