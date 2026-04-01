#pragma once

#include "../color_types.h"
#include "../../m3/hct/hct.h"

#include <vector>
#include <map>
#include <optional>

namespace wallwatch{
using namespace material_color_utilities;

class TemperatureCache{
  public:
    explicit TemperatureCache(HCT input);
    HCT GetComplement();
    std::vector<HCT> GetAnalogousColors();
    std::vector<HCT> GetAnalogousColors(int count, int divisions);
    double GetRelativeTemperature(HCT hct);
    static double RawTemperature(HCT color);

  private:
    HCT m_input;
    std::optional<HCT> m_precomputed_complement;
    std::optional<std::vector<Hct>> m_precomputed_hcts_by_temp;
    std::optional<std::vector<Hct>> m_precomputed_hcts_by_hue;
    std::optional<std::map<HCT, double>> m_precomputed_temps_by_hct;

    HCT GetColdest();
    HCT GetWarmest();

    static bool IsBetween(double angle, double a, double b);
    std::vector<HCT> GetHctsByHue();
    std::vector<HCT> GetHctsByTemp();
    std::map<HCT, double> GetTempsByHct();
};

}  //namespace wallwatch
