#pragma once
#include <QObject>
#include <QJsonObject>
#include <QDir>
#include <QSaveFile>

namespace wallwatch{
class Exporter{
  public:
    static void exportAll(const QJsonObject& root);

  private:
    static void toRust(const QJsonObject &colors, const QString &baseDir);
    static void toShell(const QJsonObject &colors, const QString &baseDir);
    static void toXresources(const QJsonObject &colors, const QString &baseDir);
    static QString getExportDir();
};
}
