#include "dislike.h"
#include <cmath>

namespace wallwatch{

bool IsDisliked(HCT hct) {
  double roundedHue = std::round(hct.get_hue());

  bool hue_passes = roundedHue >= 90.0 && roundedHue <= 111.0;
  bool chroma_passes = std::round(hct.get_chroma()) > 16.0;
  bool tone_passes = std::round(hct.get_tone()) < 65.0;

  return hue_passes && chroma_passes && tone_passes;
}

HCT FixIfDisliked(HCT hct) {
  if (IsDisliked(hct)) {
    return HCT(hct.get_hue(), hct.get_chroma(), 70.0);
  }

  return hct;
}
}   //namespace wallwatch
