#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("calc3D");
    QApplication::setOrganizationName("calc3D");

    MainWindow window;
    window.show();

    return app.exec();
}
