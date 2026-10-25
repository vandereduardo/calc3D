/**
 * Streaming metadata reader for slicer-generated G-code.
 *
 * The caller supplies an async iterable of lines (for example, from Node's
 * readline over a file stream or a browser's decoded File stream). This module
 * never executes G-code and does not retain the file contents.
 */

const MAX_LINE_LENGTH = 1_000_000;
const RELEVANT_FIELDS = new Set([
  "printer_model",
  "filament_type",
  "filament_colour",
  "filament_settings_id",
  "filament used [g]",
  "total filament used [g]",
  "filament used [mm]",
  "filament used [cm3]",
  "estimated printing time (normal mode)",
  "print_time",
]);

function normalizeKey(value) {
  return value.trim().toLowerCase().replace(/\s+/g, " ");
}

function splitDelimited(value, delimiter) {
  const parts = [];
  let current = "";
  let quote = null;
  let escaped = false;

  for (const char of value) {
    if (escaped) {
      current += char;
      escaped = false;
      continue;
    }

    if (char === "\\") {
      current += char;
      escaped = true;
      continue;
    }

    if ((char === '"' || char === "'") && (!quote || quote === char)) {
      quote = quote ? null : char;
      current += char;
      continue;
    }

    if (char === delimiter && !quote) {
      parts.push(current.trim());
      current = "";
      continue;
    }

    current += char;
  }

  parts.push(current.trim());
  return parts.map((part) => unquote(part));
}

function unquote(value) {
  const trimmed = value.trim();
  if (trimmed.length >= 2) {
    const first = trimmed[0];
    const last = trimmed[trimmed.length - 1];
    if ((first === '"' || first === "'") && last === first) {
      return trimmed.slice(1, -1).replace(/\\([\\"'])/g, "$1");
    }
  }
  return trimmed;
}

function parseNumberList(value, key, warnings) {
  if (value == null || value.trim() === "") return [];

  return splitDelimited(value, ",").map((part, index) => {
    if (part.trim() === "") {
      warnings.push({
        code: "invalid-number",
        field: key,
        index,
        message: `O valor ${index + 1} de ${key} está vazio.`,
      });
      return null;
    }
    const number = Number(part);
    if (!Number.isFinite(number)) {
      warnings.push({
        code: "invalid-number",
        field: key,
        index,
        message: `Não foi possível interpretar o valor ${index + 1} de ${key}.`,
      });
      return null;
    }
    return number;
  });
}

function parseDurationSeconds(value) {
  if (!value) return null;
  const text = value.trim();

  const clock = text.match(/(?:^|\s)(\d+):(\d{1,2}):(\d{1,2}(?:\.\d+)?)(?:\s|$)/);
  if (clock) {
    return Number(clock[1]) * 3600 + Number(clock[2]) * 60 + Number(clock[3]);
  }

  const hours = text.match(/(\d+(?:\.\d+)?)\s*h\b/i);
  const minutes = text.match(/(\d+(?:\.\d+)?)\s*m\b/i);
  const seconds = text.match(/(\d+(?:\.\d+)?)\s*s\b/i);
  if (!hours && !minutes && !seconds) return null;

  return Number(hours?.[1] ?? 0) * 3600
    + Number(minutes?.[1] ?? 0) * 60
    + Number(seconds?.[1] ?? 0);
}

function parseGenerator(value) {
  if (!value) return { id: "unknown", name: null, version: null };

  const anycubicNext = value.match(/Anycubic\s*Slicer\s*Next|AnycubicSlicerNext/i);
  const version = value.match(/\b(\d+\.\d+(?:\.\d+){0,2})\b/)?.[1] ?? null;

  if (anycubicNext) {
    return { id: "anycubic-slicer-next", name: "Anycubic Slicer Next", version, raw: value.trim() };
  }

  return { id: "unknown", name: null, version, raw: value.trim() };
}

function getLast(fields, key) {
  const values = fields.get(key);
  return values?.at(-1) ?? null;
}

function alignList(values, count, field, warnings) {
  if (!values.length) return [];
  if (values.length !== count) {
    warnings.push({
      code: "material-list-length-mismatch",
      field,
      expected: count,
      found: values.length,
      message: `A lista ${field} tem ${values.length} itens, mas o consumo tem ${count}.`,
    });
  }
  return values;
}

/**
 * Parse metadata from an async iterable of text lines.
 *
 * @param {AsyncIterable<string>} lines Lines from a streaming reader.
 * @returns {Promise<object>} Normalized metadata and non-fatal warnings.
 */
export async function parseGcodeLines(lines) {
  const fields = new Map();
  const warnings = [];
  let generatorText = null;
  let printerModel = null;
  let inStatistics = false;
  let statsPrintTime = null;
  let lastNormalEstimate = null;
  let lineNumber = 0;

  for await (const line of lines) {
    lineNumber += 1;
    if (typeof line !== "string") {
      warnings.push({
        code: "non-text-line",
        line: lineNumber,
        message: "Uma linha não textual foi ignorada.",
      });
      continue;
    }

    if (line.length > MAX_LINE_LENGTH) {
      warnings.push({
        code: "line-too-long",
        line: lineNumber,
        message: "Uma linha excede o limite de leitura e foi ignorada.",
      });
      continue;
    }

    const comment = line.match(/^\s*;\s?(.*)$/);
    if (!comment) continue;

    const payload = comment[1].trim();
    const normalizedPayload = normalizeKey(payload);

    if (normalizedPayload === "statistics = begin") {
      inStatistics = true;
      continue;
    }
    if (normalizedPayload === "statistics = end") {
      inStatistics = false;
      continue;
    }

    const generatorMatch = payload.match(/^generated by\s+(.+)$/i);
    if (generatorMatch) generatorText = generatorMatch[1].trim();

    const separator = payload.indexOf("=");
    if (separator < 0) continue;

    const key = normalizeKey(payload.slice(0, separator));
    const value = payload.slice(separator + 1).trim();
    if (key === "printer_model") printerModel = unquote(value);
    if (key === "print_time" && inStatistics) statsPrintTime = value;
    if (key === "estimated printing time (normal mode)") lastNormalEstimate = value;
    if (RELEVANT_FIELDS.has(key)) {
      const list = fields.get(key) ?? [];
      list.push(value);
      fields.set(key, list);
    }
  }

  const slicer = parseGenerator(generatorText);
  const statsSeconds = parseDurationSeconds(statsPrintTime);
  const estimateSeconds = parseDurationSeconds(lastNormalEstimate);
  const printTimeSeconds = statsSeconds ?? estimateSeconds;
  const printTimeSource = statsSeconds != null
    ? "statistics.print_time"
    : estimateSeconds != null
      ? "estimated printing time (normal mode)"
      : null;

  const perMaterialGrams = parseNumberList(getLast(fields, "filament used [g]"), "filament used [g]", warnings);
  const totalList = parseNumberList(getLast(fields, "total filament used [g]"), "total filament used [g]", warnings);
  const perMaterialTotal = perMaterialGrams.length && perMaterialGrams.every((value) => value != null)
    ? perMaterialGrams.reduce((sum, value) => sum + value, 0)
    : null;
  const totalFilamentGrams = totalList[0] ?? perMaterialTotal;

  const materialTypes = alignList(splitDelimited(getLast(fields, "filament_type") ?? "", ";"), perMaterialGrams.length, "filament_type", warnings);
  const materialColors = alignList(splitDelimited(getLast(fields, "filament_colour") ?? "", ";"), perMaterialGrams.length, "filament_colour", warnings);
  const materialProfiles = alignList(splitDelimited(getLast(fields, "filament_settings_id") ?? "", ";"), perMaterialGrams.length, "filament_settings_id", warnings);
  const filamentLengths = alignList(parseNumberList(getLast(fields, "filament used [mm]"), "filament used [mm]", warnings), perMaterialGrams.length, "filament used [mm]", warnings);
  const filamentVolumes = alignList(parseNumberList(getLast(fields, "filament used [cm3]"), "filament used [cm3]", warnings), perMaterialGrams.length, "filament used [cm3]", warnings);

  const materials = perMaterialGrams.map((massGrams, index) => ({
    index,
    massGrams,
    type: materialTypes[index] ?? null,
    color: materialColors[index] ?? null,
    profile: materialProfiles[index] ?? null,
    lengthMm: filamentLengths[index] ?? null,
    volumeCm3: filamentVolumes[index] ?? null,
  }));

  if (!generatorText) {
    warnings.push({ code: "slicer-not-detected", message: "O fatiador não foi identificado no cabeçalho." });
  }
  if (printerModel && !/anycubic\s*kobra\s*x/i.test(printerModel)) {
    warnings.push({ code: "printer-not-in-initial-scope", message: "A impressora não é a Kobra X do primeiro perfil planejado." });
  }
  if (printTimeSeconds == null) {
    warnings.push({ code: "print-time-not-found", message: "Não foi encontrado um tempo final de impressão reconhecido." });
  }
  if (!perMaterialGrams.length && !totalList.length) {
    warnings.push({ code: "filament-usage-not-found", message: "Não foi encontrado consumo de filamento em gramas." });
  }
  if (statsSeconds != null && estimateSeconds != null && Math.abs(statsSeconds - estimateSeconds) > 1) {
    warnings.push({
      code: "print-time-mismatch",
      statisticsSeconds: statsSeconds,
      estimateSeconds,
      message: "O tempo do bloco statistics difere da última estimativa normal encontrada.",
    });
  }
  if (totalList.length && perMaterialTotal != null) {
    if (Math.abs(perMaterialTotal - totalFilamentGrams) > 0.02) {
      warnings.push({
        code: "filament-total-mismatch",
        perMaterialGrams: perMaterialTotal,
        totalFilamentGrams,
        message: "O total de gramas difere da soma dos valores por material.",
      });
    }
  }

  return {
    slicer,
    printerModel,
    printTime: {
      seconds: printTimeSeconds,
      source: printTimeSource,
      statisticsSeconds: statsSeconds,
      lastNormalEstimateSeconds: estimateSeconds,
    },
    materials,
    totalFilamentGrams,
    materialCount: perMaterialGrams.length,
    warnings,
    detectedFields: [...fields.keys()],
  };
}
