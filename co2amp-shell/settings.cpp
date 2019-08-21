#include "mainwindow.h"


void MainWindow::SaveSettings(QString what_to_save)
{
    QSettings settings("project.ini", QSettings::IniFormat);

    if(what_to_save == "all"){
        settings.setValue("co2amp/version",     "2019.08" );
        settings.setValue("grid/vc",             Memorized.vc);
        settings.setValue("grid/precision_t",    Memorized.precision_t);
        settings.setValue("grid/precision_r",    Memorized.precision_r);
        settings.setValue("grid/t_min",          Memorized.t_min);
        settings.setValue("grid/t_max",          Memorized.t_max);
        settings.setValue("grid/time_tick",      Memorized.time_tick);
        settings.setValue("debug/debugLevel",    spinBox_debugLevel->text());
        settings.setValue("debug/noprop",        Memorized.noprop);

        // Write configuration files
        int i;
        int config_file_count = Memorized.configFile_id.size();

        QFile file;
        QTextStream out(&file);

        file.setFileName("config_files.yml");
        file.open(QFile::WriteOnly | QFile::Truncate);

        for(i=0; i<config_file_count; i++){
            out << "- id: " << Memorized.configFile_id[i] << "\n"
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
        settings.setValue("plot/optic",          Memorized.optic);
        settings.setValue("plot/pulse",          Memorized.pulse);
        settings.setValue("plot/energyPlot",     comboBox_energyPlot->currentIndex());
        settings.setValue("plot/passes",         lineEdit_passes->text());
        settings.setValue("plot/timeScale",      comboBox_timeScale->currentIndex());
        settings.setValue("plot/freqScale",      comboBox_freqScale->currentIndex());
        settings.setValue("plot/log",            checkBox_log->isChecked());
        settings.setValue("plot/timeUnit",       comboBox_timeUnit->currentIndex());
        settings.setValue("plot/energyUnit",     comboBox_energyUnit->currentIndex());
        settings.setValue("plot/lengthUnit",     comboBox_lengthUnit->currentIndex());
        settings.setValue("plot/fluenceUnit",    comboBox_fluenceUnit->currentIndex());
        settings.setValue("plot/tUnit",          comboBox_tUnit->currentIndex());
        settings.setValue("plot/powerUnit",      comboBox_powerUnit->currentIndex());
        settings.setValue("plot/frequencyUnit",  comboBox_frequencyUnit->currentIndex());
        settings.setValue("plot/dischargeUnits", comboBox_dischargeUnits->currentIndex());
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
            if(file_record.size() == 2){ // "- id: ...", "type: ..."
                Memorized.configFile_id.append(file_record[0].split(": ")[1]);   // "- id: ..."
                Memorized.configFile_type.append(file_record[1].split(": ")[1]); // "  type: ..."
                file.setFileName(Memorized.configFile_id[i] + ".yml");
                file.open(QIODevice::ReadOnly | QFile::Text);
                Memorized.configFile_content.append(in.readAll());
                file.close();
            }
        }
    }
    PopulateConfigFileList();

    // Calculation net
    Memorized.vc            =                settings.value("grid/vc", 30).toString();
    Memorized.precision_t   =                settings.value("grid/precision_t", 6).toInt();
    Memorized.precision_r   =                settings.value("grid/precision_r", 1).toInt();
    Memorized.t_min         =                settings.value("grid/t_min", -100e-12).toString();
    Memorized.t_max         =                settings.value("grid/t_max", 400e-12).toString();
    Memorized.time_tick     =                settings.value("grid/time_tick", 2e-9).toString();

    // Debugging
    spinBox_debugLevel     -> setValue(      settings.value("debug/debugLevel", 0).toInt());
    Memorized.noprop        =                settings.value("debug/noprop", 0).toBool();

    // Plot
    Memorized.optic         =                 settings.value("plot/optic", 0).toInt();
    Memorized.pulse         =                 settings.value("plot/pulse", 0).toInt();
    lineEdit_passes        -> setText        (settings.value("plot/passes", "1,2").toString());
    if(comboBox_pulse->count() == 1) // single pulse
        comboBox_energyPlot-> setCurrentIndex(settings.value("plot/energyPlot", 1).toInt());
    else
        comboBox_energyPlot-> setCurrentIndex(settings.value("plot/energyPlot", 3).toInt());
    comboBox_timeScale     -> setCurrentIndex(settings.value("plot/timeScale", 2).toInt());
    comboBox_freqScale     -> setCurrentIndex(settings.value("plot/freqScale", 2).toInt());
    checkBox_log           -> setChecked     (settings.value("plot/log", 0).toBool());
    comboBox_timeUnit      -> setCurrentIndex(settings.value("plot/timeUnit", 2).toInt());      // def: us
    comboBox_energyUnit    -> setCurrentIndex(settings.value("plot/energyUnit", 2).toInt());    // def: J
    comboBox_lengthUnit    -> setCurrentIndex(settings.value("plot/lengthUnit", 2).toInt());    // def: mm
    comboBox_fluenceUnit   -> setCurrentIndex(settings.value("plot/fluenceUnit", 4).toInt());   // def: J/cm2
    comboBox_tUnit         -> setCurrentIndex(settings.value("plot/tUnit", 4).toInt());         // def: ps
    comboBox_powerUnit     -> setCurrentIndex(settings.value("plot/powerUnit", 2).toInt());     // def: TW
    comboBox_frequencyUnit -> setCurrentIndex(settings.value("plot/frequencyUnit", 0).toInt()); // def: THz
    comboBox_dischargeUnits-> setCurrentIndex(settings.value("plot/dischargeUnits", 1).toInt());// def: kV, kA

    // //////////////////////// backwards compatibility start ////////////////////////////////////////////
    // Backward compatibolity broken in 2019 major prigram re-dezign.
    // Intention is to support backwords campatibility fron now on (at least untill next big upgrade)
    // //////////////////////// backwards compatibility end /////////////////////////////////////////////

    Saved = Memorized;
    flag_projectloaded = true;
    UpdateControls();
}


void MainWindow::MemorizeSettings()
{
    /////////////////////////////////// CALCULATION GRID //////////////////////////////////////
    Memorized.vc          = lineEdit_vc->text();
    Memorized.t_min       = lineEdit_t_min->text();
    Memorized.t_max       = lineEdit_t_max->text();
    Memorized.time_tick   = lineEdit_time_tick->text();
    Memorized.precision_t = comboBox_precision_t->currentIndex();
    Memorized.precision_r = comboBox_precision_r->currentIndex();
    /////////////////////////////////// DEBUGGING //////////////////////////////////////
    Memorized.noprop      = checkBox_noprop->isChecked();
    /////////////////////////////////// PLOT //////////////////////////////////////
    Memorized.optic       = comboBox_optic->currentIndex();
    Memorized.pulse       = comboBox_pulse->currentIndex();
}
