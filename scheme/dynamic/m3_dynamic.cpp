#include "m3_dynamic.h"
#include "dynamicscheme.h"
#include "../../colorspace/color_types.h"
#include "../../colorspace/color_conversion.h"
#include "../../m3/utils/utils.h"
#include "../dislike/dislike.h"
#include "tonal_pair.h"
#include <cmath>

namespace wallwatch{

bool IsFidelity(const DynamicScheme& scheme){
    return scheme.variant == Variant::kFidelity || scheme.variant == Variant::kContent;
}

bool IsMonochrome(const DynamicScheme& scheme){
    return scheme.variant == Variant::kMonochrome;
}

Vec3 XyzInViewingConditions(CAM cam, ViewingConditions conditions){
    double alpha = (cam.chroma == 0.0 || cam.j == 0.0) ? 0.0 : cam.chroma/std::sqrt(cam.j/100.0);
    double t = std::pow(alpha / std::pow(1.64 - std::pow(0.29, conditions.background_y_to_white_point_y), 0.73), 1.0 / 0.9);
    double h_rad = degToRad(cam.hue);
    double e_hue = 0.25 * (std::cos(h_rad + 2.0) + 3.8);
    double ac = conditions.aw * std::pow(cam.j/100.0, 1.0 / conditions.c/ conditions.z);

    double p1 = e_hue * (50000.0/13.0) * conditions.n_c * conditions.ncb;
    double p2 = (ac/conditions.nbb);

    double h_sin = std::sin(h_rad);
    double h_cos = std::cos(h_rad);

    double gamma = 23.0 * (p2 + 0.305) *t/(23.0 * p1 + 11 * t * h_cos + 108.0 * t * h_sin);
    double a = gamma * h_cos;
    double b = gamma * h_sin;
    double r_a = (460.0 * p2 + 451.0 * a + 288.0 * b) / 1403.0;
    double g_a = (460.0 * p2 - 891.0 * a - 261.0 * b) / 1403.0;
    double b_a = (460.0 * p2 - 220.0 * a - 6300.0 * b) / 1403.0;

    double r_c_base = std::fmax(0, (27.13 * std::fabs(r_a)) / (400.0 - std::fabs(r_a)));
    double r_c = Signum(r_a) * (100.0/conditions.fl) * std::pow(r_c_base, 1.0/0.42);

    double g_c_base = std::fmax(0, (27.13 * std::fabs(g_a)) / (400.0 - std::fabs(g_a)));
    double g_c = Signum(g_a) * (100.0/conditions.fl) * std::pow(g_c_base, 1.0/0.42);

    double b_c_base = std::fmax(0, (27.13 * std::fabs(b_a))/ (400.0 - std::fabs(b_a)));
    double b_c = Signum(b_a) * (100.0/conditions.fl) * std::pow(b_c_base, 1.0/0.42);

    double r_f = r_c / conditions.rgb_d[0];
    double g_f = g_c / conditions.rgb_d[1];
    double b_f = b_c / conditions.rgb_d[2];

    double x = 1.86206786 * r_f - 1.01125463 * g_f + 0.14918677 * b_f;
    double y = 0.38752654 * r_f + 0.62144744 * g_f - 0.00897398 * b_f;
    double z = -0.01584150 * r_f - 0.03412294 * g_f + 1.04996444 * b_f;
    return{x,y,z};
}

HCT InViewingConditions(HCT hct, ViewingConditions vc){
    CAM cam16 = CamFromInt(hct.ToInt());
    Vec3 viewed_in_vc = XyzInViewingConditions(cam16, vc);
    CAM recast_in_vc = CamFromXyzAndViewingConditions(viewed_in_vc.a, viewed_in_vc.b, viewed_in_vc.c, kDefaultViewingConditions);
    HCT recast_hct = Hct(recast_in_vc.hue, recast_in_vc.chroma, LstarFromY(viewed_in_vc.b));
    return recast_hct;
}

double FindDesiredChromaByTone(double hue, double chroma, double tone, bool decreasing_tone){
    double ans = tone;
    HCT closest_to_chroma = Hct(hue, chroma, tone);
    if(closest_to_chroma.get_chroma() < chroma){
        double chroma_peak = closest_to_chroma.get_chroma();
        while(closest_to_chroma.get_chroma() < chroma){
            ans += decreasing_tone ? -1.0 : 1.0;
            HCT potential_solution = Hct(hue, chroma, ans);
            if(chroma_peak > potential_solution.get_chroma()){
                break;
            }
            if(std::abs(potential_solution.get_chroma() - chroma) < 0.4){
                break;
            }
            double potential_delta = std::abs(potential_solution.get_chroma() - chroma);
            double current_delta = std::abs(closest_to_chroma.get_chroma() - chroma);
            if(potential_delta < current_delta){
                closest_to_chroma = potential_solution;
            }
            chroma_peak = std::fmax(chroma_peak, potential_solution.get_chroma());
        }
    }
    return ans;
}

constexpr double kContentAccentToneDelta = 15.0;
DynamicColor highestSurface(const DynamicScheme& s){
    return s.is_dark ? MaterialDynamicColors::SurfaceBright() : MaterialDynamicColors::SurfaceDim();
}

DynamicColor MaterialDynamicColors::PrimaryPaletteKeyColor(){
    return DynamicColor::FromPalette(
        "primary_palette_key_color",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return s.primary_palette.get_keyColor().get_tone();}
    );
}

DynamicColor MaterialDynamicColors::SecondaryPaletteKeyColor(){
    return DynamicColor::FromPalette(
        "secondary_palette_key_color",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return s.secondary_palette.get_keyColor().get_tone();}
    );
}

DynamicColor MaterialDynamicColors::TertiaryPaletteKeyColor(){
    return DynamicColor::FromPalette(
        "tertiary_palette_key_color",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{return s.tertiary_palette.get_keyColor().get_tone();}
    );
}

DynamicColor MaterialDynamicColors::NeutralPaletteKeyColor(){
    return DynamicColor::FromPalette(
        "neutral_palette_key_color",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.neutral_palette.get_keyColor().get_tone();}
    );
}

DynamicColor MaterialDynamicColors::NeutralVariantPaletteKeyColor(){
    return DynamicColor::FromPalette(
        "neutral_variant_palette_key_color",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_variant_palette;},
        [](const DynamicScheme& s) -> double{return s.neutral_variant_palette.get_keyColor().get_tone();}
    );
}

using std::nullopt;

DynamicColor MaterialDynamicColors::Background(){
    return DynamicColor(
        "background",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 6.0 : 98.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::OnBackground(){
    return DynamicColor(
        "on_background",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 90.0 : 10.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return Background();},
        nullopt,
        ContrastCurve(3.0, 3.0, 4.5, 7.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Surface(){
    return DynamicColor(
        "surface",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 6.0 : 98.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceDim(){
    return DynamicColor(
        "surface_dim",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 6.0 : ContrastCurve(87.0, 87.0, 80.0, 75.0).get(s.contrastLevel);},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceBright() {
    return DynamicColor(
        "surface_bright",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(24.0, 24.0, 29.0, 34.0).get(s.contrastLevel) : 98.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceContainerLowest() {
    return DynamicColor(
        "surface_container_lowest",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(4.0, 4.0, 2.0, 0.0).get(s.contrastLevel) : 100.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceContainerLow() {
    return DynamicColor(
        "surface_container_low",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(10.0, 10.0, 11.0, 12.0).get(s.contrastLevel) : ContrastCurve(96.0, 96.0, 96.0, 95.0).get(s.contrastLevel);},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceContainer() {
    return DynamicColor(
        "surface_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(12.0, 12.0, 16.0, 20.0).get(s.contrastLevel) : ContrastCurve(94.0, 94.0, 92.0, 90.0).get(s.contrastLevel);},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceContainerHigh() {
    return DynamicColor(
        "surface_container_high",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(17.0, 17.0, 21.0, 25.0).get(s.contrastLevel) : ContrastCurve(92.0, 92.0, 88.0, 85.0).get(s.contrastLevel);},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceContainerHighest() {
    return DynamicColor(
        "surface_container_highest",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? ContrastCurve(22.0, 22.0, 26.0, 30.0).get(s.contrastLevel) : ContrastCurve(90.0, 90.0, 84.0, 80.0).get(s.contrastLevel);},
        true, nullopt, nullopt, nullopt, nullopt
    );
}
DynamicColor MaterialDynamicColors::OnSurface(){
    return DynamicColor(
        "on_surface",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 90.0 : 10.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceVariant() {
    return DynamicColor(
        "surface_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_variant_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 30.0 : 90.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::OnSurfaceVariant() {
    return DynamicColor(
        "on_surface_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_variant_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 80.0 : 30.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::InverseSurface() {
    return DynamicColor(
        "inverse_surface",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 90.0 : 20.0;},
        false, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::InverseOnSurface() {
    return DynamicColor(
        "inverse_on_surface",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 20.0 : 95.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return InverseSurface();},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Outline() {
    return DynamicColor(
        "outline",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_variant_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 60.0 : 50.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.5, 3.0, 4.5, 7.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::OutlineVariant() {
    return DynamicColor(
        "outline_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_variant_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 30.0 : 80.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Shadow() {
    return DynamicColor(
        "shadow",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return 0.0;},
        false, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::Scrim() {
    return DynamicColor(
        "scrim",
        [](const DynamicScheme& s) -> TonalPalette{return s.neutral_palette;},
        [](const DynamicScheme& s) -> double{return 0.0;},
        false, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::SurfaceTint() {
    return DynamicColor(
        "surface_tint",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 80.0 : 40.0;},
        true, nullopt, nullopt, nullopt, nullopt
    );
}

DynamicColor MaterialDynamicColors::Primary() {
    return DynamicColor(
        "primary",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)){
                return s.is_dark ? 100.0 : 0.0;
            } return s.is_dark ? 80.0 : 40.0;
        },
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 7.0),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::PrimaryContainer(), MaterialDynamicColors::Primary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnPrimary() {
    return DynamicColor(
        "on_primary",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{
            if (IsMonochrome(s)) {
            return s.is_dark ? 10.0 : 90.0;
            }
            return s.is_dark ? 20.0 : 100.0;
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::Primary();},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::PrimaryContainer() {
    return DynamicColor(
        "primary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsFidelity(s)){
                return s.source.get_tone();
            }
            if(IsMonochrome(s)){
                return s.is_dark ? 85.0 : 25.0;
            }
                return s.is_dark ? 30.0 : 90.0;
        },
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::PrimaryContainer(), MaterialDynamicColors::Primary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnPrimaryContainer() {
    return DynamicColor(
        "on_primary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsFidelity(s)){
                return ForegroundTone(PrimaryContainer().tone(s), 4.5);
            }
            if(IsMonochrome(s)) {
                return s.is_dark ? 0.0 : 100.0;
            }
            return s.is_dark ? 90.0 : 30.0;
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::PrimaryContainer();},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::InversePrimary() {
    return DynamicColor(
        "inverse_primary",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 40.0 : 80.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return InverseSurface();},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 7.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Secondary() {
    return DynamicColor(
        "secondary",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 80.0 : 40.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 7.0),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::SecondaryContainer(), MaterialDynamicColors::Secondary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnSecondary() {
    return DynamicColor(
        "on_secondary",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)){
                return s.is_dark ? 10.0 : 100.0;
            }else{
                return s.is_dark ? 20.0 : 100.0;
            }
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::Secondary();},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::SecondaryContainer() {
    return DynamicColor(
        "secondary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{
            double initTone = s.is_dark ? 30.0 : 90.0;
            if(IsMonochrome(s)){
                return s.is_dark ? 30.0 : 85.0;
            }
            if(!IsFidelity(s)){
                return initTone;
            }
            return FindDesiredChromaByTone(s.secondary_palette.get_hue(), s.secondary_palette.get_chroma(), initTone, s.is_dark ? false : true);
        },
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::SecondaryContainer(), MaterialDynamicColors::Secondary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnSecondaryContainer() {
    return DynamicColor(
        "on_secondary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)) {
                return s.is_dark ? 90.0 : 10.0;
            }
            if(!IsFidelity(s)) {
                return s.is_dark ? 90.0 : 30.0;
            }
            return ForegroundTone(SecondaryContainer().tone(s), 4.5);
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::SecondaryContainer();},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Tertiary() {
    return DynamicColor(
        "tertiary",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{
            if (IsMonochrome(s)) {
                return s.is_dark ? 90.0 : 25.0;
            }
                return s.is_dark ? 80.0 : 40.0;
        },
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 7.0),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::TertiaryContainer(), MaterialDynamicColors::Tertiary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnTertiary() {
    return DynamicColor(
        "on_tertiary",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{
            if (IsMonochrome(s)) {
                return s.is_dark ? 10.0 : 90.0;
            }
                return s.is_dark ? 20.0 : 100.0;
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::Tertiary();},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::TertiaryContainer() {
    return DynamicColor(
        "tertiary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)){
                return s.is_dark ? 60.0 : 49.0;
            }
            if(!IsFidelity(s)){
                return s.is_dark ? 30.0 : 90.0;
            }
            HCT proposedHct = Hct(s.tertiary_palette.get(s.source.get_tone()));
            return FixIfDisliked(proposedHct).get_tone();
        },
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::TertiaryContainer(), MaterialDynamicColors::Tertiary(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnTertiaryContainer() {
    return DynamicColor(
        "on_tertiary_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)) {
                return s.is_dark ? 0.0 : 100.0;
            }
            if(!IsFidelity(s)) {
                return s.is_dark ? 90.0 : 30.0;
            }
            return ForegroundTone(TertiaryContainer().tone(s), 4.5);
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::TertiaryContainer();},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::Error() {
    return DynamicColor(
        "error",
        [](const DynamicScheme& s) -> TonalPalette{return s.error_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 80.0 : 40.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 7.0),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::ErrorContainer(), MaterialDynamicColors::Error(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnError() {
    return DynamicColor(
        "on_error",
        [](const DynamicScheme& s) -> TonalPalette{return s.error_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 20.0 : 100.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::Error();},
        nullopt,
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::ErrorContainer() {
    return DynamicColor(
        "error_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.error_palette;},
        [](const DynamicScheme& s) -> double{return s.is_dark ? 30.0 : 90.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::ErrorContainer(), MaterialDynamicColors::Error(), 10.0, TonePolarity::kNearer, false);
        }
    );
}

DynamicColor MaterialDynamicColors::OnErrorContainer() {
    return DynamicColor(
        "on_error_container",
        [](const DynamicScheme& s) -> TonalPalette{return s.error_palette;},
        [](const DynamicScheme& s) -> double{
            if(IsMonochrome(s)){
                return s.is_dark ? 90.0 : 10.0;
            }
                return s.is_dark ? 90.0 : 30.0;
        },
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::ErrorContainer();},
        nullopt,
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::PrimaryFixed() {
    return DynamicColor(
        "primary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 40.0 : 90.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::PrimaryFixed(), MaterialDynamicColors::PrimaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::PrimaryFixedDim() {
    return DynamicColor(
        "primary_fixed_dim",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 30.0 : 80.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::PrimaryFixed(), MaterialDynamicColors::PrimaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::OnPrimaryFixed() {
    return DynamicColor(
        "on_primary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 100.0 : 10.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::PrimaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::PrimaryFixed();},
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::OnPrimaryFixedVariant() {
    return DynamicColor(
        "on_primary_fixed_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.primary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 90.0 : 30.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::PrimaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::PrimaryFixed();},
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::SecondaryFixed() {
    return DynamicColor(
        "secondary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 80.0 : 90.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::SecondaryFixed(), MaterialDynamicColors::SecondaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::SecondaryFixedDim() {
    return DynamicColor(
        "secondary_fixed_dim",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 70.0 : 80.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::SecondaryFixed(), MaterialDynamicColors::SecondaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::OnSecondaryFixed() {
    return DynamicColor(
        "on_secondary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return 10.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::SecondaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::SecondaryFixed();},
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::OnSecondaryFixedVariant() {
    return DynamicColor(
        "on_secondary_fixed_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.secondary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 25.0 : 30.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::SecondaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::SecondaryFixed();},
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::TertiaryFixed() {
    return DynamicColor(
        "tertiary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 40.0 : 90.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::TertiaryFixed(), MaterialDynamicColors::TertiaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::TertiaryFixedDim() {
    return DynamicColor(
        "tertiary_fixed_dim",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 30.0 : 80.0;},
        true,
        [](const DynamicScheme& s) -> DynamicColor{return highestSurface(s);},
        nullopt,
        ContrastCurve(1.0, 1.0, 3.0, 4.5),
        [](const DynamicScheme& s) -> TonalPair{
            return TonalPair(MaterialDynamicColors::TertiaryFixed(), MaterialDynamicColors::TertiaryFixedDim(), 10.0, TonePolarity::kLighter, true);
        }
    );
}

DynamicColor MaterialDynamicColors::OnTertiaryFixed() {
    return DynamicColor(
        "on_tertiary_fixed",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 100.0 : 10.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::TertiaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::TertiaryFixed();},
        ContrastCurve(4.5, 7.0, 11.0, 21.0),
        nullopt
    );
}

DynamicColor MaterialDynamicColors::OnTertiaryFixedVariant() {
    return DynamicColor(
        "on_tertiary_fixed_variant",
        [](const DynamicScheme& s) -> TonalPalette{return s.tertiary_palette;},
        [](const DynamicScheme& s) -> double{return IsMonochrome(s) ? 90.0 : 30.0;},
        false,
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::TertiaryFixedDim();},
        [](const DynamicScheme& s) -> DynamicColor{return MaterialDynamicColors::TertiaryFixed();},
        ContrastCurve(3.0, 4.5, 7.0, 11.0),
        nullopt
    );
}
}
