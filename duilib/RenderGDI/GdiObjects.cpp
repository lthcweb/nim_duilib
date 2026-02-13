#include "GdiObjects.h"
#include <algorithm>
#include <cstring>

namespace ui {

bool Font_GDI::InitFont(const UiFont& fontInfo){ m_fontInfo = fontInfo; return true; }
DString Font_GDI::FontName() const { return m_fontInfo.m_fontName; }
int Font_GDI::FontSize() const { return m_fontInfo.m_fontSize; }
bool Font_GDI::IsBold() const { return m_fontInfo.m_bBold; }
bool Font_GDI::IsUnderline() const { return m_fontInfo.m_bUnderline; }
bool Font_GDI::IsItalic() const { return m_fontInfo.m_bItalic; }
bool Font_GDI::IsStrikeOut() const { return m_fontInfo.m_bStrikeOut; }

uint32_t FontMgr_GDI::GetFontCount() const { return static_cast<uint32_t>(m_fontNames.size()); }
bool FontMgr_GDI::GetFontName(uint32_t i, DString& n) const { if (i>=m_fontNames.size()) return false; n=m_fontNames[i]; return true; }
bool FontMgr_GDI::HasFontName(const DString& n) const { return std::find(m_fontNames.begin(), m_fontNames.end(), n) != m_fontNames.end(); }
void FontMgr_GDI::SetDefaultFontName(const DString& n) { m_defaultFontName = n; if (!n.empty() && !HasFontName(n)) m_fontNames.push_back(n); }
bool FontMgr_GDI::LoadFontFile(const DString& p) { if (!p.empty() && !HasFontName(p)) m_fontNames.push_back(p); return !p.empty(); }
bool FontMgr_GDI::LoadFontFileData(const void* d, size_t l) { UNUSED_VARIABLE(d); return l > 0; }
void FontMgr_GDI::ClearFontFiles() { m_fontNames.clear(); if (!m_defaultFontName.empty()) m_fontNames.push_back(m_defaultFontName); }
void FontMgr_GDI::ClearFontCache() {}

bool Bitmap_GDI::Init(uint32_t w, uint32_t h, const void* bits, float s, BitmapAlphaType a){ UNUSED_VARIABLE(s); UNUSED_VARIABLE(a); m_width=w; m_height=h; m_bits.resize((size_t)w*h); if(bits) memcpy(m_bits.data(), bits, m_bits.size()*sizeof(uint32_t)); else memset(m_bits.data(),0,m_bits.size()*sizeof(uint32_t)); return (w>0)&&(h>0); }
uint32_t Bitmap_GDI::GetWidth() const { return m_width; }
uint32_t Bitmap_GDI::GetHeight() const { return m_height; }
UiSize Bitmap_GDI::GetSize() const { return UiSize((int32_t)m_width,(int32_t)m_height); }
void* Bitmap_GDI::LockPixelBits() { return m_bits.empty()?nullptr:m_bits.data(); }
void Bitmap_GDI::UnLockPixelBits() {}
IBitmap* Bitmap_GDI::Clone() { Bitmap_GDI* p=new Bitmap_GDI(); p->m_width=m_width; p->m_height=m_height; p->m_bits=m_bits; return p; }
uint32_t* Bitmap_GDI::GetBits(){ return m_bits.data(); }
const uint32_t* Bitmap_GDI::GetBits() const { return m_bits.data(); }

Pen_GDI::Pen_GDI(UiColor c,float w):m_color(c),m_width(w){}
void Pen_GDI::SetWidth(float w){m_width=w;} float Pen_GDI::GetWidth() const{return m_width;}
void Pen_GDI::SetColor(UiColor c){m_color=c;} UiColor Pen_GDI::GetColor() const{return m_color;}
void Pen_GDI::SetStartCap(LineCap c){m_startCap=c;} IPen::LineCap Pen_GDI::GetStartCap() const{return m_startCap;}
void Pen_GDI::SetEndCap(LineCap c){m_endCap=c;} IPen::LineCap Pen_GDI::GetEndCap() const{return m_endCap;}
void Pen_GDI::SetDashCap(LineCap c){m_dashCap=c;} IPen::LineCap Pen_GDI::GetDashCap() const{return m_dashCap;}
void Pen_GDI::SetLineJoin(LineJoin j){m_lineJoin=j;} IPen::LineJoin Pen_GDI::GetLineJoin() const{return m_lineJoin;}
void Pen_GDI::SetDashStyle(DashStyle s){m_dashStyle=s;} IPen::DashStyle Pen_GDI::GetDashStyle() const{return m_dashStyle;}
IPen* Pen_GDI::Clone() const { Pen_GDI* p=new Pen_GDI(m_color,m_width); p->m_startCap=m_startCap;p->m_endCap=m_endCap;p->m_dashCap=m_dashCap;p->m_lineJoin=m_lineJoin;p->m_dashStyle=m_dashStyle;return p; }

Brush_GDI::Brush_GDI(UiColor c):m_color(c){} IBrush* Brush_GDI::Clone(){ return new Brush_GDI(m_color);} UiColor Brush_GDI::GetColor() const{return m_color;}

void Matrix_GDI::Translate(int x,int y){UNUSED_VARIABLE(x);UNUSED_VARIABLE(y);} void Matrix_GDI::Scale(float x,float y){UNUSED_VARIABLE(x);UNUSED_VARIABLE(y);} void Matrix_GDI::Rotate(float a){UNUSED_VARIABLE(a);} void Matrix_GDI::RotateAt(float a,const UiPoint& c){UNUSED_VARIABLE(a);UNUSED_VARIABLE(c);}

void Path_GDI::SetFillType(FillType m){m_fillType=m;} IPath::FillType Path_GDI::GetFillType(){return m_fillType;}
void Path_GDI::AddLine(int x1,int y1,int x2,int y2){m_bounds.Union(UiRect(x1,y1,x2,y2));}
void Path_GDI::AddLines(const UiPoint* p,int c){ if(p&&c>0){ for(int i=1;i<c;i++) AddLine(p[i-1].x,p[i-1].y,p[i].x,p[i].y);} }
void Path_GDI::AddBezier(int,int,int,int,int,int,int,int){}
void Path_GDI::AddBeziers(const UiPoint*,int){}
void Path_GDI::AddRect(const UiRect& r){m_bounds.Union(r);} void Path_GDI::AddEllipse(const UiRect& r){m_bounds.Union(r);} void Path_GDI::AddArc(const UiRect& r,float,float){m_bounds.Union(r);} 
void Path_GDI::AddPolygon(const UiPoint* p,int c){ if(p&&c>0){ UiRect r(p[0].x,p[0].y,p[0].x,p[0].y); for(int i=1;i<c;i++){ r.left=std::min(r.left,p[i].x);r.top=std::min(r.top,p[i].y);r.right=std::max(r.right,p[i].x);r.bottom=std::max(r.bottom,p[i].y);} m_bounds.Union(r);} }
void Path_GDI::AddPolygon(const UiPointF* p,int c){ if(p&&c>0){ UiRect r((int)p[0].x,(int)p[0].y,(int)p[0].x,(int)p[0].y); for(int i=1;i<c;i++){ r.left=std::min(r.left,(int)p[i].x);r.top=std::min(r.top,(int)p[i].y);r.right=std::max(r.right,(int)p[i].x);r.bottom=std::max(r.bottom,(int)p[i].y);} m_bounds.Union(r);} }
void Path_GDI::Transform(IMatrix* m){UNUSED_VARIABLE(m);} UiRect Path_GDI::GetBounds(const IPen* p){UNUSED_VARIABLE(p);return m_bounds;} void Path_GDI::Close(){} void Path_GDI::Reset(){m_bounds.Clear();} IPath* Path_GDI::Clone(){ Path_GDI* p=new Path_GDI(); p->m_fillType=m_fillType;p->m_bounds=m_bounds;return p; }

} // namespace ui
