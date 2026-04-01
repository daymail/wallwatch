#pragma once
#include "../m3/utils/utils.h"

#include <stdint.h>
#include <vector>
#include <map>
namespace wallwatch{
using namespace material_color_utilities;

struct QuantizerResult{
    std::map<Argb, uint32_t> color_to_count;
    std::map<Argb, Argb> input_pixel_to_cluster_pixel;
};

QuantizerResult QuantizeWsmeans(const std::vector<Argb>& input_pixels, const std::vector<Argb>& starting_clusters, uint16_t max_colors);

}  //namespace wallwatch x mcu
