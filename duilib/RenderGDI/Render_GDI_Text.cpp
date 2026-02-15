// Render_GDI.cpp 的补充部分 - 文本绘制


#include "Render_GDI.h"
#include "Font_GDI.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/StringUtil.h"

#include <mutex>
#include <unordered_map>

namespace ui {

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

namespace {

struct GdiRichTextSnapshot {
    std::wstring text;
    UiColor textColor;
    UiColor bgColor;
    SharePtr<UiFontEx> fontInfo;
    float rowSpacingMul = 1.0f;
    float rowSpacingAdd = 0.0f;
    uint16_t textStyle = 0;
};

struct GdiPendingText {
    size_t dataIndex = 0;
    uint32_t rowIndex = 0;
    std::wstring text;
    UiRect rect;
    std::shared_ptr<IFont> spFont;
    UiColor textColor;
    UiColor bgColor;
    uint16_t textStyle = 0;
};

struct GdiRichTextCacheData {
    UiRect textRect;
    UiSize scrollOffset;
    std::vector<GdiRichTextSnapshot> richTextData;
    std::vector<GdiPendingText> pending;
};

std::unordered_map<const DrawRichTextCache*, GdiRichTextCacheData> g_gdiRichTextCaches;
std::mutex g_gdiRichTextCacheLock;

DString ToDString(const std::wstring_view& ws)
{
#ifdef DUILIB_UNICODE
    return DString(ws.begin(), ws.end());
#else
    return StringUtil::UTF16ToUTF8(std::wstring(ws));
#endif
}

std::vector<GdiRichTextSnapshot> BuildSnapshots(const std::vector<RichTextData>& richTextData)
{
    std::vector<GdiRichTextSnapshot> snapshots;
    snapshots.reserve(richTextData.size());
    for (const RichTextData& v : richTextData) {
        GdiRichTextSnapshot s;
        s.text.assign(v.m_textView.begin(), v.m_textView.end());
        s.textColor = v.m_textColor;
        s.bgColor = v.m_bgColor;
        s.fontInfo = v.m_pFontInfo;
        s.rowSpacingMul = v.m_fRowSpacingMul;
        s.rowSpacingAdd = v.m_fRowSpacingAdd;
        s.textStyle = v.m_textStyle;
        snapshots.push_back(std::move(s));
    }
    return snapshots;
}

bool IsSameSnapshots(const std::vector<GdiRichTextSnapshot>& lhs, const std::vector<RichTextData>& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const GdiRichTextSnapshot& a = lhs[i];
        const RichTextData& b = rhs[i];
        if ((a.text != std::wstring(b.m_textView.begin(), b.m_textView.end())) ||
            (a.textColor != b.m_textColor) ||
            (a.bgColor != b.m_bgColor) ||
            (a.rowSpacingMul != b.m_fRowSpacingMul) ||
            (a.rowSpacingAdd != b.m_fRowSpacingAdd) ||
            (a.textStyle != b.m_textStyle)) {
            return false;
        }
        if ((a.fontInfo == nullptr) != (b.m_pFontInfo == nullptr)) {
            return false;
        }
        if ((a.fontInfo != nullptr) && (b.m_pFontInfo != nullptr) && !(*a.fontInfo == *b.m_pFontInfo)) {
            return false;
        }
    }
    return true;
}

size_t GetUtf16CharCountLocal(const wchar_t* srcPtr, size_t textStartIndex)
{
    if (srcPtr != nullptr) {
        const uint16_t* src = reinterpret_cast<const uint16_t*>(srcPtr + textStartIndex);
        if ((*src >= 0xD800) && (*src <= 0xDBFF)) {
            return 2;
        }
    }
    return 1;
}

int32_t MeasureTextWidth(Render_GDI* render, IFont* pFont, const std::wstring_view& text)
{
    MeasureStringParam p;
    p.pFont = pFont;
    p.uFormat = TEXT_SINGLELINE;
    return render->MeasureString(ToDString(text), p).Width();
}

int32_t MeasureLineHeight(Render_GDI* render, IFont* pFont)
{
    MeasureStringParam p;
    p.pFont = pFont;
    p.uFormat = TEXT_SINGLELINE;
    int32_t h = render->MeasureString(_T("Hg"), p).Height();
    return std::max<int32_t>(1, h);
}

void LayoutRichText(Render_GDI* render,
                    const UiRect& textRect,
                    const UiSize& scrollOffset,
                    IRenderFactory* pRenderFactory,
                    const std::vector<RichTextData>& richTextData,
                    uint8_t uFade,
                    bool bMeasureOnly,
                    RichTextLineInfoParam* pLineInfoParam,
                    std::vector<GdiPendingText>* pPending,
                    std::vector<std::vector<UiRect>>* pRichTextRects)
{
    if ((render == nullptr) || (pRenderFactory == nullptr) || textRect.IsEmpty()) {
        return;
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(richTextData.size());
    }
    if (pPending != nullptr) {
        pPending->clear();
    }

    int32_t x = textRect.left - scrollOffset.cx;
    int32_t y = textRect.top - scrollOffset.cy;
    uint32_t rowIndex = 0;

    if ((pLineInfoParam != nullptr) && (pLineInfoParam->m_pLineInfoList != nullptr)) {
        pLineInfoParam->m_pLineInfoList->clear();
    }

    for (size_t dataIndex = 0; dataIndex < richTextData.size(); ++dataIndex) {
        const RichTextData& data = richTextData[dataIndex];
        if (data.m_textView.empty() || (data.m_pFontInfo == nullptr)) {
            continue;
        }

        std::shared_ptr<IFont> spFont(pRenderFactory->CreateIFont());
        if ((spFont == nullptr) || !spFont->InitFont(*data.m_pFontInfo)) {
            continue;
        }

        const int32_t lineHeight = std::max<int32_t>(1, static_cast<int32_t>(MeasureLineHeight(render, spFont.get()) * data.m_fRowSpacingMul + data.m_fRowSpacingAdd));
        const bool wordWrap = (data.m_textStyle & TEXT_WORD_WRAP) != 0;

        size_t i = 0;
        while (i < data.m_textView.size()) {
            const wchar_t ch = data.m_textView[i];
            if (ch == L'\r') {
                ++i;
                continue;
            }
            if (ch == L'\n') {
                x = textRect.left - scrollOffset.cx;
                y += lineHeight;
                ++rowIndex;
                ++i;
                continue;
            }

            size_t charCount = GetUtf16CharCountLocal(data.m_textView.data(), i);
            if (i + charCount > data.m_textView.size()) {
                charCount = 1;
            }
            std::wstring glyph(data.m_textView.substr(i, charCount));
            int32_t cw = MeasureTextWidth(render, spFont.get(), glyph);
            if (cw <= 0) {
                cw = 1;
            }

            if (wordWrap && (x > textRect.left - scrollOffset.cx) && ((x + cw) > (textRect.right - scrollOffset.cx))) {
                x = textRect.left - scrollOffset.cx;
                y += lineHeight;
                ++rowIndex;
            }

            UiRect rcGlyph(x, y, x + cw, y + lineHeight);

            if (pRichTextRects != nullptr) {
                (*pRichTextRects)[dataIndex].push_back(rcGlyph);
            }

            if (pPending != nullptr) {
                GdiPendingText pt;
                pt.dataIndex = dataIndex;
                pt.rowIndex = rowIndex;
                pt.text = glyph;
                pt.rect = rcGlyph;
                pt.spFont = spFont;
                pt.textColor = data.m_textColor;
                pt.bgColor = data.m_bgColor;
                pt.textStyle = data.m_textStyle;
                pPending->push_back(std::move(pt));
            }

            if (!bMeasureOnly) {
                UiRect rcPaint = rcGlyph;
                if (!rcPaint.IsEmpty()) {
                    if (data.m_bgColor.GetARGB() != 0) {
                        render->FillRect(rcPaint, data.m_bgColor, uFade);
                    }
                    DrawStringParam drawParam;
                    drawParam.textRect = rcPaint;
                    drawParam.dwTextColor = data.m_textColor;
                    drawParam.uFade = uFade;
                    drawParam.pFont = spFont.get();
                    drawParam.uFormat = TEXT_SINGLELINE | (data.m_textStyle & (TEXT_HCENTER | TEXT_RIGHT | TEXT_VCENTER | TEXT_BOTTOM | TEXT_NOCLIP));
                    render->DrawString(ToDString(glyph), drawParam);
                }
            }

            x += cw;
            i += charCount;

            if (y >= (textRect.bottom - scrollOffset.cy)) {
                break;
            }
        }
        if (y >= (textRect.bottom - scrollOffset.cy)) {
            break;
        }
    }
}

} // namespace

void Render_GDI::MeasureRichText(const UiRect& textRect,
                                 const UiSize& szScrollOffset,
                                 IRenderFactory* pRenderFactory,
                                 const std::vector<RichTextData>& richTextData,
                                 std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText"));
    LayoutRichText(this, textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, nullptr, nullptr, pRichTextRects);
}

void Render_GDI::MeasureRichText2(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText2"));
    LayoutRichText(this, textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, nullptr, pRichTextRects);
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
    std::vector<GdiPendingText> pending;
    LayoutRichText(this, textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, &pending, pRichTextRects);

    DrawRichTextCache* pRaw = reinterpret_cast<DrawRichTextCache*>(new uint8_t[1]);
    spDrawRichTextCache = std::shared_ptr<DrawRichTextCache>(pRaw, [](DrawRichTextCache* p) {
        std::lock_guard<std::mutex> lock(g_gdiRichTextCacheLock);
        g_gdiRichTextCaches.erase(p);
        delete[] reinterpret_cast<uint8_t*>(p);
    });

    GdiRichTextCacheData cacheData;
    cacheData.textRect = textRect;
    cacheData.scrollOffset = szScrollOffset;
    cacheData.richTextData = BuildSnapshots(richTextData);
    cacheData.pending.swap(pending);

    std::lock_guard<std::mutex> lock(g_gdiRichTextCacheLock);
    g_gdiRichTextCaches[pRaw] = std::move(cacheData);
}

void Render_GDI::DrawRichText(const UiRect& textRect,
                              const UiSize& szScrollOffset,
                              IRenderFactory* pRenderFactory,
                              const std::vector<RichTextData>& richTextData,
                              uint8_t uFade,
                              std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::DrawRichText"));
    LayoutRichText(this, textRect, szScrollOffset, pRenderFactory, richTextData, uFade, false, nullptr, nullptr, pRichTextRects);
}

bool Render_GDI::CreateDrawRichTextCache(const UiRect& textRect,
                                         const UiSize& szScrollOffset,
                                         IRenderFactory* pRenderFactory,
                                         const std::vector<RichTextData>& richTextData,
                                         std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    PerformanceStat statPerformance(_T("Render_GDI::CreateDrawRichTextCache"));
    spDrawRichTextCache.reset();
    MeasureRichText3(textRect, szScrollOffset, pRenderFactory, richTextData, nullptr, spDrawRichTextCache, nullptr);
    return spDrawRichTextCache != nullptr;
}

bool Render_GDI::IsValidDrawRichTextCache(const UiRect& textRect,
                                          const std::vector<RichTextData>& richTextData,
                                          const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    if (spDrawRichTextCache == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_gdiRichTextCacheLock);
    auto it = g_gdiRichTextCaches.find(spDrawRichTextCache.get());
    if (it == g_gdiRichTextCaches.end()) {
        return false;
    }
    if ((it->second.textRect.Width() != textRect.Width()) || (it->second.textRect.Height() != textRect.Height())) {
        return false;
    }
    return IsSameSnapshots(it->second.richTextData, richTextData);
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

    if (spUpdateDrawRichTextCache == nullptr) {
        return false;
    }
    spOldDrawRichTextCache = spUpdateDrawRichTextCache;
    richTextDataNew.clear();
    return true;
}

bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    std::lock_guard<std::mutex> lock(g_gdiRichTextCacheLock);
    auto it1 = g_gdiRichTextCaches.find(&first);
    auto it2 = g_gdiRichTextCaches.find(&second);
    if ((it1 == g_gdiRichTextCaches.end()) || (it2 == g_gdiRichTextCaches.end())) {
        return false;
    }
    const GdiRichTextCacheData& a = it1->second;
    const GdiRichTextCacheData& b = it2->second;
    if ((a.textRect.left != b.textRect.left) || (a.textRect.top != b.textRect.top) ||
        (a.textRect.right != b.textRect.right) || (a.textRect.bottom != b.textRect.bottom) ||
        (a.scrollOffset.cx != b.scrollOffset.cx) || (a.scrollOffset.cy != b.scrollOffset.cy) || (a.pending.size() != b.pending.size())) {
        return false;
    }
    if (a.richTextData.size() != b.richTextData.size()) {
        return false;
    }
    for (size_t i = 0; i < a.richTextData.size(); ++i) {
        if ((a.richTextData[i].text != b.richTextData[i].text) ||
            (a.richTextData[i].textColor != b.richTextData[i].textColor) ||
            (a.richTextData[i].bgColor != b.richTextData[i].bgColor) ||
            (a.richTextData[i].textStyle != b.richTextData[i].textStyle)) {
            return false;
        }
    }
    for (size_t i = 0; i < a.pending.size(); ++i) {
        if ((a.pending[i].dataIndex != b.pending[i].dataIndex) ||
            (a.pending[i].rowIndex != b.pending[i].rowIndex) ||
            (a.pending[i].text != b.pending[i].text) ||
            (a.pending[i].rect != b.pending[i].rect) ||
            (a.pending[i].textColor != b.pending[i].textColor) ||
            (a.pending[i].bgColor != b.pending[i].bgColor) ||
            (a.pending[i].textStyle != b.pending[i].textStyle)) {
            return false;
        }
    }
    return true;
}

void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                       const UiRect& textRect,
                                       const UiSize& szNewScrollOffset,
                                       const std::vector<int32_t>& rowXOffset,
                                       uint8_t uFade,
                                       std::vector<std::vector<UiRect>>* pRichTextRects)
{
    if (spDrawRichTextCache == nullptr) {
        return;
    }

    GdiRichTextCacheData cache;
    {
        std::lock_guard<std::mutex> lock(g_gdiRichTextCacheLock);
        auto it = g_gdiRichTextCaches.find(spDrawRichTextCache.get());
        if (it == g_gdiRichTextCaches.end()) {
            return;
        }
        cache = it->second;
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(cache.richTextData.size());
    }

    const int32_t dx = (textRect.left - cache.textRect.left) - (szNewScrollOffset.cx - cache.scrollOffset.cx);
    const int32_t dy = (textRect.top - cache.textRect.top) - (szNewScrollOffset.cy - cache.scrollOffset.cy);

    for (const GdiPendingText& pt : cache.pending) {
        UiRect rc = pt.rect;
        rc.Offset(dx, dy);
        if (pt.rowIndex < rowXOffset.size()) {
            rc.Offset(rowXOffset[pt.rowIndex], 0);
        }

        if (pRichTextRects != nullptr) {
            (*pRichTextRects)[pt.dataIndex].push_back(rc);
        }

        if (pt.bgColor.GetARGB() != 0) {
            FillRect(rc, pt.bgColor, uFade);
        }

        DrawStringParam drawParam;
        drawParam.textRect = rc;
        drawParam.dwTextColor = pt.textColor;
        drawParam.uFade = uFade;
        drawParam.pFont = pt.spFont.get();
        drawParam.uFormat = TEXT_SINGLELINE | (pt.textStyle & (TEXT_HCENTER | TEXT_RIGHT | TEXT_VCENTER | TEXT_BOTTOM | TEXT_NOCLIP));
        DrawString(ToDString(pt.text), drawParam);
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

