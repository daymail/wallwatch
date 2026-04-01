#pragma once

#include "../colorspace/color_types.h"

namespace wallwatch{

enum class SchemeStyle{
    TonalSpot,
    Expressive,
    Fidelity,
    Vibrant,
    FruitSalad,
    Monochrome,
    Neutral,
    Rainbow

};

RGB primaryKey(const RGB& source);
RGB secondaryKey(const RGB& source);
RGB tertiaryKey(const RGB& source);
RGB neutralKey(const RGB& source);
RGB neutralVariantKey(const RGB& source);

Variant generateKeyColors(const RGB& source, SchemeStyle style = SchemeStyle::Vibrant);
}   //namespace wallwatch
