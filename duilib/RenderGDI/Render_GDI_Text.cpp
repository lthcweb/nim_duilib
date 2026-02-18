// Render_GDI.cpp 的补充部分 - 文本绘制


#include "Render_GDI.h"
#include "Font_GDI.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/PerformanceUtil.h"
#include <mutex>
#include <unordered_map>

namespace ui {

namespace
{
struct GdiRichTextSegment
{
    std::wstring text;
    UiColor textColor;
    UiColor bgColor;
    SharePtr<UiFontEx> fontInfo;
    float rowSpacingMul = 1.0f;
    float rowSpacingAdd = 0.0f;
    uint16_t textStyle = 0;
};

struct GdiRichTextCacheData
{
    UiRect textRect;
    std::vector<GdiRichTextSegment> segments;
};

static std::mutex g_cacheMutex;
static std::unordered_map<const DrawRichTextCache*, GdiRichTextCacheData> g_cacheDataMap;

static std::vector<GdiRichTextSegment> ConvertSegments(const std::vector<RichTextData>& richTextData)
{
    std::vector<GdiRichTextSegment> segments;
    segments.reserve(richTextData.size());
    for (const auto& src : richTextData) {
        GdiRichTextSegment seg;
        seg.text.assign(src.m_textView.data(), src.m_textView.size());
        seg.textColor = src.m_textColor;
        seg.bgColor = src.m_bgColor;
        seg.fontInfo = src.m_pFontInfo;
        seg.rowSpacingMul = src.m_fRowSpacingMul;
        seg.rowSpacingAdd = src.m_fRowSpacingAdd;
        seg.textStyle = src.m_textStyle;
        segments.push_back(std::move(seg));
    }
    return segments;
}

static bool IsSegmentSame(const GdiRichTextSegment& seg, const RichTextData& src)
{
    if (seg.textColor != src.m_textColor || seg.bgColor != src.m_bgColor ||
        seg.rowSpacingMul != src.m_fRowSpacingMul || seg.rowSpacingAdd != src.m_fRowSpacingAdd ||
        seg.textStyle != src.m_textStyle ||
        seg.text.size() != src.m_textView.size() ||
        !std::equal(seg.text.begin(), seg.text.end(), src.m_textView.begin())) {
        return false;
    }
    if ((seg.fontInfo == nullptr) != (src.m_pFontInfo == nullptr)) {
        return false;
    }
    if ((seg.fontInfo != nullptr) && (*seg.fontInfo != *src.m_pFontInfo)) {
        return false;
    }
    return true;
}
}

UiRect Render_GDI::MeasureString(const DString& strText, const MeasureStringParam& measureParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        return UiRect();
    }

    PerformanceStat statPerformance(_T("Render_GDI::MeasureString"));

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return UiRect();
    }

    ASSERT(measureParam.pFont != nullptr);
    if (measureParam.pFont == nullptr || m_pGraphics == nullptr) {
        return UiRect();
    }

    Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(measureParam.pFont);
    ASSERT(pGdiFont != nullptr);
    if (pGdiFont == nullptr || pGdiFont->GetFont() == nullptr) {
        return UiRect();
    }

#ifdef DUILIB_UNICODE
    const wchar_t* szText = strText.c_str();
    int textLen = static_cast<int>(strText.length());
#else
    std::wstring wsText = StringUtil::UTF8ToUTF16(strText);
    const wchar_t* szText = wsText.c_str();
    int textLen = static_cast<int>(wsText.length());
#endif

    // 设置字符串格式
    Gdiplus::StringFormat stringFormat;

    // 单行/多行模式
    if (measureParam.uFormat & TEXT_SINGLELINE) {
        stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    }

    // 水平对齐
    if (measureParam.uFormat & TEXT_HCENTER) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (measureParam.uFormat & TEXT_RIGHT) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    }

    // 垂直对齐
    if (measureParam.uFormat & TEXT_VCENTER) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (measureParam.uFormat & TEXT_BOTTOM) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    }

    // 测量文本
    Gdiplus::RectF layoutRect;
    if (measureParam.rectSize != DUI_NOSET_VALUE) {
        if (measureParam.uFormat & TEXT_VERTICAL) {
            layoutRect.Width = 10000.0f;
            layoutRect.Height = static_cast<Gdiplus::REAL>(measureParam.rectSize);
        }
        else {
            layoutRect.Width = static_cast<Gdiplus::REAL>(measureParam.rectSize);
            layoutRect.Height = 10000.0f;
        }
    }
    else {
        layoutRect.Width = 10000.0f;
        layoutRect.Height = 10000.0f;
    }

    Gdiplus::RectF boundingBox;
    m_pGraphics->MeasureString(szText, textLen, pGdiFont->GetFont(),
                              layoutRect, &stringFormat, &boundingBox);

    UiRect rc;
    rc.left = 0;
    rc.top = 0;
    rc.right = static_cast<int>(boundingBox.Width + 0.5f);
    rc.bottom = static_cast<int>(boundingBox.Height + 0.5f);

    return rc;
}

void Render_GDI::DrawString(const DString& strText, const DrawStringParam& drawParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        return;
    }

    PerformanceStat statPerformance(_T("Render_GDI::DrawString"));

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }

    ASSERT(!drawParam.textRect.IsEmpty());
    if (drawParam.textRect.IsEmpty()) {
        return;
    }

    ASSERT(drawParam.pFont != nullptr);
    if (drawParam.pFont == nullptr || m_pGraphics == nullptr) {
        return;
    }

    Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(drawParam.pFont);
    ASSERT(pGdiFont != nullptr);
    if (pGdiFont == nullptr || pGdiFont->GetFont() == nullptr) {
        return;
    }

#ifdef DUILIB_UNICODE
    const wchar_t* szText = strText.c_str();
    int textLen = static_cast<int>(strText.length());
#else
    std::wstring wsText = StringUtil::UTF8ToUTF16(strText);
    const wchar_t* szText = wsText.c_str();
    int textLen = static_cast<int>(wsText.length());
#endif

    // 设置文本颜色
    Gdiplus::Color textColor = UiColorToGdiplusColor(drawParam.dwTextColor, drawParam.uFade);
    Gdiplus::SolidBrush brush(textColor);

    // 设置绘制区域
    Gdiplus::RectF rect(drawParam.textRect.left + m_pPointOrg->X,
                       drawParam.textRect.top + m_pPointOrg->Y,
                       static_cast<Gdiplus::REAL>(drawParam.textRect.Width()),
                       static_cast<Gdiplus::REAL>(drawParam.textRect.Height()));

    // 设置字符串格式
    Gdiplus::StringFormat stringFormat;

    // 单行/多行模式
    if (drawParam.uFormat & TEXT_SINGLELINE) {
        stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    }

    // 自动换行
    if (drawParam.uFormat & TEXT_WORD_WRAP) {
        // GDI+ 默认支持自动换行
    }

    // 水平对齐
    if (drawParam.uFormat & TEXT_HCENTER) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (drawParam.uFormat & TEXT_RIGHT) {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    }

    // 垂直对齐
    if (drawParam.uFormat & TEXT_VCENTER) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    }
    else if (drawParam.uFormat & TEXT_BOTTOM) {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
    }
    else {
        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    }

    // 省略号
    if (drawParam.uFormat & TEXT_END_ELLIPSIS) {
        stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    }
    else if (drawParam.uFormat & TEXT_PATH_ELLIPSIS) {
        stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
    }

    // 不裁剪
    if (drawParam.uFormat & TEXT_NOCLIP) {
        stringFormat.SetFormatFlags(stringFormat.GetFormatFlags() | Gdiplus::StringFormatFlagsNoClip);
    }

    // 绘制文本
    m_pGraphics->DrawString(szText, textLen, pGdiFont->GetFont(),
                           rect, &stringFormat, &brush);
}

// 富文本绘制相关函数 - 这些需要更复杂的实现
void Render_GDI::MeasureRichText(const UiRect& textRect,
                                 const UiSize& szScrollOffset,
                                 IRenderFactory* pRenderFactory,
                                 const std::vector<RichTextData>& richTextData,
                                 std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText"));
    UNUSED_VARIABLE(szScrollOffset);
    UNUSED_VARIABLE(pRenderFactory);
    if (pRichTextRects != nullptr) {
        pRichTextRects->assign(richTextData.size(), std::vector<UiRect>());
    }

    int32_t x = textRect.left;
    int32_t y = textRect.top;
    int32_t lineHeight = 0;
    bool bSingleLine = false;
    if (!richTextData.empty()) {
        bSingleLine = (richTextData[0].m_textStyle & TEXT_SINGLELINE) != 0;
    }

    for (size_t i = 0; i < richTextData.size(); ++i) {
        const auto& textData = richTextData[i];
        if ((textData.m_pFontInfo == nullptr) || textData.m_textView.empty()) {
            continue;
        }
        if (pRenderFactory == nullptr) {
            continue;
        }
        std::unique_ptr<IFont> spFont(pRenderFactory->CreateIFont());
        if ((spFont == nullptr) || !spFont->InitFont(*textData.m_pFontInfo)) {
            continue;
        }

        MeasureStringParam mp;
        mp.pFont = spFont.get();
        mp.uFormat = TEXT_SINGLELINE | (textData.m_textStyle & (TEXT_END_ELLIPSIS | TEXT_PATH_ELLIPSIS));

        std::wstring text(textData.m_textView.data(), textData.m_textView.size());
        size_t start = 0;
        while (start <= text.size()) {
            size_t pos = text.find(L'\n', start);
            std::wstring line = (pos == std::wstring::npos) ? text.substr(start) : text.substr(start, pos - start);
            if (!line.empty() && (line.back() == L'\r')) {
                line.pop_back();
            }
            UiRect sz = MeasureString(line.empty() ? DString(_T(" ")) : DString(line.c_str()), mp);
            int32_t w = sz.Width();
            int32_t h = std::max(sz.Height(), 1);
            if (!bSingleLine && (x + w > textRect.right) && (x > textRect.left)) {
                x = textRect.left;
                y += lineHeight;
                lineHeight = 0;
            }
            UiRect drawRect(x, y, x + w, y + h);
            if (pRichTextRects != nullptr) {
                (*pRichTextRects)[i].push_back(drawRect);
            }
            x += w;
            lineHeight = std::max(lineHeight, h);
            if ((pos != std::wstring::npos) && !bSingleLine) {
                x = textRect.left;
                y += lineHeight;
                lineHeight = 0;
                start = pos + 1;
            }
            else {
                break;
            }
        }
    }
}

void Render_GDI::MeasureRichText2(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText2"));
    UNUSED_VARIABLE(pLineInfoParam);
    MeasureRichText(textRect, szScrollOffset, pRenderFactory, richTextData, pRichTextRects);
}

void Render_GDI::MeasureRichText3(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText3"));
    MeasureRichText(textRect, szScrollOffset, pRenderFactory, richTextData, pRichTextRects);
    CreateDrawRichTextCache(textRect, szScrollOffset, pRenderFactory, richTextData, spDrawRichTextCache);
}

void Render_GDI::DrawRichText(const UiRect& textRect,
                              const UiSize& szScrollOffset,
                              IRenderFactory* pRenderFactory,
                              const std::vector<RichTextData>& richTextData,
                              uint8_t uFade,
                              std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::DrawRichText"));
    UNUSED_VARIABLE(szScrollOffset);
    if (pRichTextRects != nullptr) {
        pRichTextRects->assign(richTextData.size(), std::vector<UiRect>());
    }
    if ((pRenderFactory == nullptr) || richTextData.empty() || textRect.IsEmpty()) {
        return;
    }

    int32_t x = textRect.left;
    int32_t y = textRect.top;
    int32_t lineHeight = 0;
    bool bSingleLine = (richTextData[0].m_textStyle & TEXT_SINGLELINE) != 0;

    for (size_t i = 0; i < richTextData.size(); ++i) {
        const auto& textData = richTextData[i];
        if ((textData.m_pFontInfo == nullptr) || textData.m_textView.empty()) {
            continue;
        }
        std::unique_ptr<IFont> spFont(pRenderFactory->CreateIFont());
        if ((spFont == nullptr) || !spFont->InitFont(*textData.m_pFontInfo)) {
            continue;
        }

        DrawStringParam dp;
        dp.pFont = spFont.get();
        dp.dwTextColor = textData.m_textColor;
        dp.uFade = uFade;
        dp.uFormat = TEXT_SINGLELINE;
        MeasureStringParam mp;
        mp.pFont = spFont.get();
        mp.uFormat = TEXT_SINGLELINE;

        std::wstring text(textData.m_textView.data(), textData.m_textView.size());
        size_t start = 0;
        while (start <= text.size()) {
            size_t pos = text.find(L'\n', start);
            std::wstring line = (pos == std::wstring::npos) ? text.substr(start) : text.substr(start, pos - start);
            if (!line.empty() && (line.back() == L'\r')) {
                line.pop_back();
            }
            DString drawText = line.empty() ? DString(_T(" ")) : DString(line.c_str());
            UiRect sz = MeasureString(drawText, mp);
            int32_t w = sz.Width();
            int32_t h = std::max(sz.Height(), 1);
            if (!bSingleLine && (x + w > textRect.right) && (x > textRect.left)) {
                x = textRect.left;
                y += lineHeight;
                lineHeight = 0;
            }
            UiRect drawRect(x, y, x + w, y + h);

            if (!textData.m_bgColor.IsEmpty()) {
                FillRect(drawRect, textData.m_bgColor, uFade);
            }
            dp.textRect = drawRect;
            DrawString(drawText, dp);
            if (pRichTextRects != nullptr) {
                (*pRichTextRects)[i].push_back(drawRect);
            }

            x += w;
            lineHeight = std::max(lineHeight, h);
            if ((pos != std::wstring::npos) && !bSingleLine) {
                x = textRect.left;
                y += lineHeight;
                lineHeight = 0;
                start = pos + 1;
            }
            else {
                break;
            }
        }
    }
}

bool Render_GDI::CreateDrawRichTextCache(const UiRect& textRect,
                                         const UiSize& szScrollOffset,
                                         IRenderFactory* pRenderFactory,
                                         const std::vector<RichTextData>& richTextData,
                                         std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    PerformanceStat statPerformance(_T("Render_GDI::CreateDrawRichTextCache"));
    UNUSED_VARIABLE(szScrollOffset);
    UNUSED_VARIABLE(pRenderFactory);
    spDrawRichTextCache.reset();
    if (richTextData.empty() || textRect.IsEmpty()) {
        return false;
    }

    // 使用 opaque 指针作为缓存句柄，避免改动公共接口
    DrawRichTextCache* key = reinterpret_cast<DrawRichTextCache*>(new uint8_t[1]);
    spDrawRichTextCache = std::shared_ptr<DrawRichTextCache>(
        key,
        [](DrawRichTextCache* p) {
            {
                std::lock_guard<std::mutex> guard(g_cacheMutex);
                g_cacheDataMap.erase(p);
            }
            delete[] reinterpret_cast<uint8_t*>(p);
        });

    GdiRichTextCacheData cacheData;
    cacheData.textRect = textRect;
    cacheData.segments = ConvertSegments(richTextData);
    {
        std::lock_guard<std::mutex> guard(g_cacheMutex);
        g_cacheDataMap[key] = std::move(cacheData);
    }
    return true;
}

bool Render_GDI::IsValidDrawRichTextCache(const UiRect& textRect,
                                          const std::vector<RichTextData>& richTextData,
                                          const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    if (spDrawRichTextCache == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> guard(g_cacheMutex);
    auto it = g_cacheDataMap.find(spDrawRichTextCache.get());
    if (it == g_cacheDataMap.end()) {
        return false;
    }
    const GdiRichTextCacheData& cacheData = it->second;
    if (cacheData.textRect != textRect || cacheData.segments.size() != richTextData.size()) {
        return false;
    }
    for (size_t i = 0; i < richTextData.size(); ++i) {
        if (!IsSegmentSame(cacheData.segments[i], richTextData[i])) {
            return false;
        }
    }
    return true;
}

bool Render_GDI::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                         const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                         std::vector<RichTextData>& richTextDataNew,
                                         size_t nStartLine,
                                         const std::vector<size_t>& modifiedLines,
                                         size_t nModifiedRows,
                                         const std::vector<size_t>& deletedLines,
                                         size_t nDeletedRows,
                                         const std::vector<int32_t>& rowRectTopList)
{
    PerformanceStat statPerformance(_T("Render_GDI::UpdateDrawRichTextCache"));
    UNUSED_VARIABLE(nStartLine);
    UNUSED_VARIABLE(modifiedLines);
    UNUSED_VARIABLE(nModifiedRows);
    UNUSED_VARIABLE(deletedLines);
    UNUSED_VARIABLE(nDeletedRows);
    UNUSED_VARIABLE(rowRectTopList);
    UNUSED_VARIABLE(richTextDataNew);
    if ((spOldDrawRichTextCache == nullptr) || (spUpdateDrawRichTextCache == nullptr)) {
        return false;
    }
    spOldDrawRichTextCache = spUpdateDrawRichTextCache;
    return true;
}

bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    return (&first == &second);
}

void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                       const UiRect& textRect,
                                       const UiSize& szNewScrollOffset,
                                       const std::vector<int32_t>& rowXOffset,
                                       uint8_t uFade,
                                       std::vector<std::vector<UiRect>>* pRichTextRects)
{
    UNUSED_VARIABLE(szNewScrollOffset);
    UNUSED_VARIABLE(rowXOffset);
    if (spDrawRichTextCache == nullptr) {
        return;
    }

    GdiRichTextCacheData cacheData;
    {
        std::lock_guard<std::mutex> guard(g_cacheMutex);
        auto it = g_cacheDataMap.find(spDrawRichTextCache.get());
        if (it == g_cacheDataMap.end()) {
            return;
        }
        cacheData = it->second;
    }

    std::vector<RichTextData> richTextData;
    richTextData.reserve(cacheData.segments.size());
    std::vector<std::wstring> textHolder;
    textHolder.reserve(cacheData.segments.size());
    for (const auto& seg : cacheData.segments) {
        textHolder.push_back(seg.text);
        RichTextData d;
        d.m_textView = textHolder.back();
        d.m_textColor = seg.textColor;
        d.m_bgColor = seg.bgColor;
        d.m_pFontInfo = seg.fontInfo;
        d.m_fRowSpacingMul = seg.rowSpacingMul;
        d.m_fRowSpacingAdd = seg.rowSpacingAdd;
        d.m_textStyle = seg.textStyle;
        richTextData.push_back(d);
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        DrawRichText(textRect, UiSize(), pRenderFactory, richTextData, uFade, pRichTextRects);
    }
}

size_t Render_GDI::GetUTF16CharCount(const DStringW::value_type* srcPtr, size_t textStartIndex) const
{
    if (srcPtr != nullptr) {
        ASSERT(sizeof(uint16_t) == sizeof(DStringW::value_type));
        const uint16_t* src = (const uint16_t*)(srcPtr + textStartIndex);
        // 检查是否为高代理字符（UTF-16 代理对）
        if ((*src >= 0xD800) && (*src <= 0xDBFF)) {
            // 高代理字符，需要两个字符
            return 2;
        }
    }
    return 1;
}

} // namespace ui
