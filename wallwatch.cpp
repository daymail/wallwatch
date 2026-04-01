#include <iostream>
#include <cassert>
#include <iomanip>
#include "scheme/tone.h"
#include "colorspace/colorspace.h"
#include "colorspace/lab.h"
#include "quantize/wu.h"

using namespace wallwatch;

void print(double i);

void testExtremeGamutMapping() {
    material_color_utilities::Hct extreme(300.0, 200.0, 30.0);
    RGB rgb = hct_rgb_gamutMap(extreme);
    assert(isValidRgb(rgb));
    std::cout << "Mapped RGB: " << rgb.r << ", " << rgb.g << ", " << rgb.b << std::endl;
}

int main(int argc, char* argv[]){
    testExtremeGamutMapping();
    std::cout << "=====================================>TESTS: " << std::endl;
    double srgb = 0.5;
    double linear_val = wallwatch::linearize(srgb);
    double compressed_val = wallwatch::compress(linear_val);
    std::cout << "Original: " << srgb << std::endl;
    std::cout << "Linear: " << linear_val << std::endl;
    std::cout << "Compressed(sRGB): " << compressed_val << std::endl;

    wallwatch::RGB input{1.2,0.34,0.9};
    wallwatch::RGB one = wallwatch::clamp(input);
    print(one.r);

    RGB_U8 nrgb{212,99,31};
    RGB rgb = rgbU8ToRgb(nrgb);
    unsigned int argb = rgbToArgb(rgb);
    material_color_utilities::Hct hct3 = material_color_utilities::Hct(argb);
    std::cout << "RGB: (" << (int)nrgb.r << ", " << (int)nrgb.g << ", " << (int)nrgb.b << ")" << std::endl;
    std::cout << "(Converted) RGB: (" << rgb.r << ", " << rgb.g << ", " << rgb.b << ")" << std::endl;
    std::cout << "HCT: (Hue=" << hct3.get_hue() << ", Chroma=" << hct3.get_chroma() << ", Tone=" << hct3.get_tone() << ")\n";

    XYZ xyz = rgbToXyz(rgb);
    xyz.x *= 100;
    xyz.y *= 100;
    xyz.z *= 100;
    LAB lab = xyzToLab(xyz);
    LCH lch = labToLch(lab);
    material_color_utilities::Hct hct = lchToHct(lch);
    LCH lch2hct = hctToLch(hct);
    std::cout << "RGB: (" << rgb.r << ", " << rgb.g << ", " << rgb.b << ")\n";
    std::cout << "XYZ: (" << xyz.x << ", " << xyz.y << ", " << xyz.z << ")\n";
    std::cout << "LAB: (" << lab.l << ", " << lab.a << ", " << lab.b << ")\n";
    std::cout << "LCH: (L=" << lch.l << ", C=" << lch.c << ", H=" << lch.h << ")\n";
    std::cout << "HCT: (Hue=" << hct.get_hue() << ", Chroma=" << hct.get_chroma() << ", Tone=" << hct.get_tone() << ")\n";

    RGB hct2rgb = hctToRgb(hct);
    std::cout << "HCT -> RGB: (" << hct2rgb.r << ", " << hct2rgb.g << ", " << hct2rgb.b << ")\n";

    auto two = wallwatch::clamp(wallwatch::RGB{0.2,-0.3,1.0});
    print(two.g);

    LCH original{53.585, 0.0, 0.0};  // Middle gray
    auto hct2 = lchToHct(original);
    LCH result = hctToLch(hct2);

    std::cout << "Original LCH: " << original.l << ", " << original.c << ", " << original.h << std::endl;
    std::cout << "Round trip:    " << result.l << ", " << result.c << ", " << result.h << std::endl;

    std::cout << "=========================================================================" << std::endl;
    Argb argb2 = IntFromLab(lab);
    std::cout << "ARGB FROM LAB: 0x" << std::hex << argb2 << std::dec << std::endl;
    LAB lab2 = LabFromInt(argb2);
    std::cout << "LAB from ARGB: " << lab2.l << ", " << lab2.a << ", " << lab2.b << ")\n";
    return 0;
}

void print(double i){
    std::cout << i << std::endl;
}

