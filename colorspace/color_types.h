#pragma once
#include "../m3/hct/hct.h"
#include "../m3/hct/cam.h"
#include <cstdint>

namespace wallwatch{
struct RGB_U8{uint8_t r,g,b;};
struct RGBA{double r,g,b,a;};
struct RGB {double r,g,b;};
struct XYZ {double x,y,z;};
struct LAB{
  double l = 0.0;
  double a = 0.0;
  double b = 0.0;
  double DeltaE(const LAB& lab){
    double d_l = l - lab.l;
    double d_a = a - lab.a;
    double d_b = b - lab.b;
    return (d_l * d_l) + (d_a * d_a) + (d_b * d_b);
  }
  std::string ToString() {
    return "Lab: L* " + std::to_string(l) + " a* " + std::to_string(a) +
           " b* " + std::to_string(b);
  }
};

struct LCH {double l,c,h;};
typedef material_color_utilities::Argb Argb;
typedef material_color_utilities::Hct HCT;
typedef material_color_utilities::Cam CAM;
}   //namespace wallwatch

