#include "filewatcher.h"

namespace wallwatch {

WallWatch::WallWatch(QObject* parent) : QObject(parent), m_dir_is_valid(false) {
    m_wallDir.setPath(QDir::homePath() + "/.local/share/wallpapers");
    if (!m_wallDir.exists()) {
        if (!m_wallDir.mkpath(".")) {
            qWarning() << "Failed to create wall-directory: " << m_wallDir.absolutePath();
            m_dir_is_valid = false;
            return;
        }
    }
    m_dir_is_valid = true;
    m_watcher = new QFileSystemWatcher(this);
    if (m_wallDir.exists()) {
        if (!m_watcher->addPath(m_wallDir.absolutePath())) {
            qWarning() << "Failed to watch directory: " << m_wallDir.absolutePath();
        }
    }
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &WallWatch::processDir);
    processDir();
}

bool WallWatch::isValid() const {
    return m_dir_is_valid;
}

void WallWatch::manualWallSet(const QString& path) {
    if (!QFile::exists(path)) {
        qWarning() << "Source does not exist: " << path;
        return;
    }
    QString dest = m_wallDir.absoluteFilePath(QFileInfo(path).fileName());
    QFile::copy(path, dest);
}

QStringList WallWatch::getWallpapers() const{
    QDir dir(m_wallDir);
    if (!dir.exists()){
        return QStringList();
    }
    QStringList files = dir.entryList(
        {"*.jpg", "*.png", "*.jpeg", "*.webp", "*.mp4", "*.avi", "*.mkv", "*.mov", "*.webm", "*.gif"},
        QDir::Files,
        QDir::Name
    );
    QStringList fullPaths;
    for (const QString& file : files){
        fullPaths.append(dir.absoluteFilePath(file));
    }
    return fullPaths;
}

void WallWatch::processDir() {
    if (!m_dir_is_valid) return;
    QDir dir(m_wallDir);
    if (!dir.exists()) {
        qWarning() << "Wallpaper directory absent: " << dir.absolutePath();
        m_dir_is_valid = false;
        return;
    }
    QStringList images = dir.entryList(
        {"*.jpg", "*.png", "*.jpeg", "*.webp"},
        QDir::Files,
        QDir::Time
    );
    if (!images.isEmpty()) {
        QString newWall = dir.absoluteFilePath(images.first());
        if (newWall != m_currentWall) {
            m_currentWall = newWall;
            emit wallChanged(newWall);
        }
    } else {
        qDebug() << "No wallpapers found in path: " << dir.absolutePath();
    }
}

WallInfo WallWatch::getWallpaperInfo(int idx = 0) const{
    QStringList wallpapers = getWallpapers();
    if(idx < wallpapers.size()){
        return  WallInfo(wallpapers[idx]);
    }
    return WallInfo();
}

std::vector<WallInfo> WallWatch::getAllWallpapersInfo() const{
    std::vector<WallInfo> infos;
    QStringList wallpapers = getWallpapers();
    for(const QString& path : wallpapers){
        infos.push_back(WallInfo(path));
    }
    return infos;
}

}  //namespace wallwatch
