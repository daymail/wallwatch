#pragma once
#include "../m3/utils/utils.h"

#include <stdint.h>
#include <vector>
namespace wallwatch{
using namespace material_color_utilities;
std::vector<Argb> QuantizeWu(const std::vector<Argb>& pixels, uint16_t max);
}  //namespace wallwatch
