#include "color_conversion.h"
#include "color_types.h"
#include "../m3/hct/hct.h"
#include "../m3/hct/cam.h"
#include "../m3/utils/utils.h"

#include <cmath>
#include <cstdint>

namespace wallwatch{

RGB rgbU8ToRgb(const RGB_U8& nrgb){
    RGB rgb;
    rgb.r = nrgb.r/255.0;
    rgb.g = nrgb.g/255.0;
    rgb.b = nrgb.b/255.0;
    return rgb;
}

RGB_U8 rgbToRgbU8(const RGB& rgb){
    return RGB_U8{
        static_cast<uint8_t>(clamp(rgb.r, 0.0, 1.0) * 255.0 + 0.5),
        static_cast<uint8_t>(clamp(rgb.g, 0.0, 1.0) * 255.0 + 0.5),
        static_cast<uint8_t>(clamp(rgb.b, 0.0, 1.0) * 255.0 + 0.5)
    };
}

RGBA argbToRgba(unsigned int argb) {
    return RGBA{
        ((argb >> 16) & 0xFF)/255.0,
        ((argb >> 8) & 0xFF)/255.0,
        (argb & 0xFF)/255.0,
        ((argb >> 24) & 0xFF)/255.0
    };
}

unsigned int rgbaToArgb(const RGBA& rgba) {
    uint8_t r = static_cast<uint8_t>(clamp(rgba.r, 0.0, 1.0) * 255.0 + 0.5);
    uint8_t g = static_cast<uint8_t>(clamp(rgba.g, 0.0, 1.0) * 255.0 + 0.5);
    uint8_t b = static_cast<uint8_t>(clamp(rgba.b, 0.0, 1.0) * 255.0 + 0.5);
    uint8_t a = static_cast<uint8_t>(clamp(rgba.a, 0.0, 1.0) * 255.0 + 0.5);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

XYZ rgbToXyz(const RGB& rgb){
    double r = linearize(rgb.r);
    double g = linearize(rgb.g);
    double b = linearize(rgb.b);

    double x = RGB_TO_XYZ[0][0]*r + RGB_TO_XYZ[0][1]*g + RGB_TO_XYZ[0][2]*b;
    double y = RGB_TO_XYZ[1][0]*r + RGB_TO_XYZ[1][1]*g + RGB_TO_XYZ[1][2]*b;
    double z = RGB_TO_XYZ[2][0]*r + RGB_TO_XYZ[2][1]*g + RGB_TO_XYZ[2][2]*b;

    return XYZ{x,y,z};
}

RGB xyzToRgb(const XYZ& xyz){
    double x = xyz.x; double y = xyz.y; double z = xyz.z;

    double r = XYZ_TO_RGB[0][0]*x + XYZ_TO_RGB[0][1]*y + XYZ_TO_RGB[0][2]*z;
    double g = XYZ_TO_RGB[1][0]*x + XYZ_TO_RGB[1][1]*y + XYZ_TO_RGB[1][2]*z;
    double b = XYZ_TO_RGB[2][0]*x + XYZ_TO_RGB[2][1]*y + XYZ_TO_RGB[2][2]*z;

    r = clamp(r, 0.0, 1.0);
    g = clamp(g, 0.0, 1.0);
    b = clamp(b, 0.0, 1.0);

    return RGB{
        compress(r),
        compress(g),
        compress(b),
    };
}

LAB xyzToLab(const XYZ& xyz, const WhitePoint& whitepoint){
    double xr = xyz.x/whitepoint.x;
    double yr = xyz.y/whitepoint.y;
    double zr = xyz.z/whitepoint.z;

    auto f = [](double t) -> double {
        if (t > LAB_EPSILON) {
            return std::cbrt(t);
        } else {
            return (LAB_KAPPA * t + 16.0) / 116.0;
        }
    };

    double fx = f(xr);
    double fy = f(yr);
    double fz = f(zr);

    double l = 116.0 * fy - 16.0;
    double a = 500.0 * (fx - fy);
    double b = 200.0 * (fy - fz);

    l = clamp(l, 0.0, 100.0);
    a = clamp(a, -128.0, 127.0);
    b = clamp(b, -128.0, 127.0);

    return LAB{l,a,b};
}

XYZ labToXyz(const LAB& lab, const WhitePoint& whitepoint){
    double fy = (lab.l + 16.0)/116.0;
    double fx = fy + (lab.a/500.0);
    double fz = fy - (lab.b/200.0);

    double xr = f_inv(fx);
    double yr = f_inv(fy);
    double zr = f_inv(fz);

    return XYZ{
        xr * whitepoint.x,
        yr * whitepoint.y,
        zr * whitepoint.z
    };
}

XYZ lchToXyz(const LCH& lch, const WhitePoint& whitePoint) {
    double L = lch.l;

    double h_rad = degToRad(lch.h);

    double a = lch.c * std::cos(h_rad);
    double b = lch.c * std::sin(h_rad);

    double fy = (L + 16.0) / 116.0;
    double fx = fy + (a / 500.0);
    double fz = fy - (b / 200.0);

    double xr = f_inv(fx);
    double yr = f_inv(fy);
    double zr = f_inv(fz);

    double X = xr * whitePoint.x;
    double Y = yr * whitePoint.y;
    double Z = zr * whitePoint.z;
    return XYZ{X, Y, Z};
}

LAB lchToLab(const LCH& lch){
    double l = lch.l;
    double h_deg = degToRad(lch.h);
    double a = lch.c * std::cos(lch.h);
    double b = lch.c * std::sin(h_deg);

    return LAB{l,a,b};
}

LCH labToLch(const LAB& lab){
    double l = lab.l;
    double c = std::hypot(lab.a, lab.b);
    double h = 0.0;
    if (c > 1e-6) {
        double h_rad = std::atan2(lab.b, lab.a);
        h = radToDeg(h_rad);
        if (h < 0.0) h += 360.0;
    }

    return LCH{l,c,h};
}

LCH xyzToLch(const XYZ& xyz){
    LAB lab = xyzToLab(xyz, D65);
    return labToLch(lab);
}

unsigned int rgbToArgb(const RGB& rgb, uint8_t a){
    uint8_t r = static_cast<uint8_t>(clamp(rgb.r, 0.0, 1.0) * 255.0 + 0.5);
    uint8_t g = static_cast<uint8_t>(clamp(rgb.g, 0.0, 1.0) * 255.0 + 0.5);
    uint8_t b = static_cast<uint8_t>(clamp(rgb.b, 0.0, 1.0) * 255.0 + 0.5);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

RGB argbToRgb(unsigned int argb){
    double r = ((argb >> 16) & 0xFF)/255.0;
    double g = ((argb >> 8) & 0xFF)/255.0;
    double b = (argb & 0xFF)/255.0;
    return RGB{r,g,b};
}

HCT lchToHct(const LCH& lch){
    LAB lab = lchToLab(lch);
    XYZ xyz = lchToXyz(lch, D65);

    auto cam = material_color_utilities::CamFromXyzAndViewingConditions(
        xyz.x, xyz.y, xyz.z,
        material_color_utilities::kDefaultViewingConditions
    );
    return HCT(cam.hue, cam.chroma, lch.l);
}

LCH hctToLch(const HCT& hct) {
    double L = hct.get_tone();

    double hue = hct.get_hue();
    double chroma = hct.get_chroma();

    double h_rad = degToRad(hue);
    double a = chroma * std::cos(h_rad);
    double b = chroma * std::sin(h_rad);

    LAB lab{L, a, b};
    return labToLch(lab);
}

HCT rgbToHct(const RGB& rgb){
    unsigned int argb = rgbToArgb(rgb, 255);
    return HCT(argb);
}

RGB hctToRgb(const HCT& hct){
    unsigned int argb = hct.ToInt();
    return argbToRgb(argb);
}

LAB rgbToLab(const RGB& rgb){
    XYZ xyz = rgbToXyz(rgb);
    return xyzToLab(xyz, D65);
}

RGB labToRgb(const LAB& lab){
    XYZ xyz = labToXyz(lab, D65);
    return xyzToRgb(xyz);
}

LCH rgbToLch(const RGB& rgb){
    LAB lab = rgbToLab(rgb);
    return labToLch(lab);
}

RGB lchToRgb(const LCH& lch){
    LAB lab = lchToLab(lch);
    XYZ xyz = labToXyz(lab, D65);
    return xyzToRgb(xyz);
}

bool isValidRgb(const RGB& rgb){
    return rgb.r >= 0.0 && rgb.r <= 1.0 &&
           rgb.g >= 0.0 && rgb.g <= 1.0 &&
           rgb.b >= 0.0 && rgb.b <= 1.0;
}

bool isValidLab(const LAB& lab){
    return lab.l >= 0.0 && lab.l <= 100.0 &&
           lab.a >= -128.0 && lab.a <= 128.0 &&
           lab.b >= -128.0 && lab.b <= 128.0;
}

bool isValidLch(const LCH& lch){
    return lch.l >= 0.0 && lch.l <= 100.0 &&
           lch.c >= 0.0 && lch.c <= 150.0 &&
           lch.h >= 0.0 && lch.h < 360.0;
}

RGB blend(const RGB& a, const RGB& b, double t){
    t = clamp(t, 0.0, 1.0);
    return RGB{
        a.r * (1.0 - t) + b.r * t,
        a.g * (1.0 - t) + b.g * t,
        a.b * (1.0 - t) + b.b * t
    };
}

LAB adjustLightness(const LAB& lab, double deltaL){
    return LAB{
        clamp(lab.l + deltaL, 0.0, 100.0),
        lab.a,
        lab.b
    };
}

LCH adjustChroma(const LCH& lch, double deltaC){
    return LCH{
        lch.l,
        clamp(lch.c + deltaC, 0.0, 150.0),
        lch.h
    };
}

LCH rotateHue(const LCH& lch, double degrees){
    double newHue = lch.h + degrees;
    if (newHue >= 360.0) newHue -= 360.0;
    if (newHue < 0.0) newHue += 360.0;
    return LCH{lch.l, lch.c, newHue};
}

HCT lchToHctSimple(const LCH& lch){
    auto m3Hct = lchToHct(lch);
    return HCT{m3Hct.get_hue(), m3Hct.get_chroma(), m3Hct.get_tone()};
}

LCH hctToLchSimple(const HCT& hct){
    return hctToLch(hct);
}

RGB hctToRgbSimple(const HCT& hct){
    return hctToRgb(hct);
}

HCT rgbToHctSimple(const RGB& rgb){
    auto m3Hct = rgbToHct(rgb);
    return HCT{m3Hct.get_hue(), m3Hct.get_chroma(), m3Hct.get_tone()};
}

RGB hct_rgb_gamutMap(const HCT& hct){
    double hue = hct.get_hue();
    double chroma = hct.get_chroma();
    double tone = hct.get_tone();

    RGB rgb = hctToRgb(hct);
    if(isValidRgb(rgb)){
        return rgb;
    }

    double low = 0.0;
    double high = chroma;

    for(int i=0;i<20;i++){
        double mid = (low + high)/2.0;
        HCT testHct(hue, mid, tone);
        RGB testRgb = hctToRgb(testHct);

        if(isValidRgb(testRgb)){
            low = mid;
        }else{
            high = mid;
        }
    }
    HCT resultHct(hue, low, tone);
    return hctToRgb(resultHct);
}

}   //namespace wallwatch
