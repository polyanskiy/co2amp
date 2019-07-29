#include "mainwindow.h"


void MainWindow::SaveSettings(QString what_to_save)
{
    QSettings settings("project.ini", QSettings::IniFormat);

    if(what_to_save == "all"){
        settings.setValue("general/version", version );
        //settings.setValue("general/n_pulses", Memorized.n_pulses );
        //settings.setValue("general/from_file", Memorized.from_file );
        //settings.setValue("general/input_file", Memorized.input_file );

        //settings.setValue("co2amp/E0", Memorized.E0);
        //settings.setValue("co2amp/w0", Memorized.w0);
        //settings.setValue("co2amp/tau0", Memorized.tau0);
        //settings.setValue("co2amp/t_inj", Memorized.t_inj);
        //settings.setValue("co2amp/Dt_train", Memorized.Dt_train);

        settings.setValue("co2amp/noprop", Memorized.noprop);  //move to debug!

        settings.setValue("co2amp/vc", Memorized.vc);
        settings.setValue("co2amp/precision_t", Memorized.precision_t);
        settings.setValue("co2amp/precision_r", Memorized.precision_r);
        settings.setValue("co2amp/t_pulse_min", Memorized.t_pulse_min);
        settings.setValue("co2amp/t_pulse_max", Memorized.t_pulse_max);

        settings.setValue("debug/debugLevel", spinBox_debugLevel->text());

        // save optic list, optic specs (yaml), and optical configuration in separate files
        int i;
        int optic_count = Memorized.configFile_basename.size();
        QFile file;
        QTextStream out(&file);

        file.setFileName("optics.txt");
        file.open(QFile::WriteOnly | QFile::Truncate);
        for(i=0; i<optic_count; i++){
            out << Memorized.configFile_basename[i] << "\t"
                << Memorized.configFile_type[i] << "\t"
                << "optic" << QString().setNum(i) << ".yml\n";
        }
        file.close();

        file.setFileName("layout.txt");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << Memorized.layout;// << "\n";
        file.close();

        for(i=0; i<optic_count; i++){
            file.setFileName("optic" + QString().setNum(i)+".yml");
            file.open(QFile::WriteOnly | QFile::Truncate);
            out << Memorized.configFile_content[i];
            file.close();
        }

        Saved = Memorized;
    }

    if(what_to_save == "all" || what_to_save == "plot"){
        settings.setValue("plot/optic", Memorized.optic);
        settings.setValue("plot/pulse", Memorized.pulse);
        settings.setValue("plot/energyPlot", comboBox_energyPlot->currentIndex());
        settings.setValue("plot/passes", lineEdit_passes->text());
        settings.setValue("plot/timeScale", comboBox_timeScale->currentIndex());
        settings.setValue("plot/freqScale", comboBox_freqScale->currentIndex());
        settings.setValue("plot/log", checkBox_log->isChecked());
    }

}

void MainWindow::LoadSettings(QString path)
{
    QSettings settings(path, QSettings::IniFormat);

    flag_projectloaded = false;

    /*// Input pulse
    Memorized.from_file = settings.value("general/from_file", 0).toBool();
    Memorized.input_file = settings.value("general/input_file", QString()).toString();
    Memorized.E0 = settings.value("co2amp/E0", 0.01).toString();
    Memorized.w0 = settings.value("co2amp/w0", 2.5).toString();
    Memorized.tau0 = settings.value("co2amp/tau0", 5).toString();
    Memorized.t_inj = settings.value("co2amp/t_inj", 0.65).toString();
    Memorized.n_pulses = settings.value("general/n_pulses", 1).toInt();
    Memorized.Dt_train = settings.value("co2amp/Dt_train", 0.1).toString();*/

    // Opticss and optical layout
    Memorized.configFile_basename.clear();
    Memorized.configFile_type.clear();
    Memorized.configFile_content.clear();
    //Memorized.layout = settings.value("co2amp/layout", "P1 (1000) A1 (1000) P1").toString();

    int i;//, j;
    QString str;
    QStringList list, line;
    QFile file, file2;
    QTextStream in(&file), in2(&file2);

    file.setFileName("layout.txt");
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        Memorized.layout = in.readAll();
        file.close();
    }

    file.setFileName("optics.txt");
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        str = in.readAll();
        file.close();
        list = str.split("\n");
        for(i=0; i<list.size(); i++){
            line = list[i].split("\t");
            if(line.size() == 3){
                Memorized.configFile_basename.append(line[0]);
                Memorized.configFile_type.append(line[1]);
                file2.setFileName(line[2]);
                file2.open(QIODevice::ReadOnly | QFile::Text);
                Memorized.configFile_content.append(in2.readAll());
                file2.close();
            }
        }
    }

    PopulateConfigFileList();

    // Calculation net
    Memorized.vc = settings.value("co2amp/vc", 30).toString();
    Memorized.precision_t = settings.value("co2amp/precision_t", 6).toInt();
    Memorized.precision_r = settings.value("co2amp/precision_r", 1).toInt();
    Memorized.t_pulse_min = settings.value("co2amp/t_pulse_min", -100).toString();
    Memorized.t_pulse_max = settings.value("co2amp/t_pulse_max", 400).toString();

    // Plot
    Memorized.optic = settings.value("plot/optic", 0).toInt();
    Memorized.pulse = settings.value("plot/pulse", 0).toInt();
    lineEdit_passes->setText(settings.value("plot/passes", "1,2").toString());
    /*if(Saved.n_pulses == 1) // single pulse
        comboBox_energyPlot->setCurrentIndex(settings.value("plot/energyPlot", 1).toInt());
    else
        comboBox_energyPlot->setCurrentIndex(settings.value("plot/energyPlot", 3).toInt());*/
    comboBox_timeScale->setCurrentIndex(settings.value("plot/timeScale", 2).toInt());
    comboBox_freqScale->setCurrentIndex(settings.value("plot/freqScale", 2).toInt());
    checkBox_log->setChecked(settings.value("plot/log", 0).toBool());

    // Debugging
    spinBox_debugLevel->setValue(settings.value("debug/debugLevel", 0).toInt());
    Memorized.noprop = settings.value("co2amp/noprop", 0).toBool();

    // //////////////////////// backwards compatibility start ////////////////////////////////////////////
    /*double w0 = settings.value("co2amp/w0", 0).toDouble();
    if(w0 >= 800)
        Memorized.vc = QString::number(w0*2.99792458e-2); // 1/cm -> THz
    if(settings.value("co2pump/D_interel", "not found").toString() != "not found") // missprint in old versions: "co2pump" instead of "co2amp"
        Memorized.D_interel  = settings.value("co2pump/D_interel", 8.5).toString();
    if(settings.value("co2amp/p_626","not found").toString() != "not found"){
        Memorized.p_CO2.setNum(settings.value("co2amp/p_626",0).toDouble()+settings.value("co2amp/p_628",0).toDouble()+settings.value("co2amp/p_828",0).toDouble()+
                 settings.value("co2amp/p_636",0).toDouble()+settings.value("co2amp/p_638",0).toDouble()+settings.value("co2amp/p_838",0).toDouble());
        Memorized.percent_13C.setNum((settings.value("co2amp/p_636",0).toDouble()+settings.value("co2amp/p_638",0).toDouble()
                        +settings.value("co2amp/p_838",0).toDouble())/Memorized.p_CO2.toDouble()*100);
        Memorized.percent_18O.setNum((settings.value("co2amp/p_628",0).toDouble()+2*settings.value("co2amp/p_828",0).toDouble()
                        +settings.value("co2amp/p_638",0).toDouble()+2*settings.value("co2amp/p_838",0).toDouble())
                       /(2*Memorized.p_CO2.toDouble())*100);
        if(settings.value("co2amp/C1i","not found").toString() != "not found"){
            double C1i = settings.value("co2amp/C1i", 81000).toDouble();
            double C2i = settings.value("co2amp/C2i", 0.12).toDouble();
            double C3i = settings.value("co2amp/C3i", 1).toDouble();
            double C4i = settings.value("co2amp/C4i", 0).toDouble();
            double C5i = settings.value("co2amp/C5i", 0.12).toDouble();
            double C6i = settings.value("co2amp/C6i", 1).toDouble();
            double C1u = settings.value("co2amp/C1u", 600000).toDouble();
            double C2u = settings.value("co2amp/C2u", 1).toDouble();
            double C3u = settings.value("co2amp/C3u", 0).toDouble();
            double C4u = settings.value("co2amp/C4u", 1).toDouble();
            Memorized.discharge = "";
            double t;
            for(t=0; t<10.001; t+=0.01){
                Memorized.discharge += QString::number(t) + "\t";
                Memorized.discharge += QString::number(C1i*pow(t/C2i,C3i)*exp(-t/C2i)+C4i*pow(t/C5i,C6i)*exp(-t/C5i)) + "\t";
                Memorized.discharge += QString::number(C1u*exp(-t/C2u)+C3u*exp(-t/C4u)) + "\n";
            }
        }
    }*/
    // //////////////////////// backwards compatibility end /////////////////////////////////////////////

    Saved = Memorized;
    flag_projectloaded = true;
    UpdateControls();
}


void MainWindow::MemorizeSettings()
{
    /////////////////////////////////// CALCULATION NET //////////////////////////////////////
    Memorized.vc = lineEdit_vc->text();
    Memorized.t_pulse_min = lineEdit_t_pulse_min->text();
    Memorized.t_pulse_max = lineEdit_t_pulse_max->text();
    Memorized.precision_t = comboBox_precision_t->currentIndex();
    Memorized.precision_r = comboBox_precision_r->currentIndex();
    /////////////////////////////////// DEBUGGING //////////////////////////////////////
    Memorized.noprop = checkBox_noprop->isChecked();
    /////////////////////////////////// PLOT //////////////////////////////////////
    Memorized.optic = comboBox_optic->currentIndex();
    Memorized.pulse = comboBox_pulse->currentIndex();
}
