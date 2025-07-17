#include <QApplication>
#include "MainWindow_ui.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
