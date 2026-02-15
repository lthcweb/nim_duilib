#ifndef UI_RENDER_FONTMGR_GDI_H_
#define UI_RENDER_FONTMGR_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"
#include <vector>
#include <string>
#include <map>

namespace ui
{

/** GDI+ 字体管理器实现类
*/
class UILIB_API FontMgr_GDI : public IFontMgr
{
public:
    FontMgr_GDI();
    virtual ~FontMgr_GDI();

    FontMgr_GDI(const FontMgr_GDI&) = delete;
    FontMgr_GDI& operator=(const FontMgr_GDI&) = delete;

public:
    virtual uint32_t GetFontCount() const override;
    virtual bool GetFontName(uint32_t nIndex, DString& fontName) const override;
    virtual bool HasFontName(const DString& fontName) const override;
    virtual void SetDefaultFontName(const DString& fontName) override;
    virtual bool LoadFontFile(const DString& fontFilePath) override;
    virtual bool LoadFontFileData(const void* data, size_t length) override;
    virtual void ClearFontFiles() override;
    virtual void ClearFontCache() override;

private:
    /** 加载系统字体
    */
    void LoadSystemFonts();

private:
    std::vector<DString> m_fontNames;
    DString m_defaultFontName;
    std::vector<Gdiplus::PrivateFontCollection*> m_privateFonts;
    std::map<DString, int> m_fontIndexMap;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_FONTMGR_GDI_H_
