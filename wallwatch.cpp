#include <QCoreApplication>
#include <iostream>
#include <QObject>
#include <QString>

#include "themer.h"
#include "filewatcher.h"
#include "m3/score/score.h"
#include "m3/hct/hct.h"
#include "quantize/imgproc.h"
#include "quantize/celebi.h"
#include "colorspace/color_types.h"
#include "colorspace/colorspace.h"
#include "scheme/dynamic/dynamicscheme.h"
#include "scheme/variants/scheme_tonal_spot.h"

using namespace wallwatch;

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);

    WallWatch watcher;
    QStringList wallpapers = watcher.getWallpapers();
    if (wallpapers.isEmpty()){
        std::cout << "No wallpaper found" << std::endl;
        return 1;
    }

    Themer* myThemer = new Themer();
    auto info = watcher.getWallpaperInfo(1);
    std::string path = info.path.toStdString();

    std::vector<Argb> pixels;
    if(info.isVideo){
        pixels = wallwatch::ExtractPixels(path);
        std::cout << "used MEEEEEE" << std::endl;
    }else{pixels = wallwatch::ImageToPixels(path);}

    QuantizerResult pixelColors = QuantizeCelebi(pixels, 128);
    std::vector<Argb> ranked = RankedSuggestions(pixelColors.color_to_count);

    if(!ranked.empty()){
        Argb seed = ranked[0];
        HCT source = Hct(seed);
        bool isDark = false;

        SchemeTonalSpot scheme(source, isDark, 0.0);
        myThemer->updateAndSave(scheme);
        std::cout << "Theme applied successfully from: " << path << std::endl;
    }

    return 0;
}
