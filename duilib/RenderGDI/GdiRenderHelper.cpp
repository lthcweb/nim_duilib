#include "GdiRenderHelper.h"

#include <algorithm>

namespace ui
{

static constexpr float kOneThird = 1.0f / 3.0f;

void RGBtoHSL(UiColor rgb, float* h, float* s, float* l)
{
    ASSERT((h != nullptr) && (s != nullptr) && (l != nullptr));
    if ((h == nullptr) || (s == nullptr) || (l == nullptr)) {
        return;
    }

    const float r = static_cast<float>(GetRValue(rgb)) / 255.0f;
    const float g = static_cast<float>(GetGValue(rgb)) / 255.0f;
    const float b = static_cast<float>(GetBValue(rgb)) / 255.0f;
    const float m = std::min(std::min(r, g), b);
    const float M = std::max(std::max(r, g), b);

    *l = (m + M) * 0.5f;
    if (M == m) {
        *h = 0;
        *s = 0;
        return;
    }

    const float f = (r == m) ? (g - b) : ((g == m) ? (b - r) : (r - g));
    const float i = (r == m) ? 3.0f : ((g == m) ? 5.0f : 1.0f);
    *h = (i - f / (M - m));
    if (*h >= 6.0f) {
        *h -= 6.0f;
    }
    *h *= 60.0f;
    *s = (2 * (*l) <= 1) ? ((M - m) / (M + m)) : ((M - m) / (2 - M - m));
}

UiColor HSLtoRGB(float h, float s, float l, uint8_t alpha)
{
    const float q = (2 * l < 1) ? (l * (1 + s)) : (l + s - l * s);
    const float p = 2 * l - q;
    const float hh = h / 360.0f;

    const float tr = hh + kOneThird;
    const float tg = hh;
    const float tb = hh - kOneThird;

    auto normalize = [](float t) {
        if (t < 0) {
            return t + 1.0f;
        }
        if (t > 1) {
            return t - 1.0f;
        }
        return t;
    };

    const float ntr = normalize(tr);
    const float ntg = normalize(tg);
    const float ntb = normalize(tb);

    auto convert = [p, q](float t) {
        if (6 * t < 1) {
            return p + (q - p) * 6 * t;
        }
        if (2 * t < 1) {
            return q;
        }
        if (3 * t < 2) {
            return p + (q - p) * 6 * (2.0f * kOneThird - t);
        }
        return p;
    };

    const uint8_t rr = static_cast<uint8_t>(std::clamp(convert(ntb) * 255.0f, 0.0f, 255.0f));
    const uint8_t gg = static_cast<uint8_t>(std::clamp(convert(ntg) * 255.0f, 0.0f, 255.0f));
    const uint8_t bb = static_cast<uint8_t>(std::clamp(convert(ntr) * 255.0f, 0.0f, 255.0f));
    return UiColor(UiColor::MakeARGB(alpha, rr, gg, bb));
}

bool MakeImageDest(const UiRect& rcControl,
                   const UiSize& szImage,
                   const DString& sAlign,
                   const UiPadding& rcPadding,
                   UiRect& rcDest)
{
    rcDest = rcControl;

    if (sAlign.find(_T("left")) != DString::npos) {
        rcDest.left = rcControl.left;
        rcDest.right = rcDest.left + szImage.cx;
    }
    else if (sAlign.find(_T("hcenter")) != DString::npos) {
        rcDest.left = rcControl.left + (rcControl.Width() - szImage.cx) / 2;
        rcDest.right = rcDest.left + szImage.cx;
    }
    else if (sAlign.find(_T("right")) != DString::npos) {
        rcDest.left = rcControl.right - szImage.cx;
        rcDest.right = rcDest.left + szImage.cx;
    }

    if (sAlign.find(_T("top")) != DString::npos) {
        rcDest.top = rcControl.top;
        rcDest.bottom = rcDest.top + szImage.cy;
    }
    else if (sAlign.find(_T("vcenter")) != DString::npos) {
        rcDest.top = rcControl.top + (rcControl.Height() - szImage.cy) / 2;
        rcDest.bottom = rcDest.top + szImage.cy;
    }
    else if (sAlign.find(_T("bottom")) != DString::npos) {
        rcDest.top = rcControl.bottom - szImage.cy;
        rcDest.bottom = rcDest.top + szImage.cy;
    }

    rcDest.Deflate(rcPadding);
    return !rcDest.IsEmpty();
}

#if defined(DUILIB_BUILD_FOR_WIN)

static COLORREF PixelAlpha(COLORREF clrSrc, double srcDarken, COLORREF clrDest, double destDarken)
{
    return RGB(GetRValue(clrSrc) * srcDarken + GetRValue(clrDest) * destDarken,
               GetGValue(clrSrc) * srcDarken + GetGValue(clrDest) * destDarken,
               GetBValue(clrSrc) * srcDarken + GetBValue(clrDest) * destDarken);
}

bool AlphaBitBlt(HDC hDC,
                 int32_t nDestX,
                 int32_t nDestY,
                 int32_t dwWidth,
                 int32_t dwHeight,
                 HDC hSrcDC,
                 int32_t nSrcX,
                 int32_t nSrcY,
                 int32_t wSrc,
                 int32_t hSrc,
                 BLENDFUNCTION ftn)
{
    HDC hTempDC = ::CreateCompatibleDC(hDC);
    if (hTempDC == nullptr) {
        return false;
    }

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = dwWidth;
    bmi.bmiHeader.biHeight = dwHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = dwWidth * dwHeight;

    COLORREF* pSrcBits = nullptr;
    HBITMAP hSrcDib = ::CreateDIBSection(hSrcDC, &bmi, DIB_RGB_COLORS, (void**)&pSrcBits, nullptr, 0);
    if ((hSrcDib == nullptr) || (pSrcBits == nullptr)) {
        ::DeleteDC(hTempDC);
        return false;
    }

    HGDIOBJ hOldBmp = ::SelectObject(hTempDC, hSrcDib);
    ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
    ::SelectObject(hTempDC, hOldBmp);

    COLORREF* pDestBits = nullptr;
    HBITMAP hDestDib = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pDestBits, nullptr, 0);
    if ((hDestDib == nullptr) || (pDestBits == nullptr)) {
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return false;
    }

    ::SelectObject(hTempDC, hDestDib);
    ::BitBlt(hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
    ::SelectObject(hTempDC, hOldBmp);

    for (int32_t pixel = 0; pixel < dwWidth * dwHeight; ++pixel, ++pSrcBits, ++pDestBits) {
        const BYTE nAlpha = LOBYTE((*pSrcBits) >> 24);
        double srcDarken = static_cast<double>(nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
        if (srcDarken < 0.0) {
            srcDarken = 0.0;
        }
        *pDestBits = PixelAlpha(*pSrcBits, srcDarken, *pDestBits, 1.0 - srcDarken);
    }

    ::SelectObject(hTempDC, hDestDib);
    ::BitBlt(hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
    ::SelectObject(hTempDC, hOldBmp);

    ::DeleteObject(hDestDib);
    ::DeleteObject(hSrcDib);
    ::DeleteDC(hTempDC);
    return true;
}

#endif

} // namespace ui
