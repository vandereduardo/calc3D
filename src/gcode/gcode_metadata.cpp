#include "gcode_metadata.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace gcode {
namespace {

constexpr qint64 MaximumLineLength = 1'000'000;
constexpr qint64 ReadChunkSize = 256 * 1024;
constexpr int MaximumWarnings = 100;

const QStringList RelevantFields = {
    "printer_model",
    "filament_type",
    "filament_colour",
    "filament_settings_id",
    "filament used [g]",
    "total filament used [g]",
    "total filament weight [g]",
    "filament used [mm]",
    "filament used [cm3]",
    "estimated printing time (normal mode)",
    "print_time"
};

void addWarning(QStringList& warnings, const QString& warning)
{
    if (warnings.size() < MaximumWarnings) {
        warnings.push_back(warning);
    }
}

QString normalizeKey(QString value)
{
    return value.simplified().toLower();
}

QString unquote(QString value)
{
    value = value.trimmed();
    if (value.size() >= 2) {
        const QChar first = value.front();
        if ((first == QLatin1Char('"') || first == QLatin1Char('\'')) && value.back() == first) {
            value = value.sliced(1, value.size() - 2);
            value.replace(QRegularExpression(QStringLiteral(R"(\\([\\"']))")), QStringLiteral("\\1"));
        }
    }
    return value;
}

QStringList splitDelimited(const QString& value, QChar delimiter)
{
    QStringList parts;
    QString current;
    QChar quote;
    bool escaped = false;

    for (const QChar character : value) {
        if (escaped) {
            current += character;
            escaped = false;
            continue;
        }
        if (character == QLatin1Char('\\')) {
            current += character;
            escaped = true;
            continue;
        }
        if ((character == QLatin1Char('"') || character == QLatin1Char('\''))
            && (quote.isNull() || quote == character)) {
            quote = quote.isNull() ? character : QChar();
            current += character;
            continue;
        }
        if (character == delimiter && quote.isNull()) {
            parts.push_back(unquote(current));
            current.clear();
            continue;
        }
        current += character;
    }
    parts.push_back(unquote(current));
    return parts;
}

QVector<std::optional<double>> parseNumberList(
    const QString& value, const QString& field, QStringList& warnings)
{
    QVector<std::optional<double>> numbers;
    if (value.trimmed().isEmpty()) {
        return numbers;
    }

    const QStringList parts = splitDelimited(value, QLatin1Char(','));
    numbers.reserve(parts.size());
    for (qsizetype index = 0; index < parts.size(); ++index) {
        bool ok = false;
        const double number = parts[index].toDouble(&ok);
        if (!ok || !std::isfinite(number) || number < 0.0) {
            numbers.push_back(std::nullopt);
            addWarning(warnings, QStringLiteral("O valor %1 de %2 é inválido.").arg(index + 1).arg(field));
        } else {
            numbers.push_back(number);
        }
    }
    return numbers;
}

std::optional<double> captureDurationUnit(const QString& value, const QString& unit)
{
    const QRegularExpression expression(
        QStringLiteral("(\\d+(?:\\.\\d+)?)\\s*%1\\b").arg(unit),
        QRegularExpression::CaseInsensitiveOption);
    const auto match = expression.match(value);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    bool ok = false;
    const double number = match.captured(1).toDouble(&ok);
    if (!ok || !std::isfinite(number) || number < 0.0) {
        return std::nullopt;
    }
    return number;
}

std::optional<double> parseDurationSeconds(const QString& value)
{
    const QString text = value.trimmed();
    if (text.isEmpty()) {
        return std::nullopt;
    }

    static const QRegularExpression clockExpression(
        QStringLiteral(R"((?:^|\s)(\d+):(\d{1,2}):(\d{1,2}(?:\.\d+)?)(?:\s|$))"));
    const auto clock = clockExpression.match(text);
    if (clock.hasMatch()) {
        bool hoursOk = false;
        bool minutesOk = false;
        bool secondsOk = false;
        const double hours = clock.captured(1).toDouble(&hoursOk);
        const double minutes = clock.captured(2).toDouble(&minutesOk);
        const double seconds = clock.captured(3).toDouble(&secondsOk);
        if (hoursOk && minutesOk && secondsOk && minutes < 60.0 && seconds < 60.0) {
            return hours * 3600.0 + minutes * 60.0 + seconds;
        }
        return std::nullopt;
    }

    const auto days = captureDurationUnit(text, "d");
    const auto hours = captureDurationUnit(text, "h");
    const auto minutes = captureDurationUnit(text, "m");
    const auto seconds = captureDurationUnit(text, "s");
    if (!days && !hours && !minutes && !seconds) {
        return std::nullopt;
    }

    return days.value_or(0.0) * 86400.0
        + hours.value_or(0.0) * 3600.0
        + minutes.value_or(0.0) * 60.0
        + seconds.value_or(0.0);
}

void alignStrings(QStringList& values, qsizetype count, const QString& field, QStringList& warnings)
{
    if (!values.isEmpty() && values.size() != count) {
        addWarning(warnings,
            QStringLiteral("A lista %1 tem %2 itens, mas foram encontradas %3 quantidades de filamento.")
                .arg(field).arg(values.size()).arg(count));
    }
}

} // namespace

Metadata parseMetadata(QFile& file)
{
    Metadata metadata;
    if (!file.isOpen() || !file.isReadable()) {
        addWarning(metadata.warnings, QStringLiteral("Não foi possível ler o arquivo G-code."));
        return metadata;
    }

    QHash<QString, QString> fields;
    QString generatorText;
    std::optional<QString> statisticsTime;
    std::optional<QString> totalEstimatedTime;
    std::optional<QString> normalTime;
    std::optional<QString> modelPrintingTime;
    bool inStatistics = false;
    qint64 lineNumber = 0;
    QSet<int> usedMaterialIndices;
    const auto processLine = [&](const QByteArray& line) {
        qsizetype first = 0;
        while (first < line.size()) {
            const char byte = line.at(first);
            if (byte != ' ' && byte != '\t' && byte != '\r' && byte != '\v' && byte != '\f') {
                break;
            }
            ++first;
        }
        if (first >= line.size()) {
            return;
        }
        if (line.at(first) != ';') {
            const QByteArray command = line.sliced(first).trimmed();
            int materialIndex = -1;
            if (!command.isEmpty() && (command.at(0) == 'T' || command.at(0) == 't')) {
                qsizetype digitEnd = 1;
                while (digitEnd < command.size() && command.at(digitEnd) >= '0' && command.at(digitEnd) <= '9') {
                    ++digitEnd;
                }
                if (digitEnd > 1 && (digitEnd == command.size()
                    || command.at(digitEnd) == ' ' || command.at(digitEnd) == '\t'
                    || command.at(digitEnd) == ';')) {
                    bool ok = false;
                    const int index = command.sliced(1, digitEnd - 1).toInt(&ok);
                    if (ok && index >= 0 && index < 32) {
                        materialIndex = index;
                    }
                }
            } else if (command.startsWith("M620")) {
                static const QRegularExpression bambuToolExpression(
                    QStringLiteral(R"(^M620\s+S(\d+)A(?:\s|$))"),
                    QRegularExpression::CaseInsensitiveOption);
                const auto match = bambuToolExpression.match(QString::fromLatin1(command));
                if (match.hasMatch()) {
                    bool ok = false;
                    const int index = match.captured(1).toInt(&ok);
                    if (ok && index >= 0 && index < 32) {
                        materialIndex = index;
                    }
                }
            }
            if (materialIndex >= 0) {
                usedMaterialIndices.insert(materialIndex);
            }
            return;
        }

        const QString text = QString::fromUtf8(line.sliced(first + 1)).trimmed();
        const QString normalizedPayload = normalizeKey(text);
        if (normalizedPayload == "statistics = begin") {
            inStatistics = true;
            return;
        }
        if (normalizedPayload == "statistics = end") {
            inStatistics = false;
            return;
        }

        static const QRegularExpression generatorExpression(
            QStringLiteral(R"(^generated (?:by|with)\s+(.+)$)"), QRegularExpression::CaseInsensitiveOption);
        if (text.startsWith(QStringLiteral("generated by"), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("generated with"), Qt::CaseInsensitive)) {
            const auto generatorMatch = generatorExpression.match(text);
            if (generatorMatch.hasMatch()) {
                generatorText = generatorMatch.captured(1).trimmed();
            }
        }

        if (text.startsWith(QStringLiteral("BambuStudio "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("Bambu Studio "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("AnycubicSlicerNext "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("OrcaSlicer "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("PrusaSlicer "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("Cura_SteamEngine "), Qt::CaseInsensitive)
            || text.startsWith(QStringLiteral("Creality Print "), Qt::CaseInsensitive)) {
            generatorText = text;
        }

        static const QRegularExpression bambuTimeExpression(
            QStringLiteral(R"(total estimated time\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression modelTimeExpression(
            QStringLiteral(R"(model printing time\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        if (text.contains(QStringLiteral("total estimated time"), Qt::CaseInsensitive)) {
            const auto totalTimeMatch = bambuTimeExpression.match(text);
            if (totalTimeMatch.hasMatch()) {
                totalEstimatedTime = totalTimeMatch.captured(1).trimmed();
                if (!metadata.detectedFields.contains(QStringLiteral("total estimated time"))) {
                    metadata.detectedFields.push_back(QStringLiteral("total estimated time"));
                }
            }
        }
        if (text.contains(QStringLiteral("model printing time"), Qt::CaseInsensitive)) {
            const auto modelTimeMatch = modelTimeExpression.match(text);
            if (modelTimeMatch.hasMatch()) {
                modelPrintingTime = modelTimeMatch.captured(1).trimmed();
                if (!metadata.detectedFields.contains(QStringLiteral("model printing time"))) {
                    metadata.detectedFields.push_back(QStringLiteral("model printing time"));
                }
            }
        }

        const qsizetype equalsSeparator = text.indexOf(QLatin1Char('='));
        const qsizetype colonSeparator = text.indexOf(QLatin1Char(':'));
        qsizetype separator = equalsSeparator;
        if (colonSeparator >= 0 && (separator < 0 || colonSeparator < separator)) {
            separator = colonSeparator;
        }
        if (separator < 0) {
            return;
        }
        const QString key = normalizeKey(text.first(separator));
        const QString value = text.sliced(separator + 1).trimmed();
        if (key == "print_time" && inStatistics) {
            statisticsTime = value;
        }
        if (key == "estimated printing time (normal mode)") {
            normalTime = value;
        }
        if (RelevantFields.contains(key)) {
            fields.insert(key, value);
            if (!metadata.detectedFields.contains(key)) {
                metadata.detectedFields.push_back(key);
            }
        }
    };

    QByteArray chunk;
    QByteArray unfinishedLine;
    bool skippingOversizedLine = false;
    while (!file.atEnd()) {
        chunk = file.read(ReadChunkSize);
        if (file.error() != QFileDevice::NoError) {
            addWarning(metadata.warnings, QStringLiteral("Erro ao ler o arquivo: %1").arg(file.errorString()));
            break;
        }
        if (chunk.isEmpty()) {
            break;
        }

        qsizetype cursor = 0;
        while (cursor < chunk.size()) {
            const qsizetype newline = chunk.indexOf('\n', cursor);
            if (newline < 0) {
                const QByteArray remainder = chunk.sliced(cursor);
                if (!skippingOversizedLine) {
                    if (unfinishedLine.size() + remainder.size() > MaximumLineLength) {
                        skippingOversizedLine = true;
                        unfinishedLine.clear();
                        addWarning(metadata.warnings,
                            QStringLiteral("A linha %1 excede o limite de leitura e foi ignorada.")
                                .arg(lineNumber + 1));
                    } else {
                        unfinishedLine.append(remainder);
                    }
                }
                break;
            }

            const QByteArray segment = chunk.sliced(cursor, newline - cursor);
            ++lineNumber;
            if (skippingOversizedLine) {
                skippingOversizedLine = false;
            } else if (unfinishedLine.size() + segment.size() > MaximumLineLength) {
                unfinishedLine.clear();
                addWarning(metadata.warnings,
                    QStringLiteral("A linha %1 excede o limite de leitura e foi ignorada.").arg(lineNumber));
            } else if (unfinishedLine.isEmpty()) {
                processLine(segment);
            } else {
                unfinishedLine.append(segment);
                processLine(unfinishedLine);
                unfinishedLine.clear();
            }
            cursor = newline + 1;
        }
    }

    if (!skippingOversizedLine && !unfinishedLine.isEmpty()) {
        ++lineNumber;
        processLine(unfinishedLine);
    }

    if (file.error() != QFileDevice::NoError) {
        addWarning(metadata.warnings, QStringLiteral("Erro ao ler o arquivo: %1").arg(file.errorString()));
    }

    const auto statsSeconds = statisticsTime ? parseDurationSeconds(*statisticsTime) : std::nullopt;
    const auto totalEstimatedSeconds = totalEstimatedTime
        ? parseDurationSeconds(*totalEstimatedTime) : std::nullopt;
    const auto estimateSeconds = normalTime ? parseDurationSeconds(*normalTime) : std::nullopt;
    const auto modelSeconds = modelPrintingTime ? parseDurationSeconds(*modelPrintingTime) : std::nullopt;
    metadata.printTimeSeconds = statsSeconds ? statsSeconds
        : (totalEstimatedSeconds ? totalEstimatedSeconds : (estimateSeconds ? estimateSeconds : modelSeconds));

    const auto generatorMatch = QRegularExpression(
        QStringLiteral(R"((\d+\.\d+(?:\.\d+){0,2})\b)")).match(generatorText);
    if (generatorText.contains(QRegularExpression(QStringLiteral(R"(Anycubic\s*Slicer\s*Next|AnycubicSlicerNext)"),
            QRegularExpression::CaseInsensitiveOption))) {
        metadata.slicerName = QStringLiteral("Anycubic Slicer Next");
    } else if (generatorText.contains(QRegularExpression(QStringLiteral(R"(Bambu\s*Studio|BambuStudio)"),
                   QRegularExpression::CaseInsensitiveOption))) {
        metadata.slicerName = QStringLiteral("Bambu Studio");
    } else if (generatorText.contains(QRegularExpression(QStringLiteral(R"(OrcaSlicer)"),
                   QRegularExpression::CaseInsensitiveOption))) {
        metadata.slicerName = QStringLiteral("OrcaSlicer");
    } else if (generatorText.contains(QRegularExpression(QStringLiteral(R"(PrusaSlicer)"),
                   QRegularExpression::CaseInsensitiveOption))) {
        metadata.slicerName = QStringLiteral("PrusaSlicer");
    } else if (generatorText.contains(QRegularExpression(QStringLiteral(R"(Cura_SteamEngine|Ultimaker Cura)"),
                   QRegularExpression::CaseInsensitiveOption))) {
        metadata.slicerName = QStringLiteral("Ultimaker Cura");
    } else if (!generatorText.isEmpty()) {
        metadata.slicerName = generatorText;
    }
    if (generatorMatch.hasMatch()) {
        metadata.slicerVersion = generatorMatch.captured(1);
    }

    metadata.printerModel = unquote(fields.value("printer_model"));
    if (fields.contains("printer_model") && !metadata.detectedFields.contains("printer_model")) {
        metadata.detectedFields.push_back("printer_model");
    }

    const auto perMaterialGrams = parseNumberList(fields.value("filament used [g]"), "filament used [g]", metadata.warnings);
    auto totalValues = parseNumberList(
        fields.value("total filament used [g]"), "total filament used [g]", metadata.warnings);
    if (totalValues.isEmpty()) {
        totalValues = parseNumberList(
            fields.value("total filament weight [g]"), "total filament weight [g]", metadata.warnings);
    }
    for (const auto value : totalValues) {
        if (value) {
            metadata.totalFilamentGrams = value;
            break;
        }
    }

    double sum = 0.0;
    bool allMassesValid = !perMaterialGrams.isEmpty();
    for (const auto value : perMaterialGrams) {
        if (value) {
            sum += *value;
        } else {
            allMassesValid = false;
        }
    }
    const std::optional<double> perMaterialTotal = allMassesValid
        ? std::optional<double>(sum) : std::nullopt;
    if (!metadata.totalFilamentGrams && perMaterialTotal) {
        metadata.totalFilamentGrams = perMaterialTotal;
    }

    auto types = splitDelimited(fields.value("filament_type"), QLatin1Char(';'));
    auto colors = splitDelimited(fields.value("filament_colour"), QLatin1Char(';'));
    auto profiles = splitDelimited(fields.value("filament_settings_id"), QLatin1Char(';'));
    const qsizetype materialCount = std::max({
        perMaterialGrams.size(), types.size(), colors.size(), profiles.size()
    });
    alignStrings(types, materialCount, "filament_type", metadata.warnings);
    alignStrings(colors, materialCount, "filament_colour", metadata.warnings);
    alignStrings(profiles, materialCount, "filament_settings_id", metadata.warnings);

    metadata.materials.reserve(materialCount);
    for (qsizetype index = 0; index < materialCount; ++index) {
        metadata.materials.push_back({
            index < perMaterialGrams.size() ? perMaterialGrams[index] : std::nullopt,
            index < types.size() ? types[index] : QString(),
            index < colors.size() ? colors[index] : QString(),
            index < profiles.size() ? profiles[index] : QString()
        });
    }

    if (!usedMaterialIndices.isEmpty()) {
        QVector<int> sortedIndices;
        sortedIndices.reserve(usedMaterialIndices.size());
        for (const int index : usedMaterialIndices) {
            sortedIndices.push_back(index);
        }
        std::sort(sortedIndices.begin(), sortedIndices.end());

        const bool allIndicesHaveMetadata = std::all_of(
            sortedIndices.cbegin(), sortedIndices.cend(),
            [materialCount](int index) { return index < materialCount; });
        if (allIndicesHaveMetadata) {
            QVector<MaterialMetadata> usedMaterials;
            usedMaterials.reserve(sortedIndices.size());
            for (const int index : sortedIndices) {
                usedMaterials.push_back(metadata.materials[index]);
            }
            metadata.materials = std::move(usedMaterials);
            metadata.usedMaterialIndices = sortedIndices;
            metadata.materialUsageDetected = true;

            bool hasAnyMaterialMass = false;
            for (const auto& material : metadata.materials) {
                hasAnyMaterialMass = hasAnyMaterialMass || material.grams.has_value();
            }
            if (!hasAnyMaterialMass && metadata.materials.size() == 1 && metadata.totalFilamentGrams) {
                metadata.materials[0].grams = metadata.totalFilamentGrams;
            }
        } else {
            addWarning(metadata.warnings,
                QStringLiteral("Os índices de ferramenta do G-code não correspondem às listas de materiais."));
        }
    }

    bool hasAnyMaterialMass = false;
    for (const auto& material : metadata.materials) {
        hasAnyMaterialMass = hasAnyMaterialMass || material.grams.has_value();
    }

    if (generatorText.isEmpty()) {
        addWarning(metadata.warnings, QStringLiteral("O fatiador não foi identificado no cabeçalho."));
    } else if (metadata.slicerName.isEmpty()) {
        metadata.slicerName = QStringLiteral("Fatiador não identificado (%1)").arg(generatorText);
    }
    if (!metadata.printTimeSeconds) {
        addWarning(metadata.warnings, QStringLiteral("Não foi encontrado um tempo final de impressão reconhecido."));
    }
    if (perMaterialGrams.isEmpty() && !metadata.totalFilamentGrams) {
        addWarning(metadata.warnings, QStringLiteral("Não foi encontrado consumo de filamento em gramas."));
    }
    if (!hasAnyMaterialMass && metadata.materials.size() > 1 && metadata.totalFilamentGrams) {
        addWarning(metadata.warnings,
            QStringLiteral("O arquivo informa o peso total, mas não divide as gramas por material."));
    }
    const auto fallbackTime = totalEstimatedSeconds ? totalEstimatedSeconds
        : (estimateSeconds ? estimateSeconds : modelSeconds);
    if (statsSeconds && fallbackTime && std::abs(*statsSeconds - *fallbackTime) > 1.0) {
        addWarning(metadata.warnings,
            QStringLiteral("O tempo do bloco statistics difere da última estimativa normal encontrada."));
    }
    if (metadata.totalFilamentGrams && perMaterialTotal
        && std::abs(*metadata.totalFilamentGrams - *perMaterialTotal) > 0.02) {
        addWarning(metadata.warnings,
            QStringLiteral("O total de gramas difere da soma dos valores por material."));
    }

    return metadata;
}

} // namespace gcode
