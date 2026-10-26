#pragma once

#include <QMainWindow>

#include <optional>
#include <vector>

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class QWidget;

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    struct MaterialRow final {
        QWidget* container = nullptr;
        QLabel* name = nullptr;
        QLabel* colorSwatch = nullptr;
        QLineEdit* grams = nullptr;
        QLineEdit* pricePerKg = nullptr;
        QLabel* cost = nullptr;
        QString importedName;
    };

    void addMaterialRow();
    void removeMaterialRow(QWidget* container);
    void chooseGcodeFile();
    void updateMaterialWeightTotal();
    void recalculate();
    void clearCalculation();
    void showEmptyResults();

    QVBoxLayout* materialsLayout_ = nullptr;
    std::vector<MaterialRow> materialRows_;
    QSpinBox* printHours_ = nullptr;
    QSpinBox* printMinutes_ = nullptr;
    QLineEdit* powerWatts_ = nullptr;
    QComboBox* stateTariffCombo_ = nullptr;
    QLineEdit* electricityRate_ = nullptr;
    QLineEdit* electricityFlag_ = nullptr;
    QLineEdit* printerPurchasePrice_ = nullptr;
    QLineEdit* printerLifeHours_ = nullptr;
    QLineEdit* failureRate_ = nullptr;
    QLineEdit* laborCost_ = nullptr;
    QLineEdit* fixedCost_ = nullptr;
    QLineEdit* packagingCost_ = nullptr;
    QLineEdit* shippingCost_ = nullptr;
    QComboBox* shippingMode_ = nullptr;
    QComboBox* profitMode_ = nullptr;
    QLineEdit* profitValue_ = nullptr;
    QLineEdit* salesFeePercent_ = nullptr;
    QLabel* selectedFileLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* summaryCostLabel_ = nullptr;
    QLabel* summarySalePriceLabel_ = nullptr;
    QLabel* materialWeightTotalLabel_ = nullptr;
    QLabel* materialTotalLabel_ = nullptr;
    QLabel* electricityLabel_ = nullptr;
    QLabel* printerWearLabel_ = nullptr;
    QLabel* failureAllowanceLabel_ = nullptr;
    QLabel* laborLabel_ = nullptr;
    QLabel* fixedLabel_ = nullptr;
    QLabel* packagingLabel_ = nullptr;
    QLabel* shippingInCostLabel_ = nullptr;
    QLabel* costTotalLabel_ = nullptr;
    QLabel* profitLabel_ = nullptr;
    QLabel* salesFeesLabel_ = nullptr;
    QLabel* passThroughShippingLabel_ = nullptr;
    QLabel* salePriceLabel_ = nullptr;
    QLabel* marginLabel_ = nullptr;
    QLabel* preShippingPriceLabel_ = nullptr;
    std::optional<double> importedPrintTimeSeconds_;
    std::optional<double> importedTotalFilamentGrams_;
    QString importStatus_;
};
