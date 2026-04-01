#pragma once

#include "../utils/utils.h"
#include <cstdlib>
#include <map>
#include <vector>

namespace wallwatch{
using namespace material_color_utilities;

struct ScoreOptions{
    size_t desired = 4;
    int fallback_color_argb = 0xFF0000FF;
    bool filter = true;
};

std::vector<Argb> RankedSuggestions(const std::map<Argb, uint32_t>& argb_to_population, const ScoreOptions& options = {});

}   //namespace wallwatch x mcu
