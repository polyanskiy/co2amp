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
    arguments << "-search_dir"<< "\"" + def_dir + "\"";

    if(checkBox_noprop->isChecked())
        arguments << "-noprop";

    arguments << "-debug" << spinBox_debug_level->text();

    //////////////////// Starting The Process ////////////////////
    process = new QProcess(this); 
    /*// Add def_dir to environment. This allows using a relative path
    // as "file" variable of "from file" pulses (hdf5).
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    //QString str = env.value("PATH");
    QMessageBox::information(this, "co2amp", env.value("PATH"));
    env.insert("PATH", env.value("PATH") + ";" + def_dir);
    QMessageBox::information(this, "co2amp", env.value("PATH"));
    //str = env.value("PATHEXT");
    QMessageBox::information(this, "co2amp", env.value("PATHEXT"));
    //if(str[str.size()-1] != ";") str+= ";";
    env.insert("PATHEXT", env.value("PATHEXT") + ";.H5;.h5;.*");
    QMessageBox::information(this, "co2amp", env.value("PATHEXT"));
    process->setProcessEnvironment(env);*/

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

    /*QDir dir;
    dir.setPath(def_dir);
    QStringList filelist = dir.entryList();
    int i;
    for(i=0; i<filelist.size(); i++){
        QFile file(filelist[i]);
        QFileInfo fileinfo(file);
        if(fileinfo.suffix() == "h5"){
            QMessageBox::information(this, "co2amp", work_dir  + fileinfo.fileName());
            file.copy(work_dir + fileinfo.fileName());
        }
    }*/

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
