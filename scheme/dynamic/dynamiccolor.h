#pragma once
#include "../../m3/contrast/contrast.h"
#include "../../m3/contrast/contrast_curve.h"
#include "../tone.h"
#include "../../colorspace/color_types.h"

#include <string>
#include <functional>
#include <optional>

namespace wallwatch{
struct TonalPair;
struct TonalPalette;
struct DynamicScheme;

double ForegroundTone(double tone, double ratio);
bool TonePrefersLightForeground(double tone);
bool ToneAllowsLightForeground(double tone);
double EnableLightForeground(double tone);

using namespace material_color_utilities;
struct DynamicColor{
    std::string name;
    std::function<TonalPalette(const DynamicScheme&)> palette;
    std::function<double(const DynamicScheme&)> tone;
    bool is_background;

    std::optional<std::function<DynamicColor(const DynamicScheme&)>> background;
    std::optional<std::function<DynamicColor(const DynamicScheme&)>> second_background;
    std::optional<ContrastCurve> contrast_curve;
    std::optional<std::function<TonalPair(const DynamicScheme&)>> tonal_pair;

    DynamicColor(
        std::string name,
        std::function<TonalPalette(const DynamicScheme&)> palette,
        std::function<double(const DynamicScheme&)> tone,
        bool is_background,

        std::optional<std::function<DynamicColor(const DynamicScheme&)>> background,
        std::optional<std::function<DynamicColor(const DynamicScheme&)>> second_background,
        std::optional<ContrastCurve> contrast_curve,
        std::optional<std::function<TonalPair(const DynamicScheme&)>> tonal_pair
    );

    static DynamicColor FromPalette(std::string name, std::function<TonalPalette(const DynamicScheme&)> palette, std::function<double(const DynamicScheme&)> tone);
    Argb GetArgb(const DynamicScheme& scheme);
    HCT GetHct(const DynamicScheme& scheme);
    double GetTone(const DynamicScheme& scheme);
};

}
