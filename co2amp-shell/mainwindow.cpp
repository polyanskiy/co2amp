#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
    setupUi(this);

    /////////////////////////// External programs //////////////////////////
    path_to_core = "co2amp-core";
    path_to_gnuplot = "gnuplot";
    path_to_7zip = "7za";
    #ifdef Q_OS_WIN
        path_to_core = QCoreApplication::applicationDirPath() + "/co2amp-core.exe";
        path_to_core = "\"" + QDir::toNativeSeparators(path_to_core) + "\"";
        path_to_gnuplot = QCoreApplication::applicationDirPath() + "/gnuplot/bin/gnuplot.exe";
        path_to_gnuplot = "\"" + QDir::toNativeSeparators(path_to_gnuplot) + "\"";
        path_to_7zip = QCoreApplication::applicationDirPath() + "/7-zip/x64/7za.exe";
        path_to_7zip = "\"" + QDir::toNativeSeparators(path_to_7zip) + "\"";
    #endif

    /////////////////////////// Create temporary working directory //////////////////////////
    int i = 0;
    int j;
    QString str;
    QDir dir;
    do{
        str.setNum(i);
        work_dir = QDir::tempPath() + "/co2amp/";
        for(j=0; j<6-str.size(); j++)
            work_dir+= "0";
        work_dir += str + "/";
        work_dir = QDir::toNativeSeparators(work_dir);
        dir.setPath(work_dir);
        i++;
    }
    while(dir.exists());
    dir.mkpath(work_dir);
    QDir::setCurrent(work_dir);

    project_file = QString();

    //////////////////////////////////////// Validators ///////////////////////////////////////
    lineEdit_E0->setValidator(new QDoubleValidator(this));
    lineEdit_r0->setValidator(new QDoubleValidator(this));
    lineEdit_tau0->setValidator(new QDoubleValidator(this));
    lineEdit_vc->setValidator(new QDoubleValidator(this));
    lineEdit_t_inj->setValidator(new QDoubleValidator(this));
    lineEdit_Dt_train->setValidator(new QDoubleValidator(this));
    lineEdit_p_CO2->setValidator(new QDoubleValidator(this));
    lineEdit_p_N2->setValidator(new QDoubleValidator(this));
    lineEdit_p_He->setValidator(new QDoubleValidator(this));
    lineEdit_13C->setValidator(new QDoubleValidator(0,100,3,this));
    lineEdit_18O->setValidator(new QDoubleValidator(0,100,3,this));
    lineEdit_Vd->setValidator(new QDoubleValidator(this));
    lineEdit_D_interel->setValidator(new QDoubleValidator(this));
    lineEdit_pump_wl->setValidator(new QDoubleValidator(this));
    lineEdit_pump_sigma->setValidator(new QDoubleValidator(this));
    lineEdit_pump_fluence->setValidator(new QDoubleValidator(this));
    lineEdit_T0->setValidator(new QDoubleValidator(this));
    lineEdit_t_pulse_min->setValidator(new QDoubleValidator(this));
    lineEdit_t_pulse_max->setValidator(new QDoubleValidator(this));

    //////////////////////////////////// Load session /////////////////////////////////////////
    QSettings settings("ATF", "co2amp");
    def_dir = settings.value("def_dir", "").toString();
    comboBox_plotSize->setCurrentIndex(settings.value("plot_size", "0").toInt());
    spinBox_fontSize->setValue(settings.value("plot_font_size", "10").toInt());
    checkBox_grid->setChecked(settings.value("plot_grid", 1).toBool());
    checkBox_labels->setChecked(settings.value("plot_labels", 1).toBool());
    textBrowser->setVisible(false); // hide terminal
    tabWidget_main->setCurrentIndex(0); // Calculations tab
    flag_calculating = false;
    flag_input_file_error = false;

    /////////////////////////////////// Signal-Slot Connections //////////////////////////////////
    connect(pushButton_save, SIGNAL(clicked()), this, SLOT(SaveProject()));
    connect(comboBox_component, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(comboBox_pulse, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(comboBox_energyPlot, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(lineEdit_passes, SIGNAL(returnPressed()), this, SLOT(Plot()));
    connect(comboBox_timeScale, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(comboBox_freqScale, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(checkBox_grid, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(checkBox_labels, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(comboBox_plotSize, SIGNAL(activated(QString)), this, SLOT(Plot()));
    connect(spinBox_fontSize, SIGNAL(valueChanged(int)), this, SLOT(Plot()));
    connect(checkBox_log, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(pushButton_update, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(plainTextEdit_comments, SIGNAL(textChanged()), this, SLOT(Comments()));
    connect(checkBox_from_file, SIGNAL(clicked()), this, SLOT(OnModified()));
    connect(spinBox_n_pulses, SIGNAL(valueChanged(int)), this, SLOT(OnModified()));
    connect(lineEdit_input_file, SIGNAL(textChanged(QString)), this, SLOT(OnModified()));
    connect(lineEdit_E0, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_r0, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_tau0, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_vc, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_t_inj, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_Dt_train, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_p_CO2, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_p_N2, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_p_He, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_13C, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_18O, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_T0, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(plainTextEdit_components, SIGNAL(textChanged()), this, SLOT(OnModified()));
    connect(plainTextEdit_layout, SIGNAL(textChanged()), this, SLOT(OnModified()));
    connect(checkBox_noprop, SIGNAL(clicked()), this, SLOT(OnModified()));
    connect(radioButton_discharge, SIGNAL(clicked()), this, SLOT(OnModified()));
    connect(radioButton_optical, SIGNAL(clicked()), this, SLOT(OnModified()));
    connect(lineEdit_Vd, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_D_interel, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_pump_wl, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_pump_sigma, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_pump_fluence, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(plainTextEdit_discharge, SIGNAL(textChanged()), this, SLOT(OnModified()));
    connect(comboBox_precision_t, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnModified()));
    connect(comboBox_precision_r, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnModified()));
    connect(lineEdit_t_pulse_min, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_t_pulse_max, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_passes, SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(pushButton_calculate, SIGNAL(clicked()), this, SLOT(Calculate()));
    connect(pushButton_abort, SIGNAL(clicked()), this, SLOT(Abort()));



    // Process command line
    QStringList arg = qApp->arguments();
    if( arg.count()>1 && QFile::exists(arg.last()) ){
        project_file = arg.last();
        LoadProject();
    }
    else
        NewProject();

    /////////////////////////////// Create program window /////////////////////////////////////
    restoreGeometry(settings.value("window_geometry").toByteArray());
    show();
}

MainWindow::~MainWindow()
{
    if(flag_calculating)
        process->kill();

    // Save session
    QSettings settings("ATF", "co2amp");
    settings.setValue("def_dir", def_dir);
    settings.setValue("window_geometry", saveGeometry());
    settings.setValue("plot_size", comboBox_plotSize->currentIndex());
    settings.setValue("plot_font_size", spinBox_fontSize->value());
    settings.setValue("plot_grid", checkBox_grid->isChecked());
    settings.setValue("plot_labels", checkBox_labels->isChecked());

    // Remove temporary working directory
    ClearWorkDir();
    QDir dir;
    QDir::setCurrent(QDir::homePath()); // cannot remove current dir under Windows
    dir.rmpath(work_dir);
}


void MainWindow::Abort()
{
    flag_calculation_success = false;
    process->kill();
    UpdateControls();
}


void MainWindow::on_toolButton_input_file_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, QString(), def_dir, tr("CO2 projects with field (*.co2x)"));
    if(str == QString())
        return;
    Memorized.input_file = QDir::toNativeSeparators(str);
    lineEdit_input_file->setText(Memorized.input_file);
}


void MainWindow::Calculate()
{
    BeforeProcessStarted(); //prepare everything

    // Writing optical configuration files
    QFile file;
    QTextStream out(&file);

    file.setFileName("optics_components.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << Memorized.components << "\n";
    file.close();

    file.setFileName("optics_layout.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << Memorized.layout << "\n";
    file.close();

    file.setFileName("discharge.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << Memorized.discharge << "\n";
    file.close();

    // Composing arguments string
    QStringList arguments;

    arguments << path_to_core;

    if(Memorized.from_file) // Load field from file
        arguments << "-from_file 1";
    else{
        arguments << "-from_file 0";
        arguments << "-E0" << Memorized.E0;
        arguments << "-r0" << Memorized.r0;
        arguments << "-tau0" << Memorized.tau0;
    }
    // vc, n_pulses, and Dt_train may be different form memorized if loading input pulse from file
    arguments << "-vc" << lineEdit_vc->text();
    arguments << "-t_inj" << Memorized.t_inj;
    arguments << "-n_pulses" << spinBox_n_pulses->text();
    if(spinBox_n_pulses->text() != "1")
        arguments << "-Dt_train" << lineEdit_Dt_train->text();

    arguments << "-p_626" << QString::number(Memorized.p_CO2.toDouble() * (1-Memorized.percent_13C.toDouble()/100) * (1-Memorized.percent_18O.toDouble()/100)*(1-Memorized.percent_18O.toDouble()/100));
    arguments << "-p_628" << QString::number(Memorized.p_CO2.toDouble() * (1-Memorized.percent_13C.toDouble()/100) * 2*(1-Memorized.percent_18O.toDouble()/100)*(Memorized.percent_18O.toDouble()/100));
    arguments << "-p_828" << QString::number(Memorized.p_CO2.toDouble() * (1-Memorized.percent_13C.toDouble()/100) * (Memorized.percent_18O.toDouble()/100)*(Memorized.percent_18O.toDouble()/100));
    arguments << "-p_636" << QString::number(Memorized.p_CO2.toDouble() *   (Memorized.percent_13C.toDouble()/100) * (1-Memorized.percent_18O.toDouble()/100)*(1-Memorized.percent_18O.toDouble()/100));
    arguments << "-p_638" << QString::number(Memorized.p_CO2.toDouble() *   (Memorized.percent_13C.toDouble()/100) * 2*(1-Memorized.percent_18O.toDouble()/100)*(Memorized.percent_18O.toDouble()/100));
    arguments << "-p_838" << QString::number(Memorized.p_CO2.toDouble() *   (Memorized.percent_13C.toDouble()/100) * (Memorized.percent_18O.toDouble()/100)*(Memorized.percent_18O.toDouble()/100));
    arguments << "-p_N2" << Memorized.p_N2;
    arguments << "-p_He" << Memorized.p_He;
    arguments << "-T0" << Memorized.T0;

    arguments << "-pumping" << Memorized.pumping;
    if(Memorized.pumping=="discharge"){
        arguments << "-Vd" << Memorized.Vd;
        arguments << "-D" << Memorized.D_interel;
    }
    else{ // optical pumping
        arguments << "-pump_wl" << Memorized.pump_wl;
        arguments << "-pump_sigma" << Memorized.pump_sigma;
        arguments << "-pump_fluence" << Memorized.pump_fluence;
    }

    // n0, x0, t_pulse_min, and t_pulse_max may be different form memorized if loading input pulse from file
    arguments << "-n0" << comboBox_precision_t->currentText();
    arguments << "-x0" << comboBox_precision_r->currentText();
    arguments << "-t_pulse_lim" << QString::number(lineEdit_t_pulse_max->text().toDouble()-lineEdit_t_pulse_min->text().toDouble());
    arguments << "-t_pulse_shift" << QString::number(-1*lineEdit_t_pulse_min->text().toDouble());

    if(Memorized.noprop)
        arguments << "-noprop";

    arguments << "-debug" << spinBox_debugLevel->text();
    arguments << "-bands" << QString::number(checkBox_regBand->isChecked() + 2*checkBox_hotBand->isChecked() + 4*checkBox_seqBand->isChecked());

    //////////////////// Starting The Process ////////////////////
    process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(UpdateTerminal()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(AfterProcessFinished()));
    process->setWorkingDirectory(work_dir);
    process->start(arguments.join(" "));
}


void MainWindow::NewProject()
{
    ClearWorkDir();
    project_file = QString();
    LoadSettings(QString());
    SaveSettings("all"); // save all settings - input and plot
    MainWindow::setWindowTitle("untitled - co2amp");
    ShowFigures();
    flag_calculation_success = false;
    flag_plot_modified = false;
    flag_comments_modified = false;
    flag_field_ready_to_save = false;
    UpdateControls();
}


void MainWindow::on_pushButton_new_clicked()
{
    if(SaveBeforeClose())
        NewProject();
}


void MainWindow::on_pushButton_open_clicked()
{
    if (SaveBeforeClose()){
        QString str;
        if(project_file == QString())
            str = QFileDialog::getOpenFileName(this, QString(), def_dir, tr("CO2 projects (*.co2;*.co2x)"));
        else
            str = QFileDialog::getOpenFileName(this, QString(), project_file, tr("CO2 projects (*.co2;*.co2x)"));
        if(str != QString()){
            project_file = QDir::toNativeSeparators(str);
            LoadProject();
        }
    }
}


void MainWindow::on_pushButton_saveas_clicked()
{
    QString str, selfilter;
    QFileInfo fileinfo;

    if(project_file == QString()){
        if(flag_field_ready_to_save)
            str = QFileDialog::getSaveFileName(this, QString(), def_dir, tr("CO2 project (*.co2);;CO2 project with field (*.co2x)"));
        else
            str = QFileDialog::getSaveFileName(this, QString(), def_dir, tr("CO2 project (*.co2)"));
    }
    else{
        if(flag_field_ready_to_save){
            fileinfo.setFile(project_file);
            selfilter = fileinfo.suffix()=="co2x" ? tr("CO2 project with field (*.co2x)") : nullptr;
            str = QFileDialog::getSaveFileName(this, QString(), project_file, tr("CO2 project (*.co2);;CO2 project with field (*.co2x)"), &selfilter);
        }
        else
            str = QFileDialog::getSaveFileName(this, QString(), project_file, tr("CO2 project (*.co2)"));
    }

    if(str == QString())
        return;

    fileinfo.setFile(str);
    QDir dir = fileinfo.dir();
    if(fileinfo.suffix()!="co2" && fileinfo.suffix()!="co2x"){
        QMessageBox mb( "Error - co2amp", "Wrong or missing extension (must be .co2 or .co2x)", QMessageBox::Warning, QMessageBox::Ok, 0, 0);
        mb.exec();
        return;
    }
    if(fileinfo.suffix()=="co2x" && !flag_field_ready_to_save){
        QMessageBox mb( "Error - co2amp", "Not enough data to save as a .co2x file. Run calculations or use .co2 extension.", QMessageBox::Warning, QMessageBox::Ok, 0, 0);
        mb.exec();
        return;
    }
    project_file = QDir::toNativeSeparators(dir.absolutePath() + "/" + fileinfo.fileName());
    def_dir = QDir::toNativeSeparators(dir.absolutePath());
    SaveProject();
    fileinfo.setFile(project_file);
    MainWindow::setWindowTitle(fileinfo.fileName() + " - co2amp");
}


void MainWindow::SaveProject()
{
    QFileInfo fileinfo(project_file);
    QFile::remove(project_file);
    QFile::remove("field_in.bin");
    QProcess *proc;
    proc = new QProcess(this);
    if(fileinfo.suffix()=="co2x") // extended project file suitable for sequensing
        proc->start(path_to_7zip + " a -tzip \"" + project_file + "\" *.dat ; *.txt ; *.png ; *.ini ; *.bin");
    else // basic (small) project file (.co2)
        proc->start(path_to_7zip + " a -tzip \"" + project_file + "\" *.dat ; *.txt ; *.png ; *.ini"); // don't include field (field.bin)
    proc->waitForFinished();
    flag_plot_modified = false;
    flag_comments_modified = false;
    UpdateControls();
}


void MainWindow::LoadProject()
{
    if(project_file != QString()){
        this->setCursor(Qt::WaitCursor);
        ClearWorkDir();
        QProcess *proc;
        proc = new QProcess(this);
        proc->start(path_to_7zip + " e -y \"" + project_file + "\"");
        proc->waitForFinished();
        LoadSettings("project.ini");
        //ShowFigures();
        Plot();
        QFileInfo fileinfo(project_file);
        def_dir = QDir::toNativeSeparators(fileinfo.dir().absolutePath());
        LoadInputPulse();

        MainWindow::setWindowTitle(fileinfo.fileName() + " - co2amp");;
        flag_calculation_success = true;
        flag_plot_modified = false;
        flag_comments_modified = false;
        if(fileinfo.suffix()=="co2x" && QFile::exists("field.bin"))
            flag_field_ready_to_save = true;
        UpdateControls();
        this->setCursor(Qt::ArrowCursor);
    }
    else
        MainWindow::setWindowTitle("co2amp");
}


void MainWindow::UpdateTerminal()
{
    QStringList strlist;
    QString str;

    str = process->readAllStandardOutput();

    #ifdef Q_OS_WIN
        strlist = str.split("\r\n");
        str = strlist.join("\n");
    #endif

    strlist = str.split("\r");

    int number_of_returns = strlist.size() - 1;

    int i;
    for (i=0; i<=number_of_returns; i++){
        textBrowser->insertPlainText(strlist[i]);
        textBrowser->moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    }
    textBrowser->moveCursor(QTextCursor::End);
}


void MainWindow::BeforeProcessStarted()
{
    flag_calculating = true;
    flag_calculation_success = true;
    ClearWorkDir();
    SaveSettings("all"); // save all settings - input and plot
    LoadInputPulse();
    UpdateControls();
    timer.start();
}


void MainWindow::AfterProcessFinished()
{
    bool showtime, save;
    flag_calculating = false;
    QMessageBox mb( "Info - co2amp", "Calculation time: " + QString::number(timer.elapsed()/1000) + " s", QMessageBox::Information, QMessageBox::Ok, 0, 0);
    if(flag_calculation_success){
        flag_field_ready_to_save = true;
        save = checkBox_saveWhenFinished->isChecked();
        showtime = checkBox_showCalculationTime->isChecked();
        Plot();
        tabWidget_main->setCurrentIndex(1); // Output tab
        if(save)
            SaveProject();
        if(showtime)
            mb.exec();
    }
    UpdateControls();
}


bool MainWindow::SaveBeforeClose() // return: TRUE if ok to close, FALSE otherwise
{
    QMessageBox::StandardButton mb;
    if(flag_calculating){ // co2amp-core running
         mb = QMessageBox::warning(this, tr("Calculation in progress - co2amp"), tr("Cannot close: Calculation in progress.\nWait for calculation to complete or abort it."), QMessageBox::Ok);
         return false;
    }
    if(!flag_calculation_success) // cannot save if calculations not completed nothing to save
        return true;
    if(!flag_plot_modified && !flag_comments_modified) // nothing changed
        return true;

    mb = QMessageBox::warning(this, tr("Project modified - co2amp"), tr("The project has been modified.\nDo you want to save changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (mb == QMessageBox::Save){
        if(project_file != QString()){
            SaveProject();
            return true;
        }
        else{
            on_pushButton_saveas_clicked();
            if(project_file != QString())
                return true;
            else
                return false;
        }
    }

    if (mb == QMessageBox::Discard)
        return true;

    return false; // Cancel clicked
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if(SaveBeforeClose())
        event->accept();
    else
        event->ignore();
}


void MainWindow::OnModified()
{
    if(!flag_projectloaded)
        return;
    MemorizeSettings();  // Load values from edit boxes to memory
    LoadInputPulse();
    UpdateControls();
}


void MainWindow::LoadInputPulse()
{
    if(!Memorized.from_file)
        return;

    flag_input_file_error = false;

    QFile::remove("field_in.bin");

    QString file_to_load;
    QFileInfo fileinfo(Memorized.input_file);
    if(fileinfo.fileName()==Memorized.input_file) //file name without directory - assume same directory as the project file
        file_to_load = QDir::toNativeSeparators(def_dir+"/"+Memorized.input_file);
    else
        file_to_load = Memorized.input_file;

    QFile file(file_to_load);
    if(Memorized.input_file=="" || !file.exists()){
        flag_input_file_error = true;
        return;
    }

    QProcess *proc;
    proc = new QProcess(this);
    proc->start(path_to_7zip + " e -y -otmp \"" + file_to_load + "\" project.ini");
    proc->waitForFinished();
    proc->start(path_to_7zip + " e -y -otmp \"" + file_to_load + "\" field.bin");
    proc->waitForFinished();
    QFile::copy(QDir::toNativeSeparators("tmp/project.ini"), "input.ini");
    QFile::copy(QDir::toNativeSeparators("tmp/field.bin"), "field_in.bin");
    QFile::remove(QDir::toNativeSeparators("tmp/project.ini"));
    QFile::remove(QDir::toNativeSeparators("tmp/field.bin"));
    QDir dir;
    dir.rmdir("tmp");

    QSettings settings("input.ini", QSettings::IniFormat);

    if( settings.value("co2amp/t_pulse_min", "x").toString() == "x"
           || settings.value("co2amp/t_pulse_max", "x").toString() == "x"
           || settings.value("co2amp/vc", "x").toString() == "x"
           || settings.value("general/n_pulses", "x").toString() == "x"
           || !QFile::exists("field_in.bin") )
        flag_input_file_error = true;
    else{
        Saved.t_pulse_min = settings.value("co2amp/t_pulse_min", "").toString();
        Saved.t_pulse_max = settings.value("co2amp/t_pulse_max", "").toString();
        Saved.vc = settings.value("co2amp/vc", "").toString();
        Saved.Dt_train = settings.value("co2amp/Dt_train", "").toString();
        Saved.n_pulses = settings.value("general/n_pulses", 1).toInt();
        Saved.precision_t = settings.value("co2amp/precision_t", 2).toInt();
        Saved.precision_r = settings.value("co2amp/precision_r", 2).toInt();

        Memorized.t_pulse_min = Saved.t_pulse_min;
        Memorized.t_pulse_max = Saved.t_pulse_max;
        Memorized.vc = Saved.vc;
        Memorized.Dt_train = Saved.Dt_train;
        Memorized.n_pulses = Saved.n_pulses;
        Memorized.precision_t = Saved.precision_t;
        Memorized.precision_r = Saved.precision_r;
    }
    QFile::remove("input.ini");
}


void MainWindow::ClearWorkDir()
{
    QDir dir;
    dir.setPath(work_dir);
    QStringList strlist = dir.entryList();
    int i;
    for(i=0; i<strlist.size(); i++)
        dir.remove(strlist[i]);
}


void MainWindow::Comments()
{
    if(!flag_projectloaded)
        return;
    SaveSettings("comments");
    flag_comments_modified = true;
    UpdateControls();
}
