#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtConcurrent>
#include <iostream>
#include <QObject>
#include <QString>
#include <chrono>

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

auto start = std::chrono::high_resolution_clock::now();
    app.setApplicationName("wallwatch");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("WALLWATCH: M3 Wallpaper Theme Generator.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption wallOption({"w", "wallpaper"}, "Path to wallpaper file.", "path");
    parser.addOption(wallOption);

    QCommandLineOption variantOption({"V", "variant"}, "Theme variant to apply.", "name", "vibrant");
    parser.addOption(variantOption);

    QCommandLineOption lightOption({"l", "light"}, "Generate light mode theme instead of dark.");
    parser.addOption(lightOption);

    parser.process(app);

    QString rawPath = parser.value(wallOption);
    if(rawPath.isEmpty()){
        std::cerr << "ERROR: No wallpaper path provided. Use -w <path>" << std::endl;
        parser.showHelp(1);
    }

    if(rawPath.startsWith("~")){
        rawPath.replace(0, 1, QDir::homePath());
    }
    QString path = QFileInfo(rawPath).absoluteFilePath();

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
    bool useDark = !parser.isSet(lightOption);
    QString preferredVariant = parser.value(variantOption);

    if(myThemer.fromCache(contentHash, cachePath, preferredVariant, useDark, outPath)){
        std::cout << "Restored theme from library." << std::endl;
        return 0;
    }else if(preferredVariant != "content" && 
         myThemer.fromCache(contentHash, cachePath, "content", useDark, outPath)){
        return 0;
    }

    std::vector<Argb> pixels;
    pixels = wallwatch::ExtractPixels(path.toStdString(), 64);

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
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Operation took: " << duration.count() << " μs" << std::endl;
    return 0;
}
