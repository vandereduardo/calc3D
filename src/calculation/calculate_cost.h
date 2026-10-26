#pragma once

#include <string>
#include <vector>

namespace calculation {

struct MaterialInput final {
    std::string name;
    double grams = 0.0;
    double pricePerKg = 0.0;
};

struct CalculationInput final {
    std::vector<MaterialInput> materials;
    double printTimeSeconds = 0.0;
    double powerWatts = 0.0;
    double electricityRatePerKwh = 0.0;
    double electricityFlagPerKwh = 0.0;
    double printerPurchasePrice = 0.0;
    double printerLifeHours = 0.0;
    double failureRatePercent = 0.0;
    double laborCost = 0.0;
    double fixedCost = 0.0;
    double packagingCost = 0.0;
    double shippingCost = 0.0;
    std::string shippingMode = "none";
    std::string profitMode = "percent";
    double profitValue = 0.0;
    double salesFeePercent = 0.0;
};

struct MaterialResult final {
    std::string name;
    double grams = 0.0;
    double pricePerKg = 0.0;
    double cost = 0.0;
};

struct CostBreakdown final {
    double durationHours = 0.0;
    std::vector<MaterialResult> materials;
    double effectiveElectricityRatePerKwh = 0.0;
    double materialCost = 0.0;
    double electricityCost = 0.0;
    double printerWear = 0.0;
    double failureAllowance = 0.0;
    double labor = 0.0;
    double fixed = 0.0;
    double packaging = 0.0;
    double shippingIncludedInCost = 0.0;
    double shippingPassedThrough = 0.0;
    double costTotal = 0.0;
    double profit = 0.0;
    double salesFees = 0.0;
    double priceBeforePassThroughShipping = 0.0;
    double salePrice = 0.0;
    double realMarginPercent = 0.0;
};

CostBreakdown calculateCost(const CalculationInput& input);

} // namespace calculation
