#ifndef UI_RENDER_GDI_BITMAP_GDI_H_
#define UI_RENDER_GDI_BITMAP_GDI_H_

#include "duilib/Render/IRender.h"
#include <vector>

namespace ui {

class Bitmap_GDI : public IBitmap {
public:
    bool Init(uint32_t nWidth, uint32_t nHeight,
              const void* pPixelBits, float fImageSizeScale = 1.0f,
              BitmapAlphaType alphaType = BitmapAlphaType::kPremul_SkAlphaType) override;
    uint32_t GetWidth() const override;
    uint32_t GetHeight() const override;
    UiSize GetSize() const override;
    void* LockPixelBits() override;
    void UnLockPixelBits() override;
    IBitmap* Clone() override;

    uint32_t* GetBits();
    const uint32_t* GetBits() const;

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<uint32_t> m_bits;
};

} // namespace ui

#endif
