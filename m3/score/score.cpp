#include "score.h"
#include "../../colorspace/color_types.h"

#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#include <cstdint>

namespace wallwatch{
using namespace material_color_utilities;
constexpr double kTargetChroma = 48.0;
constexpr double kWeightProportion = 0.7;
constexpr double kWeightChromaAbove = 0.3;
constexpr double kWeightChromaBelow = 0.1;
constexpr double kCutoffChroma = 5.0;
constexpr double kCutoffExcitedProportion = 0.01;

bool CompareStoredHCT(const std::pair<HCT, double>& a, const std::pair<HCT, double>& b){
    return a.second > b.second;
}

std::vector<Argb> RankedSuggestions(const std::map<Argb, uint32_t>& argb_to_population, const ScoreOptions& options){
    std::vector<HCT> color_hct;
    std::vector<uint32_t> hue_population(360, 0);

    double population_sum = 0;
    for (const auto& [argb, population] : argb_to_population){
        HCT hct(argb);
        color_hct.push_back(hct);
        int hue = floor(hct.get_hue());
        hue_population[hue] += population;
        population_sum += population;
    }

    std::vector<double> hue_excited_proportions(360, 0.0);
    for (int hue = 0; hue < 360; hue++) {
        double proportion = hue_population[hue] / population_sum;
        for (int i = hue - 14; i < hue + 16; i++) {
        int neighbor_hue = SanitizeDegreesInt(i);
        hue_excited_proportions[neighbor_hue] += proportion;
        }
    }

    std::vector<std::pair<HCT, double>> scored_hcts;
    for(HCT hct : color_hct){
        int hue = SanitizeDegreesInt(std::round(hct.get_hue()));
        double proportion = hue_excited_proportions[hue];
        if(options.filter && (hct.get_chroma() < kCutoffChroma || proportion <= kCutoffExcitedProportion)){
            continue;
        }

        double proportion_score = proportion * 100.0 * kWeightProportion;
        double chroma_weight = hct.get_chroma() < kTargetChroma ? kWeightChromaBelow : kWeightChromaAbove;
        double chroma_score = (hct.get_chroma() - kTargetChroma) * chroma_weight;
        double score = proportion_score + chroma_score;
        scored_hcts.push_back({hct, score});
    }
    std::sort(scored_hcts.begin(), scored_hcts.end(), CompareStoredHCT);

    std::vector<HCT> chosen_colors;
    for(int difference_degrees = 90; difference_degrees >= 15; difference_degrees--){
        chosen_colors.clear();
        for(auto entry : scored_hcts){
            HCT hct = entry.first;
            auto duplicate_hue = std::find_if(chosen_colors.begin(), chosen_colors.end(), [&hct, difference_degrees](HCT chosen_hct){
                    return DiffDegrees(hct.get_hue(), chosen_hct.get_hue()) < difference_degrees;
            });
            if(duplicate_hue == chosen_colors.end()){
                chosen_colors.push_back(hct);
                if(chosen_colors.size() >= options.desired) break;
            }
        }
        if(chosen_colors.size() >= options.desired) break;
    }

    std::vector<Argb> colors;
    if(chosen_colors.empty()){
        colors.push_back(options.fallback_color_argb);
    }
    for(auto chosen_hct : chosen_colors){
        colors.push_back(chosen_hct.ToInt());
    }
    return colors;
}

}  //namespace wallwatch x mcu
