#pragma once
#include <QDir>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QSize>
#include <QString>
#include <opencv2/opencv.hpp>
#include <QStringList>
#include <QObject>
#include <vector>

namespace wallwatch {
struct WallInfo{
    QString path;
    QString name;
    QString type;
    QString lastModified;
    QString orientation;
    qint64 size;
    bool isVideo;

    int width;
    int height;
    double aspectRatio;
    WallInfo() = default;
    WallInfo(const QString& path): path(path){
        width=0;height=0;aspectRatio=0.0;
        QFileInfo info(path);
        name = info.fileName();
        type = info.suffix().toLower();
        lastModified = info.lastModified().toString("yyyy-MM-dd hh:mm:ss");
        size = info.size();

        QStringList videoExts = {"mp4", "avi", "mkv", "mov", "webm", "gif"};
        isVideo = videoExts.contains(type);
        cv::Mat frame;

        QByteArray localPath = path.toLocal8Bit();
        std::string stdPath = localPath.constData();
        if(isVideo){
            cv::VideoCapture cap(stdPath);
            if(cap.isOpened()){
                cap.read(frame);
                cap.release();
            }
        }
        if(frame.empty()){
            frame = cv::imread(stdPath, cv::IMREAD_UNCHANGED);
        }
        if(!frame.empty()){
            width =frame.cols;
            height = frame.rows;
            if(height > 0){
                aspectRatio = static_cast<double>(width) / height;
            }else{
                qWarning() << "Final height is 0 for:" << name; 
            }
        }
        orientation = calcOrientation(width, height);
    }
    QString calcOrientation(int width, int height) const{
        if(width<=0)return "unknown";
        if(width > height)return "landscape";
        if(width < height)return "portrait";
        return "square";
    }

    QString getHumanReadableSize() const {
        if (size < 1024) return QString::number(size) + " B";
        if (size < 1024 * 1024) return QString::number(size / 1024.0, 'f', 1) + " KB";
        if (size < 1024 * 1024 * 1024) return QString::number(size / (1024.0 * 1024.0), 'f', 1) + " MB";
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
    }

    QString getLastModifiedString() const{
        return lastModified;
    }
};

class WallWatch: public QObject {
    Q_OBJECT

public:
    WallWatch(QObject* parent = nullptr);
    ~WallWatch() = default;

    bool isValid() const;
    void manualWallSet(const QString& path);
    QStringList getWallpapers() const;
    WallInfo getWallpaperInfo(const QString& path) const;
    std::vector<WallInfo> getAllWallpapersInfo() const;

private slots:
    void processDir();

signals:
    void wallChanged(const QString& wall);

private:
    QDir m_wallDir;
    QFileSystemWatcher* m_watcher;
    bool m_dir_is_valid;
    QString m_currentWall;
};

}   //namespace wallwatch
