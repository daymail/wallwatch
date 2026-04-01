#pragma once
#include "../colorspace/color_types.h"
namespace wallwatch{

class TonalPalette{
  public:
    explicit TonalPalette(Argb argb);
    TonalPalette(HCT);
    TonalPalette(double hue, double chroma);
    TonalPalette(double hue, double chroma, HCT key);

    Argb get(int tone) const;
    double get_hue() const{return m_hue;}
    double get_chroma() const{return m_chroma;}
    HCT get_keyColor() const{return keyColor;}

  private:
    double m_hue;
    double m_chroma;
    HCT keyColor;

};

//TODO: add caching to this class
class AnchorTone{
  public:
    explicit AnchorTone(double hue, double target_chroma);
    HCT create();

  private:
    double max_chroma(double tone);

    const double max_chroma_val = 200.0;
    double m_hue;
    double m_target_chroma;

};

}   //namespace wallwatch
