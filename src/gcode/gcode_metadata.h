#pragma once

#include <QFile>
#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

namespace gcode {

struct MaterialMetadata final {
    std::optional<double> grams;
    QString type;
    QString color;
    QString profile;
};

struct Metadata final {
    QString slicerName;
    QString slicerVersion;
    QString printerModel;
    QStringList detectedFields;
    QVector<int> usedMaterialIndices;
    bool materialUsageDetected = false;
    std::optional<double> printTimeSeconds;
    std::optional<double> totalFilamentGrams;
    QVector<MaterialMetadata> materials;
    QStringList warnings;
};

// Reads comments and metadata only. G-code instructions are never executed.
Metadata parseMetadata(QFile& file);

} // namespace gcode
