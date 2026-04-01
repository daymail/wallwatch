#pragma once
#include <array>

namespace wallwatch{

constexpr double PI = 3.141592653589793238462643383279502884;
// D65 Illuminant (standard daylight) - XYZ coordinates
struct WhitePoint{
    double x;
    double y;
    double z;
};

constexpr WhitePoint D65{95.047, 100.0, 108.883};
constexpr WhitePoint D55{95.682, 100.0, 92.149};
constexpr WhitePoint D75{94.972, 100.0, 122.638};

// sRGB to XYZ conversion matrix (row-major)
constexpr std::array<std::array<double, 3>, 3> RGB_TO_XYZ = {{
    {0.4124564,  0.3575761,  0.1804375},
    {0.2126729,  0.7151522,  0.0721750},
    {0.0193339,  0.1191920,  0.9503041}
}};

// XYZ to sRGB conversion matrix (inverse of above)
constexpr std::array<std::array<double, 3>, 3> XYZ_TO_RGB = {{
    { 3.2404542, -1.5371385, -0.4985314},
    {-0.9692660,  1.8760108,  0.0415560},
    { 0.0556434, -0.2040259,  1.0572252}
}};

// Gamma correction constants (sRGB)
constexpr double GAMMA_CUTOFF = 0.04045;      // Linear threshold
constexpr double GAMMA_SLOPE = 12.92;         // Linear segment slope
constexpr double GAMMA_A = 1.055;             // Exponential segment multiplier
constexpr double GAMMA_EXPONENT = 2.4;        // Exponential segment power

// CIE LAB constants
constexpr double LAB_EPSILON = 216.0/24389.0;      // (6/29)^3, threshold for f(t)
constexpr double LAB_KAPPA = 24389.0/27.0;           // (29/3)^3, scaling factor

// Clamping limits
struct RGBLimits{
    static constexpr double MIN = 0.0;
    static constexpr double MAX = 1.0;
};

struct XYZLimits{
    static constexpr double Y_MIN = 0.0;
    static constexpr double Y_MAX = 100.0;
};

struct LABLimits{
    static constexpr double L_MIN = 0.0;
    static constexpr double L_MAX = 100.0;
    static constexpr double A_MIN = -128.0;
    static constexpr double A_MAX = 127.0;
    static constexpr double B_MIN = -128.0;
    static constexpr double B_MAX = 127.0;
};

struct LCHLimits{
    static constexpr double L_MIN = 0.0;
    static constexpr double L_MAX = 100.0;
    static constexpr double C_MIN = 0.0;
    static constexpr double C_MAX = 150.0;    // Approximate max chroma
    static constexpr double H_MIN = 0.0;
    static constexpr double H_MAX = 360.0;
};

struct HCTLimits{
    static constexpr double H_MIN = 0.0;
    static constexpr double H_MAX = 360.0;
    static constexpr double C_MIN = 0.0;
    static constexpr double C_MAX = 150.0;    // Approximate max chroma
    static constexpr double T_MIN = 0.0;
    static constexpr double T_MAX = 100.0;
};

struct AdobeRGBToXYZ{
    static constexpr std::array<std::array<double, 3>, 3> MATRIX = {{
        {0.5767309, 0.1855540, 0.1881852},
        {0.2973769, 0.6273491, 0.0752741},
        {0.0270343, 0.0706872, 0.9911085}
    }};
};

struct DisplayP3ToXYZ{
    static constexpr std::array<std::array<double, 3>, 3> MATRIX = {{
        {0.515102, 0.291965, 0.157153},
        {0.241168, 0.692251, 0.066581},
        {-0.001049, 0.041881, 0.784378}
    }};
};

} //namespace wallwatch
