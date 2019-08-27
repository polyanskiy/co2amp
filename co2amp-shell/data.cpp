#include "co2amp.h"


int MainWindow::PassNumber(int i)
{
    QStringList list = lineEdit_passes->text().split(",", QString::SkipEmptyParts);
    if(i < list.count()){
        //if(list[i].toInt() >= 0)
            return list[i].toInt();
            //return list[i].toInt() - 1;
    }
    return -1;
}


QString MainWindow::Type(QString id)
{
    if(configFile_type.count()==0)
        return QString();

    for(int i=0; i < configFile_id.count(); i++){
        if(configFile_id[i] == id)
            return configFile_type[i];
    }

    return QString();
}


/*int MainWindow::AmNumber(int optic_n)
{
    if(configFile_type.count()==0)
        return -1;

    int count_am = 0;
    int count_other = 0;
    int i;
    for(i=0; i < configFile_type.count(); i++){
        if(configFile_type[i] == "PULSE"|| configFile_type[i] == "COMPONENT")
            count_other++;
        if(configFile_type[i] == "A")
            count_am++;
        if(i-count_other == optic_n)
            break;
    }

    if(configFile_type[i]=="A")
        return count_am-1;
    return -1;
}*/


void MainWindow::SelectEnergies()
{
    QString line;
    QRegExp separators("[\t\n]");
    int pulse_n = comboBox_pulse->currentIndex();
    int optic_n = comboBox_optic->currentIndex();

    QFile file_all("energy.dat");
    file_all.open(QFile::ReadOnly);
    QFile file_sel("energy_selected.dat");
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
