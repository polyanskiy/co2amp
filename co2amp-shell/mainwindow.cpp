#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
    setupUi(this);

    /////////////////////////// External programs //////////////////////////
    path_to_core    = QStandardPaths::findExecutable("co2amp-core");
    path_to_7zip    = QStandardPaths::findExecutable("7z");
    path_to_gnuplot = QStandardPaths::findExecutable("gnuplot");

    #ifdef Q_OS_WIN
        // co2amp-core
        if(path_to_core==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\co2amp.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QString(QCoreApplication::applicationDirPath()),
                QString(getenv("PROGRAMFILES"))+"/co2amp"
            };
            path_to_core = QStandardPaths::findExecutable("co2amp-core", searchpaths);
        }
        // 7-Zip
        if(path_to_7zip==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\7zFM.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\7-Zip", QSettings::NativeFormat).value("Path").toString(),
                QString(getenv("PROGRAMFILES"))+"/7-Zip",
                QString(getenv("PROGRAMFILES"))+" (x86)/7-Zip"
            };
            path_to_7zip = QStandardPaths::findExecutable("7z", searchpaths);
        }
        // Gnuplot
        if(path_to_gnuplot==""){
            QStringList searchpaths =
            {
                QFileInfo(QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\gnuplot.exe",
                                                        QSettings::NativeFormat).value("Default").toString()).absoluteDir().absolutePath(),
                QString(getenv("PROGRAMFILES"))+"/gnuplot\\bin",
                QString(getenv("PROGRAMFILES"))+" (x86)/gnuplot/bin"
            };
            path_to_gnuplot = QStandardPaths::findExecutable("gnuplot", searchpaths);
        }
    #endif

    if(path_to_core=="")
        QMessageBox().critical(this, "co2amp", "\'co2amp-core\' executable not found. Try re-installing co2amp.");
    if(path_to_7zip=="")
        QMessageBox().critical(this, "co2amp", "7-Zip not found. Please (re)install -  it\'s free.");
    if(path_to_gnuplot=="")
        QMessageBox().critical(this, "co2amp", "Gnuplot not found. Please (re)install -  it\'s free.");

    if(path_to_core=="" || path_to_7zip=="" || path_to_gnuplot=="")
        return;

    /////////////////////////// Flags //////////////////////////
    flag_projectloaded = false;
    flag_calculating = false;
    flag_calculation_success = false;
    flag_field_ready_to_save = false;
    flag_input_file_error = false;
    flag_plot_modified = false;
    flag_plot_postponed = false;

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
    lineEdit_vc        -> setValidator(new QDoubleValidator(this));
    lineEdit_t_min     -> setValidator(new QDoubleValidator(this));
    lineEdit_t_max     -> setValidator(new QDoubleValidator(this));
    lineEdit_time_tick -> setValidator(new QDoubleValidator(this));

    //////////////////////////////////// Load session /////////////////////////////////////////
    QSettings settings("ATF", "co2amp");
    def_dir             = settings.value("def_dir", "").toString();
    yaml_dir            = settings.value("yaml_dir", "").toString();
    comboBox_size      -> setCurrentIndex(settings.value("plot_size", "0").toInt());
    spinBox_width      -> setValue(settings.value("plot_width", "1600").toInt());
    spinBox_height     -> setValue(settings.value("plot_height", "1200").toInt());
    doubleSpinBox_zoom -> setValue(settings.value("plot_zoom", "1").toDouble());
    checkBox_grid      -> setChecked(settings.value("plot_grid", 1).toBool());
    checkBox_labels    -> setChecked(settings.value("plot_labels", 1).toBool());
    tabWidget_main     -> setCurrentIndex(0); // always set to input tab

    /////////////////////////////////// Signal-Slot Connections //////////////////////////////////
    // YamlAutoFormat()
    connect(pushButton_fixFormat,    SIGNAL(clicked()),           this, SLOT(YamlFixFormat()));
    // Calculate()
    connect(pushButton_go,           SIGNAL(clicked()),           this, SLOT(Calculate()));
    F6 = new QShortcut(QKeySequence("F6"), this);
    connect(F6,                      SIGNAL(activated()),         this, SLOT(Calculate()));
    // Abort()
    connect(pushButton_abort,        SIGNAL(clicked()), this, SLOT(Abort()));
    // SaveProject()
    connect(pushButton_save,         SIGNAL(clicked()), this, SLOT(SaveProject()));
    // Plot() "Graph view" settings are considered application preferences
    // they are stored in registry, and not saved as part of project
    // modifying them causes graphs to re-plot but doesn't rise the "modified" flag
    connect(comboBox_size,           SIGNAL(activated(QString)),  this, SLOT(Plot()));
    connect(checkBox_grid,           SIGNAL(clicked()),           this, SLOT(Plot()));
    connect(checkBox_labels,         SIGNAL(clicked()),           this, SLOT(Plot()));
    connect(pushButton_update,       SIGNAL(clicked()),           this, SLOT(Plot()));
    // FlagModifiedAndPlot()
    connect(comboBox_optic,          SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_pulse,          SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_energyPlot,     SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_timeScale,      SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_freqScale,      SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(checkBox_log,            SIGNAL(clicked()),           this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_timeUnit,       SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_energyUnit,     SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_lengthUnit,     SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_fluenceUnit,    SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_tUnit,          SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_powerUnit,      SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_frequencyUnit,  SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    connect(comboBox_dischargeUnits, SIGNAL(activated(QString)),  this, SLOT(FlagModifiedAndPlot()));
    // PostponePlot() ...without rising the "modified" flag
    connect(spinBox_width,           SIGNAL(valueChanged(int)),   this, SLOT(PostponePlot()));
    connect(spinBox_height,          SIGNAL(valueChanged(int)),   this, SLOT(PostponePlot()));
    connect(doubleSpinBox_zoom,      SIGNAL(valueChanged(double)),this, SLOT(PostponePlot()));
    // FlagModifiedAndPostponePlot()
    connect(lineEdit_passes,         SIGNAL(textEdited(QString)), this, SLOT(FlagModifiedAndPostponePlot()));
    // PlotIfPostponed()
    connect(lineEdit_passes,         SIGNAL(returnPressed()),     this, SLOT(PlotIfPostponed()));
    connect(spinBox_width,           SIGNAL(editingFinished()),   this, SLOT(PlotIfPostponed()));
    connect(spinBox_height,          SIGNAL(editingFinished()),   this, SLOT(PlotIfPostponed()));
    connect(doubleSpinBox_zoom,      SIGNAL(editingFinished()),   this, SLOT(PlotIfPostponed()));
    // OnModified()
    connect(lineEdit_vc,             SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(checkBox_noprop,         SIGNAL(clicked()),           this, SLOT(OnModified()));
    connect(comboBox_precision_t,    SIGNAL(activated(QString)),  this, SLOT(OnModified()));
    connect(comboBox_precision_r,    SIGNAL(activated(QString)),  this, SLOT(OnModified()));
    connect(lineEdit_t_min,          SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_t_max,          SIGNAL(textEdited(QString)), this, SLOT(OnModified()));
    connect(lineEdit_time_tick,      SIGNAL(textEdited(QString)), this, SLOT(OnModified()));

    /////////////////////////////// Process command line //////////////////////////////////////
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

    // Initialize YAML highlighter
    highlighter = new YamlHighlighter(plainTextEdit_configFile_content->document());
}

MainWindow::~MainWindow()
{
    if(flag_calculating)
        process->kill();

    // Save session
    QSettings settings("ATF", "co2amp");
    settings.setValue("def_dir", def_dir);
    settings.setValue("yaml_dir", yaml_dir);
    settings.setValue("window_geometry", saveGeometry());
    settings.setValue("plot_size", comboBox_size->currentIndex());
    settings.setValue("plot_width", spinBox_width->value());
    settings.setValue("plot_height", spinBox_height->value());
    settings.setValue("plot_zoom", doubleSpinBox_zoom->value());
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
    flag_calculating = false;
    process->kill();
    UpdateControls();
}


void MainWindow::Calculate()
{
    BeforeProcessStarted(); //prepare everything

    // Composing arguments string
    QStringList arguments;

    arguments << "\"" + path_to_core + "\"";

    arguments << "-vc"        << lineEdit_vc->text();
    // vc, n0, x0, t_min, and t_max may be different form memorized if loading input pulse from file
    arguments << "-n0"        << comboBox_precision_t->currentText();
    arguments << "-x0"        << comboBox_precision_r->currentText();
    arguments << "-t_min"     << QString::number(lineEdit_t_min->text().toDouble());
    arguments << "-t_max"     << QString::number(lineEdit_t_max->text().toDouble());
    arguments << "-time_tick" << QString::number(lineEdit_time_tick->text().toDouble());

    if(Memorized.noprop)
        arguments << "-noprop";

    arguments << "-debug" << spinBox_debugLevel->text();

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
    flag_projectloaded = false;
    flag_calculating = false;
    flag_calculation_success = false;
    flag_results_modified = false;
    flag_field_ready_to_save = false;
    flag_input_file_error = false;
    flag_plot_modified = false;
    flag_plot_postponed = false;
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
            str = QFileDialog::getOpenFileName(this, QString(), def_dir, "CO2 projects (*.co2;*.co2x)");
        else
            str = QFileDialog::getOpenFileName(this, QString(), project_file, "CO2 projects (*.co2;*.co2x)");
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
            str = QFileDialog::getSaveFileName(this, QString(), def_dir, "CO2 project (*.co2);;CO2 project with field (*.co2x)");
        else
            str = QFileDialog::getSaveFileName(this, QString(), def_dir, "CO2 project (*.co2)");
    }
    else{
        if(flag_field_ready_to_save){
            fileinfo.setFile(project_file);
            selfilter = fileinfo.suffix()=="co2x" ? "CO2 project with field (*.co2x)" : nullptr;
            str = QFileDialog::getSaveFileName(this, QString(), project_file, "CO2 project (*.co2);;CO2 project with field (*.co2x)", &selfilter);
        }
        else
            str = QFileDialog::getSaveFileName(this, QString(), project_file, "CO2 project (*.co2)");
    }

    if(str == QString())
        return;

    fileinfo.setFile(str);
    QDir dir = fileinfo.dir();
    if(fileinfo.suffix()!="co2" && fileinfo.suffix()!="co2x"){
        QMessageBox().warning(this, "co2amp", "Wrong or missing extension (must be .co2 or .co2x)");
        return;
    }
    if(fileinfo.suffix()=="co2x" && !flag_field_ready_to_save){
        QMessageBox().warning(this, "co2amp", "Not enough data to save as a .co2x file. Run calculations or use .co2 extension.");
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
    if(QFile::exists(project_file) && !QFile::remove(project_file)){// cannot remove project file (e.g. it's open in another program)      
        QMessageBox().warning(this, "co2amp", "Cannot save to the existing file (file may be open in another program)");
        return;
    }

    // temporary stuff - delete later
    QFile::remove("field_in.bin");
    QFile::remove("re.dat");
    QFile::remove("im.dat");

    QProcess *proc;
    proc = new QProcess(this);
    if(fileinfo.suffix()=="co2x") // extended project file suitable for sequensing
        proc->start("\"" + path_to_7zip + "\" a -tzip \"" + project_file + "\" *.dat ; *.txt ; *.ini ; *.yml; *.bin");
    else // basic (small) project file (.co2)
        proc->start("\"" + path_to_7zip + "\" a -tzip \"" + project_file + "\" *.dat ; *.txt ; *.ini ; *.yml"); // don't include field (field.bin)
    proc->waitForFinished();
    delete proc;
    flag_results_modified = false;
    flag_plot_modified = false;
    UpdateControls();
}


void MainWindow::LoadProject()
{
    if(project_file != QString()){
        this->setCursor(Qt::WaitCursor);
        ClearWorkDir();
        QProcess *proc;
        proc = new QProcess(this);
        proc->start("\"" + path_to_7zip + "\" e -y \"" + project_file + "\"");
        proc->waitForFinished();
        delete proc;
        LoadSettings("project.ini");
        QFileInfo fileinfo(project_file);
        def_dir = QDir::toNativeSeparators(fileinfo.dir().absolutePath());

        MainWindow::setWindowTitle(fileinfo.fileName() + " - co2amp");
        flag_projectloaded = true;
        flag_calculating = false;
        flag_calculation_success = true;
        flag_results_modified = false;
        flag_plot_modified = false;
        flag_plot_postponed = false;
        if(fileinfo.suffix()=="co2x" && !flag_input_file_error)
            flag_field_ready_to_save = true;
        UpdateControls();
        if(tabWidget_main->currentIndex() == 2) //output tab
            Plot();
        else
            flag_plot_postponed = true;
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

    str.replace(QRegularExpression("\\r\\n"), "\n"); // Windows "\r\n" endline -> "\n"

    strlist = str.split("\r");

    int number_of_returns = strlist.size() - 1;

    for (int i=0; i<=number_of_returns; i++){
        textBrowser_terminal->insertPlainText(strlist[i]);
        textBrowser_terminal->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    }
    textBrowser_terminal->moveCursor(QTextCursor::End);
}


void MainWindow::BeforeProcessStarted()
{
    textBrowser_terminal->clear();
    flag_calculating = true;
    flag_calculation_success = true;
    flag_results_modified = false;
    ClearWorkDir();
    SaveSettings("all"); // save all settings - input and plot
    UpdateControls();
    tabWidget_main->setCurrentIndex(1); // Process tab
}


void MainWindow::AfterProcessFinished()
{
    delete process;
    bool save;
    flag_calculating = false;
    if(flag_calculation_success){
        flag_field_ready_to_save = true;
        save = checkBox_saveWhenFinished->isChecked();
        flag_plot_postponed = true;
        flag_results_modified = true;
        if(tabWidget_main->currentIndex()==2) // Output tab (Plot will be called)
            Plot();
        //tabWidget_main->setCurrentIndex(2); // Output tab (Plot will be called)
        if(save)
            SaveProject();
    }
    UpdateControls();
}


bool MainWindow::SaveBeforeClose() // return: TRUE if ok to close, FALSE otherwise
{
    QMessageBox::StandardButton mb;
    if(flag_calculating){ // co2amp-core running
         mb = QMessageBox::warning(this, "Calculation in progress - co2amp",
                                   "Cannot close: Calculation in progress.\nWait for calculation to complete or abort it.",
                                   QMessageBox::Ok);
         return false;
    }
    if(!flag_calculation_success) // cannot save if calculations not completed nothing to save
        return true;
    if(!flag_results_modified && !flag_plot_modified) // nothing changed
        return true;

    mb = QMessageBox::warning(this, "Project modified - co2amp", "The project has been modified.\nDo you want to save changes?",
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

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
    //if(!flag_projectloaded)
        //return;
    MemorizeSettings();  // Load values from edit boxes to memory
    UpdateControls();
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


void MainWindow::on_tabWidget_main_currentChanged(int tab)
{
    if(tab != 2)
        return;
    if(flag_plot_postponed)
        Plot();
}


void MainWindow::FlagModifiedAndPostponePlot()
{
    flag_plot_modified = true;
    flag_plot_postponed = true;
}



void MainWindow::PostponePlot()
{
    flag_plot_postponed = true;
}


void MainWindow::PlotIfPostponed()
{
    if(flag_plot_postponed)
        Plot();
}


void MainWindow::FlagModifiedAndPlot()
{
    flag_plot_modified = true;
    Plot();
}
