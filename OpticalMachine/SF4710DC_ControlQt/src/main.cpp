#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("SF4710DC 光机控制"));
    QCoreApplication::setOrganizationName(QStringLiteral("OpticalMachine"));

    MainWindow window;
    window.show();

    return app.exec();
}
