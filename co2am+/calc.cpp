#include "co2am+.h"


void MainWindow::Calculate()
{
    if(flag_calculating) // avoid starting second process when F6 pressed
        return;

    BeforeProcessStarted(); //prepare everything

    // Composing arguments string
    QStringList arguments;

    arguments << "\"" + path_to_co2amp + "\"";
    arguments << "-v0"        << lineEdit_v0->text();
    arguments << "-n0"        << comboBox_precision_t->currentText();
    arguments << "-x0"        << comboBox_precision_r->currentText();
    arguments << "-t_min"     << lineEdit_t_min->text();
    arguments << "-t_max"     << lineEdit_t_max->text();
    arguments << "-time_tick" << lineEdit_time_tick->text();
    arguments << "-method"    << QString::number(comboBox_method->currentIndex());
    arguments << "-search_dir"<< "\"" + def_dir + "\"";

    arguments << "-debug" << spinBox_debug_level->text();

    //////////////////// Starting The Process ////////////////////
    process = new QProcess(this); 
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(WriteToTerminal()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(AfterProcessFinished()));
    process->setWorkingDirectory(work_dir);
    process->start(arguments.join(" "));
}


void MainWindow::Abort()
{
    flag_calculating = false;
    process->kill();
    Update();
}


void MainWindow::BeforeProcessStarted()
{
    textBrowser_terminal->clear();
    flag_calculating = true;
    ClearWorkDir();
    UpdateConfigurationFiles();
    Update();
    tabWidget_main->setCurrentIndex(1); // Process tab
    ClearPlot();
}


void MainWindow::AfterProcessFinished()
{
    delete process;
    flag_calculating = false;
    flag_project_modified = true;
    flag_plot_postponed = true;
    if(checkBox_saveWhenFinished->isChecked())
        SaveProject(); // must save before Update()
    if(tabWidget_main->currentIndex() == 2)
        Plot(); // this calls Update()
    else
        Update();
}
