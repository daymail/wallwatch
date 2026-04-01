#include "celebi.h"
#include "wu.h"
#include "wsmeans.h"
#include "../m3/utils/utils.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace wallwatch{
using namespace material_color_utilities;

QuantizerResult QuantizeCelebi(const std::vector<Argb>& pixels, uint16_t max_colors){
    if(max_colors == 0 || pixels.empty()){
        return QuantizerResult();
    }
    if(max_colors > 256){
        max_colors = 256;
    }
    int pixel_count = pixels.size();

    std::vector<Argb> opaque_pixels;
    opaque_pixels.reserve(pixel_count);
    for(int i = 0;i<pixel_count;i++){
        int pixel = pixels[i];
        if(!IsOpaque(pixel)){
            continue;
        }
        opaque_pixels.push_back(pixel);
    }
    std::vector<Argb> wu_result = QuantizeWu(opaque_pixels, max_colors);
    QuantizerResult result = QuantizeWsmeans(opaque_pixels, wu_result, max_colors);
    return result;
}

}  //namespace wallwatch x mcu
