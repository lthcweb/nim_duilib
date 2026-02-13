#include "Render_GDI.h"
#include "duilib/Render/BitmapAlpha.h"
#include <algorithm>
#include <cstring>

namespace ui {

// 当前未实现能力说明：
// 1) 富文本缓存、复杂路径、阴影模糊等在旧版 UIRender.cpp 中依赖较多 GDI+/自定义流程，
//    当前先留空以保证接口齐全和条件编译可落地。
// 2) 后续可按模块逐步把 UIRender.cpp 对应能力迁移到此类中。

Render_GDI::Render_GDI(void* platformData, RenderBackendType backendType):
    m_platformData(platformData),
    m_backendType(backendType)
{
}
Render_GDI::~Render_GDI() = default;

RenderType Render_GDI::GetRenderType() const { return RenderType::kRenderType_GDI; }
RenderBackendType Render_GDI::GetRenderBackendType() const { return RenderBackendType::kRaster_BackendType; }
int32_t Render_GDI::GetWidth() const { return (int32_t)m_bitmap.GetWidth(); }
int32_t Render_GDI::GetHeight() const { return (int32_t)m_bitmap.GetHeight(); }
bool Render_GDI::Resize(int32_t width, int32_t height) { return m_bitmap.Init(width, height, nullptr); }
UiPoint Render_GDI::OffsetWindowOrg(UiPoint ptOffset){ UiPoint old=m_windowOrg; m_windowOrg.Offset(ptOffset.x, ptOffset.y); return old; }
UiPoint Render_GDI::SetWindowOrg(UiPoint pt){ UiPoint old=m_windowOrg; m_windowOrg=pt; return old; }
UiPoint Render_GDI::GetWindowOrg() const { return m_windowOrg; }
void Render_GDI::SaveClip(int32_t& nState){ m_clipStates.push_back(m_clipStack.empty()?UiRect():m_clipStack.back()); nState=(int32_t)m_clipStates.size(); }
void Render_GDI::RestoreClip(int32_t nState){ if (nState>0 && (size_t)nState<=m_clipStates.size()){ UiRect rc=m_clipStates[nState-1]; m_clipStack.clear(); if(!rc.IsEmpty()) m_clipStack.push_back(rc);} }
void Render_GDI::SetClip(const UiRect& rc, bool bIntersect){ if(m_clipStack.empty()||!bIntersect) m_clipStack={rc}; else { UiRect t=m_clipStack.back(); t.Intersect(rc); m_clipStack.back()=t; } }
void Render_GDI::SetRoundClip(const UiRect& rcItem, float rx, float ry, bool bIntersect){ UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); SetClip(rcItem,bIntersect); }
void Render_GDI::ClearClip(){ m_clipStack.clear(); }

bool Render_GDI::BlitRect(const UiRect& dst, const Render_GDI* src, const UiPoint& srcPt){
    if((src==nullptr)||src->IsEmpty()||IsEmpty()) return false;
    UiRect d=dst; d.Intersect(UiRect(0,0,GetWidth(),GetHeight()));
    if(d.IsEmpty()) return false;
    for(int y=0;y<d.Height();++y){
        for(int x=0;x<d.Width();++x){
            const uint32_t* sp = src->Pixel(srcPt.x+x, srcPt.y+y);
            uint32_t* dp = Pixel(d.left+x, d.top+y);
            if(sp&&dp) *dp=*sp;
        }
    }
    return true;
}

bool Render_GDI::BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, RopMode rop){ UNUSED_VARIABLE(rop); return BlitRect(UiRect(x,y,x+cx,y+cy), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc,ySrc)); }
bool Render_GDI::StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, RopMode rop){ UNUSED_VARIABLE(widthSrc); UNUSED_VARIABLE(heightSrc); UNUSED_VARIABLE(rop); return BlitRect(UiRect(xDest,yDest,xDest+widthDest,yDest+heightDest), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc,ySrc)); }
bool Render_GDI::AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, uint8_t alpha){ UNUSED_VARIABLE(widthSrc); UNUSED_VARIABLE(heightSrc); UNUSED_VARIABLE(alpha); return BlitRect(UiRect(xDest,yDest,xDest+widthDest,yDest+heightDest), dynamic_cast<Render_GDI*>(pSrcRender), UiPoint(xSrc,ySrc)); }

void Render_GDI::DrawImage(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest, const UiRect&, const UiRect& rcSource, const UiRect&, uint8_t, const TiledDrawParam*, bool){ DrawImageRect(rcDest, pBitmap, rcDest, rcSource); }
void Render_GDI::DrawImage(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest,  const UiRect& rcSource, uint8_t, const TiledDrawParam*, bool){ DrawImageRect(rcDest, pBitmap, rcDest, rcSource); }
void Render_GDI::DrawImageRect(const UiRect&, IBitmap* pBitmap, const UiRect& rcDest, const UiRect& rcSource, uint8_t, IMatrix*){
    Bitmap_GDI* bmp = dynamic_cast<Bitmap_GDI*>(pBitmap);
    if ((bmp == nullptr) || bmp->GetBits() == nullptr || IsEmpty()) return;
    for (int y=0; y<rcDest.Height(); ++y){
        for (int x=0; x<rcDest.Width(); ++x){
            int sx = rcSource.left + x * rcSource.Width() / std::max(rcDest.Width(),1);
            int sy = rcSource.top + y * rcSource.Height() / std::max(rcDest.Height(),1);
            const uint32_t* sp = bmp->GetBits() + sy * bmp->GetWidth() + sx;
            uint32_t* dp = Pixel(rcDest.left + x, rcDest.top + y);
            if (sp && dp) *dp = *sp;
        }
    }
}

void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, UiColor, int32_t){}
void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, UiColor, float){}
void Render_GDI::DrawLine(const UiPointF&, const UiPointF&, UiColor, float){}
void Render_GDI::DrawLine(const UiPoint&, const UiPoint&, IPen*){}
void Render_GDI::DrawLine(const UiPointF&, const UiPointF&, IPen*){}
void Render_GDI::DrawRect(const UiRect& rc, UiColor c, int32_t w, bool b){ DrawRect(rc,c,(float)w,b);} 
void Render_GDI::DrawRect(const UiRectF& rc, UiColor c, int32_t w, bool b){ DrawRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),c,(float)w,b);} 
void Render_GDI::DrawRect(const UiRect& rc, UiColor c, float, bool){ FillRect(UiRect(rc.left,rc.top,rc.right,rc.top+1),c); FillRect(UiRect(rc.left,rc.bottom-1,rc.right,rc.bottom),c); FillRect(UiRect(rc.left,rc.top,rc.left+1,rc.bottom),c); FillRect(UiRect(rc.right-1,rc.top,rc.right,rc.bottom),c);} 
void Render_GDI::DrawRect(const UiRectF& rc, UiColor c, float w, bool b){ DrawRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),c,w,b);} 
void Render_GDI::DrawRect(const UiRect& rc, IPen* p, bool b){ DrawRect(rc, p?p->GetColor():UiColor(), p?p->GetWidth():1.0f, b);} 
void Render_GDI::DrawRect(const UiRectF& rc, IPen* p, bool b){ DrawRect(rc, p?p->GetColor():UiColor(), p?p->GetWidth():1.0f, b);} 

void Render_GDI::FillRect(const UiRect& rc, UiColor color, uint8_t uFade){ UiRect r=rc; r.Intersect(UiRect(0,0,GetWidth(),GetHeight())); uint32_t v=UiColor::MakeARGB(uFade,color.GetR(),color.GetG(),color.GetB()); for(int y=r.top;y<r.bottom;++y){ for(int x=r.left;x<r.right;++x){ uint32_t* p=Pixel(x,y); if(p) *p=v; } } }
void Render_GDI::FillRect(const UiRectF& rc, UiColor c, uint8_t u){ FillRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),c,u); }
void Render_GDI::FillRect(const UiRect& rc, UiColor c1, UiColor, int8_t, uint8_t u){ FillRect(rc,c1,u); }
void Render_GDI::FillRect(const UiRectF& rc, UiColor c1, UiColor c2, int8_t d, uint8_t u){ UNUSED_VARIABLE(c2); UNUSED_VARIABLE(d); FillRect(rc,c1,u); }

void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor c, int32_t w){ UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); DrawRect(rc,c,w,false); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor c, int32_t w){ DrawRoundRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),rx,ry,c,w); }
void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor c, float w){ UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); DrawRect(rc,c,w,false); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor c, float w){ DrawRoundRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),rx,ry,c,w); }
void Render_GDI::DrawRoundRect(const UiRect& rc, float rx, float ry, IPen* p){ DrawRoundRect(rc,rx,ry,p?p->GetColor():UiColor(),p?p->GetWidth():1.0f); }
void Render_GDI::DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* p){ DrawRoundRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),rx,ry,p); }
void Render_GDI::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor c, uint8_t u){ UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); FillRect(rc,c,u); }
void Render_GDI::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor c, uint8_t u){ FillRoundRect(UiRect((int)rc.left,(int)rc.top,(int)rc.right,(int)rc.bottom),rx,ry,c,u); }
void Render_GDI::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor c1, UiColor, int8_t, uint8_t u){ FillRoundRect(rc,rx,ry,c1,u); }
void Render_GDI::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor c1, UiColor c2, int8_t d, uint8_t u){ UNUSED_VARIABLE(c2); UNUSED_VARIABLE(d); FillRoundRect(rc,rx,ry,c1,u); }

void Render_GDI::DrawArc(const UiRect&, float, float, bool, const IPen*, UiColor*, const UiRect*){}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, UiColor, int32_t){}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, UiColor, float){}
void Render_GDI::DrawCircle(const UiPoint&, int32_t, IPen*){}
void Render_GDI::FillCircle(const UiPoint&, int32_t, UiColor, uint8_t){}
void Render_GDI::DrawPath(const IPath*, const IPen*){}
void Render_GDI::FillPath(const IPath*, const IBrush*){}
void Render_GDI::FillPath(const IPath*, const UiRect&, UiColor, UiColor, int8_t){}
UiRect Render_GDI::MeasureString(const DString& strText, const MeasureStringParam& measureParam){ int h=(measureParam.pFont!=nullptr)?measureParam.pFont->FontSize():14; int w=(int)strText.size()*h/2; return UiRect(0,0,w,h); }
void Render_GDI::DrawString(const DString&, const DrawStringParam&){}
void Render_GDI::MeasureRichText(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, std::vector<std::vector<UiRect>>*){}
void Render_GDI::MeasureRichText2(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, RichTextLineInfoParam*, std::vector<std::vector<UiRect>>*){}
void Render_GDI::MeasureRichText3(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, RichTextLineInfoParam*, std::shared_ptr<DrawRichTextCache>&, std::vector<std::vector<UiRect>>*){}
void Render_GDI::DrawRichText(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, uint8_t, std::vector<std::vector<UiRect>>*){}
bool Render_GDI::CreateDrawRichTextCache(const UiRect&, const UiSize&, IRenderFactory*, const std::vector<RichTextData>&, std::shared_ptr<DrawRichTextCache>&){ return false; }
bool Render_GDI::IsValidDrawRichTextCache(const UiRect&, const std::vector<RichTextData>&, const std::shared_ptr<DrawRichTextCache>&){ return false; }
bool Render_GDI::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>&, const std::shared_ptr<DrawRichTextCache>&, std::vector<RichTextData>&, size_t, const std::vector<size_t>&, size_t, const std::vector<size_t>&, size_t, const std::vector<int32_t>&){ return false; }
bool Render_GDI::IsDrawRichTextCacheEqual(const DrawRichTextCache&, const DrawRichTextCache&) const { return false; }
void Render_GDI::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>&, const UiRect&, const UiSize&, const std::vector<int32_t>&, uint8_t, std::vector<std::vector<UiRect>>*){}
void Render_GDI::DrawBoxShadow(const UiRect&, const UiSize&, const UiPoint&, int32_t, int32_t, UiColor){}

IBitmap* Render_GDI::MakeImageSnapshot(){ return m_bitmap.Clone(); }
void Render_GDI::ClearAlpha(const UiRect& rcDirty, uint8_t alpha){ BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.ClearAlpha(rcDirty, alpha); }
void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha){ BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.RestoreAlpha(rcDirty, rcShadowPadding, alpha); }
void Render_GDI::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding){ BitmapAlpha alphaFix((uint8_t*)m_bitmap.GetBits(), GetWidth(), GetHeight(), 4); alphaFix.RestoreAlpha(rcDirty, rcShadowPadding); }

#ifdef DUILIB_BUILD_FOR_WIN
HDC Render_GDI::GetRenderDC(HWND hWnd){ UNUSED_VARIABLE(hWnd); return nullptr; }
void Render_GDI::ReleaseRenderDC(HDC hdc){ UNUSED_VARIABLE(hdc); }
#endif

void Render_GDI::Clear(const UiColor& uiColor){ FillRect(UiRect(0,0,GetWidth(),GetHeight()), uiColor, uiColor.GetA()); }
void Render_GDI::ClearRect(const UiRect& rcDirty, const UiColor& uiColor){ FillRect(rcDirty, uiColor, uiColor.GetA()); }
std::unique_ptr<IRender> Render_GDI::Clone(){ auto p=std::make_unique<Render_GDI>(m_platformData,m_backendType); p->Resize(GetWidth(),GetHeight()); p->WritePixels(m_bitmap.GetBits(), (size_t)GetWidth()*GetHeight()*sizeof(uint32_t), UiRect(0,0,GetWidth(),GetHeight())); p->SetRenderDpi(m_spRenderDpi); return p; }

bool Render_GDI::ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen){ if(!dstPixels) return false; size_t need=(size_t)rc.Width()*rc.Height()*sizeof(uint32_t); if(dstPixelsLen<need) return false; for(int y=0;y<rc.Height();++y){ memcpy((uint8_t*)dstPixels + (size_t)y*rc.Width()*sizeof(uint32_t), Pixel(rc.left, rc.top+y), (size_t)rc.Width()*sizeof(uint32_t)); } return true; }
bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc){ return WritePixels(srcPixels,srcPixelsLen,rc,rc); }
bool Render_GDI::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint){ UNUSED_VARIABLE(rcPaint); if(!srcPixels) return false; size_t need=(size_t)rc.Width()*rc.Height()*sizeof(uint32_t); if(srcPixelsLen<need) return false; for(int y=0;y<rc.Height();++y){ memcpy(Pixel(rc.left, rc.top+y), (uint8_t*)srcPixels + (size_t)y*rc.Width()*sizeof(uint32_t), (size_t)rc.Width()*sizeof(uint32_t)); } return true; }
RenderClipType Render_GDI::GetClipInfo(std::vector<UiRect>& clipRects){ clipRects.clear(); if(m_clipStack.empty()) return RenderClipType::kEmpty; clipRects.push_back(m_clipStack.back()); return RenderClipType::kRect; }
bool Render_GDI::IsClipEmpty() const { return !m_clipStack.empty() && m_clipStack.back().IsEmpty(); }
bool Render_GDI::IsEmpty() const { return (GetWidth() <= 0) || (GetHeight() <= 0) || (m_bitmap.GetBits() == nullptr); }
void Render_GDI::SetRenderDpi(const IRenderDpiPtr& spRenderDpi){ m_spRenderDpi = spRenderDpi; }
bool Render_GDI::PaintAndSwapBuffers(IRenderPaint* pRenderPaint){ return (pRenderPaint != nullptr) ? pRenderPaint->DoPaint(UiRect(0,0,GetWidth(),GetHeight())) : false; }
bool Render_GDI::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw){ UNUSED_VARIABLE(rcWnd); UNUSED_VARIABLE(rx); UNUSED_VARIABLE(ry); UNUSED_VARIABLE(bRedraw); return false; }
bool Render_GDI::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw){ UNUSED_VARIABLE(rcWnd); UNUSED_VARIABLE(bRedraw); return false; }
void Render_GDI::ClearWindowRgn(bool bRedraw){ UNUSED_VARIABLE(bRedraw); }

inline uint32_t* Render_GDI::Pixel(int32_t x, int32_t y){ if(x<0||y<0||x>=GetWidth()||y>=GetHeight()) return nullptr; return m_bitmap.GetBits()+ (size_t)y*GetWidth()+x; }
inline const uint32_t* Render_GDI::Pixel(int32_t x, int32_t y) const { if(x<0||y<0||x>=GetWidth()||y>=GetHeight()) return nullptr; return m_bitmap.GetBits()+ (size_t)y*GetWidth()+x; }

} // namespace ui
