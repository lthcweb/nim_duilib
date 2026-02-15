#ifndef UI_RENDER_RENDER_GDI_H_
#define UI_RENDER_RENDER_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include <Windows.h>

#include "GDIPlus_defs.h"

namespace ui
{

/** GDI+ 渲染器实现类
*/
class UILIB_API Render_GDI : public IRender
{
public:
    Render_GDI();
    virtual ~Render_GDI();

    Render_GDI(const Render_GDI&) = delete;
    Render_GDI& operator=(const Render_GDI&) = delete;

public:
    // 基础接口
    virtual RenderType GetRenderType() const override;
    virtual RenderBackendType GetRenderBackendType() const override;
    virtual int32_t GetWidth() const override;
    virtual int32_t GetHeight() const override;
    virtual bool Resize(int32_t width, int32_t height) override;

    // 坐标变换
    virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) override;
    virtual UiPoint SetWindowOrg(UiPoint pt) override;
    virtual UiPoint GetWindowOrg() const override;

    // 裁剪区域
    virtual void SaveClip(int32_t& nState) override;
    virtual void RestoreClip(int32_t nState) override;
    virtual void SetClip(const UiRect& rc, bool bIntersect = true) override;
    virtual void SetRoundClip(const UiRect& rcItem, float rx, float ry, bool bIntersect = true) override;
    virtual void ClearClip() override;

    // 位图操作
    virtual bool BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy,
        IRender* pSrcRender, int32_t xSrc, int32_t ySrc,
        RopMode rop) override;

    virtual bool StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
        IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
        RopMode rop) override;

    virtual bool AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
        IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
        uint8_t alpha = 255) override;

// 图片绘制
    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
        const UiRect& rcDest, const UiRect& rcDestCorners,
        const UiRect& rcSource, const UiRect& rcSourceCorners,
        uint8_t uFade = 255,
        const TiledDrawParam* pTiledDrawParam = nullptr,
        bool bWindowShadowMode = false) override;

    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
        const UiRect& rcDest, const UiRect& rcSource,
        uint8_t uFade = 255,
        const TiledDrawParam* pTiledDrawParam = nullptr,
        bool bWindowShadowMode = false) override;

    virtual void DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
        const UiRect& rcDest, const UiRect& rcSource,
        uint8_t uFade = 255, IMatrix* pMatrix = nullptr) override;

// 线条绘制
    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth) override;
    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, float fWidth) override;
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth) override;
    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen) override;
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, IPen* pen) override;

    // 矩形绘制
    virtual void DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect = false) override;
    virtual void DrawRect(const UiRectF& rc, UiColor penColor, int32_t nWidth, bool bLineInRect = false) override;
    virtual void DrawRect(const UiRect& rc, UiColor penColor, float fWidth, bool bLineInRect = false) override;
    virtual void DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect = false) override;
    virtual void DrawRect(const UiRect& rc, IPen* pen, bool bLineInRect = false) override;
    virtual void DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect = false) override;

    virtual void FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;

    // 圆角矩形绘制
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, int32_t nWidth) override;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, int32_t nWidth) override;
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, float fWidth) override;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth) override;
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, IPen* pen) override;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen) override;

    virtual void FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;

    // 圆形绘制
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int32_t nWidth) override;
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, float fWidth) override;
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, IPen* pen) override;
    virtual void FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade = 255) override;

    // 弧形绘制
    virtual void DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter,
        const IPen* pen,
        UiColor* gradientColor = nullptr, const UiRect* gradientRect = nullptr) override;

// 路径绘制
    virtual void DrawPath(const IPath* path, const IPen* pen) override;
    virtual void FillPath(const IPath* path, const IBrush* brush) override;
    virtual void FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) override;

    // 文本绘制
    virtual UiRect MeasureString(const DString& strText, const MeasureStringParam& measureParam) override;
    virtual void DrawString(const DString& strText, const DrawStringParam& drawParam) override;

    // 富文本绘制
    virtual void MeasureRichText(const UiRect& textRect,
        const UiSize& szScrollOffset,
        IRenderFactory* pRenderFactory,
        const std::vector<RichTextData>& richTextData,
        std::vector<std::vector<UiRect>>* pRichTextRects) override;

    virtual void MeasureRichText2(const UiRect& textRect,
        const UiSize& szScrollOffset,
        IRenderFactory* pRenderFactory,
        const std::vector<RichTextData>& richTextData,
        RichTextLineInfoParam* pLineInfoParam,
        std::vector<std::vector<UiRect>>* pRichTextRects) override;

    virtual void MeasureRichText3(const UiRect& textRect,
        const UiSize& szScrollOffset,
        IRenderFactory* pRenderFactory,
        const std::vector<RichTextData>& richTextData,
        RichTextLineInfoParam* pLineInfoParam,
        std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
        std::vector<std::vector<UiRect>>* pRichTextRects) override;

    virtual void DrawRichText(const UiRect& textRect,
        const UiSize& szScrollOffset,
        IRenderFactory* pRenderFactory,
        const std::vector<RichTextData>& richTextData,
        uint8_t uFade = 255,
        std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) override;

    virtual bool CreateDrawRichTextCache(const UiRect& textRect,
        const UiSize& szScrollOffset,
        IRenderFactory* pRenderFactory,
        const std::vector<RichTextData>& richTextData,
        std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache) override;

    virtual bool IsValidDrawRichTextCache(const UiRect& textRect,
        const std::vector<RichTextData>& richTextData,
        const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache) override;

    virtual bool UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
        const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
        std::vector<RichTextData>& richTextDataNew,
        size_t nStartLine,
        const std::vector<size_t>& modifiedLines,
        size_t nModifiedRows,
        const std::vector<size_t>& deletedLines,
        size_t nDeletedRows,
        const std::vector<int32_t>& rowRectTopList) override;

    virtual bool IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const override;

    virtual void DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
        const UiRect& textRect,
        const UiSize& szNewScrollOffset,
        const std::vector<int32_t>& rowXOffset,
        uint8_t uFade,
        std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) override;

// 阴影绘制
    virtual void DrawBoxShadow(const UiRect& rc,
        const UiSize& roundSize,
        const UiPoint& cpOffset,
        int32_t nBlurRadius,
        int32_t nSpreadRadius,
        UiColor dwColor) override;

// 位图操作
    virtual IBitmap* MakeImageSnapshot() override;
    virtual void ClearAlpha(const UiRect& rcDirty, uint8_t alpha = 0) override;
    virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) override;
    virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding = UiPadding()) override;

#ifdef DUILIB_BUILD_FOR_WIN
    virtual HDC GetRenderDC(HWND hWnd) override;
    virtual void ReleaseRenderDC(HDC hdc) override;
#endif

    // 公共接口
    virtual void Clear(const UiColor& uiColor) override;
    virtual void ClearRect(const UiRect& rcDirty, const UiColor& uiColor) override;
    virtual std::unique_ptr<IRender> Clone() override;
    virtual bool ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen) override;
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc) override;
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint) override;
    virtual RenderClipType GetClipInfo(std::vector<UiRect>& clipRects) override;
    virtual bool IsClipEmpty() const override;
    virtual bool IsEmpty() const override;
    virtual void SetRenderDpi(const IRenderDpiPtr& spRenderDpi) override;
    virtual bool PaintAndSwapBuffers(IRenderPaint* pRenderPaint) override;
    virtual bool SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw) override;
    virtual bool SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw) override;
    virtual void ClearWindowRgn(bool bRedraw) override;

public:
    /** 获取 GDI+ Graphics 对象
    */
    Gdiplus::Graphics* GetGraphics() const { return m_pGraphics; }

    /** 获取坐标原点
    */
    Gdiplus::PointF& GetPointOrg() const { return *m_pPointOrg; }

    /** 获取 DPI 接口
    */
    IRenderDpiPtr GetRenderDpi() const { return m_spRenderDpi; }

    /** 获取像素数据指针
    */
    void* GetPixelBits() const;

private:
    /** 初始化渐变画刷
    */
    void InitGradientBrush(Gdiplus::LinearGradientBrush& brush, const Gdiplus::RectF& rc,
        UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const;

/** UiColor 转换为 GDI+ Color
*/
    Gdiplus::Color UiColorToGdiplusColor(UiColor color, uint8_t alpha = 255) const;

    /** 根据 IPen 设置 GDI+ Pen
    */
    void SetPenByIPen(Gdiplus::Pen& pen, const IPen* pPen);

    /** 获取合成模式
    */
    Gdiplus::CompositingMode GetCompositingMode(RopMode rop) const;

    /** 计算字符数量（UTF-16）
    */
    size_t GetUTF16CharCount(const DStringW::value_type* srcPtr, size_t textStartIndex) const;

private:
    Gdiplus::Bitmap* m_pBitmap;          // 位图对象
    Gdiplus::Graphics* m_pGraphics;      // 绘图对象
    Gdiplus::PointF* m_pPointOrg;        // 坐标原点
    int32_t m_nWidth;                     // 宽度
    int32_t m_nHeight;                    // 高度
    std::vector<Gdiplus::GraphicsState> m_stateStack; // 状态栈
    IRenderDpiPtr m_spRenderDpi;         // DPI 接口
    RenderBackendType m_backendType;     // 后端类型
    HWND m_hWnd;                         // 窗口句柄
    HDC m_hDC;                           // 设备上下文
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_RENDER_GDI_H_
