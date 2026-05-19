#include "themer.h"
#include "scheme/variants/scheme_header.h"
#include "colorspace/blend.h"
#include "m3/hct/hct.h"
#include <QProcess>
#include <qdebug.h>
#include <qprocess.h>

namespace wallwatch {

QString Themer::registryPath(){
    return QDir::homePath() + "/.cache/wallwatch/wallcache/registry.json";
}

QJsonObject Themer::loadRegistry(){
    QFile file(registryPath());
    if(!file.open(QIODevice::ReadOnly)){
        return QJsonObject();
    }
    return QJsonDocument::fromJson(file.readAll()).object();
}

void Themer::saveRegistry(const QJsonObject &registry){
    QFileInfo info(registryPath());
    QDir().mkpath(info.absolutePath());

    QSaveFile file(registryPath());
    if(file.open(QIODevice::WriteOnly)){
        file.write(QJsonDocument(registry).toJson(QJsonDocument::Compact));
        file.commit();
    }
}

void Themer::registerWallpaper(const QByteArray &hash, const QString &wallpaper, uint32_t seedArgb){
    QJsonObject registry = loadRegistry();
    QJsonObject wallpapers = registry["wallpapers"].toObject();
    QString hashStr = QString::fromUtf8(hash);

    QJsonObject wallData;
    wallData["filename"] = QFileInfo(wallpaper).fileName();
    wallData["seed_argb"] = QString("0x%1").arg(seedArgb, 8, 16, QChar('0')).toUpper();
    wallpapers[hashStr] = wallData;
    registry["wallpapers"] = wallpapers;
    saveRegistry(registry);
}


bool Themer::applyFromCache(const QByteArray &hash, const QString &variant, bool isDark, const QString &schemePath){
    QJsonObject registry = loadRegistry();
    QJsonObject wallpapers = registry["wallpapers"].toObject();
    QString hashStr = QString::fromUtf8(hash);

    if(!wallpapers.contains(hashStr)) return false;
    QJsonObject wallData = wallpapers[hashStr].toObject();
    bool ok;
    uint32_t seed = wallData["seed_argb"].toString().toUInt(&ok, 16);
    if(!ok) return false;

    m_lastHash = hash;
    std::unique_ptr<DynamicScheme> scheme;
    material_color_utilities::Hct sourceHct(seed);
    QString v = variant.toLower().trimmed();
    if(v  == "vibrant") scheme = std::make_unique<SchemeVibrant>(sourceHct, isDark);
    else if(v  == "tonal_spot") scheme = std::make_unique<SchemeTonalSpot>(sourceHct, isDark);
    else if(v  == "fidelity") scheme = std::make_unique<SchemeFidelity>(sourceHct, isDark);
    else if(v  == "neutral") scheme = std::make_unique<SchemeNeutral>(sourceHct, isDark);
    else if(v  == "rainbow") scheme = std::make_unique<SchemeRainbow>(sourceHct, isDark);
    else if(v  == "expressive") scheme = std::make_unique<SchemeExpressive>(sourceHct, isDark);
    else if(v  == "fruit_salad") scheme = std::make_unique<SchemeFruitSalad>(sourceHct, isDark);
    else if(v  == "monochrome") scheme = std::make_unique<SchemeMonochrome>(sourceHct, isDark);
    else scheme = std::make_unique<SchemeContent>(sourceHct, isDark);

    QJsonObject state;
    state["current_hash"] = hashStr;
    state["variant"] = v;
    state["is_dark"] = isDark;

    registry["state"] = state;
    saveRegistry(registry);
    updateScheme(*scheme, v, wallData["filename"].toString(), schemePath);
    return true;
}

void Themer::updateScheme(const DynamicScheme &newTheme, const QString &variant, const QString &fileName, const QString &outPath){
    QJsonObject root;
    QJsonObject colors;

    root["hash"] = QString::fromUtf8(m_lastHash);
    root["variant"] = variant;
    root["filename"] = fileName;
    root["theme"] = newTheme.is_dark ? "Dark Theme" : "Light Theme";

    auto hex = [](uint32_t argb) -> QString{
        return QString("#%1").arg(argb & 0xFFFFFF, 6, 16, QChar('0')).toUpper();
    };

    material_color_utilities::Argb seed = newTheme.SourceColorArgb();
    bool isDark = newTheme.is_dark;

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

    QFileInfo fileInfo(outPath);
    QDir().mkpath(fileInfo.absolutePath());
    QSaveFile liveFile(outPath);
    if(liveFile.open(QIODevice::WriteOnly)){
        liveFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        if(!liveFile.commit()){
            qWarning() << "Failed to atomic-commit active sys-config out to: " << outPath;
        }
    }
}

}  //namespace wallwatch
