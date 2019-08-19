#include "mainwindow.h"


int MainWindow::DatasetNumber(int pulse_n, int optic_n, int pass_n, QString filename)
{
    if(pass_n < 0)
        return -1;

    int set_n = -1;
    bool found = false;
    QString line;
    QRegExp separators("[ \t\n\r]");

    QFile file(filename);
    file.open(QFile::ReadOnly);

    line = file.readLine();
    while(!file.atEnd()){
        line = file.readLine();
        if(line == QString())
            continue;
        if(line[0] == '#'){
            set_n ++;
            if(line.section(separators, 1, 1).toInt() == pulse_n && line.section(separators, 3, 3).toInt() == optic_n && line.section(separators, 5, 5).toInt() == pass_n){
                found = true;
                break;
            }
        }
    }

    file.close();

    if(found)
        return set_n;
    else
        return -1;
}


int MainWindow::PassNumber(int i)
{
    QStringList list = lineEdit_passes->text().split(",", QString::SkipEmptyParts);
    if(list.count() >= i+1){
        if(list[i].toInt() >= 1)
            return list[i].toInt() - 1;
    }
    return -1;
}


int MainWindow::AmNumber(int optic_n)
{
    QStringList list;
    list = Saved.configFile_type;

    if(list.count()==0)// || list[optic_n] != "A")
        return -1;

    int count_am = 0;
    int count_other = 0;
    int i;
    for(i=0; i < list.count(); i++){
        if(list[i] == "PULSE" || list[i] == "COMPONENT" || list[i] == "COMMENT")
            count_other++;
        if(list[i] == "A")
            count_am++;
        if(i-count_other == optic_n)
            break;
    }

    //QMessageBox().warning(this, "co2amp", list[i]);

    if(list[i]=="A")
        return count_am-1;
    return -1;
}
