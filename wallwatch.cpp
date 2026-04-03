#include <iostream>
#include <QObject>
#include <QString>

#include "filewatcher.h"
#include "colorspace/color_types.h"
#include "quantize/imgproc.h"
int main(int argc, char *argv[]){
    wallwatch::WallWatch watcher;

    QStringList wallpapers = watcher.getWallpapers();
    if (wallpapers.isEmpty()){
        std::cout << "No wallpaper found" << std::endl;
    }
    QString wallpaper = wallpapers[3].trimmed();
    std::string path = wallpaper.toStdString();

    while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
        path.pop_back();
    }

    auto info = watcher.getWallpaperInfo(0);
    std::cout << "Full path: " << info.path.toStdString() << std::endl;
    std::vector<wallwatch::Argb> pixels;
    if(info.isVideo){
        pixels = wallwatch::ExtractPixels(path);
        std::cout << "used MEEEEEE" << std::endl;
    }else{pixels = wallwatch::ImageToPixels(path);}
    std::cout << "Pixels: " << pixels.size() << std::endl;

    return 0;
}

