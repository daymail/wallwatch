#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>
#include "scheme/dynamic/dynamicscheme.h"

namespace wallwatch {

class Themer {
public:
    explicit Themer() : m_theme(nullptr) {}

    void updateAndSave(const DynamicScheme& newTheme, const QString& path = "/home/src/.local/share/wallscript/scheme.json") {
        m_theme = std::make_unique<DynamicScheme>(newTheme);
        saveToFile(path);
    }

private:
    std::unique_ptr<DynamicScheme> m_theme;

    QString hex(uint32_t argb) const {
        return QString("#%1").arg(argb & 0xFFFFFF, 6, 16, QChar('0')).toUpper();
    }

    void saveToFile(const QString& fullPath) {
        if (!m_theme) return;

        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.absolutePath());

        QJsonObject colors;

        // 1. CORE PALETTE KEYS (Source colors for the scheme)
        colors["primaryPaletteKeyColor"] = hex(m_theme->GetPrimaryPaletteKeyColor());
        colors["secondaryPaletteKeyColor"] = hex(m_theme->GetSecondaryPaletteKeyColor());
        colors["tertiaryPaletteKeyColor"] = hex(m_theme->GetTertiaryPaletteKeyColor());
        colors["neutralPaletteKeyColor"] = hex(m_theme->GetNeutralPaletteKeyColor());
        colors["neutralVariantPaletteKeyColor"] = hex(m_theme->GetNeutralVariantPaletteKeyColor());

        // --- 2. SURFACE & BACKGROUND ROLES ---
        colors["background"] = hex(m_theme->GetBackground());
        colors["onBackground"] = hex(m_theme->GetOnBackground());
        colors["surface"] = hex(m_theme->GetSurface());
        colors["onSurface"] = hex(m_theme->GetOnSurface());
        colors["surfaceVariant"] = hex(m_theme->GetSurfaceVariant());
        colors["onSurfaceVariant"] = hex(m_theme->GetOnSurfaceVariant());
        colors["surfaceDim"] = hex(m_theme->GetSurfaceDim());
        colors["surfaceBright"] = hex(m_theme->GetSurfaceBright());

        // Surface Containers
        colors["surfaceContainerLowest"] = hex(m_theme->GetSurfaceContainerLowest());
        colors["surfaceContainerLow"] = hex(m_theme->GetSurfaceContainerLow());
        colors["surfaceContainer"] = hex(m_theme->GetSurfaceContainer());
        colors["surfaceContainerHigh"] = hex(m_theme->GetSurfaceContainerHigh());
        colors["surfaceContainerHighest"] = hex(m_theme->GetSurfaceContainerHighest());

        // Inverse Surfaces
        colors["inverseSurface"] = hex(m_theme->GetInverseSurface());
        colors["inverseOnSurface"] = hex(m_theme->GetInverseOnSurface());

        // 3. ACCENT ROLES (Primary, Secondary, Tertiary)
        colors["primary"] = hex(m_theme->GetPrimary());
        colors["onPrimary"] = hex(m_theme->GetOnPrimary());
        colors["primaryContainer"] = hex(m_theme->GetPrimaryContainer());
        colors["onPrimaryContainer"] = hex(m_theme->GetOnPrimaryContainer());
        colors["inversePrimary"] = hex(m_theme->GetInversePrimary());

        colors["secondary"] = hex(m_theme->GetSecondary());
        colors["onSecondary"] = hex(m_theme->GetOnSecondary());
        colors["secondaryContainer"] = hex(m_theme->GetSecondaryContainer());
        colors["onSecondaryContainer"] = hex(m_theme->GetOnSecondaryContainer());

        colors["tertiary"] = hex(m_theme->GetTertiary());
        colors["onTertiary"] = hex(m_theme->GetOnTertiary());
        colors["tertiaryContainer"] = hex(m_theme->GetTertiaryContainer());
        colors["onTertiaryContainer"] = hex(m_theme->GetOnTertiaryContainer());

        // 4. ERROR ROLES
        colors["error"] = hex(m_theme->GetError());
        colors["onError"] = hex(m_theme->GetOnError());
        colors["errorContainer"] = hex(m_theme->GetErrorContainer());
        colors["onErrorContainer"] = hex(m_theme->GetOnErrorContainer());

        // 5. FIXED ROLES
        colors["primaryFixed"] = hex(m_theme->GetPrimaryFixed());
        colors["primaryFixedDim"] = hex(m_theme->GetPrimaryFixedDim());
        colors["onPrimaryFixed"] = hex(m_theme->GetOnPrimaryFixed());
        colors["onPrimaryFixedVariant"] = hex(m_theme->GetOnPrimaryFixedVariant());

        colors["secondaryFixed"] = hex(m_theme->GetSecondaryFixed());
        colors["secondaryFixedDim"] = hex(m_theme->GetSecondaryFixedDim());
        colors["onSecondaryFixed"] = hex(m_theme->GetOnSecondaryFixed());
        colors["onSecondaryFixedVariant"] = hex(m_theme->GetOnSecondaryFixedVariant());

        colors["tertiaryFixed"] = hex(m_theme->GetTertiaryFixed());
        colors["tertiaryFixedDim"] = hex(m_theme->GetTertiaryFixedDim());
        colors["onTertiaryFixed"] = hex(m_theme->GetOnTertiaryFixed());
        colors["onTertiaryFixedVariant"] = hex(m_theme->GetOnTertiaryFixedVariant());

        // 6. UTILITY ROLES
        colors["outline"] = hex(m_theme->GetOutline());
        colors["outlineVariant"] = hex(m_theme->GetOutlineVariant());
        colors["shadow"] = hex(m_theme->GetShadow());
        colors["scrim"] = hex(m_theme->GetScrim());
        colors["surfaceTint"] = hex(m_theme->GetSurfaceTint());

        QJsonObject root;
        root["colors"] = colors;

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(root);
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
    }
};

}  //namespace wallwatch
