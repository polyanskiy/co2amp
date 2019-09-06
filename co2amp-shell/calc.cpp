#include "co2amp.h"


void MainWindow::Calculate()
{
    BeforeProcessStarted(); //prepare everything

    // Composing arguments string
    QStringList arguments;

    arguments << "\"" + path_to_core + "\"";
    arguments << "-vc"        << lineEdit_vc->text();
    arguments << "-n0"        << comboBox_precision_t->currentText();
    arguments << "-x0"        << comboBox_precision_r->currentText();
    arguments << "-t_min"     << lineEdit_t_min->text();
    arguments << "-t_max"     << lineEdit_t_max->text();
    arguments << "-time_tick" << lineEdit_time_tick->text();

    if(checkBox_noprop->isChecked())
        arguments << "-noprop";

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
    flag_calculating         = true;
    //flag_project_modified    = false;
    ClearWorkDir();
    UpdateConfigurationFiles();
    Update();
    tabWidget_main->setCurrentIndex(1); // Process tab
}


void MainWindow::AfterProcessFinished()
{
    flag_calculating = false;
    //if(process->exitCode()==EXIT_SUCCESS){
        flag_project_modified = true;
        flag_plot_postponed = true;
        //tabWidget_main->setCurrentIndex(2); // Output tab (Plot will be called)
        if(tabWidget_main->currentIndex()==2)
            Plot();
        if(checkBox_saveWhenFinished->isChecked())
            SaveProject();
    //}
    delete process;
    Update();
}