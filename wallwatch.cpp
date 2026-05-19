#include <chrono>
#define SOCK_PATH "/tmp/scoutd/scoutd.sock"
#include <QCoreApplication>
#include <QtConcurrent>
#include <sys/socket.h>
#include <sys/un.h>
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

int main(int argc, char *argv[]){
    auto start = std::chrono::high_resolution_clock::now();
    QCoreApplication app(argc, argv);

    if(argc < 3){
        std::cerr << "Usage: wallwatch -w <path> [-V <variant>] [-l]" << std::endl;
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

    Themer myThemer;

    const char* genPath = std::getenv("SCRIPT_GEN");
    const char* cachePathEnv = std::getenv("WALL_CACHE");
    QString outPath = (genPath == nullptr || std::string(genPath).empty()) ? QDir::homePath() + "/.local/share/wallscript/scheme.json" : QString::fromUtf8(genPath) + "scheme.json";
    QString cachePath = (cachePathEnv == nullptr) ? QDir::homePath() + "/.cache/wallwatch/wallcache/" : QString::fromUtf8(cachePathEnv);

    QByteArray contentHash = getFileHash(path);

    bool restored = myThemer.applyFromCache(contentHash, preferredVariant, useDark, outPath);
    if(!restored && preferredVariant != "content"){
        restored = myThemer.applyFromCache(contentHash, "content", useDark, outPath);
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
            uint32_t seedArgb = source.ToInt();
            myThemer.registerWallpaper(contentHash, path, seedArgb);
            myThemer.applyFromCache(contentHash, preferredVariant, useDark, outPath);
            QThreadPool::globalInstance() ->start([outPath](){
                    QFile activeFile(outPath);
                    if(activeFile.open(QIODevice::ReadOnly)){
                        QJsonDocument doc = QJsonDocument::fromJson(activeFile.readAll());
                        Exporter::exportAll(doc.object());
                    }
            });
        }
    }

    //socket(ignore)
    if(QFile::exists(QStringLiteral(SOCK_PATH))){
        int client_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if(client_fd >= 1){
            struct sockaddr_un addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
            if(::connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0){
                std::string logMsg = "Applied theme update: <" + preferredVariant.toStdString() + (useDark ? ":Dark>" : ":Light>") + " ~ " + path.toStdString();
                ::write(client_fd, logMsg.c_str(), logMsg.length());
                ::shutdown(client_fd, SHUT_WR);
            }
            ::close(client_fd);
        }
    }
    //===socket
    QThreadPool::globalInstance()->waitForDone();
    auto stop = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Execution completed in: " << total << " ms" << " file: " << path.toStdString() << std::endl;
    QFile file("output.txt");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "Execution completed in: " << total << " ms" << " file: " << path;
        file.close();
    }
    return 0;
}
