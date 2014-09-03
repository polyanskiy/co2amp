#include "mainwindow.h"


void MainWindow::UpdateControls()
{
    int i, index;
    bool bl;
    QString str;

    //////////////////////////// block signals ///////////////////////////
    BlockSignals(true);

    /////////// check if there is any active madium section among the components //////////
    noam = true;
    QStringList list1, list2;
    list1 = Memorized.components.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    for(i=0; i<=list1.count()-1; i++){
        list2 = list1[i].split(QRegExp("[- \t\n]"), QString::SkipEmptyParts);
        if(list2.count() >= 2 && list2[1]=="AM")
            noam = false;
    }

    ////////////////////////////////// GUI CONTROLS //////////////////////////////////
    if(!flag_calculating){
        checkBox_saveWhenFinished->setChecked(0);
        textBrowser->clear();
    }
    textBrowser->setVisible(flag_calculating);
    scrollArea_input->setVisible(!flag_calculating);
    pushButton_calculate->setVisible(!flag_calculating);
    pushButton_abort->setVisible(flag_calculating);
    checkBox_saveWhenFinished->setVisible(flag_calculating);
    checkBox_saveWhenFinished->setDisabled(project_file==QString());
    checkBox_showCalculationTime->setVisible(flag_calculating);
    pushButton_new->setDisabled(flag_calculating);
    pushButton_open->setDisabled(flag_calculating);
    pushButton_save->setDisabled(flag_calculating || !flag_calculation_success || !(flag_plot_modified || flag_comments_modified) || project_file==QString());
    pushButton_saveas->setDisabled(flag_calculating || !flag_calculation_success);

    ////////////////////////////////// NUMBER OF PASSES ///////////////////////////////////
    spinBox_n_pulses->setValue(Memorized.n_pulses);
    bl = (Memorized.n_pulses > 1); // not "single pulse"
    label_Dt_train->setVisible(bl);
    lineEdit_Dt_train->setVisible(bl);
    label_Dt_train_2->setVisible(bl);

    /////////////////////////////////// INPUT PULSE //////////////////////////////////////
    checkBox_from_file->setChecked(Memorized.from_file);
    if(!lineEdit_input_file->hasFocus())
        lineEdit_input_file->setText(Memorized.input_file);
    bl = checkBox_from_file->isChecked();
    if(!lineEdit_E0->hasFocus())
        lineEdit_E0->setText(Memorized.E0);
    if(!lineEdit_r0->hasFocus())
        lineEdit_r0->setText(Memorized.r0);
    if(!lineEdit_tau0->hasFocus())
        lineEdit_tau0->setText(Memorized.tau0);
    QPalette *palette = new QPalette();
    if(bl & flag_input_file_error){
        lineEdit_vc->setText("-?-");
        label_vc_3->setText("");
        lineEdit_Dt_train->setText("-?-");
        palette->setColor(QPalette::Text,Qt::red);
    }
    else{
        if(!lineEdit_vc->hasFocus())
            lineEdit_vc->setText(Memorized.vc);
        if(!lineEdit_Dt_train->hasFocus())
            lineEdit_Dt_train->setText(Memorized.Dt_train);
        palette->setColor(QPalette::Text,Qt::black);
    }
    lineEdit_input_file->setPalette(*palette);
    if(!flag_input_file_error)
        label_vc_3->setText("λ = " + QString::number(299.792458/lineEdit_vc->text().toDouble()) + " μm"); // wl[um] = c[m/s] / nu[THz] * 1e-6
    lineEdit_input_file->setVisible(bl);
    toolButton_input_file->setVisible(bl);
    lineEdit_E0->setVisible(!bl);
    lineEdit_r0->setVisible(!bl);
    lineEdit_tau0->setVisible(!bl);
    lineEdit_vc->setEnabled(!bl);
    label_E0->setVisible(!bl);
    label_E0_2->setVisible(!bl);
    label_r0->setVisible(!bl);
    label_r0_2->setVisible(!bl);
    label_tau0->setVisible(!bl);
    label_tau0_2->setVisible(!bl);
    spinBox_n_pulses->setEnabled(!bl);
    lineEdit_Dt_train->setEnabled(!bl);
    if(!lineEdit_t_inj->hasFocus())
        lineEdit_t_inj->setText(Memorized.t_inj);

    /////////////////////////////////// GAS MIXTURE //////////////////////////////////////
    bl = noam;
    if(bl){
        lineEdit_p_CO2->setText("-");
        lineEdit_p_N2->setText("-");
        lineEdit_p_He->setText("-");
    }
    else{
        if(!lineEdit_p_CO2->hasFocus())
            lineEdit_p_CO2->setText(Memorized.p_CO2);
        if(!lineEdit_p_N2->hasFocus())
            lineEdit_p_N2->setText(Memorized.p_N2);
        if(!lineEdit_p_He->hasFocus())
            lineEdit_p_He->setText(Memorized.p_He);
    }
    groupBox_mixture->setEnabled(!bl);
    groupBox_pumping->setEnabled(!bl);

    /////////////////////////////////// OPTICS, GEOMETRY //////////////////////////////////////
    if(!plainTextEdit_components->hasFocus())
        plainTextEdit_components->setPlainText(Memorized.components);
    if(!plainTextEdit_layout->hasFocus())
        plainTextEdit_layout->setPlainText(Memorized.layout);
    checkBox_noprop->setChecked(Memorized.noprop);

    /////////////////////////////////// PUMPING //////////////////////////////////////
    radioButton_discharge->setChecked(Memorized.pumping=="discharge");
    radioButton_optical->setChecked(Memorized.pumping=="optical");

    bl = (Memorized.pumping=="discharge");
    lineEdit_Vd->setVisible(bl);
    label_Vd->setVisible(bl);
    label_Vd_2->setVisible(bl);
    lineEdit_D_interel->setVisible(bl);
    label_D_interel->setVisible(bl);
    label_D_interel_2->setVisible(bl);
    toolButton_discharge->setVisible(bl);
    plainTextEdit_discharge->setVisible(bl);

    bl = (Memorized.pumping=="optical");
    lineEdit_pump_wl->setVisible(bl);
    label_pump_wl->setVisible(bl);
    label_pump_wl_2->setVisible(bl);
    lineEdit_pump_sigma->setVisible(bl);
    label_pump_sigma->setVisible(bl);
    label_pump_sigma_2->setVisible(bl);
    lineEdit_pump_fluence->setVisible(bl);
    label_pump_fluence->setVisible(bl);
    label_pump_fluence_2->setVisible(bl);

    bl = noam || Memorized.p_CO2.toDouble()+Memorized.p_N2.toDouble()+Memorized.p_He.toDouble() == 0;// Zero total pressure
    groupBox_pumping->setEnabled(!bl);
    lineEdit_T0->setEnabled(!bl);
    label_T0_1->setEnabled(!bl);
    label_T0_2->setEnabled(!bl);
    if(bl){
        lineEdit_Vd->setText("-");
        lineEdit_D_interel->setText("-");
        plainTextEdit_discharge->setPlainText("-no active medium-");
        lineEdit_pump_wl->setText("-");
        lineEdit_pump_sigma->setText("-");
        lineEdit_pump_fluence->setText("-");
        lineEdit_T0->setText("-");
    }
    else{
        if(!lineEdit_Vd->hasFocus())
            lineEdit_Vd->setText(Memorized.Vd);
        if(!lineEdit_D_interel->hasFocus())
            lineEdit_D_interel->setText(Memorized.D_interel);
        if(!lineEdit_pump_wl->hasFocus())
            lineEdit_pump_wl->setText(Memorized.pump_wl);
        if(!lineEdit_pump_sigma->hasFocus())
            lineEdit_pump_sigma->setText(Memorized.pump_sigma);
        if(!lineEdit_pump_fluence->hasFocus())
            lineEdit_pump_fluence->setText(Memorized.pump_fluence);
        if(!lineEdit_T0->hasFocus())
            lineEdit_T0->setText(Memorized.T0);
        if(!plainTextEdit_discharge->hasFocus())// && plainTextEdit_discharge->toPlainText() != Memorized.discharge)
            plainTextEdit_discharge->setPlainText(Memorized.discharge);
    }

    bl = noam || Memorized.p_CO2.toDouble() == 0;// Zero CO2 pressure
    if(bl){
        lineEdit_13C->setText("-");
        lineEdit_18O->setText("-");
    }
    else{
        if(!lineEdit_13C->hasFocus())
            lineEdit_13C->setText(Memorized.percent_13C);
        if(!lineEdit_18O->hasFocus())
            lineEdit_18O->setText(Memorized.percent_18O);
    }
    lineEdit_13C->setEnabled(!bl);
    lineEdit_18O->setEnabled(!bl);
    label_13C->setEnabled(!bl);
    label_18O->setEnabled(!bl);
    label_13C_2->setEnabled(!bl);
    label_18O_2->setEnabled(!bl);

    /////////////////////////////////// CALCULATION NET //////////////////////////////////////
    bl = checkBox_from_file->isChecked();
    if(bl & flag_input_file_error){
        lineEdit_t_pulse_min->setText("-?-");
        lineEdit_t_pulse_max->setText("-?-");
    }
    else{
        if(!lineEdit_t_pulse_min->hasFocus())
            lineEdit_t_pulse_min->setText(Memorized.t_pulse_min);
        if(!lineEdit_t_pulse_max->hasFocus())
            lineEdit_t_pulse_max->setText(Memorized.t_pulse_max);
        comboBox_precision_t->setCurrentIndex(Memorized.precision_t);
        comboBox_precision_r->setCurrentIndex(Memorized.precision_r);
    }
    comboBox_precision_t->setEnabled(!bl);
    comboBox_precision_r->setEnabled(!bl);
    lineEdit_t_pulse_min->setEnabled(!bl);
    lineEdit_t_pulse_max->setEnabled(!bl);

    /////////////////////// POPULATE COMBOBOXES IN THE OUTPUT TAB //////////////////////////////
    //components
    //index = comboBox_component->currentIndex();
    comboBox_component->clear();
    list1 = Saved.components.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    for(i=0; i<=list1.count()-1; i++){
        list2 = list1[i].split(QRegExp("[- \t\n]"), QString::SkipEmptyParts);
        if(list2.count() >= 1)
            comboBox_component->addItem(list2[0]);
    }
    if(Memorized.component == -1 || Memorized.component+1 > comboBox_component->count())
        comboBox_component->setCurrentIndex(0);
    else
        comboBox_component->setCurrentIndex(Memorized.component);

    //pulse
    //index = comboBox_pulse->currentIndex();
    comboBox_pulse->clear();
    for(i=0; i<=Saved.n_pulses-1; i++)
        comboBox_pulse->addItem(QString::number(i+1));
    if(Memorized.pulse == -1 || Memorized.pulse+1 > comboBox_pulse->count())
        comboBox_pulse->setCurrentIndex(0);
    else
        comboBox_pulse->setCurrentIndex(Memorized.pulse);
    groupBox_pulse->setDisabled(Saved.n_pulses == 1); // Hide pulse combo if n_pulses==1

    //energyPlot
    index = comboBox_energyPlot->currentIndex();
    comboBox_energyPlot->clear();
    comboBox_energyPlot->addItem("all");
    comboBox_energyPlot->addItem("component");
    if(Saved.n_pulses > 1){
        comboBox_energyPlot->addItem("pulse");
        comboBox_energyPlot->addItem("component, pulse");
    }
    if(index == -1 || index+1 > comboBox_pulse->count()){
        if(Saved.n_pulses == 1) // single pulse
            comboBox_energyPlot->setCurrentIndex(1);
        else
            comboBox_energyPlot->setCurrentIndex(3);
    }
    else
        comboBox_energyPlot->setCurrentIndex(index);

    //////////////////////// unblock signals /////////////////////////////
    BlockSignals(false);
}


void MainWindow::BlockSignals(bool block)
{
    checkBox_from_file->blockSignals(block);
    lineEdit_input_file->blockSignals(block);
    checkBox_noprop->blockSignals(block);
    plainTextEdit_components->blockSignals(block);
    plainTextEdit_layout->blockSignals(block);
    plainTextEdit_discharge->blockSignals(block);
    comboBox_precision_t->blockSignals(block);
    comboBox_precision_r->blockSignals(block);
    spinBox_n_pulses->blockSignals(block);
    plainTextEdit_comments->blockSignals(block);
}
