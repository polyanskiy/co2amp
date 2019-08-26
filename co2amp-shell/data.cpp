#include "co2amp.h"


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
    if(configFile_type.count()==0)
        return -1;

    int count_am = 0;
    int count_other = 0;
    int i;
    for(i=0; i < configFile_type.count(); i++){
        if(configFile_type[i] == "PULSE"|| configFile_type[i] == "COMPONENT" || configFile_type[i] == "COMMENT")
            count_other++;
        if(configFile_type[i] == "A")
            count_am++;
        if(i-count_other == optic_n)
            break;
    }

    if(configFile_type[i]=="A")
        return count_am-1;
    return -1;
}


void MainWindow::SelectEnergies()
{
    QString line;
    QRegExp separators("[\t\n]");
    int pulse_n = comboBox_pulse->currentIndex();
    int optic_n = comboBox_optic->currentIndex();

    QFile file_all("data_energy.dat");
    file_all.open(QFile::ReadOnly);
    QFile file_sel("data_energy_selected.dat");
    QTextStream out(&file_sel);
    file_sel.open(QFile::WriteOnly);

    line = file_all.readLine();
    while(line != QString()){
    if(line[0]!='#'){ // skip comments
        switch(comboBox_energyPlot->currentIndex()){
        case 0: // all energies
        out << line.section(separators, 0, 1) << "\n";
        break;
        case 1: // optic
        if(line.section(separators, 3, 3).toInt() == optic_n)
            out << line.section(separators, 0, 1) << "\n";
        break;
        case 2: // pulse
        if(line.section(separators, 2, 2).toInt() == pulse_n)
            out << line.section(separators, 0, 1) << "\n";
        break;
        case 3: // optic + pulse
        if(line.section(separators, 2, 2).toInt() == pulse_n && line.section(separators, 3, 3).toInt() == optic_n)
            out << line.section(separators, 0, 1) << "\n";
        break;
        }
    }
    line = file_all.readLine();
    }

    file_sel.close();
    file_all.close();
}
