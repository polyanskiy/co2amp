#include "mainwindow.h"


void MainWindow::UpdateControls()
{
    int i, index;
    bool bl;
    QString str;

    //QCoreApplication::processEvents(QEventLoop::AllEvents,1000);

    //////////////////////////// block signals ///////////////////////////
    BlockSignals(true);

    /////////// check if there is any active madium section among the optics //////////
    /*noam = true;
    QStringList list1, list2;
    list1 = Memorized.optics;//.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    for(i=0; i<=list1.count()-1; i++){
        list2 = list1[i].split(QRegExp("[- \t\n]"), QString::SkipEmptyParts);
        if(list2.count() >= 2 && list2[1]=="AM")
            noam = false;
    }*/

    ////////////////////////////////// GUI CONTROLS //////////////////////////////////
    if(!flag_calculating){
        checkBox_saveWhenFinished->setChecked(0);
        checkBox_showCalculationTime->setChecked(0);
        //textBrowser->clear();
    }
    pushButton_new->setDisabled(flag_calculating);
    pushButton_open->setDisabled(flag_calculating);
    pushButton_save->setDisabled(flag_calculating || !flag_calculation_success || !(flag_plot_modified || flag_comments_modified || flag_results_modified) || project_file==QString());
    pushButton_saveas->setDisabled(flag_calculating || !flag_calculation_success);
    pushButton_go->setDisabled(flag_calculating);
    //textBrowser_terminal->setEnabled(flag_calculating); //terminal
    pushButton_abort->setEnabled(flag_calculating);
    checkBox_saveWhenFinished->setEnabled(flag_calculating && project_file!=QString());
    checkBox_showCalculationTime->setEnabled(flag_calculating);


    /////////////////////////////// CONFIGURATION FILES /////////////////////////////////
    int config_file_count = listWidget_configFile_list->count();
    int current_config_file = listWidget_configFile_list->currentRow();
    bl = config_file_count > 0;
    if(bl){
        plainTextEdit_configFile_content->setPlainText(Memorized.configFile_content[current_config_file]);
        label_configFile_info->setText(Memorized.configFile_id[current_config_file]
                                       + ".yml   (type: " + Memorized.configFile_type[current_config_file] + ")");
    }
    else{
        plainTextEdit_configFile_content->setPlainText("");
        label_configFile_info->setText("");
    }
    pushButton_configFile_load->setEnabled(bl);
    pushButton_configFile_save->setEnabled(bl);
    plainTextEdit_configFile_content->setEnabled(bl);
    toolButton_configFile_remove->setEnabled(bl);
    toolButton_configFile_rename->setEnabled(bl);
    toolButton_configFile_up->setEnabled(current_config_file > 0);
    toolButton_configFile_down->setEnabled(current_config_file >= 0 && current_config_file < config_file_count-1);

    checkBox_noprop->setChecked(Memorized.noprop);


    /////////////////////////////////// CALCULATION GRID //////////////////////////////////////


    if(!lineEdit_vc->hasFocus())
        lineEdit_vc->setText(Memorized.vc);
    if(!lineEdit_t_min->hasFocus())
        lineEdit_t_min->setText(Memorized.t_min);
    if(!lineEdit_t_max->hasFocus())
        lineEdit_t_max->setText(Memorized.t_max);
    if(!lineEdit_clock_tick->hasFocus())
        lineEdit_clock_tick->setText(Memorized.clock_tick);
    comboBox_precision_t->setCurrentIndex(Memorized.precision_t);
    comboBox_precision_r->setCurrentIndex(Memorized.precision_r);
    double delta_t = (lineEdit_t_max->text().toDouble()-lineEdit_t_min->text().toDouble())/(comboBox_precision_t->currentText().toDouble()-1);
    double delta_v = 1.0/(lineEdit_t_max->text().toDouble()-lineEdit_t_min->text().toDouble());
    label_deltas->setText("(Δt = " + QString::number(delta_t) + " s;   Δν = " + QString::number(delta_v) + " Hz)");
    label_um->setText("(λ = " + QString::number(2.99792458e14/lineEdit_vc->text().toDouble()) + " µm)"); // wl[um] = c[m/s] / nu[Hz] * 1e6

    /*comboBox_precision_t->setEnabled(!bl);
    comboBox_precision_r->setEnabled(!bl);
    lineEdit_t_min->setEnabled(!bl);
    lineEdit_t_max->setEnabled(!bl);*/


    /////////////////////// POPULATE COMBOBOXES IN THE OUTPUT TAB //////////////////////////////
    //optic and pulse
    comboBox_optic->clear();
    comboBox_pulse->clear();
    for(i=0; i<=Saved.configFile_id.count()-1; i++){
        QString type = Saved.configFile_type[i];
        if(type != "PULSE" && type != "LAYOUT" && type != "COMMENT")
            comboBox_optic->addItem(Saved.configFile_id[i]);
        if(type == "PULSE")
            comboBox_pulse->addItem(Saved.configFile_id[i]);
    }
    if(Memorized.optic == -1 || Memorized.optic+1 > comboBox_optic->count())
        comboBox_optic->setCurrentIndex(0);
    else
        comboBox_optic->setCurrentIndex(Memorized.optic);   
    if(Memorized.pulse == -1 || Memorized.pulse+1 > comboBox_pulse->count())
        comboBox_pulse->setCurrentIndex(0);
    else
        comboBox_pulse->setCurrentIndex(Memorized.pulse);

    //energyPlot
    index = comboBox_energyPlot->currentIndex();
    comboBox_energyPlot->clear();
    comboBox_energyPlot->addItem("all");
    comboBox_energyPlot->addItem("optic");
    if(comboBox_pulse->count() > 1){
        comboBox_energyPlot->addItem("pulse");
        comboBox_energyPlot->addItem("optic, pulse");
    }
    if(index == -1 || index+1 > comboBox_energyPlot->count()){
        if(comboBox_pulse->count() == 1) // single pulse
            comboBox_energyPlot->setCurrentIndex(1);
        else
            comboBox_energyPlot->setCurrentIndex(3);
    }
    else
        comboBox_energyPlot->setCurrentIndex(index);

    /////////////////////// ENABLE/DISABLE CONTROLS IN OUTPUT TAB //////////////////////////////
    bl = (flag_projectloaded || flag_calculation_success);
    //scrollArea_plotControls->setEnabled(bl);
    pushButton_update->setEnabled(bl);
    if(!bl)
        ClearPlot();

    bl = (comboBox_size->currentText()=="Custom");
    spinBox_width->setVisible(bl);
    spinBox_height->setVisible(bl);

    //////////////////////// unblock signals /////////////////////////////
    BlockSignals(false);
}


void MainWindow::BlockSignals(bool block)
{
    checkBox_noprop->blockSignals(block);
    listWidget_configFile_list->blockSignals(block);
    plainTextEdit_configFile_content->blockSignals(block);
    comboBox_precision_t->blockSignals(block);
    comboBox_precision_r->blockSignals(block);
}
