#ifndef UI_RENDER_BITMAP_GDI_H_
#define UI_RENDER_BITMAP_GDI_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "GDIPlus_defs.h"

namespace ui
{

/** GDI+ 位图实现类
*/
class UILIB_API Bitmap_GDI : public IBitmap
{
public:
    Bitmap_GDI();
    virtual ~Bitmap_GDI();

    Bitmap_GDI(const Bitmap_GDI&) = delete;
    Bitmap_GDI& operator=(const Bitmap_GDI&) = delete;

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

public:
    /** 获取 GDI+ Bitmap 对象
    */
    Gdiplus::Bitmap* GetBitmap() const { return m_pBitmap; }

private:
    Gdiplus::Bitmap* m_pBitmap;
    uint32_t m_nWidth;
    uint32_t m_nHeight;
    float m_fImageSizeScale;
    Gdiplus::BitmapData* m_pLockedBitmapData;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_RENDER_BITMAP_GDI_H_
