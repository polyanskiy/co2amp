#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow win;
    win.version = 20140217;
    win.show();
    return app.exec();
}
