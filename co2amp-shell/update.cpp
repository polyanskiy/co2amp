#include "co2amp.h"


void MainWindow::Update()
{
    int index;
    bool bl;
    QString str;

    //////////////////////////////////// GUI CONTROLS ///////////////////////////////////
    if(!flag_calculating){
        checkBox_saveWhenFinished->setChecked(0);
    }
    pushButton_new            -> setDisabled(flag_calculating);
    pushButton_open           -> setDisabled(flag_calculating);
    pushButton_save           -> setDisabled(flag_calculating
                                             || !flag_project_modified
                                             || project_file==QString());
    pushButton_saveas         -> setDisabled(flag_calculating);
    pushButton_go             -> setDisabled(flag_calculating);
    pushButton_abort          -> setEnabled (flag_calculating);
    checkBox_saveWhenFinished -> setEnabled (flag_calculating && project_file!=QString());

    /////////////////////////////// CONFIGURATION FILES /////////////////////////////////
    int config_file_count   = listWidget_configFile_list->count();
    int current_config_file = listWidget_configFile_list->currentRow();
    bl = config_file_count > 0 ;

    plainTextEdit_configFile_content->blockSignals(true);
    if(bl){
        if(!plainTextEdit_configFile_content->hasFocus())
            plainTextEdit_configFile_content->setPlainText(configFile_content[current_config_file]);
        label_configFile_info->setText(configFile_id[current_config_file]
                                       + ".yml (type: " + configFile_type[current_config_file] + ")");
    }
    else{
        plainTextEdit_configFile_content->setPlainText("");
        label_configFile_info->setText("");
    }
    plainTextEdit_configFile_content->blockSignals(false);

    pushButton_configFile_load       -> setEnabled(bl && !flag_calculating);
    pushButton_configFile_save       -> setEnabled(bl);
    pushButton_fixFormat             -> setEnabled(bl && !flag_calculating);
    plainTextEdit_configFile_content -> setEnabled(bl && !flag_calculating);
    toolButton_configFile_add        -> setEnabled(!flag_calculating);
    toolButton_configFile_remove     -> setEnabled(bl && !flag_calculating);
    toolButton_configFile_rename     -> setEnabled(bl && !flag_calculating);
    toolButton_configFile_up         -> setEnabled(current_config_file > 0 && !flag_calculating);
    toolButton_configFile_down       -> setEnabled(current_config_file >= 0 && !flag_calculating
                                                   && current_config_file < config_file_count-1);

    ///////////////////////////////// CALCULATION GRID //////////////////////////////////
    double delta_t = (lineEdit_t_max->text().toDouble()-lineEdit_t_min->text().toDouble())/comboBox_precision_t->currentText().toDouble();
    double delta_v = 1.0/(lineEdit_t_max->text().toDouble()-lineEdit_t_min->text().toDouble());
    label_deltas->setText("(Δt = " + QString::number(delta_t) + " s;   Δν = " + QString::number(delta_v) + " Hz)");
    label_um->setText("(λ = " + QString::number(2.99792458e14/lineEdit_vc->text().toDouble()) + " µm)"); // wl[um] = c[m/s] / nu[Hz] * 1e6
    tmp_precision_t = comboBox_precision_t->currentIndex();
    tmp_precision_r = comboBox_precision_r->currentIndex();
    groupBox_grid->setDisabled(flag_calculating);

    ////////////////////////////// CALCULATION PARAMETERS ///////////////////////////////
    groupBox_parameters->setDisabled(flag_calculating);

    ///////////////////////////////////// DEBUGGING /////////////////////////////////////
    groupBox_debugging->setDisabled(flag_calculating);

    /////////////////////// POPULATE COMBOBOXES IN THE OUTPUT TAB ///////////////////////
    //optic and pulse
    int optic_n = comboBox_optic->currentIndex();
    int pulse_n = comboBox_pulse->currentIndex();
    comboBox_optic->blockSignals(true);
    comboBox_pulse->blockSignals(true);
    comboBox_optic->clear();
    comboBox_pulse->clear();
    for(int i=0; i<configFile_id.count(); i++){
        QString type = configFile_type[i];
        if(type != "PULSE" && type != "LAYOUT" && type != "COMMENT")
            comboBox_optic->addItem(configFile_id[i]);
        if(type == "PULSE")
            comboBox_pulse->addItem(configFile_id[i]);
    }
    comboBox_optic->setCurrentIndex(optic_n);
    comboBox_pulse->setCurrentIndex(pulse_n);
    comboBox_optic->blockSignals(false);
    comboBox_pulse->blockSignals(false);

    //energyPlot
    index = comboBox_energyPlot->currentIndex();
    comboBox_energyPlot     -> blockSignals(true);
    comboBox_energyPlot     -> clear();
    comboBox_energyPlot     -> addItem("all");
    comboBox_energyPlot     -> addItem("optic");
    if(comboBox_pulse->count() > 1){
        comboBox_energyPlot -> addItem("pulse");
        comboBox_energyPlot -> addItem("optic, pulse");
    }
    if(index == -1 || index+1 > comboBox_energyPlot->count()){
        if(comboBox_pulse->count() == 1) // single pulse
            comboBox_energyPlot->setCurrentIndex(1);
        else
            comboBox_energyPlot->setCurrentIndex(3);
    }
    else
        comboBox_energyPlot -> setCurrentIndex(index);
    comboBox_energyPlot     -> blockSignals(false);

    /////////////////////// ENABLE/DISABLE CONTROLS IN OUTPUT TAB ///////////////////////
    bl = (QFile::exists("data_energy.dat") || QFile::exists("data_discharge.dat"));
    tab_output->setEnabled(bl);

    bl = (comboBox_size->currentText()=="Custom");
    spinBox_width ->setVisible(bl);
    spinBox_height->setVisible(bl);
}
