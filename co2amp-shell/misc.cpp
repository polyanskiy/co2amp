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


int MainWindow::AmNumber(int optic_number)
{
    QStringList list;
    list = Saved.optic_type;

    if(list.count()==0 || list[optic_number] != "A")
        return -1;

    int i;
    int count = 0;
    for(i=0; i <= list.count()-1; i++){
        if(list[i] == "AM")
            count++;
        if(i == optic_number)
            break;
    }

    return count-1;
}
