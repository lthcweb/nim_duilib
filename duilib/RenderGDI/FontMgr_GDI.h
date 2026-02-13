#ifndef UI_RENDER_GDI_FONT_MGR_GDI_H_
#define UI_RENDER_GDI_FONT_MGR_GDI_H_

#include "duilib/Render/IRender.h"
#include <vector>

namespace ui {

class FontMgr_GDI : public IFontMgr {
public:
    uint32_t GetFontCount() const override;
    bool GetFontName(uint32_t nIndex, DString& fontName) const override;
    bool HasFontName(const DString& fontName) const override;
    void SetDefaultFontName(const DString& fontName) override;
    bool LoadFontFile(const DString& fontFilePath) override;
    bool LoadFontFileData(const void* data, size_t length) override;
    void ClearFontFiles() override;
    void ClearFontCache() override;

private:
    DString m_defaultFontName;
    std::vector<DString> m_fontNames;
};

} // namespace ui

#endif
