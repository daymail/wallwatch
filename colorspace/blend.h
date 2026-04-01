#pragma once
#include "color_types.h"
#include "../m3/utils/utils.h"
#include <cstdint>

namespace wallwatch{
using namespace material_color_utilities;

Argb BlendHarmonize(const Argb design_color, const Argb key_color);
Argb BlendHctHue(const Argb from, const Argb to, const double amount);
Argb BlendCam16Ucs(const Argb from, const Argb to, const double amount);

}   //namespace wallwatch
