#pragma once
#include "dynamiccolor.h"
#include "../tone.h"
#include "variants.h"

#include <optional>
#include <vector>

namespace wallwatch{
struct DynamicScheme{
    HCT source;
    Variant variant;
    double contrastLevel;
    bool is_dark;
    TonalPalette primary_palette;
    TonalPalette secondary_palette;
    TonalPalette tertiary_palette;
    TonalPalette neutral_palette;
    TonalPalette neutral_variant_palette;
    TonalPalette error_palette;

    DynamicScheme(HCT source, Variant variant, double contrastLevel, bool is_dark,
            TonalPalette primary_palette,
            TonalPalette secondary_palette,
            TonalPalette tertiary_palette,
            TonalPalette neutral_palette,
            TonalPalette neutral_variant_palette,
            std::optional<TonalPalette> error_palette = std::nullopt);

    static double GetRotatedHue(HCT source, std::vector<double> hues, std::vector<double> rotations);

    Argb SourceColorArgb() const;
    Argb GetPrimaryPaletteKeyColor() const;
    Argb GetSecondaryPaletteKeyColor() const;
    Argb GetTertiaryPaletteKeyColor() const;
    Argb GetNeutralPaletteKeyColor() const;
    Argb GetNeutralVariantPaletteKeyColor() const;
    Argb GetBackground() const;
    Argb GetOnBackground() const;
    Argb GetSurface() const;
    Argb GetSurfaceDim() const;
    Argb GetSurfaceBright() const;
    Argb GetSurfaceContainerLowest() const;
    Argb GetSurfaceContainerLow() const;
    Argb GetSurfaceContainer() const;
    Argb GetSurfaceContainerHigh() const;
    Argb GetSurfaceContainerHighest() const;
    Argb GetOnSurface() const;
    Argb GetSurfaceVariant() const;
    Argb GetOnSurfaceVariant() const;
    Argb GetInverseSurface() const;
    Argb GetInverseOnSurface() const;
    Argb GetOutline() const;
    Argb GetOutlineVariant() const;
    Argb GetShadow() const;
    Argb GetScrim() const;
    Argb GetSurfaceTint() const;
    Argb GetPrimary() const;
    Argb GetOnPrimary() const;
    Argb GetPrimaryContainer() const;
    Argb GetOnPrimaryContainer() const;
    Argb GetInversePrimary() const;
    Argb GetSecondary() const;
    Argb GetOnSecondary() const;
    Argb GetSecondaryContainer() const;
    Argb GetOnSecondaryContainer() const;
    Argb GetTertiary() const;
    Argb GetOnTertiary() const;
    Argb GetTertiaryContainer() const;
    Argb GetOnTertiaryContainer() const;
    Argb GetError() const;
    Argb GetOnError() const;
    Argb GetErrorContainer() const;
    Argb GetOnErrorContainer() const;
    Argb GetPrimaryFixed() const;
    Argb GetPrimaryFixedDim() const;
    Argb GetOnPrimaryFixed() const;
    Argb GetOnPrimaryFixedVariant() const;
    Argb GetSecondaryFixed() const;
    Argb GetSecondaryFixedDim() const;
    Argb GetOnSecondaryFixed() const;
    Argb GetOnSecondaryFixedVariant() const;
    Argb GetTertiaryFixed() const;
    Argb GetTertiaryFixedDim() const;
    Argb GetOnTertiaryFixed() const;
    Argb GetOnTertiaryFixedVariant() const;
};
} //namespace wallwatch
