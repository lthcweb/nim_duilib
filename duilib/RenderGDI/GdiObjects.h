#ifndef UI_RENDER_GDI_OBJECTS_H_
#define UI_RENDER_GDI_OBJECTS_H_

#include "duilib/Render/IRender.h"
#include <vector>

namespace ui {

class Font_GDI : public IFont {
public:
    virtual bool InitFont(const UiFont& fontInfo) override;
    virtual DString FontName() const override;
    virtual int FontSize() const override;
    virtual bool IsBold() const override;
    virtual bool IsUnderline() const override;
    virtual bool IsItalic() const override;
    virtual bool IsStrikeOut() const override;
private:
    UiFont m_fontInfo;
};

class FontMgr_GDI : public IFontMgr {
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
    DString m_defaultFontName;
    std::vector<DString> m_fontNames;
};

class Bitmap_GDI : public IBitmap {
public:
    virtual bool Init(uint32_t nWidth, uint32_t nHeight,
                      const void* pPixelBits, float fImageSizeScale = 1.0f,
                      BitmapAlphaType alphaType = BitmapAlphaType::kPremul_SkAlphaType) override;
    virtual uint32_t GetWidth() const override;
    virtual uint32_t GetHeight() const override;
    virtual UiSize GetSize() const override;
    virtual void* LockPixelBits() override;
    virtual void UnLockPixelBits() override;
    virtual IBitmap* Clone() override;

    uint32_t* GetBits();
    const uint32_t* GetBits() const;

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<uint32_t> m_bits;
};

class Pen_GDI : public IPen {
public:
    Pen_GDI(UiColor color, float width);
    virtual void SetWidth(float fWidth) override;
    virtual float GetWidth() const override;
    virtual void SetColor(UiColor color) override;
    virtual UiColor GetColor() const override;
    virtual void SetStartCap(LineCap cap) override;
    virtual LineCap GetStartCap() const override;
    virtual void SetEndCap(LineCap cap) override;
    virtual LineCap GetEndCap() const override;
    virtual void SetDashCap(LineCap cap) override;
    virtual LineCap GetDashCap() const override;
    virtual void SetLineJoin(LineJoin join) override;
    virtual LineJoin GetLineJoin() const override;
    virtual void SetDashStyle(DashStyle style) override;
    virtual DashStyle GetDashStyle() const override;
    virtual IPen* Clone() const override;

private:
    UiColor m_color;
    float m_width;
    LineCap m_startCap = kButt_Cap;
    LineCap m_endCap = kButt_Cap;
    LineCap m_dashCap = kButt_Cap;
    LineJoin m_lineJoin = kMiter_Join;
    DashStyle m_dashStyle = kDashStyleSolid;
};

class Brush_GDI : public IBrush {
public:
    explicit Brush_GDI(UiColor color);
    virtual IBrush* Clone() override;
    virtual UiColor GetColor() const override;
private:
    UiColor m_color;
};

class Matrix_GDI : public IMatrix {
public:
    virtual void Translate(int offsetX, int offsetY) override;
    virtual void Scale(float scaleX, float scaleY) override;
    virtual void Rotate(float angle) override;
    virtual void RotateAt(float angle, const UiPoint& center) override;
};

class Path_GDI : public IPath {
public:
    virtual void SetFillType(FillType mode) override;
    virtual FillType GetFillType() override;
    virtual void AddLine(int x1, int y1, int x2, int y2) override;
    virtual void AddLines(const UiPoint* points, int count) override;
    virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
    virtual void AddBeziers(const UiPoint* points, int count) override;
    virtual void AddRect(const UiRect& rect) override;
    virtual void AddEllipse(const UiRect& rect) override;
    virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) override;
    virtual void AddPolygon(const UiPoint* points, int count) override;
    virtual void AddPolygon(const UiPointF* points, int count) override;
    virtual void Transform(IMatrix* pMatrix) override;
    virtual UiRect GetBounds(const IPen* pen) override;
    virtual void Close() override;
    virtual void Reset() override;
    virtual IPath* Clone() override;
private:
    FillType m_fillType = FillType::kWinding;
    UiRect m_bounds;
};

} // namespace ui

#endif
