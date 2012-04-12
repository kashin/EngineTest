#include "testwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestWindow mainWindow;
    Q_UNUSED(mainWindow);

    return app.exec();
}

