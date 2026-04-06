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

#include "scheme/dynamic/dynamicscheme.h"
#include "scheme/tone.h"

namespace wallwatch {

class Themer {
public:
    explicit Themer() : m_lastHash(""){}

    QByteArray getLastHash() const {return m_lastHash;}
    void setLastHash(const QByteArray& hash) { m_lastHash = hash; }

    bool fromCache(const QByteArray& hash, const QString& cachePath, const QString& variantName, bool isDark, const QString& schemePath);
    void saveToCache(const QByteArray& jsonData, const QByteArray& hash, const QString& variantName, bool isDark);
    void updateScheme(const QByteArray& jsonData, const QString& outPath);
    QByteArray microSerialize(const DynamicScheme& scheme, const QByteArray& hash);
    QByteArray serialize(const DynamicScheme& newTheme, const QString& variantName, const QString& wallpaperPath, const QByteArray& hash);

private:
    QByteArray m_lastHash;
};

}  //namespace wallwatch
