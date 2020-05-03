#include "co2am+.h"


void MainWindow::UpdateConfigurationFiles()
{
    // co2am+.ini
    QSettings settings("co2am+.ini", QSettings::IniFormat);

    formatVersion = 2020;

    settings.setValue("co2am+/formatVersion", formatVersion);

    settings.setValue("grid/v0",             lineEdit_v0->text());
    settings.setValue("grid/precision_t",    comboBox_precision_t->currentIndex());
    settings.setValue("grid/precision_r",    comboBox_precision_r->currentIndex());
    settings.setValue("grid/t_min",          lineEdit_t_min->text());
    settings.setValue("grid/t_max",          lineEdit_t_max->text());
    settings.setValue("grid/time_tick",      lineEdit_time_tick->text());
    settings.setValue("calc/method",         comboBox_method->currentIndex());
    settings.setValue("plot/optic",          comboBox_optic->currentIndex());
    settings.setValue("plot/pulse",          comboBox_pulse->currentIndex());
    settings.setValue("plot/energyPlot",     comboBox_energyPlot->currentIndex());
    settings.setValue("plot/passes",         lineEdit_passes->text());
    settings.setValue("plot/timeScale",      comboBox_timeScale->currentIndex());
    settings.setValue("plot/freqScale",      comboBox_freqScale->currentIndex());
    settings.setValue("plot/log",            checkBox_log->isChecked());
    settings.setValue("plot/timeUnit",       comboBox_timeUnit->currentIndex());
    settings.setValue("plot/lengthUnit",     comboBox_lengthUnit->currentIndex());
    settings.setValue("plot/tUnit",          comboBox_tUnit->currentIndex());
    settings.setValue("plot/frequencyUnit",  comboBox_frequencyUnit->currentIndex());
    settings.setValue("plot/energyUnit",     comboBox_energyUnit->currentIndex());
    settings.setValue("plot/fluenceUnit",    comboBox_fluenceUnit->currentIndex());
    settings.setValue("plot/powerUnit",      comboBox_powerUnit->currentIndex());
    settings.setValue("plot/intensityUnit",  comboBox_intensityUnit->currentIndex());
    settings.setValue("plot/dischargeUnits", comboBox_dischargeUnits->currentIndex());

    // YAML files
    QFile file;
    QTextStream out(&file);

    file.setFileName("config_files.yml");
    file.open(QFile::WriteOnly | QFile::Truncate);

    for(int i=0; i<configFile_id.size(); i++)
    {
        out << "- id: " << configFile_id[i] << "\n"
            << "  type: " <<configFile_type[i] << "\n";
    }
    file.close();

    for(int i=0; i<configFile_id.size(); i++)
    {
        file.setFileName(configFile_id[i]+".yml");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << configFile_content[i];
        file.close();
    }

    // comments.txt
    file.setFileName("comments.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << plainTextEdit_comments->toPlainText();
    file.close();
}


void MainWindow::ReadConfigurationFiles()
{
    // YAML files
    configFile_id.clear();
    configFile_type.clear();
    configFile_content.clear();

    QString str;
    QStringList file_list, file_record;
    QFile file;
    QTextStream in(&file);

    file.setFileName("config_files.yml");
    if(file.open(QIODevice::ReadOnly | QFile::Text))
    {
        str = in.readAll();
        file.close();
        file_list = str.split("- ", QString::SkipEmptyParts);
        for(int i=0; i<file_list.size(); i++)
        {
            file_record = file_list[i].split("\n", QString::SkipEmptyParts);
            if(file_record.size() == 2) // "- id: ...", "type: ..."
            {
                QString id   = file_record[0].split(": ")[1]; // "- id: ..."
                QString type = file_record[1].split(": ")[1]; // "  type: ..."
                //add element to id/type/content list
                configFile_id  .append(id);
                configFile_type.append(type);
                file.setFileName(id + ".yml");
                file.open(QIODevice::ReadOnly | QFile::Text);
                configFile_content.append(in.readAll());
                file.close();
                //add entries to "Pulse" and "Optic" combo boxes (output tab)
                if(type != "PULSE" && type != "LAYOUT")
                    comboBox_optic->addItem(id);
                if(type == "PULSE")
                    comboBox_pulse->addItem(id);
            }
        }
    }
    PopulateConfigFileList();

    // co2am+.ini
    QSettings settings("co2am+.ini", QSettings::IniFormat);
    formatVersion = settings.value("co2am+/formatVersion", "2020").toFloat(); // only change when format is changed (not every release)
    lineEdit_v0            -> setText        (settings.value("grid/v0",       "30e12").toString());
    comboBox_precision_t   -> setCurrentIndex(settings.value("grid/precision_t",    5).toInt());
    comboBox_precision_r   -> setCurrentIndex(settings.value("grid/precision_r",    5).toInt());
    lineEdit_t_min         -> setText        (settings.value("grid/t_min", "-250e-12").toString());
    lineEdit_t_max         -> setText        (settings.value("grid/t_max",  "250e-12").toString());
    lineEdit_time_tick     -> setText        (settings.value("grid/time_tick", "2e-9").toString());
    comboBox_method        -> setCurrentIndex(settings.value("calc/method",         5).toInt());
    comboBox_optic         -> setCurrentIndex(settings.value("plot/optic",          0).toInt());
    comboBox_pulse         -> setCurrentIndex(settings.value("plot/pulse",          0).toInt());
    lineEdit_passes        -> setText        (settings.value("plot/passes",
                                                                "0,1,2,3,4,5,6,7,8,9").toString());
    if(comboBox_pulse->count() == 1) // single pulse
        comboBox_energyPlot-> setCurrentIndex(settings.value("plot/energyPlot",     1).toInt());
    else
        comboBox_energyPlot-> setCurrentIndex(settings.value("plot/energyPlot",     3).toInt());
    comboBox_timeScale     -> setCurrentIndex(settings.value("plot/timeScale",      0).toInt());
    comboBox_freqScale     -> setCurrentIndex(settings.value("plot/freqScale",      0).toInt());
    checkBox_log           -> setChecked     (settings.value("plot/log",            0).toBool());
    comboBox_timeUnit      -> setCurrentIndex(settings.value("plot/timeUnit",       2).toInt()); // def: us
    comboBox_lengthUnit    -> setCurrentIndex(settings.value("plot/lengthUnit",     2).toInt()); // def: mm
    comboBox_tUnit         -> setCurrentIndex(settings.value("plot/tUnit",          4).toInt()); // def: ps
    comboBox_frequencyUnit -> setCurrentIndex(settings.value("plot/frequencyUnit",  0).toInt()); // def: THz
    comboBox_energyUnit    -> setCurrentIndex(settings.value("plot/energyUnit",     2).toInt()); // def: J
    comboBox_fluenceUnit   -> setCurrentIndex(settings.value("plot/fluenceUnit",    4).toInt()); // def: J/cm2
    comboBox_powerUnit     -> setCurrentIndex(settings.value("plot/powerUnit",      2).toInt()); // def: TW
    comboBox_intensityUnit -> setCurrentIndex(settings.value("plot/intensityUnit",  1).toInt()); // def: kW/cm2
    comboBox_dischargeUnits-> setCurrentIndex(settings.value("plot/dischargeUnits", 1).toInt()); // def: kV, kA

    // comments.txt
    file.setFileName("comments.txt");
    plainTextEdit_comments->setPlainText("");
    if(file.open(QIODevice::ReadOnly | QFile::Text))
    {
        plainTextEdit_comments->setPlainText(in.readAll());
        file.close();
    }

    // /////////////////////////////// backwards compatibility start /////////////////////////////////////
    if(!QFile::exists("co2am+.ini") && QFile::exists("project.ini")) //pre-2019
        formatVersion = 2015;

    if(formatVersion<2018.9) // not a default, but less than given version
    {
        QMessageBox::critical(this, "co2am+", "It looks like this file was created by an older "
                                              "version of co2amp/co2am+ and is not supported.\n"
                                              "Try using co2amp v.2019-04-29");
        return;
    }

    if(formatVersion>=2018.9 && formatVersion<2019.1)
    {
        lineEdit_v0            -> setText        (settings.value("grid/vc",       "30e12").toString());
    }
    // /////////////////////////////// backwards compatibility end ///////////////////////////////////////
    Update();
}
