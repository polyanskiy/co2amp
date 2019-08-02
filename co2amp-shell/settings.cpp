#include "mainwindow.h"


void MainWindow::SaveSettings(QString what_to_save)
{
    QSettings settings("project.ini", QSettings::IniFormat);

    if(what_to_save == "all"){
        settings.setValue("general/version", "20190730" );

        settings.setValue("co2amp/noprop", Memorized.noprop);  //move to debug!

        settings.setValue("co2amp/vc", Memorized.vc);
        settings.setValue("co2amp/precision_t", Memorized.precision_t);
        settings.setValue("co2amp/precision_r", Memorized.precision_r);
        settings.setValue("co2amp/t_pulse_min", Memorized.t_pulse_min);
        settings.setValue("co2amp/t_pulse_max", Memorized.t_pulse_max);
        settings.setValue("debug/debugLevel", spinBox_debugLevel->text());

        // Write configuration files
        int i;
        int config_file_count = Memorized.configFile_id.size();

        QFile file;
        QTextStream out(&file);

        file.setFileName("config_files.yml");
        file.open(QFile::WriteOnly | QFile::Truncate);

        for(i=0; i<config_file_count; i++){
            out << "- file: " << Memorized.configFile_id[i] << ".yml\n"
                << "  type: " << Memorized.configFile_type[i] << "\n";
        }
        file.close();

        for(i=0; i<config_file_count; i++){
            file.setFileName(Memorized.configFile_id[i]+".yml");
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

    // Configuration files
    Memorized.configFile_id.clear();
    Memorized.configFile_type.clear();
    Memorized.configFile_content.clear();

    int i;
    QString str;
    QStringList file_list, file_record;
    QFile file;//, file2;
    QTextStream in(&file);//, in2(&file2);


    file.setFileName("config_files.yml");
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        str = in.readAll();
        file.close();
        file_list = str.split("- ", QString::SkipEmptyParts);
        for(i=0; i<file_list.size(); i++){
            file_record = file_list[i].split("\n", QString::SkipEmptyParts);
            if(file_record.size() == 3){ // "file: ...", "type: ...", "id: ..."
                Memorized.configFile_id.append(file_record[2].split(": ")[1]);   // "  id: ..."
                Memorized.configFile_type.append(file_record[1].split(": ")[1]); // "  type: ..."
                file.setFileName(file_record[0].split(": ")[1]);                 // "  file: ..."
                file.open(QIODevice::ReadOnly | QFile::Text);
                Memorized.configFile_content.append(in.readAll());
                file.close();
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

    // Debugging
    spinBox_debugLevel->setValue(settings.value("debug/debugLevel", 0).toInt());
    Memorized.noprop = settings.value("co2amp/noprop", 0).toBool();

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
