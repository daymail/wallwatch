#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QDBusInterface>
#include <QDBusConnection>

#include "scheme/dynamic/dynamicscheme.h"
#include "scheme/tone.h"
#include "filewatcher.h"

namespace wallwatch {

class Themer {
public:
    explicit Themer() : m_lastHash(""){}

    void updateScheme(const DynamicScheme &newTheme, const QString &variant, const QString &fileName, const QString &outPath);
    void registerWallpaper(const QByteArray &hash, const QString &wallpaper, uint32_t seedArgb);
    bool applyFromCache(const QByteArray &hash, const QString &variant, bool isDark, const QString &schemePath);
private:
    QString registryPath();
    QJsonObject loadRegistry();
    void saveRegistry(const QJsonObject &registry);
    QByteArray m_lastHash;
};

}  //namespace wallwatch
