#pragma once
#include "scheme.h"
#include "../colorspace/color_types.h"
#include "../colorspace/tone.h"

namespace wallwatch{

enum class Theme{
    Light,
    Dark
};

enum class ContrastLevel{
    Standard,
    Medium,
    High
};

struct ColorRoles{
    RGB primary;
    RGB onPrimary;
    RGB primaryContainer;
    RGB onPrimaryContainer;

    RGB secondary;
    RGB onSecondary;
    RGB secondaryContainer;
    RGB onSecondaryContainer;

    RGB tertiary;
    RGB onTertiary;
    RGB tertiaryContainer;
    RGB onTertiaryContainer;

    RGB error;
    RGB onError;
    RGB errorContainer;
    RGB onErrorContainer;

    RGB surface;
    RGB onSurface;
    RGB surfaceVariant;
    RGB onSurfaceVariant;
    RGB surfaceContainerLowest;
    RGB surfaceContainerLow;
    RGB surfaceContainer;
    RGB surfaceContainerHigh;
    RGB surfaceContainerHighest;

    RGB inverseSurface;
    RGB inverseOnSurface;
    RGB inversePrimary;

    RGB outline;
    RGB outlineVariant;

    RGB primaryFixed;
    RGB primaryFixedDim;
    RGB secondaryFixed;
    RGB secondaryFixedDim;
    RGB tertiaryFixed;
    RGB tertiaryFixedDim;
};

}   //namespace wallwatch

