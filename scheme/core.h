#pragma once
#include "tone.h"

namespace wallwatch{
typedef struct{
    TonalPalette primary;
    TonalPalette secondary;
    TonalPalette tertiary;
    TonalPalette neutral;
    TonalPalette neutralVariant;
} CorePalette;

enum class Variant{
    Vibrant,
    TonalSpot,
    Neutral,
    Monochrome,
    Expressive,
    Content
};

}
