// Render_GDI.cpp 的补充部分 - 文本绘制

#ifdef DUILIB_BUILD_FOR_WIN

#include "Render_GDI.h"
#include "Font_GDI.h"
#include "duilib/Utils/PerformanceUtil.h"

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

// 富文本绘制相关函数 - 这些需要更复杂的实现
void Render_GDI::MeasureRichText(const UiRect& textRect,
                                 const UiSize& szScrollOffset,
                                 IRenderFactory* pRenderFactory,
                                 const std::vector<RichTextData>& richTextData,
                                 std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText"));
    // TODO: 实现富文本测量
    // 这需要参考 Skia 的 DrawRichText 实现
}

void Render_GDI::MeasureRichText2(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::MeasureRichText2"));
    // TODO: 实现富文本测量2
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
    // TODO: 实现富文本测量3
}

void Render_GDI::DrawRichText(const UiRect& textRect,
                              const UiSize& szScrollOffset,
                              IRenderFactory* pRenderFactory,
                              const std::vector<RichTextData>& richTextData,
                              uint8_t uFade,
                              std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_GDI::DrawRichText"));
    // TODO: 实现富文本绘制
}

bool Render_GDI::CreateDrawRichTextCache(const UiRect& textRect,
                                         const UiSize& szScrollOffset,
                                         IRenderFactory* pRenderFactory,
                                         const std::vector<RichTextData>& richTextData,
                                         std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    PerformanceStat statPerformance(_T("Render_GDI::CreateDrawRichTextCache"));
    // TODO: 实现富文本缓存创建
    return false;
}

bool Render_GDI::IsValidDrawRichTextCache(const UiRect& textRect,
                                          const std::vector<RichTextData>& richTextData,
                                          const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    // TODO: 实现富文本缓存验证
    return false;
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
    // TODO: 实现富文本缓存更新
    return false;
}

bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    // TODO: 实现富文本缓存比较
    return false;
}

void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                       const UiRect& textRect,
                                       const UiSize& szNewScrollOffset,
                                       const std::vector<int32_t>& rowXOffset,
                                       uint8_t uFade,
                                       std::vector<std::vector<UiRect>>* pRichTextRects)
{
    // TODO: 实现富文本缓存绘制
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

#endif // DUILIB_BUILD_FOR_WIN
