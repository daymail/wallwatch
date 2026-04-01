#include "dynamiccolor.h"
#include "dynamicscheme.h"
#include "../../m3/contrast/contrast_curve.h"
#include "tonal_pair.h"

#include <functional>
#include <optional>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>

namespace wallwatch{
double ForegroundTone(double tone, double ratio){
    double lighterTone = LighterUnsafe(tone, ratio);
    double darkerTone = DarkerUnsafe(tone, ratio);
    double lighterRatio = RatioOfTones(lighterTone, tone);
    double darkerRatio = RatioOfTones(darkerTone, tone);
    double preferLighter = TonePrefersLightForeground(tone);

    if(preferLighter){
        double diff = (std::abs(lighterRatio - darkerRatio) < 0.1 && lighterRatio < ratio && darkerRatio < ratio);
        return lighterRatio >= ratio || lighterRatio >= darkerRatio || diff ? lighterTone : darkerTone;
    }else{
        return darkerRatio >= ratio || darkerRatio >= lighterRatio ? darkerTone : lighterTone;
    }
}

bool TonePrefersLightForeground(double tone) { return std::round(tone) < 60; }

bool ToneAllowsLightForeground(double tone) { return std::round(tone) <= 49; }

double EnableLightForeground(double tone){
  if (TonePrefersLightForeground(tone) && !ToneAllowsLightForeground(tone)) {
    return 49.0;
  }
  return tone;
}

DynamicColor::DynamicColor(
    std::string name,
    std::function<TonalPalette(const DynamicScheme&)> palette,
    std::function<double(const DynamicScheme&)> tone,
    bool is_background,
    std::optional<std::function<DynamicColor(const DynamicScheme&)>> background,
    std::optional<std::function<DynamicColor(const DynamicScheme&)>> second_background,
    std::optional<ContrastCurve> contrast_curve,
    std::optional<std::function<TonalPair(const DynamicScheme&)>> tonal_pair
):  name(name),
    palette(palette),
    tone(tone),
    is_background(is_background),
    background(background),
    second_background(second_background),
    contrast_curve(contrast_curve),
    tonal_pair(tonal_pair)
{}

DynamicColor DynamicColor::FromPalette(std::string name, std::function<TonalPalette(const DynamicScheme&)> palette, std::function<double(const DynamicScheme&)> tone){
    return DynamicColor(name, palette, tone, false, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
}

Argb DynamicColor::GetArgb(const DynamicScheme& scheme) {
  return palette(scheme).get(GetTone(scheme));
}

Hct DynamicColor::GetHct(const DynamicScheme& scheme) {
  return HCT(GetArgb(scheme));
}

double DynamicColor::GetTone(const DynamicScheme& scheme){
    bool decreasingContrast = scheme.contrastLevel < 0;
    if(tonal_pair!=std::nullopt){
        TonalPair tonal_pair = this->tonal_pair.value()(scheme);
        DynamicColor role_a = tonal_pair.role_a;
        DynamicColor role_b = tonal_pair.role_b;
        double delta = tonal_pair.delta;
        TonePolarity polarity = tonal_pair.polarity;
        bool stay_together = tonal_pair.stay_together;

        DynamicColor bg = background.value()(scheme);
        double bg_tone = bg.GetTone(scheme);

        bool a_isNear = (polarity == TonePolarity::kNearer || (polarity == TonePolarity::kDarker && scheme.is_dark) || (polarity == TonePolarity::kLighter && !scheme.is_dark));
        DynamicColor nearer = a_isNear ? role_a : role_b;
        DynamicColor farther = a_isNear ? role_b : role_a;
        bool am_nearer = this->name == nearer.name;
        double expansion_dir = scheme.is_dark ? 1 : -1;

        double n_contrast = nearer.contrast_curve.value().get(scheme.contrastLevel);
        double f_contrast = farther.contrast_curve.value().get(scheme.contrastLevel);

        double n_initial_tone = nearer.tone(scheme);
        double n_tone = RatioOfTones(bg_tone, n_initial_tone) >= n_contrast ? n_initial_tone : ForegroundTone(bg_tone, n_contrast);

        double f_initial_tone = farther.tone(scheme);
        double f_tone = RatioOfTones(bg_tone, f_initial_tone) >= f_contrast ? f_initial_tone: ForegroundTone(bg_tone, f_contrast);

        if(decreasingContrast){
            n_tone = ForegroundTone(bg_tone, n_contrast);
            f_tone = ForegroundTone(bg_tone, f_contrast);
        }
        if((f_tone - n_tone)*expansion_dir >=delta){
        }else{
            f_tone = std::clamp(n_tone + delta * expansion_dir, 0.0, 100.0);
            if((f_tone - n_tone)*expansion_dir >= delta){
            }else{
                n_tone = std::clamp(f_tone - delta * expansion_dir, 0.0, 100.0);
            }
        }

        if(50 <= n_tone && n_tone < 60){
            if(expansion_dir > 0){
                n_tone = 60;
                f_tone = std::max(f_tone, n_tone + delta * expansion_dir);
            }else{
                n_tone = 49;
                f_tone = std::min(f_tone, n_tone + delta * expansion_dir);
            }
        }else if(50 <= f_tone && f_tone < 60){
            if(stay_together){
                if(expansion_dir > 0){
                    n_tone = 60;
                    f_tone = std::max(f_tone, n_tone + delta * expansion_dir);
                }else{
                    n_tone = 49;
                    f_tone = std::min(f_tone, n_tone + delta * expansion_dir);
                }
            }else{
                if(expansion_dir > 0){
                    f_tone = 60;
                }else{
                    f_tone = 49;
                }
            }
        }
        return am_nearer ? n_tone : f_tone;
    }else{
        double ans = tone(scheme);
        if(background == std::nullopt){
            return ans;
        }
        double bg_tone = background.value()(scheme).GetTone(scheme);
        double desired_ratio = contrast_curve.value().get(scheme.contrastLevel);

        if(RatioOfTones(bg_tone, ans) >= desired_ratio){
        }else{
            ans = ForegroundTone(bg_tone, desired_ratio);
        }

        if(decreasingContrast){
            ans = ForegroundTone(bg_tone, desired_ratio);
        }

        if(is_background && 50 <= ans && ans < 60){
            if(RatioOfTones(49, bg_tone) >= desired_ratio){
                ans = 49;
            }else{
                ans = 60;
            }
        }

        if(second_background != std::nullopt){
            double bg_tone1 = background.value()(scheme).GetTone(scheme);
            double bg_tone2 = second_background.value()(scheme).GetTone(scheme);

            double upper = std::max(bg_tone1, bg_tone2);
            double lower = std::min(bg_tone1, bg_tone2);

            if(RatioOfTones(upper, ans) >= desired_ratio && RatioOfTones(lower, ans) >= desired_ratio){
                return ans;
            }

            double lightOpt = Lighter(upper, desired_ratio);
            double darkOpt = Darker(lower, desired_ratio);

            std::vector<double> availables;
            if(lightOpt != -1){
                availables.push_back(lightOpt);
            }
            if(darkOpt != -1){
                availables.push_back(darkOpt);
            }

            bool prefersLight = TonePrefersLightForeground(bg_tone1) || TonePrefersLightForeground(bg_tone2);
            if(prefersLight){
                return (lightOpt < 0) ? 100 : lightOpt;
            }
            if(availables.size() == 1){
                return availables[0];
            }
            return(darkOpt < 0) ? 0 : darkOpt;
        }
        return ans;
    }
}

}   //namespace wallwatch
