#include "co2amp.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
    setupUi(this);

    FindExternalPrograms();

    if(path_to_core=="" || path_to_7zip=="" || path_to_gnuplot=="")
        return;

    /////////////////////////// Flags //////////////////////////
    flag_project_modified    = false;
    flag_calculating         = false;
    flag_plot_postponed      = false;
    always_ok_to_invalidate  = false;

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

    //////////////////////////// Load settings not stored in .co2 files ////////////////////////
    QSettings settings("ATF", "co2amp");
    def_dir             =                 settings.value("def_dir",       "").toString();
    yaml_dir            =                 settings.value("yaml_dir",      "").toString();
    spinBox_debug_level-> setValue       (settings.value("debug_level",    0).toInt ());
    comboBox_size      -> setCurrentIndex(settings.value("plot_size",      0).toInt());
    spinBox_width      -> setValue       (settings.value("plot_width",  1600).toInt());
    spinBox_height     -> setValue       (settings.value("plot_height", 1200).toInt());
    doubleSpinBox_zoom -> setValue       (settings.value("plot_zoom",    1.0).toFloat());
    checkBox_grid      -> setChecked     (settings.value("plot_grid",      1).toBool());
    checkBox_labels    -> setChecked     (settings.value("plot_labels",    1).toBool());
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

    /////////////////////////////// Create program window /////////////////////////////////////
    restoreGeometry(settings.value("window_geometry").toByteArray());
    SetAboutText();
    show();

    // Initialize YAML highlighter
    highlighter = new YamlHighlighter(plainTextEdit_configFile_content->document());

    /////////////////////////////// Process command line //////////////////////////////////////
    QStringList arg = qApp->arguments();
    if( arg.count()>1 && QFile::exists(arg.last()) ){
        project_file = arg.last();
        LoadProject();
    }
    else
        NewProject();
}

MainWindow::~MainWindow()
{
    if(flag_calculating)
        process->kill();

    // Save session
    QSettings settings("ATF", "co2amp");
    settings.setValue("def_dir",     def_dir);
    settings.setValue("yaml_dir",    yaml_dir);
    settings.setValue("debug_level", spinBox_debug_level->text());
    settings.setValue("plot_size",   comboBox_size      -> currentIndex());
    settings.setValue("plot_width",  spinBox_width      -> value());
    settings.setValue("plot_height", spinBox_height     -> value());
    settings.setValue("plot_zoom",   doubleSpinBox_zoom -> value());
    settings.setValue("plot_grid",   checkBox_grid      -> isChecked());
    settings.setValue("plot_labels", checkBox_labels    -> isChecked());
    settings.setValue("window_geometry", saveGeometry());

    // Remove temporary working directory
    ClearWorkDir();
    QDir dir;
    QDir::setCurrent(QDir::homePath()); // cannot remove current dir under Windows
    dir.rmpath(work_dir);
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
            str = QFileDialog::getOpenFileName(this, QString(), def_dir, "co2amp project (*.co2)");
        else
            str = QFileDialog::getOpenFileName(this, QString(), project_file, "co2amp project (*.co2)");
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

    if(project_file == QString())
        str = QFileDialog::getSaveFileName(this, QString(), def_dir, "co2amp project (*.co2)");
    else
        str = QFileDialog::getSaveFileName(this, QString(), project_file, "co2amp project (*.co2)");

    if(str == QString())
        return;

    fileinfo.setFile(str);
    QDir dir = fileinfo.dir();
    if(fileinfo.suffix()!="co2"){
        QMessageBox().warning(this, "co2amp", "Wrong or missing extension (must be \'.co2\')");
        return;
    }
    project_file = QDir::toNativeSeparators(dir.absolutePath() + "/" + fileinfo.fileName());
    def_dir = QDir::toNativeSeparators(dir.absolutePath());
    SaveProject();
    fileinfo.setFile(project_file);
    MainWindow::setWindowTitle(fileinfo.fileName() + " - co2amp");
}


void MainWindow::on_pushButton_savePulse_clicked()
{
    QStringList pulse_list;
    QString pulse;

    for(int i=0; i<configFile_id.count(); i++){
        if(configFile_type[i] == "PULSE")
            pulse_list.append(configFile_id[i]);
    }

    if(pulse_list.count() > 1){
        bool ok_pressed;
        pulse = QInputDialog().getItem(this, "co2amp", "Chose pulse to save",
                                              pulse_list, 0, false, &ok_pressed);
        if(!ok_pressed)
            return;
    }
    else
        pulse = pulse_list[0];

    QString save_path = QDir::toNativeSeparators(def_dir + "\\" + pulse + ".pulse");
    save_path = QFileDialog::getSaveFileName(this, QString(), save_path, "HDF5 (*.pulse)");
    if(save_path != QString()){
        QFile::remove(save_path);
        QFile::copy(pulse+".pulse", save_path);
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if(SaveBeforeClose())
        event->accept();
    else
        event->ignore();
}


void MainWindow::on_tabWidget_main_currentChanged(int tab)
{
    if(tab != 2)
        return;
    if(flag_plot_postponed)
        Plot();
}
