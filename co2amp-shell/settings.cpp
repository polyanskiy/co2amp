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
        settings.setValue("co2amp/r0", Memorized.r0);
        settings.setValue("co2amp/tau0", Memorized.tau0);
        settings.setValue("co2amp/vc", Memorized.vc);
        settings.setValue("co2amp/t_inj", Memorized.t_inj);
        settings.setValue("co2amp/Dt_train", Memorized.Dt_train);

        settings.setValue("co2amp/component_id", Memorized.component_id);
        settings.setValue("co2amp/layout", Memorized.layout);
        settings.setValue("co2amp/noprop", Memorized.noprop);

        //settings.setValue("co2amp/p_CO2", Memorized.p_CO2);
        //settings.setValue("co2amp/p_N2", Memorized.p_N2);
        //settings.setValue("co2amp/p_He", Memorized.p_He);
        //settings.setValue("co2amp/percent_13C", Memorized.percent_13C);
        //settings.setValue("co2amp/percent_18O", Memorized.percent_18O);
        //settings.setValue("co2amp/T0", Memorized.T0);

        //settings.setValue("co2amp/pumping", Memorized.pumping);
        //settings.setValue("co2amp/Vd", Memorized.Vd);
        //settings.setValue("co2amp/D_interel", Memorized.D_interel);
        //settings.setValue("co2amp/discharge", Memorized.discharge);
        //settings.setValue("co2amp/pump_wl", Memorized.pump_wl);
        //settings.setValue("co2amp/pump_sigma", Memorized.pump_sigma);
        //settings.setValue("co2amp/pump_fluence", Memorized.pump_fluence);

        settings.setValue("co2amp/precision_t", Memorized.precision_t);
        settings.setValue("co2amp/precision_r", Memorized.precision_r);
        settings.setValue("co2amp/t_pulse_min", Memorized.t_pulse_min);
        settings.setValue("co2amp/t_pulse_max", Memorized.t_pulse_max);

        settings.setValue("debug/debugLevel", spinBox_debugLevel->text());
        settings.setValue("debug/regBand", checkBox_regBand->isChecked());
        settings.setValue("debug/hotBand", checkBox_hotBand->isChecked());
        settings.setValue("debug/seqBand", checkBox_seqBand->isChecked());

        Saved = Memorized;
    }

    if(what_to_save == "all" || what_to_save == "plot"){
        settings.setValue("plot/component", Memorized.component);
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
    Memorized.r0 = settings.value("co2amp/r0", 0.25).toString();
    Memorized.tau0 = settings.value("co2amp/tau0", 5).toString();
    Memorized.vc = settings.value("co2amp/vc", 28.3062).toString();
    Memorized.t_inj = settings.value("co2amp/t_inj", 0.65).toString();
    Memorized.n_pulses = settings.value("general/n_pulses", 1).toInt();
    Memorized.Dt_train = settings.value("co2amp/Dt_train", 0.1).toString();

    // Calculation net
    Memorized.precision_t = settings.value("co2amp/precision_t", 6).toInt();
    Memorized.precision_r = settings.value("co2amp/precision_r", 1).toInt();
    Memorized.t_pulse_min = settings.value("co2amp/t_pulse_min", -100).toString();
    Memorized.t_pulse_max = settings.value("co2amp/t_pulse_max", 400).toString();

    // Active medium
    /*Memorized.p_CO2 = settings.value("co2amp/p_CO2", 0.5).toString();
    Memorized.p_N2 = settings.value("co2amp/p_N2", 0.5).toString();
    Memorized.p_He = settings.value("co2amp/p_He", 2).toString();
    Memorized.percent_13C = settings.value("co2amp/percent_13C", 0).toString();
    Memorized.percent_18O = settings.value("co2amp/percent_18O", 0).toString();
    Memorized.T0 = settings.value("co2amp/T0", 300).toString();*/

    // Discharge
    /*Memorized.pumping = settings.value("co2amp/pumping", "discharge").toString();
    Memorized.Vd = settings.value("co2amp/Vd", 8500).toString();
    Memorized.D_interel = settings.value("co2amp/D_interel", 8.5).toString();
    Memorized.discharge = settings.value("co2amp/discharge", "0\t0\t600000\n0.02\t6210\t588000\n0.04\t19300\t576000\n0.06\t24600\t565000\n0.08\t27700\t554000\n0.1\t29300\t543000\n0.12\t29800\t532000\n0.14\t29400\t522000\n0.16\t28500\t511000\n0.18\t27100\t501000\n0.2\t25500\t491000\n0.25\t21000\t467000\n0.3\t16600\t444000\n0.35\t12800\t423000\n0.4\t9630\t402000\n0.45\t7140\t383000\n0.5\t5230\t364000\n0.55\t3790\t346000\n0.6\t2730\t329000\n0.7\t1380\t298000\n0.8\t687\t270000\n0.9\t336\t244000\n1\t162\t221000\n1.2\t36.8\t181000\n1.5\t3.77\t134000\n2\t0.0780\t81200\n2.5\t0.00151\t49300\n3\t0.0000281\t29900\n").toString();
    Memorized.pump_wl = settings.value("co2amp/pump_wl", 2.79).toString();
    Memorized.pump_sigma = settings.value("co2amp/pump_sigma", 3e-21).toString();
    Memorized.pump_fluence = settings.value("co2amp/pump_fluence", 1).toString();*/

    // Optical layout
    Memorized.noprop = settings.value("co2amp/noprop", 0).toBool();
    //Memorized.components = settings.value("co2amp/components", "p\tPROBE\t1.0\nam\tAM\t1.0\t100.0\n").toString();
    Memorized.layout = settings.value("co2amp/layout", "p - 100 - am - 100 - p").toString();

    // Comments
    plainTextEdit_comments->setPlainText(settings.value("comments/comments", "- default configuration -").toString());

    // Plot
    Memorized.component = settings.value("plot/component", 0).toInt();
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
    checkBox_hotBand->setChecked(settings.value("debug/hotBand", 0).toBool());
    checkBox_seqBand->setChecked(settings.value("debug/seqBand", 0).toBool());

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
    if(lineEdit_r0->isVisible())
        Memorized.r0 = lineEdit_r0->text();
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
    //Memorized.components = plainTextEdit_components->toPlainText();
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
    Memorized.component = comboBox_component->currentIndex();
    Memorized.pulse = comboBox_pulse->currentIndex();
}
