#include "calculate_cost.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace calculation {
namespace {

double readNonNegative(double value, const std::string& field)
{
    if (!std::isfinite(value) || value < 0.0) {
        throw std::range_error(field + " deve ser um numero finito maior ou igual a zero.");
    }
    return value;
}

double readPercent(double value, const std::string& field)
{
    readNonNegative(value, field);
    if (value > 90.0) {
        throw std::range_error(field + " deve estar entre 0% e 90%.");
    }
    return value;
}

double roundMoney(double value)
{
    if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<double>::max() / 100.0) {
        throw std::range_error("O resultado excede o limite numerico suportado.");
    }
    return std::round((value + 1e-12) * 100.0) / 100.0;
}

} // namespace

CostBreakdown calculateCost(const CalculationInput& input)
{
    if (input.materials.empty()) {
        throw std::invalid_argument("Informe ao menos um material.");
    }

    const double printTimeSeconds = readNonNegative(input.printTimeSeconds, "Tempo de impressao");
    const double powerWatts = readNonNegative(input.powerWatts, "Consumo em watts");
    const double electricityRate = readNonNegative(input.electricityRatePerKwh, "Tarifa em R$/kWh");
    const double electricityFlag = readNonNegative(input.electricityFlagPerKwh, "Acrescimo da bandeira");
    const double printerPrice = readNonNegative(input.printerPurchasePrice, "Preco da impressora");
    const double printerLife = readNonNegative(input.printerLifeHours, "Vida util da impressora");
    const double failureRate = readPercent(input.failureRatePercent, "Taxa de falhas");
    const double labor = readNonNegative(input.laborCost, "Mao de obra");
    const double fixed = readNonNegative(input.fixedCost, "Custo fixo");
    const double packaging = readNonNegative(input.packagingCost, "Embalagem");
    const double shipping = readNonNegative(input.shippingCost, "Frete");
    const double profitValue = readNonNegative(input.profitValue, "Lucro");
    const double salesFeePercent = readPercent(input.salesFeePercent, "Taxas e impostos sobre a venda");

    if ((printerPrice > 0.0) != (printerLife > 0.0)) {
        throw std::range_error("Informe o preco e a vida util da impressora juntos para calcular o desgaste.");
    }
    if (input.shippingMode != "none" && input.shippingMode != "pass-through" && input.shippingMode != "cost") {
        throw std::range_error("Escolha uma forma valida de tratar o frete.");
    }
    if (shipping > 0.0 && input.shippingMode == "none") {
        throw std::range_error("Escolha como tratar o frete informado.");
    }
    if (input.profitMode != "percent" && input.profitMode != "fixed") {
        throw std::range_error("Escolha entre lucro percentual ou valor fixo.");
    }

    CostBreakdown result;
    result.durationHours = printTimeSeconds / 3600.0;

    for (std::size_t index = 0; index < input.materials.size(); ++index) {
        const auto& material = input.materials[index];
        const std::string prefix = "Material " + std::to_string(index + 1);
        const double grams = readNonNegative(material.grams, prefix + " (gramas)");
        const double pricePerKg = readNonNegative(material.pricePerKg, prefix + " (preco por kg)");
        const double materialCost = grams * pricePerKg / 1000.0;
        result.materialCost += materialCost;
        result.materials.push_back({material.name, grams, pricePerKg, roundMoney(materialCost)});
    }

    result.effectiveElectricityRatePerKwh = electricityRate + electricityFlag;
    result.electricityCost = powerWatts * result.durationHours / 1000.0 * result.effectiveElectricityRatePerKwh;
    if (printerLife > 0.0) {
        result.printerWear = printerPrice / printerLife * result.durationHours;
    }

    const double failureBasis = result.materialCost + result.electricityCost + result.printerWear;
    result.failureAllowance = failureRate > 0.0
        ? failureBasis * failureRate / (100.0 - failureRate)
        : 0.0;
    result.shippingIncludedInCost = input.shippingMode == "cost" ? shipping : 0.0;
    result.shippingPassedThrough = input.shippingMode == "pass-through" ? shipping : 0.0;

    result.costTotal = result.materialCost
        + result.electricityCost
        + result.printerWear
        + result.failureAllowance
        + labor + fixed + packaging + result.shippingIncludedInCost;

    result.profit = input.profitMode == "fixed"
        ? profitValue
        : result.costTotal * profitValue / 100.0;
    const double taxRate = salesFeePercent / 100.0;
    result.priceBeforePassThroughShipping = (result.costTotal + result.profit) / (1.0 - taxRate);
    result.salesFees = result.priceBeforePassThroughShipping * taxRate;
    result.salePrice = result.priceBeforePassThroughShipping + result.shippingPassedThrough;
    result.realMarginPercent = result.salePrice > 0.0
        ? result.profit / result.salePrice * 100.0
        : 0.0;

    result.materialCost = roundMoney(result.materialCost);
    result.effectiveElectricityRatePerKwh = roundMoney(result.effectiveElectricityRatePerKwh);
    result.electricityCost = roundMoney(result.electricityCost);
    result.printerWear = roundMoney(result.printerWear);
    result.failureAllowance = roundMoney(result.failureAllowance);
    result.labor = roundMoney(labor);
    result.fixed = roundMoney(fixed);
    result.packaging = roundMoney(packaging);
    result.shippingIncludedInCost = roundMoney(result.shippingIncludedInCost);
    result.shippingPassedThrough = roundMoney(result.shippingPassedThrough);
    result.costTotal = roundMoney(result.costTotal);
    result.profit = roundMoney(result.profit);
    result.salesFees = roundMoney(result.salesFees);
    result.priceBeforePassThroughShipping = roundMoney(result.priceBeforePassThroughShipping);
    result.salePrice = roundMoney(result.salePrice);
    result.realMarginPercent = roundMoney(result.realMarginPercent);
    return result;
}

} // namespace calculation
