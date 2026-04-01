#include "lab.h"
#include "color_const.h"
#include <math.h>
#include <iostream>

namespace wallwatch{

Argb IntFromLab(const LAB& lab) {
    double e = LAB_EPSILON;
    double kappa = LAB_KAPPA;
    double ke = 8.0;

    double fy = (lab.l + 16.0) / 116.0;
    double fx = (lab.a / 500.0) + fy;
    double fz = fy - (lab.b / 200.0);
    double fx3 = fx * fx * fx;
    double x_normalized = (fx3 > e) ? fx3 : (116.0 * fx - 16.0) / kappa;
    double y_normalized = (lab.l > ke) ? fy * fy * fy : (lab.l / kappa);
    double fz3 = fz * fz * fz;
    double z_normalized = (fz3 > e) ? fz3 : (116.0 * fz - 16.0) / kappa;
    double x = x_normalized * D65.x;
    double y = y_normalized * D65.y;
    double z = z_normalized * D65.z;

    double rL = XYZ_TO_RGB[0][0] * x + XYZ_TO_RGB[0][1] * y + XYZ_TO_RGB[0][2] * z;
    double gL = XYZ_TO_RGB[1][0] * x + XYZ_TO_RGB[1][1] * y + XYZ_TO_RGB[1][2] * z;
    double bL = XYZ_TO_RGB[2][0] * x + XYZ_TO_RGB[2][1] * y + XYZ_TO_RGB[2][2] * z;

    int red = material_color_utilities::Delinearized(rL);
    int green = material_color_utilities::Delinearized(gL);
    int blue = material_color_utilities::Delinearized(bL);

    return (0xFF << 24) | (red << 16) | (green << 8) | blue;
}

LAB LabFromInt(Argb argb) {
    int red = (argb >> 16) & 0xFF;
    int green = (argb >> 8) & 0xFF;
    int blue = argb & 0xFF;
    double red_l = material_color_utilities::Linearized(red);
    double green_l = material_color_utilities::Linearized(green);
    double blue_l = material_color_utilities::Linearized(blue);

    double x = RGB_TO_XYZ[0][0] * red_l + RGB_TO_XYZ[0][1] * green_l + RGB_TO_XYZ[0][2] * blue_l;
    double y = RGB_TO_XYZ[1][0] * red_l + RGB_TO_XYZ[1][1] * green_l + RGB_TO_XYZ[1][2] * blue_l;
    double z = RGB_TO_XYZ[2][0] * red_l + RGB_TO_XYZ[2][1] * green_l + RGB_TO_XYZ[2][2] * blue_l;

    double y_normalized = y / D65.y;
    double e = LAB_EPSILON;
    double kappa = LAB_KAPPA;

    double fy;
    if (y_normalized > e) {
        fy = std::pow(y_normalized, 1.0 / 3.0);
    } else {
        fy = (kappa * y_normalized + 16.0) / 116.0;
    }

    double x_normalized = x / D65.x;
    double fx;
    if (x_normalized > e) {
        fx = std::pow(x_normalized, 1.0 / 3.0);
    } else {
        fx = (kappa * x_normalized + 16.0) / 116.0;
    }

    double z_normalized = z / D65.z;
    double fz;
    if (z_normalized > e) {
        fz = std::pow(z_normalized, 1.0 / 3.0);
    } else {
        fz = (kappa * z_normalized + 16.0) / 116.0;
    }

    double l = 116.0 * fy - 16.0;
    double a = 500.0 * (fx - fy);
    double b = 200.0 * (fy - fz);

    return LAB{l, a, b};
}

} // namespace wallwatch
