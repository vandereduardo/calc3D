#include "main_window.h"

#include <QFileDialog>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("calc3D — custo de impressão 3D");
    resize(760, 700);

    auto* content = new QWidget(this);
    auto* layout = new QVBoxLayout(content);

    auto* title = new QLabel("Calculadora de impressão 3D", content);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 5);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    layout->addWidget(new QLabel(
        "Abra um G-code ou preencha os dados manualmente. Os valores serão calculados nesta tela.",
        content));

    auto* openButton = new QPushButton("Selecionar arquivo G-code…", content);
    connect(openButton, &QPushButton::clicked, this, [this] { chooseGcodeFile(); });
    layout->addWidget(openButton);

    selectedFileLabel_ = new QLabel("Nenhum arquivo selecionado", content);
    selectedFileLabel_->setWordWrap(true);
    layout->addWidget(selectedFileLabel_);

    auto* inputGroup = new QGroupBox("Dados para o cálculo", content);
    auto* form = new QFormLayout(inputGroup);
    form->addRow("Tempo de impressão (horas)", new QLineEdit(inputGroup));
    form->addRow("Filamento (gramas)", new QLineEdit(inputGroup));
    form->addRow("Preço do filamento (R$/kg)", new QLineEdit(inputGroup));
    form->addRow("Consumo médio (W)", new QLineEdit(inputGroup));
    form->addRow("Tarifa de energia (R$/kWh)", new QLineEdit(inputGroup));
    layout->addWidget(inputGroup);

    auto* resultGroup = new QGroupBox("Resultado", content);
    auto* resultLayout = new QVBoxLayout(resultGroup);
    resultLayout->addWidget(new QLabel(
        "A leitura do G-code e o cálculo serão conectados nesta etapa do protótipo.",
        resultGroup));
    layout->addWidget(resultGroup);

    statusLabel_ = new QLabel("Protótipo da interface — ainda não calcula valores.", content);
    statusLabel_->setWordWrap(true);
    layout->addWidget(statusLabel_);

    layout->addStretch();
    setCentralWidget(content);
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

    selectedFileLabel_->setText(filePath);
    statusLabel_->setText("Arquivo selecionado localmente. A extração dos dados ainda será implementada.");
}
