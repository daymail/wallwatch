#include "blend.h"
#include "../m3/utils/utils.h"
#include "../m3/hct/viewing_conditions.h"
#include "../m3/hct/cam.h"
#include "../m3/hct/hct.h"

#include <algorithm>
#include <cstdint>

namespace wallwatch{
using namespace material_color_utilities;

Argb BlendHarmonize(const Argb design_color, const Argb key_color){
    HCT from_hct(design_color);
    HCT to_hct(key_color);
    double difference = DiffDegrees(from_hct.get_hue(), to_hct.get_hue());
    double rotation = std::min(difference * 0.5, 15.0);
    double output = SanitizeDegreesDouble(from_hct.get_hue() + rotation * RotationDirection(from_hct.get_hue(), to_hct.get_hue()));
    from_hct.set_hue(output);
    return from_hct.ToInt();
}

Argb BlendHctHue(const Argb from, const Argb to, const double amount){
    int ucs = BlendCam16Ucs(from, to, amount);
    HCT ucs_hct(ucs);
    HCT from_hct(from);
    from_hct.set_hue(ucs_hct.get_hue());
    return from_hct.ToInt();
}

Argb BlendCam16Ucs(const Argb from, const Argb to, const double amount){
    CAM from_cam = CamFromInt(from);
    CAM to_cam = CamFromInt(to);

    const double a_j = from_cam.jstar;
    const double a_a = from_cam.astar;
    const double a_b = from_cam.bstar;

    const double b_j = to_cam.jstar;
    const double b_a = to_cam.astar;
    const double b_b = to_cam.bstar;

    const double jstar = a_j + (b_j - a_j) * amount;
    const double astar = a_a + (b_a - a_a) * amount;
    const double bstar = a_b + (b_b - a_b) * amount;

    const CAM blended = CamFromUcsAndViewingConditions(jstar, astar, bstar, kDefaultViewingConditions);
    return IntFromCam(blended);
}

}   //namespace wallwatch
