#include "themer.h"
#include "colorspace/blend.h"
#include "m3/hct/hct.h"

namespace wallwatch {

QByteArray Themer::microSerialize(const DynamicScheme& scheme, const QByteArray& hash){
    auto hex = [](uint32_t argb) -> QString{
        return QString("#%1").arg(argb & 0xFFFFFF, 6, 16, QChar('0')).toUpper();
    };

    QString json = QString("{\"hash\":\"%1\",\"colors\":{"
                           "\"background\":\"%2\","
                           "\"primary\":\"%3\","
                           "\"surface\":\"%4\","
                           "\"onSurface\":\"%5\""
                           "}}")
                        .arg(QString::fromUtf8(hash),
                            hex(scheme.GetBackground()),
                            hex(scheme.GetPrimary()),
                            hex(scheme.GetSurface()),
                            hex(scheme.GetOnSurface()));
    return json.toUtf8();
}


bool Themer::fromCache(const QByteArray& hash, const QString& cachePath, const QString& variantName, bool isDark, const QString& schemePath){
    QString mode = isDark ? "dark" : "light";
    QString base = cachePath;
    if(!base.endsWith('/')) base += '/';

    QString cacheFile = base + QString("%1/%2/%3.json").arg(QString::fromUtf8(hash), variantName, mode);

    if(QFile::exists(cacheFile)){
        QFile source(cacheFile);
        if(source.open(QIODevice::ReadOnly)){
            QSaveFile destination(schemePath);
            if(destination.open(QIODevice::WriteOnly)){
                destination.write(source.readAll());
                return destination.commit();
            }
        }
    }
    return false;
}

void Themer::saveToCache(const QByteArray& jsonData, const QByteArray& hash, const QString& variantName, bool isDark){
    QString mode = isDark ? "dark" : "light";
    QString cachePath = QDir::homePath() + QString("/.cache/wallwatch/wallcache/%1/%2/").arg(QString::fromUtf8(hash), variantName);
    QDir().mkpath(cachePath);

    QFile cFile(cachePath+mode+".json");
    if(cFile.open(QIODevice::WriteOnly)){
        cFile.write(jsonData);
        cFile.close();
    }
}

void Themer::updateScheme(const QByteArray& jsonData, const QString& outPath){
    QFileInfo fileInfo(outPath);
    QDir().mkpath(fileInfo.absolutePath());

    QSaveFile liveFile(outPath);
    if(liveFile.open(QIODevice::WriteOnly)){
        liveFile.write(jsonData);
        if(!liveFile.commit()){
            qWarning() << "Failed to commit updates to: " << outPath;
        }
    }
}

QByteArray Themer::serialize(const DynamicScheme& newTheme, const QString& variantName, const QString& wallpaperPath, const QByteArray& hash){
    m_lastHash = hash;

    QJsonObject root;
    QJsonObject colors;

    root["hash"] = QString::fromUtf8(hash);
    root["variant"] = variantName;

    QFileInfo wallInfo(wallpaperPath);
    QString fileName = wallInfo.fileName();
    root["filename"] = fileName;

    bool isDark = newTheme.is_dark;
    root["theme"] = isDark ? "Dark Theme" : "Light theme";

    auto hex = [](uint32_t argb) -> QString{
        return QString("#%1").arg(argb & 0xFFFFFF, 6, 16, QChar('0')).toUpper();
    };

    Argb seed = newTheme.SourceColorArgb();

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
    Argb harmonizeError = BlendHarmonize(newTheme.GetError(), seed);
    TonalPalette errorPal(harmonizeError);
    colors["error"] = hex(errorPal.get(isDark ? 80 : 40));
    colors["onError"] = hex(errorPal.get(isDark ? 20 : 100));
    colors["errorContainer"] = hex(errorPal.get(isDark ? 30 : 90));
    colors["onErrorContainer"] = hex(errorPal.get(isDark ? 90 : 10));

    Argb harmonizeSuccess = BlendHarmonize(0xFF68C285, seed);
    TonalPalette successPal(harmonizeSuccess);
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

    Argb harmonizeBrand = BlendHarmonize(0xFF7AAACE, seed);
    TonalPalette brandPal(harmonizeBrand);
    colors["brand"] = hex(brandPal.get(isDark ? 80 : 40));
    colors["onBrand"] = hex(brandPal.get(isDark ? 20 : 100));

    Argb harmonizeWarning = BlendHarmonize(0xFFFFB900, seed);
    TonalPalette warningPal(harmonizeWarning);
    colors["warning"] = hex(warningPal.get(isDark ? 80 : 40));
    colors["onWarning"] = hex(warningPal.get(isDark ? 20 : 100));
    colors["warningContainer"] = hex(warningPal.get(isDark ? 30 : 90));
    colors["onWarningContainer"] = hex(warningPal.get(isDark ? 90 : 10));

    root["colors"] = colors;
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

}  //namespace wallwatch
