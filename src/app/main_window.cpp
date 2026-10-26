#include "main_window.h"

#include "../calculation/calculate_cost.h"
#include "../gcode/gcode_metadata.h"

#include <QColor>
#include <QComboBox>
#include <QPalette>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QLocale>
#include <QWidget>
#include <QVBoxLayout>

#include <cmath>
#include <stdexcept>

namespace {

QLineEdit* makeNumericField(QWidget* parent, const QString& placeholder)
{
    auto* field = new QLineEdit(parent);
    field->setPlaceholderText(placeholder);
    field->setClearButtonEnabled(true);
    return field;
}

QWidget* makeLabeledField(QWidget* parent, const QString& labelText, QWidget* field)
{
    auto* container = new QWidget(parent);
    auto* fieldLayout = new QVBoxLayout(container);
    fieldLayout->setContentsMargins(0, 0, 0, 0);
    fieldLayout->setSpacing(4);
    auto* label = new QLabel(labelText, container);
    label->setObjectName("fieldLabel");
    fieldLayout->addWidget(label);
    fieldLayout->addWidget(field);
    return container;
}

double parseNumber(
    const QLineEdit* field, const QString& name, bool required = false, bool allowPercentSuffix = false)
{
    QString text = field->text().trimmed();
    if (text.isEmpty()) {
        if (required) {
            const std::string message = QStringLiteral("Preencha o campo %1.").arg(name).toUtf8().constData();
            throw std::invalid_argument(message);
        }
        return 0.0;
    }
    if (allowPercentSuffix && text.endsWith(QLatin1Char('%'))) {
        text.chop(1);
        text = text.trimmed();
    }

    const qsizetype comma = text.lastIndexOf(QLatin1Char(','));
    const qsizetype dot = text.lastIndexOf(QLatin1Char('.'));
    QChar decimalSeparator;
    if (comma >= 0 && dot >= 0) {
        decimalSeparator = comma > dot ? QLatin1Char(',') : QLatin1Char('.');
    } else if (comma >= 0) {
        decimalSeparator = QLatin1Char(',');
    } else if (dot >= 0) {
        decimalSeparator = QLatin1Char('.');
    }

    if (!decimalSeparator.isNull()) {
        const QChar groupSeparator = decimalSeparator == QLatin1Char(',')
            ? QLatin1Char('.') : QLatin1Char(',');
        text.remove(groupSeparator);
        if (decimalSeparator != QLatin1Char('.')) {
            text.replace(decimalSeparator, QLatin1Char('.'));
        }
    }

    bool ok = false;
    const double value = text.toDouble(&ok);
    if (!ok || !std::isfinite(value) || value < 0.0) {
        const std::string message = QStringLiteral("%1 deve ser um numero valido maior ou igual a zero.")
            .arg(name).toUtf8().constData();
        throw std::invalid_argument(message);
    }
    return value;
}

QString currency(double amount)
{
    return QLocale(QLocale::Portuguese, QLocale::Brazil).toCurrencyString(amount, QStringLiteral("R$"), 2);
}

void setMoney(QLabel* label, double amount)
{
    label->setText(currency(amount));
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("calc3D — custo de impressão 3D");
    resize(1180, 820);
    setMinimumSize(920, 640);

    auto* content = new QWidget(this);
    content->setObjectName("appContent");
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 14, 20, 14);
    layout->setSpacing(0);

    auto* header = new QWidget(content);
    header->setObjectName("appHeader");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 12);
    headerLayout->setSpacing(14);
    auto* brand = new QWidget(header);
    auto* brandLayout = new QHBoxLayout(brand);
    brandLayout->setContentsMargins(0, 0, 0, 0);
    brandLayout->setSpacing(11);
    auto* brandMark = new QLabel("3D", brand);
    brandMark->setObjectName("brandMark");
    brandMark->setFixedSize(34, 34);
    brandMark->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(brandMark);
    auto* brandText = new QVBoxLayout();
    brandText->setContentsMargins(0, 0, 0, 0);
    brandText->setSpacing(1);
    auto* title = new QLabel("Orçamento da impressão", brand);
    title->setObjectName("appTitle");
    auto* description = new QLabel("Custos por peça, com valores editáveis", brand);
    description->setObjectName("appSubtitle");
    brandText->addWidget(title);
    brandText->addWidget(description);
    brandLayout->addLayout(brandText);
    headerLayout->addWidget(brand);
    headerLayout->addStretch(1);

    auto* fileBox = new QWidget(header);
    fileBox->setObjectName("fileBox");
    auto* fileRow = new QHBoxLayout(fileBox);
    fileRow->setContentsMargins(8, 6, 8, 6);
    fileRow->setSpacing(10);
    auto* openButton = new QPushButton("Abrir G-code", fileBox);
    openButton->setObjectName("openGcodeButton");
    connect(openButton, &QPushButton::clicked, this, [this] { chooseGcodeFile(); });
    fileRow->addWidget(openButton);
    selectedFileLabel_ = new QLabel("Nenhum arquivo selecionado", fileBox);
    selectedFileLabel_->setObjectName("selectedFileLabel");
    selectedFileLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    fileRow->addWidget(selectedFileLabel_, 1);
    headerLayout->addWidget(fileBox, 1);
    layout->addWidget(header);

    auto* summaryGroup = new QGroupBox("Preço por peça", content);
    summaryGroup->setObjectName("summaryCard");
    auto* summaryForm = new QFormLayout(summaryGroup);
    summaryCostLabel_ = new QLabel("—", summaryGroup);
    summaryCostLabel_->setObjectName("summaryCostValue");
    summaryForm->addRow("Custo total", summaryCostLabel_);
    summarySalePriceLabel_ = new QLabel("—", summaryGroup);
    summarySalePriceLabel_->setObjectName("summarySaleValue");
    QFont summaryPriceFont = summarySalePriceLabel_->font();
    summaryPriceFont.setBold(true);
    summaryPriceFont.setPointSize(summaryPriceFont.pointSize() + 2);
    summarySalePriceLabel_->setFont(summaryPriceFont);
    summaryForm->addRow("Preço de venda sugerido", summarySalePriceLabel_);
    auto* scrollArea = new QScrollArea(content);
    scrollArea->setWidgetResizable(true);
    auto* scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("mainFormSurface");
    auto* pageLayout = new QGridLayout(scrollContent);
    pageLayout->setContentsMargins(12, 2, 12, 10);
    pageLayout->setHorizontalSpacing(18);
    pageLayout->setVerticalSpacing(10);
    pageLayout->setColumnStretch(0, 1);
    pageLayout->setColumnStretch(1, 1);

    auto* timeGroup = new QGroupBox("Impressão e energia", scrollContent);
    timeGroup->setObjectName("inputSection");
    auto* timeForm = new QGridLayout(timeGroup);
    timeForm->setContentsMargins(0, 4, 0, 0);
    timeForm->setHorizontalSpacing(10);
    timeForm->setVerticalSpacing(8);
    auto* timeRowWidget = new QWidget(timeGroup);
    auto* timeRow = new QHBoxLayout(timeRowWidget);
    timeRow->setContentsMargins(0, 0, 0, 0);
    timeRow->setSpacing(6);
    printHours_ = new QSpinBox(timeGroup);
    printHours_->setRange(0, 99999);
    printHours_->setSuffix(" h");
    printHours_->setSpecialValueText("0 h");
    printMinutes_ = new QSpinBox(timeGroup);
    printMinutes_->setRange(0, 59);
    printMinutes_->setSuffix(" min");
    timeRow->addWidget(printHours_);
    timeRow->addWidget(printMinutes_);
    timeRow->addStretch();
    timeForm->addWidget(makeLabeledField(timeGroup, "Tempo de impressão", timeRowWidget), 0, 0);
    powerWatts_ = makeNumericField(timeGroup, "Ex.: 200");
    timeForm->addWidget(makeLabeledField(timeGroup, "Consumo médio (W)", powerWatts_), 0, 1);
    stateTariffCombo_ = new QComboBox(timeGroup);
    stateTariffCombo_->addItem("Selecione seu estado...", QVariant());
    struct StateTariff final {
        const char* label;
        double rate;
    };
    const StateTariff stateTariffs[] = {
        {"Acre - Energisa AC (1.15)", 1.15},
        {"Alagoas - Equatorial AL (1.05)", 1.05},
        {"Amapá - CEA Equatorial (1.1)", 1.10},
        {"Amazonas - Amazonas Energia (1.25)", 1.25},
        {"Bahia - Neoenergia Coelba (1.18)", 1.18},
        {"Ceará - Enel CE (1.12)", 1.12},
        {"Distrito Federal - Neoenergia BSB (0.95)", 0.95},
        {"Espírito Santo - EDP ES (1.08)", 1.08},
        {"Goiás - Equatorial GO (1.1)", 1.10},
        {"Maranhão - Equatorial MA (1.15)", 1.15},
        {"Mato Grosso - Energisa MT (1.22)", 1.22},
        {"Mato Grosso do Sul - Energisa MS (1.2)", 1.20},
        {"Minas Gerais - Cemig (1.1)", 1.10},
        {"Pará - Equatorial PA (1.3)", 1.30},
        {"Paraíba - Energisa PB (1.05)", 1.05},
        {"Paraná - Copel (1.02)", 1.02},
        {"Pernambuco - Neoenergia PE (1.12)", 1.12},
        {"Piauí - Equatorial PI (1.18)", 1.18},
        {"Rio de Janeiro - Light / Enel (1.3)", 1.30},
        {"Rio Grande do Norte - Neoenergia Cosern (1.05)", 1.05},
        {"Rio Grande do Sul - CPFL / CEEE (1.1)", 1.10},
        {"Rondônia - Energisa RO (1.15)", 1.15},
        {"Roraima - Roraima Energia (1.12)", 1.12},
        {"Santa Catarina - Celesc (0.98)", 0.98},
        {"São Paulo - Enel SP / CPFL (1.27)", 1.27},
        {"Sergipe - Energisa SE (1.08)", 1.08},
        {"Tocantins - Energisa TO (1.15)", 1.15}
    };
    for (const StateTariff& tariff : stateTariffs) {
        stateTariffCombo_->addItem(QString::fromUtf8(tariff.label), tariff.rate);
    }
    stateTariffCombo_->setToolTip(
        "Tarifas médias estaduais de referência. Confirme o valor na sua conta de energia.");
    timeForm->addWidget(makeLabeledField(timeGroup, "Estado / distribuidora", stateTariffCombo_), 0, 2);
    electricityRate_ = makeNumericField(timeGroup, "R$/kWh, ex.: 0,75");
    timeForm->addWidget(makeLabeledField(timeGroup, "Tarifa aplicada (R$/kWh)", electricityRate_), 0, 3);
    auto* tariffSourceLabel = new QLabel(
        "Tarifa estadual estimada; ajuste o valor aplicado conforme sua conta de energia.", timeGroup);
    tariffSourceLabel->setObjectName("tariffSource");
    tariffSourceLabel->setWordWrap(true);
    electricityFlag_ = makeNumericField(timeGroup, "0,00 se não houver acréscimo");
    timeForm->addWidget(makeLabeledField(timeGroup, "Acréscimo de bandeira", electricityFlag_), 1, 0);
    timeForm->addWidget(tariffSourceLabel, 1, 1, 1, 3, Qt::AlignVCenter);
    timeForm->setColumnStretch(0, 1);
    timeForm->setColumnStretch(1, 1);
    timeForm->setColumnStretch(2, 2);
    timeForm->setColumnStretch(3, 1);
    pageLayout->addWidget(timeGroup, 0, 0, 1, 2);

    auto* materialsGroup = new QGroupBox("Materiais", scrollContent);
    materialsGroup->setObjectName("inputSection");
    auto* materialsBox = new QVBoxLayout(materialsGroup);
    materialsLayout_ = new QVBoxLayout();
    auto* materialHeader = new QHBoxLayout();
    materialHeader->setSpacing(6);
    auto* materialHeadSwatch = new QLabel(materialsGroup);
    materialHeadSwatch->setFixedWidth(22);
    auto* materialHeadName = new QLabel("FILAMENTO", materialsGroup);
    materialHeadName->setObjectName("materialHeaderLabel");
    materialHeadName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* materialHeadGrams = new QLabel("USO (g)", materialsGroup);
    materialHeadGrams->setObjectName("materialHeaderLabel");
    materialHeadGrams->setFixedWidth(86);
    auto* materialHeadPrice = new QLabel("PREÇO/kg", materialsGroup);
    materialHeadPrice->setObjectName("materialHeaderLabel");
    materialHeadPrice->setFixedWidth(104);
    auto* materialHeadCost = new QLabel("CUSTO", materialsGroup);
    materialHeadCost->setObjectName("materialHeaderLabel");
    materialHeadCost->setFixedWidth(82);
    materialHeader->addWidget(materialHeadSwatch);
    materialHeader->addWidget(materialHeadName, 1);
    materialHeader->addWidget(materialHeadGrams);
    materialHeader->addWidget(materialHeadPrice);
    materialHeader->addWidget(materialHeadCost);
    materialsBox->addLayout(materialHeader);
    materialsBox->addLayout(materialsLayout_);
    auto* addMaterialButton = new QPushButton("+ Adicionar material", materialsGroup);
    connect(addMaterialButton, &QPushButton::clicked, this, [this] { addMaterialRow(); });
    materialsBox->addWidget(addMaterialButton, 0, Qt::AlignLeft);
    materialWeightTotalLabel_ = new QLabel("Peso total dos filamentos: —", materialsGroup);
    materialWeightTotalLabel_->setObjectName("materialWeightTotal");
    materialsBox->addWidget(materialWeightTotalLabel_);
    pageLayout->addWidget(materialsGroup, 1, 0, 1, 2);

    auto* additionalGroup = new QGroupBox("Custos adicionais (opcionais)", scrollContent);
    additionalGroup->setObjectName("inputSection");
    auto* additionalForm = new QGridLayout(additionalGroup);
    additionalForm->setContentsMargins(0, 4, 0, 0);
    additionalForm->setHorizontalSpacing(10);
    additionalForm->setVerticalSpacing(7);
    printerPurchasePrice_ = makeNumericField(additionalGroup, "Preço de compra (R$)");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Impressora (R$)", printerPurchasePrice_), 0, 0);
    printerLifeHours_ = makeNumericField(additionalGroup, "Ex.: 5000");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Vida útil (horas)", printerLifeHours_), 0, 1);
    failureRate_ = makeNumericField(additionalGroup, "0 a 90");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Reserva de falhas (%)", failureRate_), 1, 0);
    laborCost_ = makeNumericField(additionalGroup, "R$ por peça");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Mão de obra (R$)", laborCost_), 1, 1);
    fixedCost_ = makeNumericField(additionalGroup, "R$ por peça");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Custos fixos (R$)", fixedCost_), 2, 0);
    packagingCost_ = makeNumericField(additionalGroup, "R$ por peça");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Embalagem (R$)", packagingCost_), 2, 1);
    shippingCost_ = makeNumericField(additionalGroup, "R$ por pedido/peça");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Frete (R$)", shippingCost_), 3, 0);
    shippingMode_ = new QComboBox(additionalGroup);
    shippingMode_->addItem("Sem frete", "none");
    shippingMode_->addItem("Repassado ao cliente (sem taxas)", "pass-through");
    shippingMode_->addItem("Incluído no custo", "cost");
    additionalForm->addWidget(makeLabeledField(additionalGroup, "Tratamento do frete", shippingMode_), 3, 1);
    pageLayout->addWidget(additionalGroup, 2, 0);

    auto* saleGroup = new QGroupBox("Lucro e venda", scrollContent);
    saleGroup->setObjectName("inputSection");
    auto* saleForm = new QGridLayout(saleGroup);
    saleForm->setContentsMargins(0, 4, 0, 0);
    saleForm->setHorizontalSpacing(10);
    saleForm->setVerticalSpacing(7);
    profitMode_ = new QComboBox(saleGroup);
    profitMode_->addItem("Percentual sobre o custo", "percent");
    profitMode_->addItem("Valor fixo por peça", "fixed");
    saleForm->addWidget(makeLabeledField(saleGroup, "Forma do lucro", profitMode_), 0, 0);
    profitValue_ = makeNumericField(saleGroup, "Porcentagem ou valor em R$");
    saleForm->addWidget(makeLabeledField(saleGroup, "Lucro (%) ou (R$)", profitValue_), 0, 1);
    salesFeePercent_ = makeNumericField(saleGroup, "0 a 90");
    saleForm->addWidget(makeLabeledField(saleGroup, "Taxas sobre a venda (%)", salesFeePercent_), 1, 0, 1, 2);
    pageLayout->addWidget(saleGroup, 2, 1);

    auto* resultsGroup = new QGroupBox("Composição do custo", content);
    resultsGroup->setObjectName("costBreakdownCard");
    auto* resultsForm = new QFormLayout(resultsGroup);
    materialTotalLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Custo total dos filamentos", materialTotalLabel_);
    electricityLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Energia", electricityLabel_);
    printerWearLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Desgaste da impressora", printerWearLabel_);
    failureAllowanceLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Reserva para falhas", failureAllowanceLabel_);
    laborLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Mão de obra", laborLabel_);
    fixedLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Custos fixos", fixedLabel_);
    packagingLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Embalagem", packagingLabel_);
    shippingInCostLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Frete incluído", shippingInCostLabel_);
    costTotalLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Custo total", costTotalLabel_);
    profitLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Lucro", profitLabel_);
    salesFeesLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Taxas da venda", salesFeesLabel_);
    preShippingPriceLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Subtotal sem frete", preShippingPriceLabel_);
    passThroughShippingLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Frete repassado", passThroughShippingLabel_);
    salePriceLabel_ = new QLabel("—", resultsGroup);
    QFont totalFont = salePriceLabel_->font();
    totalFont.setBold(true);
    totalFont.setPointSize(totalFont.pointSize() + 2);
    salePriceLabel_->setFont(totalFont);
    resultsForm->addRow("Preço sugerido", salePriceLabel_);
    marginLabel_ = new QLabel("—", resultsGroup);
    resultsForm->addRow("Margem real", marginLabel_);
    for (QLabel* value : {materialTotalLabel_, electricityLabel_, printerWearLabel_, failureAllowanceLabel_,
                          laborLabel_, fixedLabel_, packagingLabel_, shippingInCostLabel_, costTotalLabel_,
                          profitLabel_, salesFeesLabel_, preShippingPriceLabel_, passThroughShippingLabel_,
                          salePriceLabel_, marginLabel_}) {
        value->setObjectName("breakdownValue");
    }
    scrollArea->setWidget(scrollContent);

    auto* body = new QWidget(content);
    body->setObjectName("appBody");
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(12);
    bodyLayout->addWidget(scrollArea, 1);
    auto* summaryPanel = new QWidget(body);
    summaryPanel->setObjectName("summaryPanel");
    summaryPanel->setFixedWidth(300);
    auto* summaryLayout = new QVBoxLayout(summaryPanel);
    summaryLayout->setContentsMargins(14, 14, 14, 14);
    summaryLayout->setSpacing(8);
    summaryLayout->addWidget(summaryGroup);
    summaryLayout->addWidget(resultsGroup);
    summaryLayout->addStretch();
    bodyLayout->addWidget(summaryPanel);
    layout->addWidget(body, 1);

    addMaterialRow();

    auto* footer = new QWidget(content);
    footer->setObjectName("appFooter");
    auto* footerLayout = new QVBoxLayout(footer);
    footerLayout->setContentsMargins(12, 9, 12, 9);
    footerLayout->setSpacing(6);
    auto* actions = new QHBoxLayout();
    auto* clearButton = new QPushButton("Limpar dados", footer);
    connect(clearButton, &QPushButton::clicked, this, [this] { clearCalculation(); });
    actions->addWidget(clearButton, 0, Qt::AlignLeft);
    actions->addStretch();
    footerLayout->addLayout(actions);

    statusLabel_ = new QLabel(
        "Informe gramas, preço por kg, consumo e tarifa. Ajuste o tempo para calcular a energia.", footer);
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setWordWrap(true);
    footerLayout->addWidget(statusLabel_);
    layout->addWidget(footer);
    setCentralWidget(content);

    const QList<QLineEdit*> numericFields = {
        powerWatts_, electricityRate_, electricityFlag_, printerPurchasePrice_, printerLifeHours_,
        failureRate_, laborCost_, fixedCost_, packagingCost_, shippingCost_, profitValue_, salesFeePercent_
    };
    for (QLineEdit* field : numericFields) {
        connect(field, &QLineEdit::textChanged, this, [this] { recalculate(); });
    }
    connect(printHours_, &QSpinBox::valueChanged, this, [this] {
        importedPrintTimeSeconds_.reset();
        recalculate();
    });
    connect(printMinutes_, &QSpinBox::valueChanged, this, [this] {
        importedPrintTimeSeconds_.reset();
        recalculate();
    });
    connect(stateTariffCombo_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index <= 0) {
            return;
        }
        bool ok = false;
        const double rate = stateTariffCombo_->currentData().toDouble(&ok);
        if (ok) {
            electricityRate_->setText(QLocale::c().toString(rate, 'f', 2));
        }
    });
    connect(electricityRate_, &QLineEdit::textEdited, this, [this] {
        if (stateTariffCombo_->currentIndex() > 0) {
            stateTariffCombo_->setCurrentIndex(0);
        }
    });
    connect(shippingMode_, &QComboBox::currentIndexChanged, this, [this] { recalculate(); });
    connect(profitMode_, &QComboBox::currentIndexChanged, this, [this] { recalculate(); });

    const bool darkMode = palette().color(QPalette::Window).lightness() < 128;
    const QString backgroundColor = darkMode ? QStringLiteral("#141922") : QStringLiteral("#f3f5f8");
    const QString windowColor = darkMode ? QStringLiteral("#1b222d") : QStringLiteral("#ffffff");
    const QString panelColor = darkMode ? QStringLiteral("#202936") : QStringLiteral("#f8fafc");
    const QString inputColor = darkMode ? QStringLiteral("#151c26") : QStringLiteral("#ffffff");
    const QString textColor = darkMode ? QStringLiteral("#eef2f7") : QStringLiteral("#202a38");
    const QString mutedColor = darkMode ? QStringLiteral("#a4afbd") : QStringLiteral("#697586");
    const QString borderColor = darkMode ? QStringLiteral("#344050") : QStringLiteral("#dfe5ec");
    const QString accentColor = darkMode ? QStringLiteral("#8ab4ff") : QStringLiteral("#315fba");
    const QString accentSoftColor = darkMode ? QStringLiteral("#243654") : QStringLiteral("#eaf1ff");
    const QString statusBackgroundColor = darkMode ? QStringLiteral("#3a3020") : QStringLiteral("#fff7df");
    const QString statusTextColor = darkMode ? QStringLiteral("#eed28a") : QStringLiteral("#76561a");
    QString styleSheet = QStringLiteral(R"(
        QMainWindow { background-color: @BACKGROUND@; }
        QWidget { color: @TEXT@; font-family: "Segoe UI"; font-size: 9pt; }
        QWidget#appContent { background-color: @BACKGROUND@; }
        QWidget#appHeader { background-color: @WINDOW@; border-bottom: 1px solid @BORDER@; }
        QLabel { background: transparent; }
        QLabel#brandMark {
            background-color: @ACCENT@; color: #ffffff; border-radius: 9px;
            font-size: 14px; font-weight: 700;
        }
        QLabel#appTitle { color: @TEXT@; font-size: 18px; font-weight: 600; }
        QLabel#appSubtitle { color: @MUTED@; font-size: 12px; }
        QLabel#fieldLabel { color: @MUTED@; font-size: 8pt; }
        QLabel#tariffSource { color: @MUTED@; font-size: 8pt; }
        QWidget#fileBox {
            background-color: @PANEL@; border: 1px solid @BORDER@; border-radius: 7px;
        }
        QLabel#selectedFileLabel { color: @MUTED@; }
        QScrollArea { border: none; background-color: @WINDOW@; }
        QScrollArea QWidget#qt_scrollarea_viewport { background-color: @WINDOW@; }
        QWidget#mainFormSurface { background-color: @WINDOW@; }
        QWidget#appBody { background-color: @WINDOW@; border-left: 1px solid @BORDER@; border-right: 1px solid @BORDER@; }
        QWidget#appFooter { background-color: @WINDOW@; border-top: 1px solid @BORDER@; }
        QGroupBox { background: transparent; border: none; margin: 0; padding: 0; }
        QGroupBox#inputSection {
            border-top: 1px solid @BORDER@; margin-top: 16px; padding-top: 12px;
            font-size: 14px; font-weight: 600;
        }
        QGroupBox#inputSection::title {
            subcontrol-origin: margin; subcontrol-position: top left; left: 0; top: 0;
            padding: 0 8px 0 0; color: @TEXT@; background: transparent;
        }
        QGroupBox#summaryCard { margin: 0; padding: 0; }
        QGroupBox#summaryCard::title, QGroupBox#costBreakdownCard::title {
            color: @TEXT@; font-size: 14px; font-weight: 600;
        }
        QGroupBox#costBreakdownCard {
            border-top: 1px solid @BORDER@; margin-top: 8px; padding-top: 11px;
        }
        QWidget#summaryPanel {
            background-color: @PANEL@; border: 1px solid @BORDER@; border-radius: 9px;
        }
        QLabel#summaryCostValue {
            color: @TEXT@; font-size: 26px; font-weight: 600;
        }
        QLabel#summarySaleValue {
            color: @ACCENT@; font-size: 17px; font-weight: 600;
        }
        QGroupBox#summaryCard QLabel, QGroupBox#costBreakdownCard QLabel { color: @MUTED@; }
        QGroupBox#summaryCard QLabel#summaryCostValue,
        QGroupBox#summaryCard QLabel#summarySaleValue,
        QGroupBox#costBreakdownCard QLabel#breakdownValue { color: @TEXT@; }
        QLabel#breakdownValue { font-variant-numeric: tabular-nums; }
        QLabel#materialHeaderLabel { color: @MUTED@; font-size: 8pt; font-weight: 500; }
        QLabel#materialName { color: @TEXT@; font-size: 9pt; }
        QLabel#materialCost { color: @TEXT@; font-variant-numeric: tabular-nums; }
        QLabel#materialWeightTotal {
            background-color: @ACCENT_SOFT@; border-radius: 6px; color: @MUTED@; padding: 8px;
        }
        QWidget#materialRow { border-bottom: 1px solid @BORDER@; }
        QLabel#materialSwatch { border: 1px solid @BORDER@; border-radius: 4px; }
        QLineEdit, QComboBox, QSpinBox {
            min-height: 22px; padding: 4px 8px; border: 1px solid @BORDER@;
            border-radius: 6px; background-color: @INPUT@; color: @TEXT@;
            selection-background-color: @ACCENT@;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: @ACCENT@; }
        QComboBox::drop-down { width: 22px; border: none; }
        QPushButton {
            min-height: 29px; padding: 4px 10px; border: 1px solid @BORDER@;
            border-radius: 6px; background-color: @WINDOW@; color: @TEXT@;
        }
        QPushButton:hover { background-color: @ACCENT_SOFT@; }
        QPushButton#openGcodeButton {
            border-color: @ACCENT@; background-color: @ACCENT@; color: #ffffff; font-weight: 600;
        }
        QPushButton#openGcodeButton:hover { background-color: @ACCENT@; }
        QPushButton#removeMaterialButton { min-width: 26px; max-width: 26px; padding: 0; border: none; background: transparent; color: @MUTED@; }
        QPushButton#removeMaterialButton:hover { color: @ACCENT@; background: @ACCENT_SOFT@; }
        QLabel#statusLabel {
            margin-top: 2px; padding: 8px 10px; border-radius: 6px;
            background-color: @STATUS_BACKGROUND@; color: @STATUS_TEXT@; font-size: 8pt;
        }
    )");
    styleSheet.replace("@BACKGROUND@", backgroundColor)
        .replace("@WINDOW@", windowColor)
        .replace("@PANEL@", panelColor)
        .replace("@INPUT@", inputColor)
        .replace("@TEXT@", textColor)
        .replace("@MUTED@", mutedColor)
        .replace("@BORDER@", borderColor)
        .replace("@ACCENT@", accentColor)
        .replace("@ACCENT_SOFT@", accentSoftColor)
        .replace("@STATUS_BACKGROUND@", statusBackgroundColor)
        .replace("@STATUS_TEXT@", statusTextColor);
    setStyleSheet(styleSheet);
    showEmptyResults();
}

void MainWindow::addMaterialRow()
{
    const int number = static_cast<int>(materialRows_.size()) + 1;
    auto* row = new QWidget();
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(6);
    auto* name = new QLabel(QStringLiteral("Filamento %1").arg(number), row);
    name->setObjectName("materialName");
    name->setMinimumWidth(90);
    name->setMaximumWidth(150);
    name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* colorSwatch = new QLabel(row);
    colorSwatch->setObjectName("materialSwatch");
    colorSwatch->setFixedSize(22, 22);
    colorSwatch->setFrameShape(QFrame::StyledPanel);
    colorSwatch->setToolTip(QStringLiteral("Cor do filamento"));
    colorSwatch->hide();
    auto* grams = makeNumericField(row, "g");
    grams->setMaximumWidth(86);
    grams->setAlignment(Qt::AlignRight);
    auto* pricePerKg = makeNumericField(row, "R$/kg");
    pricePerKg->setMaximumWidth(104);
    pricePerKg->setAlignment(Qt::AlignRight);
    auto* cost = new QLabel("—", row);
    cost->setObjectName("materialCost");
    cost->setMinimumWidth(82);
    cost->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto* removeButton = new QPushButton("×", row);
    removeButton->setObjectName("removeMaterialButton");
    removeButton->setFixedWidth(28);
    removeButton->setToolTip("Remover material");
    grams->setToolTip("Gramas utilizadas deste material");
    pricePerKg->setToolTip("Preço de um quilograma deste material");
    rowLayout->addWidget(colorSwatch);
    rowLayout->addWidget(name, 1);
    rowLayout->addWidget(grams);
    rowLayout->addWidget(pricePerKg);
    rowLayout->addWidget(cost);
    if (number > 1) {
        rowLayout->addWidget(removeButton);
    } else {
        removeButton->hide();
    }
    row->setObjectName("materialRow");
    materialsLayout_->addWidget(row);

    materialRows_.push_back({row, name, colorSwatch, grams, pricePerKg, cost, QString()});

    connect(grams, &QLineEdit::textChanged, this, [this] {
        updateMaterialWeightTotal();
        recalculate();
    });
    connect(pricePerKg, &QLineEdit::textChanged, this, [this] { recalculate(); });
    connect(removeButton, &QPushButton::clicked, this, [this, row] { removeMaterialRow(row); });
    updateMaterialWeightTotal();
}

void MainWindow::removeMaterialRow(QWidget* container)
{
    if (materialRows_.size() <= 1) {
        return;
    }
    for (std::size_t index = 0; index < materialRows_.size(); ++index) {
        if (materialRows_[index].container != container) {
            continue;
        }
        materialRows_.erase(materialRows_.begin() + static_cast<std::ptrdiff_t>(index));
        materialsLayout_->removeWidget(container);
        delete container;
        break;
    }
    for (std::size_t index = 0; index < materialRows_.size(); ++index) {
        const QString label = materialRows_[index].importedName.isEmpty()
            ? QStringLiteral("Filamento %1").arg(index + 1)
            : materialRows_[index].importedName;
        materialRows_[index].name->setText(label);
    }
    updateMaterialWeightTotal();
    recalculate();
}

void MainWindow::updateMaterialWeightTotal()
{
    double totalGrams = 0.0;
    for (std::size_t index = 0; index < materialRows_.size(); ++index) {
        try {
            totalGrams += parseNumber(
                materialRows_[index].grams,
                QStringLiteral("gramas do material %1").arg(index + 1),
                true);
        } catch (const std::exception&) {
            if (importedTotalFilamentGrams_) {
                const QLocale brazilianLocale(QLocale::Portuguese, QLocale::Brazil);
                materialWeightTotalLabel_->setText(QStringLiteral(
                    "Peso total informado no G-code: %1 g (sem divisão por material)")
                    .arg(brazilianLocale.toString(*importedTotalFilamentGrams_, 'f', 2)));
            } else {
                materialWeightTotalLabel_->setText(
                    "Peso total dos filamentos: preencha as gramas de todos os materiais");
            }
            return;
        }
    }

    const QLocale brazilianLocale(QLocale::Portuguese, QLocale::Brazil);
    QString totalText = QStringLiteral("Peso total dos filamentos: %1 g")
        .arg(brazilianLocale.toString(totalGrams, 'f', 2));
    if (importedTotalFilamentGrams_ && std::abs(totalGrams - *importedTotalFilamentGrams_) > 0.02) {
        totalText += QStringLiteral(" (total do G-code: %1 g)")
            .arg(brazilianLocale.toString(*importedTotalFilamentGrams_, 'f', 2));
    }
    materialWeightTotalLabel_->setText(totalText);
}

void MainWindow::showEmptyResults()
{
    summaryCostLabel_->setText("—");
    summarySalePriceLabel_->setText("—");
    for (const MaterialRow& row : materialRows_) {
        row.cost->setText("—");
    }
    for (QLabel* label : {materialTotalLabel_, electricityLabel_, printerWearLabel_, failureAllowanceLabel_,
                          laborLabel_, fixedLabel_, packagingLabel_, shippingInCostLabel_, costTotalLabel_,
                          profitLabel_, salesFeesLabel_, preShippingPriceLabel_, passThroughShippingLabel_,
                          salePriceLabel_, marginLabel_}) {
        label->setText("—");
    }
}

void MainWindow::recalculate()
{
    try {
        calculation::CalculationInput input;
        input.printTimeSeconds = importedPrintTimeSeconds_.value_or(
            printHours_->value() * 3600.0 + printMinutes_->value() * 60.0);
        input.powerWatts = parseNumber(powerWatts_, "consumo médio", true);
        input.electricityRatePerKwh = parseNumber(electricityRate_, "tarifa de energia", true);
        input.electricityFlagPerKwh = parseNumber(electricityFlag_, "acréscimo da bandeira");

        for (std::size_t index = 0; index < materialRows_.size(); ++index) {
            const auto& row = materialRows_[index];
            const QString position = QStringLiteral("do material %1").arg(index + 1);
            input.materials.push_back({
                (materialRows_[index].importedName.isEmpty()
                    ? QStringLiteral("Material %1").arg(index + 1)
                    : materialRows_[index].importedName).toUtf8().constData(),
                parseNumber(row.grams, QStringLiteral("gramas %1").arg(position), true),
                parseNumber(row.pricePerKg, QStringLiteral("preço por kg %1").arg(position), true)
            });
        }

        input.printerPurchasePrice = parseNumber(printerPurchasePrice_, "preço da impressora");
        input.printerLifeHours = parseNumber(printerLifeHours_, "vida útil da impressora");
        input.failureRatePercent = parseNumber(failureRate_, "reserva para falhas", false, true);
        input.laborCost = parseNumber(laborCost_, "mão de obra");
        input.fixedCost = parseNumber(fixedCost_, "custos fixos");
        input.packagingCost = parseNumber(packagingCost_, "embalagem");
        input.shippingCost = parseNumber(shippingCost_, "frete");
        input.shippingMode = shippingMode_->currentData().toString().toUtf8().constData();
        input.profitMode = profitMode_->currentData().toString().toUtf8().constData();
        input.profitValue = parseNumber(
            profitValue_, "lucro", false, profitMode_->currentData().toString() == "percent");
        input.salesFeePercent = parseNumber(salesFeePercent_, "taxas sobre a venda", false, true);

        const calculation::CostBreakdown result = calculation::calculateCost(input);
        for (std::size_t index = 0; index < result.materials.size(); ++index) {
            const auto& material = result.materials[index];
            materialRows_[index].cost->setText(currency(material.cost));
        }
        setMoney(materialTotalLabel_, result.materialCost);
        setMoney(electricityLabel_, result.electricityCost);
        setMoney(printerWearLabel_, result.printerWear);
        setMoney(failureAllowanceLabel_, result.failureAllowance);
        setMoney(laborLabel_, result.labor);
        setMoney(fixedLabel_, result.fixed);
        setMoney(packagingLabel_, result.packaging);
        setMoney(shippingInCostLabel_, result.shippingIncludedInCost);
        setMoney(costTotalLabel_, result.costTotal);
        setMoney(summaryCostLabel_, result.costTotal);
        setMoney(profitLabel_, result.profit);
        setMoney(salesFeesLabel_, result.salesFees);
        setMoney(preShippingPriceLabel_, result.priceBeforePassThroughShipping);
        setMoney(passThroughShippingLabel_, result.shippingPassedThrough);
        setMoney(salePriceLabel_, result.salePrice);
        setMoney(summarySalePriceLabel_, result.salePrice);
        marginLabel_->setText(QStringLiteral("%1%").arg(result.realMarginPercent, 0, 'f', 2));
        statusLabel_->setText(QStringLiteral("Cálculo atualizado. Tarifa aplicada: %1/kWh.%2")
            .arg(currency(result.effectiveElectricityRatePerKwh), importStatus_.isEmpty()
                ? QString() : QStringLiteral("\n") + importStatus_));
    } catch (const std::exception& error) {
        showEmptyResults();
        statusLabel_->setText(QString::fromUtf8(error.what()));
    }
}

void MainWindow::clearCalculation()
{
    importedPrintTimeSeconds_.reset();
    importedTotalFilamentGrams_.reset();
    importStatus_.clear();
    printHours_->setValue(0);
    printMinutes_->setValue(0);
    for (QLineEdit* field : {powerWatts_, electricityRate_, electricityFlag_, printerPurchasePrice_,
                             printerLifeHours_, failureRate_, laborCost_, fixedCost_, packagingCost_,
                             shippingCost_, profitValue_, salesFeePercent_}) {
        field->clear();
    }
    shippingMode_->setCurrentIndex(0);
    profitMode_->setCurrentIndex(0);

    while (materialRows_.size() > 1) {
        auto* row = materialRows_.back().container;
        materialRows_.pop_back();
        materialsLayout_->removeWidget(row);
        delete row;
    }
    for (MaterialRow& row : materialRows_) {
        row.grams->clear();
        row.pricePerKg->clear();
        row.importedName.clear();
        row.name->setText(QStringLiteral("Filamento 1"));
        row.colorSwatch->clear();
        row.colorSwatch->setStyleSheet(QString());
        row.colorSwatch->setToolTip(QStringLiteral("Cor do filamento"));
        row.colorSwatch->hide();
    }
    selectedFileLabel_->setText("Nenhum arquivo G-code selecionado.");
    stateTariffCombo_->setCurrentIndex(0);
    updateMaterialWeightTotal();
    recalculate();
}

void MainWindow::chooseGcodeFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Selecionar G-code",
        QString(),
        "Arquivos G-code (*.gcode *.gco *.gc *.g);;Todos os arquivos (*)");

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusLabel_->setText(QStringLiteral("Não foi possível abrir o arquivo: %1").arg(file.errorString()));
        return;
    }

    const gcode::Metadata metadata = gcode::parseMetadata(file);
    importedTotalFilamentGrams_ = metadata.totalFilamentGrams;
    selectedFileLabel_->setText(filePath);
    selectedFileLabel_->setToolTip(filePath);

    if (metadata.printTimeSeconds) {
        const int roundedMinutes = static_cast<int>(std::llround(*metadata.printTimeSeconds / 60.0));
        printHours_->setValue(roundedMinutes / 60);
        printMinutes_->setValue(roundedMinutes % 60);
        importedPrintTimeSeconds_ = metadata.printTimeSeconds;
    } else {
        importedPrintTimeSeconds_.reset();
    }

    QVector<gcode::MaterialMetadata> importedMaterials = metadata.materials;
    bool hasUsableMaterialMass = false;
    for (const auto& material : importedMaterials) {
        hasUsableMaterialMass = hasUsableMaterialMass || material.grams.has_value();
    }
    if (!hasUsableMaterialMass && metadata.totalFilamentGrams) {
        if (importedMaterials.size() == 1) {
            importedMaterials[0].grams = metadata.totalFilamentGrams;
        } else if (importedMaterials.isEmpty()) {
            importedMaterials.push_back({metadata.totalFilamentGrams, {}, {}, {}});
        }
    }

    if (!importedMaterials.isEmpty()) {
        while (materialRows_.size() < static_cast<std::size_t>(importedMaterials.size())) {
            addMaterialRow();
        }
        while (materialRows_.size() > static_cast<std::size_t>(importedMaterials.size())) {
            removeMaterialRow(materialRows_.back().container);
        }
        for (qsizetype index = 0; index < importedMaterials.size(); ++index) {
            const auto& material = importedMaterials[index];
            auto& row = materialRows_[static_cast<std::size_t>(index)];
            if (material.grams) {
                row.grams->setText(QLocale::c().toString(*material.grams, 'f', 2));
            } else {
                row.grams->clear();
            }

            QStringList description;
            if (!material.type.isEmpty()) description.push_back(material.type);
            if (!material.profile.isEmpty()) description.push_back(material.profile);
            row.importedName = description.join(QStringLiteral(" · "));
            const QString shortName = material.type.isEmpty()
                ? QStringLiteral("Filamento %1").arg(index + 1)
                : QStringLiteral("%1 %2").arg(material.type).arg(index + 1);
            row.name->setText(shortName);
            row.name->setToolTip(row.importedName);
            const QColor filamentColor(material.color.trimmed());
            if (filamentColor.isValid()) {
                row.colorSwatch->setStyleSheet(QStringLiteral(
                    "background-color: %1; border: 1px solid palette(mid);").arg(filamentColor.name()));
                row.colorSwatch->setToolTip(QStringLiteral("Cor do filamento: %1")
                    .arg(material.color));
                row.colorSwatch->show();
            } else {
                row.colorSwatch->hide();
                row.colorSwatch->setStyleSheet(QString());
                row.colorSwatch->setToolTip(QStringLiteral("Cor do filamento não reconhecida: %1")
                    .arg(material.color));
                if (!material.color.isEmpty()) {
                    row.name->setToolTip(QStringLiteral("Cor informada no arquivo: %1")
                        .arg(material.color));
                } else {
                    row.name->setToolTip(QString());
                }
            }
        }
    }

    QStringList summary;
    if (!metadata.slicerName.isEmpty()) {
        summary.push_back(metadata.slicerVersion.isEmpty()
            ? metadata.slicerName
            : QStringLiteral("%1 %2").arg(metadata.slicerName, metadata.slicerVersion));
    }
    if (!metadata.printerModel.isEmpty()) {
        summary.push_back(metadata.printerModel);
    }
    if (metadata.printTimeSeconds) {
        const int minutes = static_cast<int>(std::llround(*metadata.printTimeSeconds / 60.0));
        summary.push_back(QStringLiteral("tempo %1 h %2 min").arg(minutes / 60).arg(minutes % 60));
    }
    qsizetype materialsWithMass = 0;
    for (const auto& material : importedMaterials) {
        materialsWithMass += material.grams.has_value() ? 1 : 0;
    }
    if (materialsWithMass > 0) {
        summary.push_back(QStringLiteral("%1 material(is) com gramas").arg(materialsWithMass));
    }
    if (materialsWithMass == 0 && !importedMaterials.isEmpty()) {
        summary.push_back(QStringLiteral("%1 material(is) identificado(s)").arg(importedMaterials.size()));
    }
    if (metadata.totalFilamentGrams && materialsWithMass == 0) {
        summary.push_back(QStringLiteral("%1 g no total").arg(*metadata.totalFilamentGrams, 0, 'f', 2));
    }
    importStatus_ = summary.join(QStringLiteral("; "));
    if (!metadata.warnings.isEmpty()) {
        const QStringList shownWarnings = metadata.warnings.mid(0, 3);
        importStatus_ += (importStatus_.isEmpty() ? QString() : QStringLiteral("\n"))
            + QStringLiteral("Avisos: ") + shownWarnings.join(QStringLiteral(" "));
        if (metadata.warnings.size() > shownWarnings.size()) {
            importStatus_ += QStringLiteral(" (%1 avisos no total)").arg(metadata.warnings.size());
        }
    }
    if (importStatus_.isEmpty()) {
        importStatus_ = QStringLiteral("Nenhum dado de tempo ou filamento reconhecido; os valores manuais foram mantidos.");
    }
    recalculate();
}
