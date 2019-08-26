#include "co2amp.h"


void MainWindow::FindExternalPrograms()
{
    path_to_core    = QStandardPaths::findExecutable("co2amp-core");
    path_to_7zip    = QStandardPaths::findExecutable("7z");
    path_to_gnuplot = QStandardPaths::findExecutable("gnuplot");

    #ifdef Q_OS_WIN
        // co2amp-core
        if(path_to_core==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\co2amp.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QString(QCoreApplication::applicationDirPath()),
                QString(getenv("PROGRAMFILES"))+"/co2amp"
            };
            path_to_core = QStandardPaths::findExecutable("co2amp-core", searchpaths);
        }
        // 7-Zip
        if(path_to_7zip==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\7zFM.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\7-Zip", QSettings::NativeFormat).value("Path").toString(),
                QString(getenv("PROGRAMFILES"))+"/7-Zip",
                QString(getenv("PROGRAMFILES"))+" (x86)/7-Zip"
            };
            path_to_7zip = QStandardPaths::findExecutable("7z", searchpaths);
        }
        // Gnuplot
        if(path_to_gnuplot==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\gnuplot.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QString(getenv("PROGRAMFILES"))+"/gnuplot\\bin",
                QString(getenv("PROGRAMFILES"))+" (x86)/gnuplot/bin"
            };
            path_to_gnuplot = QStandardPaths::findExecutable("gnuplot", searchpaths);
        }
    #endif

    if(path_to_core=="")
        QMessageBox().critical(this, "co2amp", "\'co2amp-core\' executable not found. Try re-installing co2amp.");
    if(path_to_7zip=="")
        QMessageBox().critical(this, "co2amp", "7-Zip not found. Please (re)install -  it\'s free.");
    if(path_to_gnuplot=="")
        QMessageBox().critical(this, "co2amp", "Gnuplot not found. Please (re)install -  it\'s free.");
}
