#include <QCoreApplication>
#include <iostream>
#include <QObject>
#include <QString>

#include "wallwatch.h"
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
    auto info = watcher.getWallpaperInfo(0);
    std::string path = info.path.toStdString();

    std::vector<Argb> pixels;
    if(info.isVideo){
        pixels = wallwatch::ExtractPixels(path);
    }else{pixels = wallwatch::ImageToPixels(path);}

    QuantizerResult pixelColors = QuantizeCelebi(pixels, 128);
    std::vector<Argb> ranked = RankedSuggestions(pixelColors.color_to_count);
    std::cout << ranked[0] << std::endl;

    if(!ranked.empty()){
        Argb seed = ranked[0];
        HCT source = Hct(seed);
        bool isDark = false;

        SchemeTonalSpot scheme(source, isDark, 0.0);
        myThemer->updateAndSave(scheme);
        std::cout << "Theme applied from: " << path << std::endl;
    }

    return 0;
}

//TODO: add: m3success, m3onSuccess, m3successContainer, m3onSuccessContainer
