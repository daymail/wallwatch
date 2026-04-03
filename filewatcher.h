#pragma once
#include <QDir>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QObject>
#include <vector>

namespace wallwatch {

struct WallInfo{
    QString path;
    QString name;
    QString type;
    QString lastModified;
    qint64 size;
    bool isVideo;

    WallInfo() = default;
    WallInfo(const QString& path): path(path){
        QFileInfo info(path);
        name = info.fileName();
        type = info.suffix().toLower();
        lastModified = info.lastModified().toString("yyyy-MM-dd hh:mm:ss");
        size = info.size();

        QStringList videoExts = {"mp4", "avi", "mkv", "mov", "webm", "gif"};
        isVideo = videoExts.contains(type);
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
    WallInfo getWallpaperInfo(int idx) const;
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
