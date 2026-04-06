#include <QCoreApplication>
#include <QtConcurrent>
#include <string_view>
#include <iostream>
#include "wallwatch.h"

using namespace wallwatch;

QByteArray getFileHash(const QString& path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) return QByteArray();
    QCryptographicHash hasher(QCryptographicHash::Md5);
    hasher.addData(file.read(1024 * 1024));
    return hasher.result().toHex();
}

std::unique_ptr<DynamicScheme> generateScheme(const HCT& source, const QString& name, bool isDark){
    if(name == "vibrant")            return std::make_unique<SchemeVibrant>(source, isDark, 0.0);
    else if(name == "tonal_spot")    return std::make_unique<SchemeTonalSpot>(source, isDark, 0.0);
    else if(name == "fidelity")      return std::make_unique<SchemeFidelity>(source, isDark, 0.0);
    else if(name == "neutral")       return std::make_unique<SchemeNeutral>(source, isDark, 0.0);
    else if(name == "rainbow")       return std::make_unique<SchemeRainbow>(source, isDark, 0.0);
    else if(name == "expressive")    return std::make_unique<SchemeExpressive>(source, isDark, 0.0);
    else if(name == "fruit_salad")   return std::make_unique<SchemeFruitSalad>(source, isDark, 0.0);
    else if(name == "monochrome")    return std::make_unique<SchemeMonochrome>(source, isDark, 0.0);
    else if(name == "content")       return std::make_unique<SchemeContent>(source, isDark, 0.0);

return std::make_unique<SchemeContent>(source, isDark, 0.0);
}


int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);

    if(argc < 3){
        std::cerr << "Usage: wallwatch -w <path> [-V variant] [-l]" << std::endl;
        return 1;
    }

    std::string_view rawPath;
    QString preferredVariant = "vibrant";
    bool useDark = true;

    for(int i=1;i<argc;++i){
        std::string_view arg = argv[i];
        if((arg == "-w" || arg == "--wallpaper") && i+1 < argc){
            rawPath = argv[++i];
        }else if((arg == "-V" || arg == "--variant") && i+1 < argc){
            preferredVariant = QString::fromUtf8(argv[++i]);
        }else if(arg == "-l" || arg == "--light"){
            useDark = false;
        }
    }
    if(rawPath.empty()) return 1;

    QString path;
    if(rawPath[0] == '~'){
        path = QDir::homePath() + QString::fromUtf8(rawPath.data()+ 1, rawPath.size() - 1);
    }else{
        path = QFileInfo(QString::fromUtf8(rawPath.data(), rawPath.size())).absoluteFilePath();
    }

    if(!QFile::exists(path)){
        std::cerr << "ERROR: File not found: " << path.toStdString() << std::endl;
        return 1;
    }

    WallWatch watcher;
    Themer myThemer;

    const char* genPath = std::getenv("SCRIPT_GEN");
    const char* cachePathEnv = std::getenv("WALL_CACHE");
    QString outPath = (genPath == nullptr || std::string(genPath).empty()) ? QDir::homePath() + "/.local/share/wallscript/scheme.json" : QString::fromUtf8(genPath) + "scheme.json";
    QString cachePath = (cachePathEnv == nullptr) ? QDir::homePath() + "/.cache/wallwatch/wallcache/" : QString::fromUtf8(cachePathEnv);

    auto info = watcher.getWallpaperInfo(path);
    QByteArray contentHash = getFileHash(info.path);

    bool restored = myThemer.fromCache(contentHash, cachePath, preferredVariant, useDark, outPath);
    if(!restored && preferredVariant != "content"){
        restored = myThemer.fromCache(contentHash, cachePath, "content", useDark, outPath);
    }

    if(!restored){
        std::vector<Argb> pixels = wallwatch::ExtractPixels(path.toStdString(), 64);

        if(pixels.empty()){return 1;}

        QuantizerResult pixelColors = QuantizeCelebi(pixels, 128);
        std::vector<Argb> ranked = RankedSuggestions(pixelColors.color_to_count);


        if(!ranked.empty()){
            Argb seed = ranked[0];
            HCT source = Hct(seed);
            source = wallwatch::FixIfDisliked(source);

            struct VariantType{QString name;};
            std::vector<QString> variantNames = {"vibrant", "tonal_spot", "fidelity", "neutral", "rainbow", "expressive", "fruit_salad", "monochrome", "content"};

            auto s = generateScheme(source, preferredVariant, useDark);
            if(s){
                QByteArray microData = myThemer.microSerialize(*s, contentHash);
                myThemer.updateScheme(microData, outPath);
            }

            bool otherMode = !useDark;
            auto sOther = generateScheme(source, preferredVariant, otherMode);
            if(sOther){
                QByteArray other = myThemer.serialize(*sOther, preferredVariant, path, contentHash);
                myThemer.saveToCache(other, contentHash, preferredVariant, otherMode);
            }

            QFuture<void> future = QtConcurrent::run([=, &myThemer](){
                auto fullscheme = generateScheme(source, preferredVariant, useDark);
                QByteArray full = myThemer.serialize(*fullscheme, preferredVariant, path, contentHash);
                myThemer.updateScheme(full, outPath);
                myThemer.saveToCache(full, contentHash, preferredVariant, useDark);

                for(const QString& name: variantNames){
                    if(name == preferredVariant) continue;
                    auto ds = generateScheme(source, name, true);
                    myThemer.saveToCache(myThemer.serialize(*ds, name, path, contentHash), contentHash, name, true);
                    auto ls = generateScheme(source, name, false);
                    myThemer.saveToCache(myThemer.serialize(*ls, name, path, contentHash), contentHash, name, false);
                }
            });
            std::cout << "Theme applied from: " << outPath.toStdString() << std::endl;
        }
    }
    return 0;
}
