#include <QCoreApplication>
#include <iostream>
#include <QObject>
#include <QString>

#include "wallwatch.h"
using namespace wallwatch;

QByteArray getFileHash(const QString& path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) return QByteArray();
    QCryptographicHash hasher(QCryptographicHash::Md5);
    hasher.addData(file.read(4 * 1024 * 1024));
    return hasher.result().toHex();
}

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);

    WallWatch watcher;
    Themer myThemer;

    const char* genPath = std::getenv("SCRIPT_GEN");
    const char* cachePathEnv = std::getenv("WALL_CACHE");
    QString outPath = (genPath == nullptr || std::string(genPath).empty()) ? QDir::homePath() + "/.local/share/wallscript/scheme.json" : QString::fromUtf8(genPath) + "scheme.json";
    QString cachePath = (cachePathEnv == nullptr) ? QString::fromStdString(std::string(getenv("HOME")) + "/.cache/wallwatch/wallcache/") : QString(cachePathEnv);

    QStringList wallpapers = watcher.getWallpapers();

    if (wallpapers.isEmpty()){
        std::cout << "No wallpaper found" << std::endl;
        return 1;
    }

    auto info = watcher.getWallpaperInfo(0);
    QString path = info.path;
    QByteArray contentHash = getFileHash(info.path);
    bool isDark = true;

    QByteArray hash = QCryptographicHash::hash(contentHash + (isDark ? "_dark" : "_light"), QCryptographicHash::Md5).toHex();

    if(myThemer.fromCache(hash, cachePath, outPath)){
        std::cout << "Restored theme from library." << std::endl;
        return 0;
    }
    std::cout << "Cache miss: Generating new theme..." << std::endl;

    std::vector<Argb> pixels;
    if(info.isVideo){
        pixels = wallwatch::ExtractPixels(path.toStdString());
    }else{pixels = wallwatch::ImageToPixels(path.toStdString());}

    QuantizerResult pixelColors = QuantizeCelebi(pixels, 128);
    std::vector<Argb> ranked = RankedSuggestions(pixelColors.color_to_count);


    if(!ranked.empty()){
        Argb seed = ranked[0];
        HCT source = Hct(seed);

        SchemeTonalSpot scheme(source, isDark, 0.0);
        myThemer.updateAndSave(scheme, path, outPath, hash);
        std::cout << "Theme applied from: " << outPath.toStdString() << std::endl;
    }

    return 0;
}
