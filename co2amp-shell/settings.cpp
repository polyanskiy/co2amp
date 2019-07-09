#include "mainwindow.h"


void MainWindow::SaveSettings(QString what_to_save)
{
    QSettings settings("project.ini", QSettings::IniFormat);

    if(what_to_save == "all"){
        settings.setValue("general/version", version );
        settings.setValue("general/n_pulses", Memorized.n_pulses );
        settings.setValue("general/from_file", Memorized.from_file );
        settings.setValue("general/input_file", Memorized.input_file );

        settings.setValue("co2amp/E0", Memorized.E0);
        settings.setValue("co2amp/w0", Memorized.w0);
        settings.setValue("co2amp/tau0", Memorized.tau0);
        settings.setValue("co2amp/vc", Memorized.vc);
        settings.setValue("co2amp/t_inj", Memorized.t_inj);
        settings.setValue("co2amp/Dt_train", Memorized.Dt_train);

        settings.setValue("co2amp/noprop", Memorized.noprop);  //move to debug!

        settings.setValue("co2amp/precision_t", Memorized.precision_t);
        settings.setValue("co2amp/precision_r", Memorized.precision_r);
        settings.setValue("co2amp/t_pulse_min", Memorized.t_pulse_min);
        settings.setValue("co2amp/t_pulse_max", Memorized.t_pulse_max);

        settings.setValue("debug/debugLevel", spinBox_debugLevel->text());
        settings.setValue("debug/regBand", checkBox_regBand->isChecked());
        settings.setValue("debug/hotBand", checkBox_hotBand->isChecked());
        settings.setValue("debug/seqBand", checkBox_seqBand->isChecked());

        // save optic list, optic specs (yaml), and optical configuration in separate files
        int i;
        int optic_count = Memorized.optic_id.size();
        QFile file;
        QTextStream out(&file);

        file.setFileName("optics.txt");
        file.open(QFile::WriteOnly | QFile::Truncate);
        for(i=0; i<optic_count; i++){
            out << Memorized.optic_id[i] << "\t"
                << Memorized.optic_type[i] << "\t"
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
            out << Memorized.optic_yaml[i];
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

    if(what_to_save == "all" || what_to_save == "comments"){
        settings.setValue("comments/comments", plainTextEdit_comments->toPlainText());
    }
}

void MainWindow::LoadSettings(QString path)
{
    QSettings settings(path, QSettings::IniFormat);

    flag_projectloaded = false;

    // Input pulse
    Memorized.from_file = settings.value("general/from_file", 0).toBool();
    Memorized.input_file = settings.value("general/input_file", QString()).toString();
    Memorized.E0 = settings.value("co2amp/E0", 0.01).toString();
    Memorized.w0 = settings.value("co2amp/w0", 2.5).toString();
    Memorized.tau0 = settings.value("co2amp/tau0", 5).toString();
    Memorized.vc = settings.value("co2amp/vc", 28.3062).toString();
    Memorized.t_inj = settings.value("co2amp/t_inj", 0.65).toString();
    Memorized.n_pulses = settings.value("general/n_pulses", 1).toInt();
    Memorized.Dt_train = settings.value("co2amp/Dt_train", 0.1).toString();

    // Opticss and optical layout
    Memorized.optic_id.clear();
    Memorized.optic_type.clear();
    Memorized.optic_yaml.clear();
    Memorized.layout = settings.value("co2amp/layout", "P1 (1000) A1 (1000) P1").toString();

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
                Memorized.optic_id.append(line[0]);
                Memorized.optic_type.append(line[1]);
                file2.setFileName(line[2]);
                file2.open(QIODevice::ReadOnly | QFile::Text);
                Memorized.optic_yaml.append(in2.readAll());
                file2.close();
            }
        }
    }

    PopulateOpticsList();

    // Calculation net
    Memorized.precision_t = settings.value("co2amp/precision_t", 6).toInt();
    Memorized.precision_r = settings.value("co2amp/precision_r", 1).toInt();
    Memorized.t_pulse_min = settings.value("co2amp/t_pulse_min", -100).toString();
    Memorized.t_pulse_max = settings.value("co2amp/t_pulse_max", 400).toString();

    // Comments
    plainTextEdit_comments->setPlainText(settings.value("comments/comments", "- default configuration -").toString());

    // Plot
    Memorized.optic = settings.value("plot/optic", 0).toInt();
    Memorized.pulse = settings.value("plot/pulse", 0).toInt();
    lineEdit_passes->setText(settings.value("plot/passes", "1,2").toString());
    if(Saved.n_pulses == 1) // single pulse
        comboBox_energyPlot->setCurrentIndex(settings.value("plot/energyPlot", 1).toInt());
    else
        comboBox_energyPlot->setCurrentIndex(settings.value("plot/energyPlot", 3).toInt());
    comboBox_timeScale->setCurrentIndex(settings.value("plot/timeScale", 2).toInt());
    comboBox_freqScale->setCurrentIndex(settings.value("plot/freqScale", 2).toInt());
    checkBox_log->setChecked(settings.value("plot/log", 0).toBool());

    // Debugging
    spinBox_debugLevel->setValue(settings.value("debug/debugLevel", 0).toInt());
    checkBox_regBand->setChecked(settings.value("debug/regBand", 1).toBool());
    checkBox_hotBand->setChecked(settings.value("debug/hotBand", 1).toBool());
    checkBox_seqBand->setChecked(settings.value("debug/seqBand", 1).toBool());
    Memorized.noprop = settings.value("co2amp/noprop", 0).toBool();

    // //////////////////////// backwards compatibility start ////////////////////////////////////////////
    double w0 = settings.value("co2amp/w0", 0).toDouble();
    if(w0 >= 800)
        Memorized.vc = QString::number(w0*2.99792458e-2); // 1/cm -> THz
    /*if(settings.value("co2pump/D_interel", "not found").toString() != "not found") // missprint in old versions: "co2pump" instead of "co2amp"
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
    /////////////////////////////////// INPUT PULSE //////////////////////////////////////
    Memorized.from_file = checkBox_from_file->isChecked();
    if(lineEdit_input_file->isVisible())
        Memorized.input_file = lineEdit_input_file->text();
    if(lineEdit_E0->isVisible())
        Memorized.E0 = lineEdit_E0->text();
    if(lineEdit_w0->isVisible())
        Memorized.w0 = lineEdit_w0->text();
    if(lineEdit_tau0->isVisible())
        Memorized.tau0 = lineEdit_tau0->text();
    if(lineEdit_vc->isVisible() && lineEdit_vc->isEnabled())
        Memorized.vc = lineEdit_vc->text();
    if(lineEdit_t_inj->isVisible())
        Memorized.t_inj = lineEdit_t_inj->text();
    if(spinBox_n_pulses->isEnabled())
        Memorized.n_pulses = spinBox_n_pulses->value();
    if(lineEdit_Dt_train->isVisible() && lineEdit_Dt_train->isEnabled())
        Memorized.Dt_train = lineEdit_Dt_train->text();
    /////////////////////////////////// OPTICS //////////////////////////////////////
    //Memorized.optics = plainTextEdit_optics->toPlainText();
    Memorized.layout = plainTextEdit_layout->toPlainText();
    Memorized.noprop = checkBox_noprop->isChecked();
    /////////////////////////////////// GAS MIXTURE //////////////////////////////////////
    /*if(lineEdit_p_CO2->isEnabled())
        Memorized.p_CO2 = lineEdit_p_CO2->text();
    if(lineEdit_p_N2->isEnabled())
        Memorized.p_N2 = lineEdit_p_N2->text();
    if(lineEdit_p_He->isEnabled())
        Memorized.p_He = lineEdit_p_He->text();
    if(lineEdit_T0->isEnabled())
        Memorized.T0 = lineEdit_T0->text();
    if(lineEdit_13C->isEnabled())
        Memorized.percent_13C = lineEdit_13C->text();
    if(lineEdit_18O->isEnabled())
        Memorized.percent_18O = lineEdit_18O->text();*/
    /////////////////////////////////// PUMPING //////////////////////////////////////
    /*if(radioButton_discharge->isEnabled() && radioButton_discharge->isChecked())
        Memorized.pumping = "discharge";
    if(radioButton_optical->isEnabled() && radioButton_optical->isChecked())
        Memorized.pumping = "optical";
    if(lineEdit_Vd->isVisible() && lineEdit_Vd->isEnabled())
        Memorized.Vd = lineEdit_Vd->text();
    if(lineEdit_D_interel->isVisible() && lineEdit_D_interel->isEnabled())
        Memorized.D_interel = lineEdit_D_interel->text();
    if(plainTextEdit_discharge->isVisible() && plainTextEdit_discharge->isEnabled())
        Memorized.discharge = plainTextEdit_discharge->toPlainText();
    if(lineEdit_pump_wl->isVisible() && lineEdit_pump_wl->isEnabled())
        Memorized.pump_wl = lineEdit_pump_wl->text();
    if(lineEdit_pump_sigma->isVisible() && lineEdit_pump_sigma->isEnabled())
        Memorized.pump_sigma = lineEdit_pump_sigma->text();
    if(lineEdit_pump_fluence->isVisible() && lineEdit_pump_fluence->isEnabled())
        Memorized.pump_fluence = lineEdit_pump_fluence->text();*/
    /////////////////////////////////// CALCULATION NET //////////////////////////////////////
    if(lineEdit_t_pulse_min->isEnabled())
        Memorized.t_pulse_min = lineEdit_t_pulse_min->text();
    if(lineEdit_t_pulse_max->isEnabled())
        Memorized.t_pulse_max = lineEdit_t_pulse_max->text();
    if(comboBox_precision_t->isEnabled())
        Memorized.precision_t = comboBox_precision_t->currentIndex();
    if(comboBox_precision_r->isEnabled())
        Memorized.precision_r = comboBox_precision_r->currentIndex();
    /////////////////////////////////// PLOT //////////////////////////////////////
    Memorized.optic = comboBox_optic->currentIndex();
    Memorized.pulse = comboBox_pulse->currentIndex();
}
