#ifndef UI_RENDER_RENDERFACTORY_GDI_H_
#define UI_RENDER_RENDERFACTORY_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui {

class FontMgr_GDI;

/** GDI+ 渲染工厂实现类
*/
class UILIB_API RenderFactory_GDI : public IRenderFactory
{
public:
    RenderFactory_GDI();
    virtual ~RenderFactory_GDI();

    RenderFactory_GDI(const RenderFactory_GDI&) = delete;
    RenderFactory_GDI& operator=(const RenderFactory_GDI&) = delete;

public:
    virtual IFont* CreateIFont() override;
    virtual IPen* CreatePen(UiColor color, float fWidth = 1) override;
    virtual IBrush* CreateBrush(UiColor color) override;
    virtual IPath* CreatePath() override;
    virtual IMatrix* CreateMatrix() override;
    virtual IBitmap* CreateBitmap() override;
    virtual IRender* CreateRender(const IRenderDpiPtr& spRenderDpi,
                                 void* platformData = nullptr,
                                 RenderBackendType backendType = RenderBackendType::kRaster_BackendType) override;
    virtual IFontMgr* GetFontMgr() const override;

private:
    FontMgr_GDI* m_pFontMgr;
    ULONG_PTR m_gdiplusToken;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_RENDERFACTORY_GDI_H_
