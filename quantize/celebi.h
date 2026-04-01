#pragma once
#include "wsmeans.h"
#include "../m3/utils/utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <vector>

namespace wallwatch{
using namespace material_color_utilities;

QuantizerResult QuantizeCelebi(const std::vector<Argb>& pixels, uint16_t max_colors);

}  //namespace wallwatch x mcu
