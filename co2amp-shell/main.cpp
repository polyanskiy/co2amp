#include <QApplication>
#include "co2amp.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow win;
    if(win.path_to_7zip=="" || win.path_to_gnuplot=="" || win.path_to_gnuplot=="")
        return 0;
    win.show();
    return app.exec();
}
