#pragma once

#include <QMainWindow>

class QLabel;

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void chooseGcodeFile();

    QLabel* selectedFileLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};
