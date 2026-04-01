#include "temperature.h"
#include "../../m3/utils/utils.h"
#include "../lab.h"
#include "../../m3/hct/hct.h"
#include "../color_types.h"
#include "../color_const.h"

#include <algorithm>
#include <map>
#include <vector>

namespace wallwatch{
using namespace material_color_utilities;

TemperatureCache::TemperatureCache(HCT input) : m_input(input){}

HCT TemperatureCache::GetComplement(){
    if(m_precomputed_complement.has_value()){
        return m_precomputed_complement.value();
    }

    double coldest_hue = GetColdest().get_hue();
    double warmest_hue = GetWarmest().get_hue();
    double coldest_temp = GetTempsByHct().at(GetColdest());
    double warmest_temp = GetTempsByHct().at(GetWarmest());

    double range = warmest_temp - coldest_temp;
    bool start_hue_is_coldest_to_warmest = IsBetween(m_input.get_hue(), coldest_hue, warmest_hue);
    double start_hue = start_hue_is_coldest_to_warmest ? warmest_hue : coldest_hue;
    double end_hue = start_hue_is_coldest_to_warmest ? coldest_hue : warmest_hue;
    double smallest_error = 1000.0;
    double direction_of_rotation =1.0;

    HCT answer = GetHctsByHue().at((int)std::round(m_input.get_hue()));
    double complement_relative = (1.0 - GetRelativeTemperature(m_input));
    for(double hue_addend = 0.0; hue_addend <= 360.0; hue_addend += 1.0){
        double hue = SanitizeDegreesDouble(start_hue + direction_of_rotation * hue_addend);
        if(!IsBetween(hue, start_hue, end_hue)){
            continue;
        }
        HCT possible_answer = GetHctsByHue().at((int)std::round(hue));
        double relative_temp = (GetTempsByHct().at(possible_answer) - coldest_temp) / range;
        double error = std::abs(complement_relative - relative_temp);
        if(error < smallest_error){
            smallest_error = error;
            answer = possible_answer;
        }
    }
    m_precomputed_complement = answer;
    return m_precomputed_complement.value();
}

std::vector<HCT> TemperatureCache::GetAnalogousColors(){
    return GetAnalogousColors(5, 12);
}

std::vector<HCT> TemperatureCache::GetAnalogousColors(int count, int divisions){
    int start_hue =(int)std::round(m_input.get_hue());
    HCT start_hct = GetHctsByHue().at(start_hue);
    double last_temp = GetRelativeTemperature(start_hct);

    std::vector<HCT> all_colors;
    double abs_temp = 0.0;
    for(int i = 0;i<360;i++){
        int hue = SanitizeDegreesInt(start_hue + i);
        HCT hct = GetHctsByHue().at(hue);
        double temp = GetRelativeTemperature(hct);
        double temp_delta = std::abs(temp - last_temp);
        last_temp = temp;
        abs_temp += temp_delta;
    }

    int hue_addend = 1;
    double temp_step = abs_temp / (double)divisions;
    double total_temp_delta = 0.0;

    last_temp = GetRelativeTemperature(start_hct);
    while (all_colors.size() < static_cast<size_t>(divisions)){
        int hue = SanitizeDegreesInt(start_hue + hue_addend);
        HCT hct = GetHctsByHue().at(hue);
        double temp = GetRelativeTemperature(hct);
        double temp_delta = std::abs(temp - last_temp);
        total_temp_delta += temp_delta;

        double desired_total_temp_delta_for_index = (all_colors.size() * temp_step);
        bool index_satisfied = total_temp_delta >= desired_total_temp_delta_for_index;
        int index_addend = 1;
        while (index_satisfied && all_colors.size() < static_cast<size_t>(divisions)){
            all_colors.push_back(hct);
            desired_total_temp_delta_for_index = ((all_colors.size() + index_addend) * temp_step);
            index_satisfied = total_temp_delta >= desired_total_temp_delta_for_index;
            index_addend++;
        }
        last_temp = temp;
        hue_addend++;

        if (hue_addend > 360) {
        while (all_colors.size() < static_cast<size_t>(divisions)) {
            all_colors.push_back(hct);
        }
        break;
        }
    }

    std::vector<HCT> answers;
    answers.push_back(m_input);

    int ccw_count = (int)std::floor(((double)count - 1.0) / 2.0);
    for (int i = 1; i < (ccw_count + 1); i++) {
        int index = 0 - i;
        while (index < 0) {
        index = all_colors.size() + index;
        }
        if (static_cast<size_t>(index) >= all_colors.size()) {
            index = index % all_colors.size();
        }
        answers.insert(answers.begin(), all_colors.at(index));
    }

    int cw_count = count - ccw_count - 1;
    for (int i = 1; i < (cw_count + 1); i++) {
        size_t index = i;
        while (index < 0) {
        index = all_colors.size() + index;
        }
        if (index >= all_colors.size()) {
        index = index % all_colors.size();
        }
        answers.push_back(all_colors.at(index));
    }
    return answers;
}

double TemperatureCache::GetRelativeTemperature(HCT hct){
    double range = GetTempsByHct().at(GetWarmest()) - GetTempsByHct().at(GetColdest());
    double difference_from_coldest = GetTempsByHct().at(hct) - GetTempsByHct().at(GetColdest());
    if(range == 0.){return 0.5;}
    return difference_from_coldest / range;
}

double TemperatureCache::RawTemperature(HCT color){
    LAB lab = LabFromInt(color.ToInt());
    double hue = SanitizeDegreesDouble(std::atan2(lab.b, lab.a) * 180.0 / PI);
    double chroma = std::hypot(lab.a, lab.b);
    return -0.5 + 0.02 * std::pow(chroma, 1.07) * std::cos(SanitizeDegreesDouble(hue - 50.) * PI / 180);
}

HCT TemperatureCache::GetColdest() {return GetHctsByTemp().at(0);}

std::vector<HCT> TemperatureCache::GetHctsByHue(){
    if (m_precomputed_hcts_by_hue.has_value()) {
        return m_precomputed_hcts_by_hue.value();
    }
    std::vector<HCT> hcts;
    for(double hue = 0.; hue <= 360.; hue += 1.){
        HCT color_at_hue(hue, m_input.get_chroma(), m_input.get_tone());
        hcts.push_back(color_at_hue);
    }
    m_precomputed_hcts_by_hue = hcts;
    return m_precomputed_hcts_by_hue.value();
}

std::vector<HCT> TemperatureCache::GetHctsByTemp(){
    if (m_precomputed_hcts_by_temp.has_value()){
        return m_precomputed_hcts_by_temp.value();
    }

    std::vector<HCT> hcts(GetHctsByHue());
    hcts.push_back(m_input);
    std::map<HCT, double> temps_by_hct(GetTempsByHct());
        std::sort(hcts.begin(), hcts.end(),
        [temps_by_hct](const HCT a, const HCT b) -> bool {
            return temps_by_hct.at(a) < temps_by_hct.at(b);
        });
  m_precomputed_hcts_by_temp = hcts;
  return m_precomputed_hcts_by_temp.value();
}

std::map<HCT, double> TemperatureCache::GetTempsByHct() {
  if (m_precomputed_temps_by_hct.has_value()) {
    return m_precomputed_temps_by_hct.value();
  }

  std::vector<HCT> all_hcts(GetHctsByHue());
  all_hcts.push_back(m_input);

  std::map<HCT, double> temperatures_by_hct;
  for (HCT hct : all_hcts) {
    temperatures_by_hct[hct] = RawTemperature(hct);
  }

  m_precomputed_temps_by_hct = temperatures_by_hct;
  return m_precomputed_temps_by_hct.value();
}

HCT TemperatureCache::GetWarmest(){
    return GetHctsByTemp().at(GetHctsByTemp().size() - 1);
}

bool TemperatureCache::IsBetween(double angle, double a, double b){
    if(a < b){
        return a <= angle && angle <= b;
    }
    return a <= angle || angle <= b;
}


}  //namespace wallwatch
