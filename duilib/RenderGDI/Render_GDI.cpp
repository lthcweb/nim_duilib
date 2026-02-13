#include "duilib/RenderGDI/Render_GDI.h"

#include "duilib/Render/BitmapAlpha.h"
#include "duilib/RenderGDI/DrawGDIImage.h"

#include <algorithm>
#include <cstring>

namespace ui {

Render_GDI::Render_GDI(void* platformData, RenderBackendType backendType) :
    m_platformData(platformData),
    m_backendType(backendType)
{
}

Render_GDI::~Render_GDI() = default;

RenderType Render_GDI::GetRenderType() const { return RenderType::kRenderType_GDI; }
RenderBackendType Render_GDI::GetRenderBackendType() const { return RenderBackendType::kRaster_BackendType; }
int32_t Render_GDI::GetWidth() const { return static_cast<int32_t>(m_bitmap.GetWidth()); }
int32_t Render_GDI::GetHeight() const { return static_cast<int32_t>(m_bitmap.GetHeight()); }
bool Render_GDI::Resize(int32_t width, int32_t height) { return m_bitmap.Init(width, height, nullptr); }
UiPoint Render_GDI::OffsetWindowOrg(UiPoint ptOffset) { UiPoint old = m_windowOrg; m_windowOrg.Offset(ptOffset.x, ptOffset.y); return old; }
UiPoint Render_GDI::SetWindowOrg(UiPoint pt) { UiPoint old = m_windowOrg; m_windowOrg = pt; return old; }
UiPoint Render_GDI::GetWindowOrg() const { return m_windowOrg; }

void Render_GDI::SaveClip(int32_t& nState)
{
    m_clipStates.push_back(m_clipStack.empty() ? UiRect() : m_clipStack.back());
    nState = static_cast<int32_t>(m_clipStates.size());
}

void Render_GDI::RestoreClip(int32_t nState)
{
    if ((nState <= 0) || (static_cast<size_t>(nState) > m_clipStates.size())) {
        return;
    }
    UiRect rcClip = m_clipStates[nState - 1];
    m_clipStack.clear();
    if (!rcClip.IsEmpty()) {
        m_clipStack.push_back(rcClip);
    }
}

void Render_GDI::SetClip(const UiRect& rc, bool bIntersect)
{
    if (m_clipStack.empty() || !bIntersect) {
        m_clipStack = { rc };
        return;
    }
    UiRect clipRect = m_clipStack.back();
    clipRect.Intersect(rc);
    m_clipStack.back() = clipRect;
}

void Render_GDI::SetRoundClip(const UiRect& rcItem, float rx, float ry, bool bIntersect)
{
    UNUSED_VARIABLE(rx);
    UNUSED_VARIABLE(ry);
    SetClip(rcItem, bIntersect);
}

void Render_GDI::ClearClip() { m_clipStack.clear(); }

bool Render_GDI::BlitRect(const UiRect& dstRect, const Render_GDI* pSrcRender, const UiPoint& srcPt)
{
    if ((pSrcRender == nullptr) || pSrcRender->IsEmpty() || IsEmpty()) {
        return false;
    }
    UiRect clipDst = dstRect;
    clipDst.Intersect(UiRect(0, 0, GetWidth(), GetHeight()));
    if (clipDst.IsEmpty()) {
        return false;
    }
    for (int y = 0; y < clipDst.Height(); ++y) {
        for (int x = 0; x < clipDst.Width(); ++x) {
            const uint32_t* pSrcPixel = pSrcRender->Pixel(srcPt.x + x, srcPt.y + y);
            uint32_t* pDstPixel = Pixel(clipDst.left + x, clipDst.top + y);
            if ((pSrcPixel != nullptr) && (pDstPixel != nullptr)) {
                *pDstPixel = *pSrcPixel;
            }
        }
    }
    return true;
}

bool Render_GDI::BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, RopMode rop)
{
    UNUSED_VARIABLE(rop);
    return BlitRect(UiRect(x, y, x + cx, y + cy), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc, ySrc));
}

bool Render_GDI::StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, RopMode rop)
{
    UNUSED_VARIABLE(widthSrc);
    UNUSED_VARIABLE(heightSrc);
    UNUSED_VARIABLE(rop);
    return BlitRect(UiRect(xDest, yDest, xDest + widthDest, yDest + heightDest), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc, ySrc));
}

bool Render_GDI::AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, uint8_t alpha)
{
    UNUSED_VARIABLE(widthSrc);
    UNUSED_VARIABLE(heightSrc);
    UNUSED_VARIABLE(alpha);
    return BlitRect(UiRect(xDest, yDest, xDest + widthDest, yDest + heightDest), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc, ySrc));
}

void Render_GDI::DrawImage(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest, const UiRect&, const UiRect& rcSource, const UiRect&, uint8_t uFade, const TiledDrawParam*, bool)
{
    DrawImageRect(rcDest, pBitmap, rcDest, rcSource, uFade, nullptr);
}

void Render_GDI::DrawImage(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest, const UiRect& rcSource, uint8_t uFade, const TiledDrawParam*, bool)
{
    DrawImageRect(rcDest, pBitmap, rcDest, rcSource, uFade, nullptr);
}

void Render_GDI::DrawImageRect(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest, const UiRect& rcSource, uint8_t uFade, IMatrix* pMatrix)
{
    UNUSED_VARIABLE(pMatrix);
    auto* pBitmapGDI = dynamic_cast<Bitmap_GDI*>(pBitmap);
    if (pBitmapGDI == nullptr) {
        return;
    }
    DrawGDIImage::DrawImageRect(m_bitmap, rcDest, *pBitmapGDI, rcSource, uFade);
}

void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, UiColor, int32_t) {}
void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, UiColor, float) {}
void Render_GDI::DrawLine(const UiPointF&, const UiPointF&, UiColor, float) {}
void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, IPen*) {}
void Render_GDI::DrawLine(const UiPointF&, const UiPointF&, IPen*) {}
void Render_GDI::DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect) { DrawRect(rc, penColor, static_cast<float>(nWidth), bLineInRect); }
void Render_GDI::DrawRect(const UiRectF& rc, UiColor penColor, int32_t nWidth, bool bLineInRect) { DrawRect(rc, penColor, static_cast<float>(nWidth), bLineInRect); }

void Render_GDI::DrawRect(const UiRect& rc, UiColor penColor, float fWidth, bool)
{
    FillRect(UiRect(rc.left, rc.top, rc.right, rc.top + static_cast<int>(fWidth)), penColor);
    FillRect(UiRect(rc.left, rc.bottom - static_cast<int>(fWidth), rc.right, rc.bottom), penColor);
    FillRect(UiRect(rc.left, rc.top, rc.left + static_cast<int>(fWidth), rc.bottom), penColor);
    FillRect(UiRect(rc.right - static_cast<int>(fWidth), rc.top, rc.right, rc.bottom), penColor);
}

void Render_GDI::DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    DrawRect(UiRect(static_cast<int>(rc.left), static_cast<int>(rc.top), static_cast<int>(rc.right), static_cast<int>(rc.bottom)), penColor, fWidth, bLineInRect);
}

void Render_GDI::DrawRect(const UiRect& rc, IPen* pen, bool bLineInRect) { if (pen != nullptr) DrawRect(rc, pen->GetColor(), pen->GetWidth(), bLineInRect); }
void Render_GDI::DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect) { if (pen != nullptr) DrawRect(rc, pen->GetColor(), pen->GetWidth(), bLineInRect); }

void Render_GDI::FillRect(const UiRect& rc, UiColor color, uint8_t uFade)
{
    if (IsEmpty() || rc.IsEmpty()) {
        return;
    }
    UiRect clip = rc;
    clip.Intersect(UiRect(0, 0, GetWidth(), GetHeight()));
    const uint32_t pixel = UiColor::MakeARGB(static_cast<uint8_t>(color.GetA() * uFade / 255), color.GetR(), color.GetG(), color.GetB());
    for (int y = clip.top; y < clip.bottom; ++y) {
        for (int x = clip.left; x < clip.right; ++x) {
            uint32_t* pDst = Pixel(x, y);
            if (pDst != nullptr) {
                *pDst = pixel;
            }
        }
    }
}

void Render_GDI::FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade) { FillRect(UiRect(static_cast<int>(rc.left), static_cast<int>(rc.top), static_cast<int>(rc.right), static_cast<int>(rc.bottom)), dwColor, uFade); }
void Render_GDI::FillRect(const UiRect& rc, UiColor dwColor, UiColor, int8_t, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::FillRect(const UiRectF& rc, UiColor dwColor, UiColor, int8_t, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::DrawRoundRect(const UiRect& rc, float, float, UiColor penColor, int32_t nWidth) { DrawRect(rc, penColor, nWidth); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float, float, UiColor penColor, int32_t nWidth) { DrawRect(rc, penColor, nWidth); }
void Render_GDI::DrawRoundRect(const UiRect& rc, float, float, UiColor penColor, float fWidth) { DrawRect(rc, penColor, fWidth); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float, float, UiColor penColor, float fWidth) { DrawRect(rc, penColor, fWidth); }
void Render_GDI::DrawRoundRect(const UiRect& rc, float, float, IPen* pen) { DrawRect(rc, pen); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float, float, IPen* pen) { DrawRect(rc, pen); }
void Render_GDI::FillRoundRect(const UiRect& rc, float, float, UiColor dwColor, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::FillRoundRect(const UiRectF& rc, float, float, UiColor dwColor, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::FillRoundRect(const UiRect& rc, float, float, UiColor dwColor, UiColor, int8_t, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::FillRoundRect(const UiRectF& rc, float, float, UiColor dwColor, UiColor, int8_t, uint8_t uFade) { FillRect(rc, dwColor, uFade); }
void Render_GDI::DrawArc(const UiRect&, float, float, bool, const IPen*, UiColor*, const UiRect*) {}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, UiColor, int32_t) {}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, UiColor, float) {}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, IPen*) {}
void Render_GDI::FillCircle(const UiPoint&, int32_t, UiColor, uint8_t) {}
void Render_GDI::DrawPath(const IPath*, const IPen*) {}
void Render_GDI::FillPath(const IPath*, const IBrush*) {}
void Render_GDI::FillPath(const IPath*, const UiRect&, UiColor, UiColor, int8_t) {}
UiRect Render_GDI::MeasureString(const DString& strText, const MeasureStringParam& measureParam) { int h = (measureParam.pFont != nullptr) ? measureParam.pFont->FontSize() : 14; int w = static_cast<int>(strText.size()) * h / 2; return UiRect(0, 0, w, h); }
void Render_GDI::DrawString(const DString&, const DrawStringParam&) {}
void Render_GDI::MeasureRichText(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, std::vector<std::vector<UiRect>>*) {}
void Render_GDI::MeasureRichText2(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, RichTextLineInfoParam*, std::vector<std::vector<UiRect>>*) {}
void Render_GDI::MeasureRichText3(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, RichTextLineInfoParam*, std::shared_ptr<DrawRichTextCache>&, std::vector<std::vector<UiRect>>*) {}
void Render_GDI::DrawRichText(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, uint8_t, std::vector<std::vector<UiRect>>*) {}
bool Render_GDI::CreateDrawRichTextCache(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, std::shared_ptr<DrawRichTextCache>&) { return false; }
bool Render_GDI::IsValidDrawRichTextCache(const UiRect&, const std::vector<RichTextData>&, const std::shared_ptr<DrawRichTextCache>&) { return false; }
bool Render_GDI::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>&, const std::shared_ptr<DrawRichTextCache>&, std::vector<RichTextData>&, size_t, const std::vector<size_t>&, size_t, const std::vector<size_t>&, size_t, const std::vector<int32_t>&) { return false; }
bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache&, const DrawRichTextCache&) const { return false; }
void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>&, const UiRect&, const UiSize&, const std::vector<int32_t>&, uint8_t, std::vector<std::vector<UiRect>>*) {}
void Render_GDI::DrawBoxShadow(const UiRect&, const UiSize&, const UiPoint&, int32_t, int32_t, UiColor) {}

IBitmap* Render_GDI::MakeImageSnapshot() { return m_bitmap.Clone(); }
void Render_GDI::ClearAlpha(const UiRect& rcDirty, uint8_t alpha) { BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.ClearAlpha(rcDirty, alpha); }
void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) { BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.RestoreAlpha(rcDirty, rcShadowPadding, alpha); }
void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) { BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.RestoreAlpha(rcDirty, rcShadowPadding); }

#ifdef DUILIB_BUILD_FOR_WIN
HDC Render_GDI::GetRenderDC(HWND hWnd) { UNUSED_VARIABLE(hWnd); return nullptr; }
void Render_GDI::ReleaseRenderDC(HDC hdc) { UNUSED_VARIABLE(hdc); }
#endif

void Render_GDI::Clear(const UiColor& uiColor) { FillRect(UiRect(0, 0, GetWidth(), GetHeight()), uiColor, uiColor.GetA()); }
void Render_GDI::ClearRect(const UiRect& rcDirty, const UiColor& uiColor) { FillRect(rcDirty, uiColor, uiColor.GetA()); }

std::unique_ptr<IRender> Render_GDI::Clone()
{
    auto pRender = std::make_unique<Render_GDI>(m_platformData, m_backendType);
    pRender->Resize(GetWidth(), GetHeight());
    pRender->WritePixels(m_bitmap.GetBits(), static_cast<size_t>(GetWidth()) * GetHeight() * sizeof(uint32_t), UiRect(0, 0, GetWidth(), GetHeight()));
    pRender->SetRenderDpi(m_spRenderDpi);
    return pRender;
}

bool Render_GDI::ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen)
{
    if ((dstPixels == nullptr) || rc.IsEmpty()) {
        return false;
    }
    size_t need = static_cast<size_t>(rc.Width()) * rc.Height() * sizeof(uint32_t);
    if (dstPixelsLen < need) {
        return false;
    }
    for (int y = 0; y < rc.Height(); ++y) {
        std::memcpy(static_cast<uint8_t*>(dstPixels) + static_cast<size_t>(y) * rc.Width() * sizeof(uint32_t), Pixel(rc.left, rc.top + y), static_cast<size_t>(rc.Width()) * sizeof(uint32_t));
    }
    return true;
}

bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc) { return WritePixels(srcPixels, srcPixelsLen, rc, rc); }

bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint)
{
    UNUSED_VARIABLE(rcPaint);
    if ((srcPixels == nullptr) || rc.IsEmpty()) {
        return false;
    }
    size_t need = static_cast<size_t>(rc.Width()) * rc.Height() * sizeof(uint32_t);
    if (srcPixelsLen < need) {
        return false;
    }
    for (int y = 0; y < rc.Height(); ++y) {
        std::memcpy(Pixel(rc.left, rc.top + y), static_cast<uint8_t*>(srcPixels) + static_cast<size_t>(y) * rc.Width() * sizeof(uint32_t), static_cast<size_t>(rc.Width()) * sizeof(uint32_t));
    }
    return true;
}

RenderClipType Render_GDI::GetClipInfo(std::vector<UiRect>& clipRects)
{
    clipRects.clear();
    if (m_clipStack.empty()) {
        return RenderClipType::kEmpty;
    }
    clipRects.push_back(m_clipStack.back());
    return RenderClipType::kRect;
}

bool Render_GDI::IsClipEmpty() const { return !m_clipStack.empty() && m_clipStack.back().IsEmpty(); }
bool Render_GDI::IsEmpty() const { return (GetWidth() <= 0) || (GetHeight() <= 0) || (m_bitmap.GetBits() == nullptr); }
void Render_GDI::SetRenderDpi(const IRenderDpiPtr& spRenderDpi) { m_spRenderDpi = spRenderDpi; }
bool Render_GDI::PaintAndSwapBuffers(IRenderPaint* pRenderPaint) { return (pRenderPaint != nullptr) ? pRenderPaint->DoPaint(UiRect(0, 0, GetWidth(), GetHeight())) : false; }
bool Render_GDI::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw) { UNUSED_VARIABLE(rcWnd); UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); UNUSED_VARIABLE(bRedraw); return false; }
bool Render_GDI::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw) { UNUSED_VARIABLE(rcWnd); UNUSED_VARIABLE(bRedraw); return false; }
void Render_GDI::ClearWindowRgn(bool bRedraw) { UNUSED_VARIABLE(bRedraw); }

uint32_t* Render_GDI::Pixel(int32_t x, int32_t y)
{
    if ((x < 0) || (y < 0) || (x >= GetWidth()) || (y >= GetHeight())) {
        return nullptr;
    }
    return m_bitmap.GetBits() + static_cast<size_t>(y) * GetWidth() + x;
}

const uint32_t* Render_GDI::Pixel(int32_t x, int32_t y) const
{
    if ((x < 0) || (y < 0) || (x >= GetWidth()) || (y >= GetHeight())) {
        return nullptr;
    }
    return m_bitmap.GetBits() + static_cast<size_t>(y) * GetWidth() + x;
}

} // namespace ui
