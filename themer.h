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

    bool fromCache(const QByteArray& hash, const QString& cachePath, const QString& schemePath){
        QString cacheFile = cachePath + hash + ".json";

        if(QFile::exists(cacheFile)){
            QFile::remove(schemePath);
            return QFile::copy(cacheFile, schemePath);
        }
        return false;
    }

    void updateAndSave(const DynamicScheme& newTheme, const QString& wallpaperPath, const QString& outPath, const QByteArray& hash){
        m_lastHash = hash;

        QJsonObject root;
        QJsonObject colors;

        root["hash"] = QString::fromUtf8(hash);

        bool isDark = newTheme.is_dark;

        auto hex = [](uint32_t argb) -> QString{
            return QString("#%1").arg(argb & 0xFFFFFF, 6, 16, QChar('0')).toUpper();
        };

        // Core Palette Keys
        colors["primaryPaletteKeyColor"] = hex(newTheme.GetPrimaryPaletteKeyColor());
        colors["secondaryPaletteKeyColor"] = hex(newTheme.GetSecondaryPaletteKeyColor());
        colors["tertiaryPaletteKeyColor"] = hex(newTheme.GetTertiaryPaletteKeyColor());
        colors["neutralPaletteKeyColor"] = hex(newTheme.GetNeutralPaletteKeyColor());
        colors["neutralVariantPaletteKeyColor"] = hex(newTheme.GetNeutralVariantPaletteKeyColor());

        // Surface & Background
        colors["background"] = hex(newTheme.GetBackground());
        colors["onBackground"] = hex(newTheme.GetOnBackground());
        colors["surface"] = hex(newTheme.GetSurface());
        colors["onSurface"] = hex(newTheme.GetOnSurface());
        colors["surfaceVariant"] = hex(newTheme.GetSurfaceVariant());
        colors["onSurfaceVariant"] = hex(newTheme.GetOnSurfaceVariant());
        colors["surfaceDim"] = hex(newTheme.GetSurfaceDim());
        colors["surfaceBright"] = hex(newTheme.GetSurfaceBright());

        // Surface Containers
        colors["surfaceContainerLowest"] = hex(newTheme.GetSurfaceContainerLowest());
        colors["surfaceContainerLow"] = hex(newTheme.GetSurfaceContainerLow());
        colors["surfaceContainer"] = hex(newTheme.GetSurfaceContainer());
        colors["surfaceContainerHigh"] = hex(newTheme.GetSurfaceContainerHigh());
        colors["surfaceContainerHighest"] = hex(newTheme.GetSurfaceContainerHighest());

        // Accents
        colors["primary"] = hex(newTheme.GetPrimary());
        colors["onPrimary"] = hex(newTheme.GetOnPrimary());
        colors["primaryContainer"] = hex(newTheme.GetPrimaryContainer());
        colors["onPrimaryContainer"] = hex(newTheme.GetOnPrimaryContainer());
        colors["inversePrimary"] = hex(newTheme.GetInversePrimary());

        colors["secondary"] = hex(newTheme.GetSecondary());
        colors["onSecondary"] = hex(newTheme.GetOnSecondary());
        colors["secondaryContainer"] = hex(newTheme.GetSecondaryContainer());
        colors["onSecondaryContainer"] = hex(newTheme.GetOnSecondaryContainer());

        colors["tertiary"] = hex(newTheme.GetTertiary());
        colors["onTertiary"] = hex(newTheme.GetOnTertiary());
        colors["tertiaryContainer"] = hex(newTheme.GetTertiaryContainer());
        colors["onTertiaryContainer"] = hex(newTheme.GetOnTertiaryContainer());

        // Error & Success
        colors["error"] = hex(newTheme.GetError());
        colors["onError"] = hex(newTheme.GetOnError());
        colors["errorContainer"] = hex(newTheme.GetErrorContainer());
        colors["onErrorContainer"] = hex(newTheme.GetOnErrorContainer());

        TonalPalette successPal(0xFF68C285);
        colors["success"] = hex(successPal.get(isDark ? 80 : 40));
        colors["onSuccess"] = hex(successPal.get(isDark ? 20 : 100));
        colors["successContainer"] = hex(successPal.get(isDark ? 30 : 90));
        colors["onSuccessContainer"] = hex(successPal.get(isDark ? 90 : 10));

        // Utility & Brand
        colors["outline"] = hex(newTheme.GetOutline());
        colors["outlineVariant"] = hex(newTheme.GetOutlineVariant());
        colors["shadow"] = hex(newTheme.GetShadow());
        colors["scrim"] = hex(newTheme.GetScrim());
        colors["surfaceTint"] = hex(newTheme.GetSurfaceTint());

        TonalPalette brandPal(0xFF5865F2);
        colors["brand"] = hex(brandPal.get(isDark ? 80 : 40));
        colors["onBrand"] = hex(brandPal.get(isDark ? 20 : 100));

        QFileInfo fileInfo(outPath);
        QDir().mkpath(fileInfo.absolutePath());

        root["colors"] = colors;
        QJsonDocument doc(root);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        QString cacheDir = QDir::homePath() + "/.cache/wallwatch/wallcache/";
        QDir().mkpath(cacheDir);
        QFile cFile(cacheDir + hash + ".json");
        if(cFile.open(QIODevice::WriteOnly)){
            cFile.write(jsonData);
            cFile.close();
        }


        QSaveFile file(outPath);
        if(file.open(QIODevice::WriteOnly)){
            QJsonDocument doc(root);
            file.write(doc.toJson(QJsonDocument::Indented));
            if(!file.commit()){
                qWarning() << "Failed to commit theme JSON to disk";
            }
        }

    }

  private:
    QByteArray m_lastHash;
};

}  //namespace wallwatch
