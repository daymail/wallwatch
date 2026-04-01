#include "tone.h"

namespace wallwatch{

TonalPalette::TonalPalette(Argb argb):keyColor(0.0,0.0,0.0){
    CAM cam = material_color_utilities::CamFromInt(argb);
    m_hue = cam.hue;
    m_chroma = cam.chroma;
    keyColor = AnchorTone(cam.hue, cam.chroma).create();
}

TonalPalette::TonalPalette(HCT hct):keyColor(hct.get_hue(), hct.get_chroma(), hct.get_tone()){
    m_hue = hct.get_hue();
    m_chroma = hct.get_chroma();
}

TonalPalette::TonalPalette(double hue, double chroma):keyColor(hue, chroma, 0.0){
  m_hue = hue;
  m_chroma = chroma;
  keyColor = AnchorTone(hue, chroma).create();
}

TonalPalette::TonalPalette(double hue, double chroma, HCT key):keyColor(key.get_hue(), key.get_chroma(), key.get_tone()){
    m_hue = hue;
    m_chroma = chroma;
}

Argb TonalPalette::get(int tone) const{
  return material_color_utilities::IntFromHcl(m_hue, m_chroma, tone);
}

//NOTE: ANCHORTONE
AnchorTone::AnchorTone(double hue, double target_chroma):m_hue(hue), m_target_chroma(target_chroma){}

HCT AnchorTone::create(){
    const int pivot = 50;
    const int step = 1;
    const double epsilon = 0.01;

    int lowerTone = 0; int upperTone = 100;
    while(lowerTone<upperTone){
        const int midTone=(upperTone+lowerTone)/2;
        bool isAscending = max_chroma(midTone) < max_chroma(midTone+step);
        bool sufficientChroma = max_chroma(midTone)  >= m_target_chroma - epsilon;

        if(sufficientChroma){
            if(abs(lowerTone-pivot) < abs(upperTone-pivot)){
                upperTone = midTone;
            }else{
                if(lowerTone==midTone){
                    return HCT(m_hue, m_target_chroma, lowerTone);
                }
                lowerTone = midTone;
            }
        }else{
            if(isAscending){
                lowerTone = midTone + step;
            }else{
                upperTone = midTone;
            }
        }
    }
    return HCT(m_hue, m_target_chroma, lowerTone);
}

double AnchorTone::max_chroma(double tone){
    double chroma = HCT(m_hue, max_chroma_val, tone).get_chroma();
    return chroma;
}

}   //namespace wallwatch
