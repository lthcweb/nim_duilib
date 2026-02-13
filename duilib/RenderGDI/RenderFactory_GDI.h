#ifndef UI_RENDER_GDI_RENDER_FACTORY_H_
#define UI_RENDER_GDI_RENDER_FACTORY_H_

#include "duilib/Render/IRender.h"

namespace ui {

class UILIB_API RenderFactory_GDI : public IRenderFactory
{
public:
    RenderFactory_GDI();
    virtual ~RenderFactory_GDI() override;

    virtual IFont* CreateIFont() override;
    virtual IPen* CreatePen(UiColor color, float fWidth = 1.0f) override;
    virtual IBrush* CreateBrush(UiColor corlor) override;
    virtual IPath* CreatePath() override;
    virtual IMatrix* CreateMatrix() override;
    virtual IBitmap* CreateBitmap() override;
    virtual IRender* CreateRender(const IRenderDpiPtr& spRenderDpi,
                                  void* platformData = nullptr,
                                  RenderBackendType backendType = RenderBackendType::kRaster_BackendType) override;
    virtual IFontMgr* GetFontMgr() const override;

private:
    std::shared_ptr<IFontMgr> m_fontMgr;
};

}

#endif
