#pragma once
#include "color_conversion.h"
#include "../m3/hct/hct.h"
#include "../m3/hct/cam.h"
#include "../m3/utils/utils.h"

#include <cmath>
#include <cstdint>

//RGB → XYZ → LAB → LCH → HCT

namespace wallwatch{

RGB rgbU8ToRgb(const RGB_U8& nrgb);
RGB_U8 rgbToRgbU8(const RGB& rgb);
RGBA argbToRgba(unsigned int argb);
unsigned int rgbaToArgb(const RGBA& rgba);
XYZ rgbToXyz(const RGB& rgb);
RGB xyzToRgb(const XYZ& xyz);
LAB xyzToLab(const XYZ& xyz, const WhitePoint& whitepoint = D65);
XYZ labToXyz(const LAB& lab, const WhitePoint& whitepoint = D65);
XYZ lchToXyz(const LCH& lch, const WhitePoint& whitePoint = D65);
LAB lchToLab(const LCH& lch);
LCH labToLch(const LAB& lab);
LCH xyzToLch(const XYZ& xyz);
unsigned int rgbToArgb(const RGB& rgb, uint8_t a = 255);
RGB argbToRgb(unsigned int argb);
HCT lchToHct(const LCH& lch);
LCH hctToLch(const HCT& hct);
HCT rgbToHct(const RGB& rgb);
RGB hctToRgb(const HCT& hct);
LAB rgbToLab(const RGB& rgb);
RGB labToRgb(const LAB& lab);
LCH rgbToLch(const RGB& rgb);
RGB lchToRgb(const LCH& lch);
bool isValidRgb(const RGB& rgb);
bool isValidLab(const LAB& lab);
bool isValidLch(const LCH& lch);
RGB blend(const RGB& a, const RGB& b, double t);
LAB adjustLightness(const LAB& lab, double deltaL);
LCH adjustChroma(const LCH& lch, double deltaC);
LCH rotateHue(const LCH& lch, double degrees);
HCT lchToHctSimple(const LCH& lch);
LCH hctToLchSimple(const HCT& hct);
RGB hctToRgbSimple(const HCT& hct);
HCT rgbToHctSimple(const RGB& rgb);
RGB hct_rgb_gamutMap(const HCT& hct);

}   //namespace wallwatch
