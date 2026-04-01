#pragma once
#include <cmath>
#include "color_const.h"
#include "color_types.h"

namespace wallwatch{
//Gamma-correction
inline double linearize(double gamma){
    if(gamma <= GAMMA_CUTOFF){
        return gamma/GAMMA_SLOPE;
    }else{
        return std::pow((gamma + (GAMMA_A - 1)) / GAMMA_A, GAMMA_EXPONENT);
    }
}

inline double compress(double linear){
    if(linear <= GAMMA_CUTOFF/ GAMMA_SLOPE){
        return linear*GAMMA_SLOPE;
    }else{
        return GAMMA_A * std::pow(linear, 1.0/GAMMA_EXPONENT) - (GAMMA_A - 1.0);
    }
}

//Clamp
inline double clamp(double value, double min, double max){
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

inline RGB clamp(const RGB& color){
    return RGB{
        clamp(color.r, RGBLimits::MIN, RGBLimits::MAX),
        clamp(color.g, RGBLimits::MIN, RGBLimits::MAX),
        clamp(color.b, RGBLimits::MIN, RGBLimits::MAX)
    };
}

inline LAB clamp(const LAB& color){
    return LAB{
        clamp(color.l, LABLimits::L_MIN, LABLimits::L_MAX),
        clamp(color.a, LABLimits::A_MIN, LABLimits::A_MAX),
        clamp(color.b, LABLimits::B_MIN, LABLimits::B_MAX)
    };
}

inline double clampHue(double hue){
    hue = std::fmod(hue, 360.0);
    if(hue < 0.0) hue +=360.0;
    return hue;
}

inline LCH clamp(const LCH& color){
    return LCH{
        clamp(color.l, LCHLimits::L_MIN, LCHLimits::L_MAX),
        clamp(color.c, LCHLimits::C_MIN, LCHLimits::C_MAX),
        clampHue(color.h)
    };
}

inline HCT clamp(const HCT& color){
    return HCT{
        clampHue(color.get_hue()),
        clamp(color.get_chroma() , HCTLimits::C_MIN, HCTLimits::C_MAX),
        clamp(color.get_tone() , HCTLimits::T_MIN, HCTLimits::T_MAX)
    };
}

inline XYZ clamp(const XYZ& color, bool clampXZ = false){
    if(clampXZ){
        return XYZ{
            clamp(color.x, 0.0, 150.0),
            clamp(color.y, XYZLimits::Y_MIN, XYZLimits::Y_MAX),
            clamp(color.z, 0.0, 150.0)
        };
    }
    return XYZ{
        color.x,
        clamp(color.y, XYZLimits::Y_MIN, XYZLimits::Y_MAX),
        color.y
    };
}

inline double f(double t){
    if(t > LAB_EPSILON){
        return std::cbrt(t);
    }else{
        return (LAB_KAPPA*t + 16.0)/116.0;
    }
}
//Compesses the dark end of the scale to match human perception
inline double f_inv(double t){
    const double delta = 6.0/29.0;
    if(t > delta){
        return t*t*t;
    }else{
        return (116.0*t - 16)/LAB_KAPPA;
    }
}

inline constexpr double degToRad(double deg){
    return deg * PI /180.0;
}

inline constexpr double radToDeg(double rad){
    return rad * 180.0/PI;
}

//Float-point tolerance lvl
inline bool isNear(double a, double b, double maxAbsDiff = 1e-9, double maxRelDiff = 1e-9){
    if(a==b) return true;
    double absDiff = std::fabs(a-b);//for values near 0
    if(absDiff <= maxAbsDiff) return true;

    double maxVal = std::max(std::fabs(a), std::fabs(b)); //large values
    if(maxVal > 0.0){
        double relDiff = absDiff/maxVal;
        if(relDiff <= maxRelDiff) return true;
    }
    return false;
}

inline bool isNearAbs(double a, double b, double tolerance = 1e-9){
    return std::fabs(a-b) <= tolerance;
}

inline bool isNearRel(double a, double b, double tolerance = 1e-9){
    double maxVal = std::max(std::fabs(a), std::fabs(b));
    if(maxVal == 0.0) return true;
    return std::fabs(a-b)/maxVal <= tolerance;
}

}   //namespace wallwatch


