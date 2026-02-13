#include "duilib/RenderGDI/DrawGDIImage.h"

#include <algorithm>

namespace ui {

bool DrawGDIImage::DrawImageRect(Bitmap_GDI& destBitmap,
                                 const UiRect& rcDest,
                                 const Bitmap_GDI& srcBitmap,
                                 const UiRect& rcSource,
                                 uint8_t uFade)
{
    if ((destBitmap.GetBits() == nullptr) || (srcBitmap.GetBits() == nullptr) || rcDest.IsEmpty() || rcSource.IsEmpty()) {
        return false;
    }
    const int destWidth = static_cast<int>(destBitmap.GetWidth());
    const int destHeight = static_cast<int>(destBitmap.GetHeight());
    const int srcWidth = static_cast<int>(srcBitmap.GetWidth());
    const int srcHeight = static_cast<int>(srcBitmap.GetHeight());

    UiRect clipDest = rcDest;
    clipDest.Intersect(UiRect(0, 0, destWidth, destHeight));
    if (clipDest.IsEmpty()) {
        return false;
    }

    auto blend = [uFade](uint32_t src, uint32_t dst) {
        const uint8_t srcA = static_cast<uint8_t>((src >> 24) & 0xFF);
        const uint8_t fadeA = static_cast<uint8_t>((srcA * uFade) / 255);
        const uint8_t dstA = static_cast<uint8_t>((dst >> 24) & 0xFF);
        const uint8_t outA = static_cast<uint8_t>(fadeA + ((dstA * (255 - fadeA)) / 255));

        auto mix = [fadeA](uint8_t sc, uint8_t dc) {
            return static_cast<uint8_t>((sc * fadeA + dc * (255 - fadeA)) / 255);
        };

        const uint8_t outR = mix(static_cast<uint8_t>((src >> 16) & 0xFF), static_cast<uint8_t>((dst >> 16) & 0xFF));
        const uint8_t outG = mix(static_cast<uint8_t>((src >> 8) & 0xFF), static_cast<uint8_t>( (dst >> 8) & 0xFF));
        const uint8_t outB = mix(static_cast<uint8_t>(src & 0xFF), static_cast<uint8_t>(dst & 0xFF));
        return (static_cast<uint32_t>(outA) << 24) | (static_cast<uint32_t>(outR) << 16) |
               (static_cast<uint32_t>(outG) << 8) | static_cast<uint32_t>(outB);
    };

    for (int y = clipDest.top; y < clipDest.bottom; ++y) {
        for (int x = clipDest.left; x < clipDest.right; ++x) {
            const int dx = x - rcDest.left;
            const int dy = y - rcDest.top;
            const int sx = rcSource.left + (dx * rcSource.Width()) / std::max(1, rcDest.Width());
            const int sy = rcSource.top + (dy * rcSource.Height()) / std::max(1, rcDest.Height());
            if ((sx < 0) || (sy < 0) || (sx >= srcWidth) || (sy >= srcHeight)) {
                continue;
            }

            uint32_t* pDst = destBitmap.GetBits() + static_cast<size_t>(y) * destWidth + x;
            const uint32_t* pSrc = srcBitmap.GetBits() + static_cast<size_t>(sy) * srcWidth + sx;
            *pDst = blend(*pSrc, *pDst);
        }
    }
    return true;
}

} // namespace ui
