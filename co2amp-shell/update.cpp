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
    //pushButton_saveas->setDisabled(flag_calculating || !flag_calculation_success);
    pushButton_calculate->setDisabled(flag_calculating);
    textBrowser_terminal->setEnabled(flag_calculating); //terminal
    pushButton_abort->setEnabled(flag_calculating);
    checkBox_saveWhenFinished->setEnabled(flag_calculating && project_file!=QString());
    checkBox_showCalculationTime->setEnabled(flag_calculating);

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
    if(!lineEdit_w0->hasFocus())
        lineEdit_w0->setText(Memorized.w0);
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
    if(!bl || !flag_input_file_error){
        label_tau0_3->setText("Δν = " + QString::number(0.44/lineEdit_tau0->text().toDouble()) + " THz"); // Δν * Δt ≈ 0.44 (Time-bandwidth product for Gaussian pulse)
        label_vc_3->setText("λ = " + QString::number(299.792458/lineEdit_vc->text().toDouble()) + " µm"); // wl[um] = c[m/s] / nu[THz] * 1e-6
        label_w0_3->setText("FWHM = " + QString::number(lineEdit_w0->text().toDouble()*2*sqrt(log(2)/2)) + " mm"); // FWHM = w* 2 * sqrt(ln(2)/2))
    }
    lineEdit_input_file->setVisible(bl);
    toolButton_input_file->setVisible(bl);
    lineEdit_E0->setVisible(!bl);
    lineEdit_w0->setVisible(!bl);
    lineEdit_tau0->setVisible(!bl);
    lineEdit_vc->setEnabled(!bl);
    label_E0->setVisible(!bl);
    label_E0_2->setVisible(!bl);
    label_w0->setVisible(!bl);
    label_w0_2->setVisible(!bl);
    label_w0_3->setVisible(!bl);
    label_tau0->setVisible(!bl);
    label_tau0_2->setVisible(!bl);
    label_tau0_3->setVisible(!bl);
    spinBox_n_pulses->setEnabled(!bl);
    lineEdit_Dt_train->setEnabled(!bl);
    if(!lineEdit_t_inj->hasFocus())
        lineEdit_t_inj->setText(Memorized.t_inj);

    /////////////////////////////// OPTICSAL OPTICSS AND LAYOUT /////////////////////////////////
    int optic_count = listWidget_optics->count();
    int current_optic = listWidget_optics->currentRow();
    bl = optic_count > 0;
    if(bl)
        plainTextEdit_optic->setPlainText(Memorized.optic_yaml[current_optic]);
    else
        plainTextEdit_optic->setPlainText("");
    pushButton_optic_load->setEnabled(bl);
    pushButton_optic_save->setEnabled(bl);
    plainTextEdit_optic->setEnabled(bl);
    toolButton_optic_remove->setEnabled(bl);
    toolButton_optic_rename->setEnabled(bl);
    toolButton_optic_up->setEnabled(current_optic > 0);
    toolButton_optic_down->setEnabled(current_optic >= 0 && current_optic < optic_count-1);

    //if(!plainTextEdit_optics->hasFocus())
    //    plainTextEdit_optics->setPlainText(Memorized.optics);
    if(!plainTextEdit_layout->hasFocus())
        plainTextEdit_layout->setPlainText(Memorized.layout);
    checkBox_noprop->setChecked(Memorized.noprop);

    /////////////////////////////////// CALCULATION NET //////////////////////////////////////
    bl = checkBox_from_file->isChecked();
    if(bl & flag_input_file_error){
        lineEdit_t_pulse_min->setText("-?-");
        lineEdit_t_pulse_max->setText("-?-");
        label_deltas->setText("");
    }
    else{
        if(!lineEdit_t_pulse_min->hasFocus())
            lineEdit_t_pulse_min->setText(Memorized.t_pulse_min);
        if(!lineEdit_t_pulse_max->hasFocus())
            lineEdit_t_pulse_max->setText(Memorized.t_pulse_max);
        comboBox_precision_t->setCurrentIndex(Memorized.precision_t);
        comboBox_precision_r->setCurrentIndex(Memorized.precision_r);
        double delta_t = (lineEdit_t_pulse_max->text().toDouble()-lineEdit_t_pulse_min->text().toDouble())/(comboBox_precision_t->currentText().toDouble()-1);
        double delta_v = 1.0/(lineEdit_t_pulse_max->text().toDouble()-lineEdit_t_pulse_min->text().toDouble());
        label_deltas->setText("(Δt = " + QString::number(delta_t) + " ps;   Δν = " + QString::number(delta_v) + " THz)");
    }
    comboBox_precision_t->setEnabled(!bl);
    comboBox_precision_r->setEnabled(!bl);
    lineEdit_t_pulse_min->setEnabled(!bl);
    lineEdit_t_pulse_max->setEnabled(!bl);

    /////////////////////// POPULATE COMBOBOXES IN THE OUTPUT TAB //////////////////////////////
    //optics
    //index = comboBox_optic->currentIndex();
    comboBox_optic->clear();
    //list1 = Saved.optic_id;//.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    for(i=0; i<=Saved.optic_id.count()-1; i++){
        //list2 = list1[i].split(QRegExp("[- \t\n]"), QString::SkipEmptyParts);
        //if(list2.count() >= 1)
            comboBox_optic->addItem(Saved.optic_id[i]);
    }
    if(Memorized.optic == -1 || Memorized.optic+1 > comboBox_optic->count())
        comboBox_optic->setCurrentIndex(0);
    else
        comboBox_optic->setCurrentIndex(Memorized.optic);

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
    comboBox_energyPlot->addItem("optic");
    if(Saved.n_pulses > 1){
        comboBox_energyPlot->addItem("pulse");
        comboBox_energyPlot->addItem("optic, pulse");
    }
    if(index == -1 || index+1 > comboBox_pulse->count()){
        if(Saved.n_pulses == 1) // single pulse
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
    checkBox_from_file->blockSignals(block);
    lineEdit_input_file->blockSignals(block);
    checkBox_noprop->blockSignals(block);
    listWidget_optics->blockSignals(block);
    plainTextEdit_optic->blockSignals(block);
    plainTextEdit_layout->blockSignals(block);
    comboBox_precision_t->blockSignals(block);
    comboBox_precision_r->blockSignals(block);
    spinBox_n_pulses->blockSignals(block);
    plainTextEdit_comments->blockSignals(block);
}
