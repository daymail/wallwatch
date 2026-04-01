#pragma once
#include "dynamiccolor.h"

namespace wallwatch{
enum class TonePolarity{
    kDarker,
    kLighter,
    kNearer,
    kFarther
};

struct TonalPair{
    DynamicColor role_a;
    DynamicColor role_b;
    bool stay_together;
    double delta;
    TonePolarity polarity;

    TonalPair(DynamicColor a, DynamicColor b, double delta, TonePolarity polarity, bool together):
        role_a(a),
        role_b(b),
        delta(delta),
        polarity(polarity),
        stay_together(together)
        {}
};

}   //namespace wallwatch
