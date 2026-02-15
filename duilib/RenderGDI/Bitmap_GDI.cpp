#include "Bitmap_GDI.h"
#include <cstring>

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

Bitmap_GDI::Bitmap_GDI()
    : m_pBitmap(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_fImageSizeScale(1.0f)
    , m_pLockedBitmapData(nullptr)
{
}

Bitmap_GDI::~Bitmap_GDI()
{
    if (m_pLockedBitmapData != nullptr) {
        UnLockPixelBits();
    }
    if (m_pBitmap != nullptr) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }
}

bool Bitmap_GDI::Init(uint32_t nWidth, uint32_t nHeight,
                     const void* pPixelBits, float fImageSizeScale,
                     BitmapAlphaType alphaType)
{
    if (nWidth == 0 || nHeight == 0) {
        return false;
    }

    // 清理旧位图
    if (m_pBitmap != nullptr) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }

    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_fImageSizeScale = fImageSizeScale;

    // 根据 AlphaType 选择像素格式
    Gdiplus::PixelFormat pixelFormat;
    switch (alphaType) {
    case BitmapAlphaType::kOpaque_SkAlphaType:
        pixelFormat = PixelFormat32bppRGB;
        break;
    case BitmapAlphaType::kPremul_SkAlphaType:
        pixelFormat = PixelFormat32bppPARGB;
        break;
    case BitmapAlphaType::kUnpremul_SkAlphaType:
        pixelFormat = PixelFormat32bppARGB;
        break;
    default:
        pixelFormat = PixelFormat32bppARGB;
        break;
    }

    if (pPixelBits == nullptr) {
        // 创建空位图
        m_pBitmap = new Gdiplus::Bitmap(nWidth, nHeight, pixelFormat);
    }
    else {
        // 从像素数据创建位图
        int stride = nWidth * 4; // 每行字节数（ARGB 格式）
        m_pBitmap = new Gdiplus::Bitmap(nWidth, nHeight, stride,
                                       pixelFormat,
                                       const_cast<BYTE*>(static_cast<const BYTE*>(pPixelBits)));
    }

    return (m_pBitmap != nullptr && m_pBitmap->GetLastStatus() == Gdiplus::Ok);
}

uint32_t Bitmap_GDI::GetWidth() const
{
    return m_nWidth;
}

uint32_t Bitmap_GDI::GetHeight() const
{
    return m_nHeight;
}

UiSize Bitmap_GDI::GetSize() const
{
    return UiSize(m_nWidth, m_nHeight);
}

void* Bitmap_GDI::LockPixelBits()
{
    if (m_pBitmap == nullptr || m_pLockedBitmapData != nullptr) {
        return nullptr;
    }

    m_pLockedBitmapData = new Gdiplus::BitmapData();
    Gdiplus::Rect rect(0, 0, m_nWidth, m_nHeight);

    Gdiplus::Status status = m_pBitmap->LockBits(&rect,
                                                 Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
                                                 PixelFormat32bppARGB,
                                                 m_pLockedBitmapData);

    if (status != Gdiplus::Ok) {
        delete m_pLockedBitmapData;
        m_pLockedBitmapData = nullptr;
        return nullptr;
    }

    return m_pLockedBitmapData->Scan0;
}

void Bitmap_GDI::UnLockPixelBits()
{
    if (m_pBitmap != nullptr && m_pLockedBitmapData != nullptr) {
        m_pBitmap->UnlockBits(m_pLockedBitmapData);
        delete m_pLockedBitmapData;
        m_pLockedBitmapData = nullptr;
    }
}

IBitmap* Bitmap_GDI::Clone()
{
    if (m_pBitmap == nullptr) {
        return nullptr;
    }

    // 锁定源位图
    void* pSrcBits = LockPixelBits();
    if (pSrcBits == nullptr) {
        return nullptr;
    }

    // 创建新位图
    Bitmap_GDI* pNewBitmap = new Bitmap_GDI();
    bool bSuccess = pNewBitmap->Init(m_nWidth, m_nHeight, nullptr, m_fImageSizeScale);

    if (bSuccess) {
        // 锁定目标位图
        void* pDestBits = pNewBitmap->LockPixelBits();
        if (pDestBits != nullptr) {
            // 复制像素数据
            size_t dataSize = m_nWidth * m_nHeight * 4;
            memcpy(pDestBits, pSrcBits, dataSize);
            pNewBitmap->UnLockPixelBits();
        }
        else {
            delete pNewBitmap;
            pNewBitmap = nullptr;
        }
    }
    else {
        delete pNewBitmap;
        pNewBitmap = nullptr;
    }

    // 解锁源位图
    UnLockPixelBits();

    return pNewBitmap;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
