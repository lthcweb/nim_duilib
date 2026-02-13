#include "RenderFactory_GDI.h"
#include "duilib/RenderGDI/GdiObjects.h"
#include "duilib/RenderGDI/Render_GDI.h"

namespace ui {

RenderFactory_GDI::RenderFactory_GDI()
{
    m_fontMgr = std::make_shared<FontMgr_GDI>();
}

RenderFactory_GDI::~RenderFactory_GDI() = default;

IFont* RenderFactory_GDI::CreateIFont(){ return new Font_GDI(); }
IPen* RenderFactory_GDI::CreatePen(UiColor color, float fWidth){ return new Pen_GDI(color, fWidth); }
IBrush* RenderFactory_GDI::CreateBrush(UiColor color){ return new Brush_GDI(color); }
IPath* RenderFactory_GDI::CreatePath(){ return new Path_GDI(); }
IMatrix* RenderFactory_GDI::CreateMatrix(){ return new Matrix_GDI(); }
IBitmap* RenderFactory_GDI::CreateBitmap(){ return new Bitmap_GDI(); }

IRender* RenderFactory_GDI::CreateRender(const IRenderDpiPtr& spRenderDpi, void* platformData, RenderBackendType backendType)
{
    IRender* pRender = new Render_GDI(platformData, backendType);
    if (pRender != nullptr) {
        pRender->SetRenderDpi(spRenderDpi);
    }
    return pRender;
}

IFontMgr* RenderFactory_GDI::GetFontMgr() const
{
    return m_fontMgr.get();
}

} // namespace ui
