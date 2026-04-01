#pragma once

#include "../m3/utils/utils.h"
#include "color_types.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_set>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <vector>

namespace wallwatch{

Argb IntFromLab(const LAB& lab);
LAB LabFromInt(const Argb argb);

}  // namespace wallwatch

