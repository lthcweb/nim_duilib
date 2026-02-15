#include "FontMgr_GDI.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

FontMgr_GDI::FontMgr_GDI()
{
    LoadSystemFonts();
}

FontMgr_GDI::~FontMgr_GDI()
{
    ClearFontFiles();
}

void FontMgr_GDI::LoadSystemFonts()
{
    // 清空现有列表
    m_fontNames.clear();
    m_fontIndexMap.clear();

    // 获取已安装的字体族
    Gdiplus::InstalledFontCollection installedFonts;
    int fontCount = installedFonts.GetFamilyCount();

    if (fontCount > 0) {
        std::vector<Gdiplus::FontFamily> fontFamilies(fontCount);
        int found = 0;
        installedFonts.GetFamilies(fontCount, fontFamilies.data(), &found);

        for (int i = 0; i < found; ++i) {
            wchar_t familyName[LF_FACESIZE];
            if (fontFamilies[i].GetFamilyName(familyName) == Gdiplus::Ok) {
#ifdef DUILIB_UNICODE
                DString fontName = familyName;
#else
                DString fontName = StringUtil::UTF16ToUTF8(familyName);
#endif
                m_fontNames.push_back(fontName);
                m_fontIndexMap[fontName] = static_cast<int>(m_fontNames.size() - 1);
            }
        }
    }

    // 排序字体名称
    std::sort(m_fontNames.begin(), m_fontNames.end());

    // 重建索引映射
    m_fontIndexMap.clear();
    for (size_t i = 0; i < m_fontNames.size(); ++i) {
        m_fontIndexMap[m_fontNames[i]] = static_cast<int>(i);
    }
}

uint32_t FontMgr_GDI::GetFontCount() const
{
    return static_cast<uint32_t>(m_fontNames.size());
}

bool FontMgr_GDI::GetFontName(uint32_t nIndex, DString& fontName) const
{
    if (nIndex >= m_fontNames.size()) {
        return false;
    }
    fontName = m_fontNames[nIndex];
    return true;
}

bool FontMgr_GDI::HasFontName(const DString& fontName) const
{
    return m_fontIndexMap.find(fontName) != m_fontIndexMap.end();
}

void FontMgr_GDI::SetDefaultFontName(const DString& fontName)
{
    m_defaultFontName = fontName;
}

bool FontMgr_GDI::LoadFontFile(const DString& fontFilePath)
{
    if (fontFilePath.empty()) {
        return false;
    }

#ifdef DUILIB_UNICODE
    const wchar_t* szPath = fontFilePath.c_str();
#else
    std::wstring wsPath = StringUtil::UTF8ToUTF16(fontFilePath);
    const wchar_t* szPath = wsPath.c_str();
#endif

    // 创建私有字体集合
    Gdiplus::PrivateFontCollection* pFontCollection = new Gdiplus::PrivateFontCollection();
    Gdiplus::Status status = pFontCollection->AddFontFile(szPath);

    if (status != Gdiplus::Ok) {
        delete pFontCollection;
        return false;
    }

    // 保存字体集合
    m_privateFonts.push_back(pFontCollection);

    // 获取字体族名称
    int familyCount = pFontCollection->GetFamilyCount();
    if (familyCount > 0) {
        std::vector<Gdiplus::FontFamily> fontFamilies(familyCount);
        int found = 0;
        pFontCollection->GetFamilies(familyCount, fontFamilies.data(), &found);

        for (int i = 0; i < found; ++i) {
            wchar_t familyName[LF_FACESIZE];
            if (fontFamilies[i].GetFamilyName(familyName) == Gdiplus::Ok) {
#ifdef DUILIB_UNICODE
                DString fontName = familyName;
#else
                DString fontName = StringUtil::UTF16ToUTF8(familyName);
#endif
                // 检查是否已存在
                if (!HasFontName(fontName)) {
                    m_fontNames.push_back(fontName);
                    m_fontIndexMap[fontName] = static_cast<int>(m_fontNames.size() - 1);
                }
            }
        }
    }

    return true;
}

bool FontMgr_GDI::LoadFontFileData(const void* data, size_t length)
{
    if (data == nullptr || length == 0) {
        return false;
    }

    // 创建私有字体集合
    Gdiplus::PrivateFontCollection* pFontCollection = new Gdiplus::PrivateFontCollection();

    // GDI+ 需要将数据写入内存并添加
    // 注意：AddMemoryFont 需要字体数据在内存中保持有效
    Gdiplus::Status status = pFontCollection->AddMemoryFont(data, static_cast<INT>(length));

    if (status != Gdiplus::Ok) {
        delete pFontCollection;
        return false;
    }

    // 保存字体集合
    m_privateFonts.push_back(pFontCollection);

    // 获取字体族名称
    int familyCount = pFontCollection->GetFamilyCount();
    if (familyCount > 0) {
        std::vector<Gdiplus::FontFamily> fontFamilies(familyCount);
        int found = 0;
        pFontCollection->GetFamilies(familyCount, fontFamilies.data(), &found);

        for (int i = 0; i < found; ++i) {
            wchar_t familyName[LF_FACESIZE];
            if (fontFamilies[i].GetFamilyName(familyName) == Gdiplus::Ok) {
#ifdef DUILIB_UNICODE
                DString fontName = familyName;
#else
                DString fontName = StringUtil::UTF16ToUTF8(familyName);
#endif
                if (!HasFontName(fontName)) {
                    m_fontNames.push_back(fontName);
                    m_fontIndexMap[fontName] = static_cast<int>(m_fontNames.size() - 1);
                }
            }
        }
    }

    return true;
}

void FontMgr_GDI::ClearFontFiles()
{
    // 删除所有私有字体集合
    for (auto pFontCollection : m_privateFonts) {
        delete pFontCollection;
    }
    m_privateFonts.clear();

    // 重新加载系统字体
    LoadSystemFonts();
}

void FontMgr_GDI::ClearFontCache()
{
    // GDI+ 没有显式的字体缓存需要清理
    // 字体缓存由 GDI+ 内部管理
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
