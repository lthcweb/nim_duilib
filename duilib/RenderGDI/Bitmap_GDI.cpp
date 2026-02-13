#include "duilib/RenderGDI/Bitmap_GDI.h"

#include <cstring>

namespace ui {

bool Bitmap_GDI::Init(uint32_t nWidth, uint32_t nHeight,
                      const void* pPixelBits, float fImageSizeScale,
                      BitmapAlphaType alphaType)
{
    UNUSED_VARIABLE(fImageSizeScale);
    UNUSED_VARIABLE(alphaType);
    m_width = nWidth;
    m_height = nHeight;
    m_bits.resize(static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight));
    if (!m_bits.empty()) {
        if (pPixelBits != nullptr) {
            std::memcpy(m_bits.data(), pPixelBits, m_bits.size() * sizeof(uint32_t));
        }
        else {
            std::memset(m_bits.data(), 0, m_bits.size() * sizeof(uint32_t));
        }
    }
    return (nWidth > 0) && (nHeight > 0);
}

uint32_t Bitmap_GDI::GetWidth() const { return m_width; }
uint32_t Bitmap_GDI::GetHeight() const { return m_height; }
UiSize Bitmap_GDI::GetSize() const { return UiSize(static_cast<int32_t>(m_width), static_cast<int32_t>(m_height)); }
void* Bitmap_GDI::LockPixelBits() { return m_bits.empty() ? nullptr : m_bits.data(); }
void Bitmap_GDI::UnLockPixelBits() {}

IBitmap* Bitmap_GDI::Clone()
{
    auto* pBitmap = new Bitmap_GDI();
    pBitmap->m_width = m_width;
    pBitmap->m_height = m_height;
    pBitmap->m_bits = m_bits;
    return pBitmap;
}

uint32_t* Bitmap_GDI::GetBits() { return m_bits.empty() ? nullptr : m_bits.data(); }
const uint32_t* Bitmap_GDI::GetBits() const { return m_bits.empty() ? nullptr : m_bits.data(); }

} // namespace ui
