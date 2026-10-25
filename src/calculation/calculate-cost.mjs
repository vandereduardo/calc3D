const MAX_RATE_PERCENT = 90;

function readNonNegative(value, field, { required = false } = {}) {
  if (value == null || value === "") {
    if (required) throw new TypeError(`${field} é obrigatório.`);
    return 0;
  }

  const number = Number(value);
  if (!Number.isFinite(number) || number < 0) {
    throw new RangeError(`${field} deve ser um número finito maior ou igual a zero.`);
  }
  return number;
}

function readPercent(value, field) {
  const number = readNonNegative(value, field);
  if (number > MAX_RATE_PERCENT) {
    throw new RangeError(`${field} deve estar entre 0% e ${MAX_RATE_PERCENT}%.`);
  }
  return number;
}

function roundMoney(value) {
  return Math.round((value + Number.EPSILON) * 100) / 100;
}

/**
 * Calculate a per-piece FDM print cost and suggested sale price.
 *
 * The caller supplies the electricity rate. This module intentionally does not
 * embed state tariffs until a verifiable source and validity period are chosen.
 *
 * @param {object} input
 * @param {Array<{grams:number, pricePerKg:number, name?:string, color?:string}>} input.materials
 * @param {number} input.printTimeSeconds
 * @param {number} input.powerWatts
 * @param {number} input.electricityRatePerKwh
 * @param {number} [input.electricityFlagPerKwh=0]
 * @param {number} [input.printerPurchasePrice=0]
 * @param {number} [input.printerLifeHours=0]
 * @param {number} [input.failureRatePercent=0]
 * @param {number} [input.laborCost=0]
 * @param {number} [input.fixedCost=0]
 * @param {number} [input.packagingCost=0]
 * @param {number} [input.shippingCost=0]
 * @param {"none"|"pass-through"|"cost"} [input.shippingMode="none"]
 * @param {"percent"|"fixed"} [input.profitMode="percent"]
 * @param {number} [input.profitValue=0]
 * @param {number} [input.salesFeePercent=0]
 * @returns {object} Rounded cost breakdown, profit, taxes, suggested price, and margin.
 */
export function calculateCost(input) {
  if (!input || typeof input !== "object") {
    throw new TypeError("Os dados do cálculo são obrigatórios.");
  }
  if (!Array.isArray(input.materials) || input.materials.length === 0) {
    throw new TypeError("Informe ao menos um material.");
  }

  const printTimeSeconds = readNonNegative(input.printTimeSeconds, "Tempo de impressão", { required: true });
  const powerWatts = readNonNegative(input.powerWatts, "Consumo em watts", { required: true });
  const electricityRatePerKwh = readNonNegative(input.electricityRatePerKwh, "Tarifa em R$/kWh", { required: true });
  const electricityFlagPerKwh = readNonNegative(input.electricityFlagPerKwh, "Acréscimo da bandeira");
  const printerPurchasePrice = readNonNegative(input.printerPurchasePrice, "Preço da impressora");
  const printerLifeHours = readNonNegative(input.printerLifeHours, "Vida útil da impressora");
  const failureRatePercent = readPercent(input.failureRatePercent, "Taxa de falhas");
  const laborCost = readNonNegative(input.laborCost, "Mão de obra");
  const fixedCost = readNonNegative(input.fixedCost, "Custo fixo");
  const packagingCost = readNonNegative(input.packagingCost, "Embalagem");
  const shippingCost = readNonNegative(input.shippingCost, "Frete");
  const salesFeePercent = readPercent(input.salesFeePercent, "Taxas e impostos sobre a venda");

  if ((printerPurchasePrice > 0) !== (printerLifeHours > 0)) {
    throw new RangeError("Informe preço e vida útil da impressora juntos para calcular o desgaste.");
  }
  if (!["none", "pass-through", "cost"].includes(input.shippingMode ?? "none")) {
    throw new RangeError("O modo de frete deve ser none, pass-through ou cost.");
  }
  if (shippingCost > 0 && (input.shippingMode ?? "none") === "none") {
    throw new RangeError("Escolha como tratar o frete informado.");
  }
  if (!["percent", "fixed"].includes(input.profitMode ?? "percent")) {
    throw new RangeError("O modo de lucro deve ser percent ou fixed.");
  }

  const materials = input.materials.map((material, index) => {
    if (!material || typeof material !== "object") {
      throw new TypeError(`O material ${index + 1} deve ser um objeto com gramas e preço por kg.`);
    }
    const grams = readNonNegative(material?.grams, `Gramas do material ${index + 1}`, { required: true });
    const pricePerKg = readNonNegative(material?.pricePerKg, `Preço por kg do material ${index + 1}`, { required: true });
    return {
      name: material.name ?? null,
      color: material.color ?? null,
      grams,
      pricePerKg,
      cost: grams * pricePerKg / 1000,
    };
  });

  const printTimeHours = printTimeSeconds / 3600;
  const materialCost = materials.reduce((sum, material) => sum + material.cost, 0);
  const effectiveRatePerKwh = electricityRatePerKwh + electricityFlagPerKwh;
  const electricityCost = powerWatts * printTimeHours / 1000 * effectiveRatePerKwh;
  const wearCost = printerLifeHours > 0
    ? printerPurchasePrice / printerLifeHours * printTimeHours
    : 0;
  const failureBasis = materialCost + electricityCost + wearCost;
  const failureAllowance = failureRatePercent > 0
    ? failureBasis * failureRatePercent / (100 - failureRatePercent)
    : 0;

  const shippingMode = input.shippingMode ?? "none";
  const includedShipping = shippingMode === "cost" ? shippingCost : 0;
  const passThroughShipping = shippingMode === "pass-through" ? shippingCost : 0;
  const costTotal = materialCost
    + electricityCost
    + wearCost
    + failureAllowance
    + laborCost
    + fixedCost
    + packagingCost
    + includedShipping;

  const profitValue = readNonNegative(input.profitValue, "Lucro");
  const profitAmount = input.profitMode === "fixed"
    ? profitValue
    : costTotal * profitValue / 100;
  const taxRate = salesFeePercent / 100;
  const priceBeforePassThroughShipping = (costTotal + profitAmount) / (1 - taxRate);
  const salesFees = priceBeforePassThroughShipping * taxRate;
  const salePrice = priceBeforePassThroughShipping + passThroughShipping;
  const realMarginPercent = salePrice > 0 ? profitAmount / salePrice * 100 : 0;

  return {
    currency: "BRL",
    durationHours: printTimeHours,
    materials: materials.map((material) => ({ ...material, cost: roundMoney(material.cost) })),
    effectiveElectricityRatePerKwh: roundMoney(effectiveRatePerKwh),
    components: {
      materials: roundMoney(materialCost),
      electricity: roundMoney(electricityCost),
      printerWear: roundMoney(wearCost),
      failureAllowance: roundMoney(failureAllowance),
      labor: roundMoney(laborCost),
      fixed: roundMoney(fixedCost),
      packaging: roundMoney(packagingCost),
      shippingIncludedInCost: roundMoney(includedShipping),
      shippingPassedThrough: roundMoney(passThroughShipping),
    },
    costTotal: roundMoney(costTotal),
    profit: roundMoney(profitAmount),
    salesFees: roundMoney(salesFees),
    priceBeforePassThroughShipping: roundMoney(priceBeforePassThroughShipping),
    salePrice: roundMoney(salePrice),
    realMarginPercent: roundMoney(realMarginPercent),
  };
}
