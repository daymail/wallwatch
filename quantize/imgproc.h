#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "../colorspace/color_types.h"

namespace wallwatch{

std::vector<Argb> ExtractPixels(const std::string& path);
std::vector<Argb> ImageToPixels(const std::string& path);
std::vector<Argb> ImageToPixels(const std::string& path, int maxDimension);

}   //namespace wallwatch
