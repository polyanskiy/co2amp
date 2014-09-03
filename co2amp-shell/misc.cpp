#include "mainwindow.h"


int MainWindow::DatasetNumber(int pulse_n, int comp_n, int pass_n, QString filename)
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
            if(line.section(separators, 1, 1).toInt() == pulse_n && line.section(separators, 3, 3).toInt() == comp_n && line.section(separators, 5, 5).toInt() == pass_n){
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


int MainWindow::AmNumber(int component_number)
{
    QStringList list, list1;

    list = Saved.components.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    if(list.count()==0)
        return -1;
    list1 = list[component_number].split(QRegExp("[ \t]"), QString::SkipEmptyParts); // is active medium?
    if(list1.count()==0 || list1[1] != "AM")
        return -1;

    int i;
    int count = 0;
    for(i=0; i <= list.count()-1; i++){
    list1 = list[i].split(QRegExp("[ \t]"), QString::SkipEmptyParts);
        if(list1.count()>=1 && list1[1] == "AM")
            count++;
        if(i == component_number)
            break;
    }

    return count-1;
}
