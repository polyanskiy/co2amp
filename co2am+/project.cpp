#include "co2am+.h"


void MainWindow::NewProject()
{
    ClearPlot();
    ClearWorkDir();
    textBrowser_terminal->clear();
    project_file = QString();
    ReadConfigurationFiles(); //loads default values
    UpdateConfigurationFiles();
    MainWindow::setWindowTitle("untitled - co2am+");
    flag_project_modified    = false;
    flag_calculating         = false;
    flag_plot_postponed      = false;
    Update();
}


bool MainWindow::SaveBeforeClose() // return: TRUE if ok to close, FALSE otherwise
{
    QMessageBox::StandardButton mb;
    if(flag_calculating) // co2amp running
    {
         mb = QMessageBox::warning(this, "Calculation in progress - co2am+",
                                   "Cannot close: Calculation in progress.\nWait for calculation to complete or abort it.",
                                   QMessageBox::Ok);
         return false;
    }

    if(!flag_project_modified) // nothing changed
        return true;

    mb = QMessageBox::warning(this, "Project modified - co2am+", "The project has been modified.\nDo you want to save changes?",
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (mb == QMessageBox::Save)
    {
        if(project_file != QString())
        {
            SaveProject();
            return true;
        }
        else
        {
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


void MainWindow::SaveProject()
{
    QFileInfo fileinfo(project_file);
    if(QFile::exists(project_file) && !QFile::remove(project_file)) // cannot remove project file (e.g. it's open in another program)
    {
        QMessageBox().warning(this, "co2am+", "Cannot save to the existing file (file may be open in another program)");
        return;
    }

    UpdateConfigurationFiles();

    QStringList arguments;
    arguments << "a";
    arguments << "-tzip";
    arguments << project_file;
    arguments << "*.dat";
    arguments << "*.txt";
    arguments << "*.ini";
    arguments << "*.yml";

    QProcess *proc;
    proc = new QProcess(this);
    proc->start(path_to_7zip, arguments);
    proc->waitForFinished(-1); // no time-out
    delete proc;

    flag_project_modified = false;
    Update();
}


void MainWindow::LoadProject()
{
    MainWindow::setWindowTitle("co2am+");

    if(project_file == QString())
        return;

    this->setCursor(Qt::WaitCursor);

    ClearPlot();
    ClearWorkDir();
    textBrowser_terminal->clear();

    QStringList arguments;
    arguments << "e";
    arguments << "-y";
    arguments << project_file;

    QProcess *proc;
    proc = new QProcess(this);
    proc->start(path_to_7zip, arguments);
    proc->waitForFinished(-1); // no time out
    delete proc;

    ReadConfigurationFiles();
    QFileInfo fileinfo(project_file);
    def_dir = QDir::toNativeSeparators(fileinfo.dir().absolutePath());

    MainWindow::setWindowTitle(fileinfo.fileName() + " - co2am+");
    flag_project_modified    = false;
    flag_calculating         = false;
    flag_plot_postponed      = true;
    Update();
    if(tabWidget_main->currentIndex() == 2) //output tab
        Plot();

    this->setCursor(Qt::ArrowCursor);
}


void MainWindow::ClearWorkDir()
{
    // remove all files from work_dir
    QDir workDir(work_dir);
    QStringList filelist = workDir.entryList();
    int i;
    for(i=0; i<filelist.size(); i++)
        workDir.remove(filelist[i]);
}


void MainWindow::CopyHitranFilesToWorkDir()
{
    QDir workDir(work_dir);
    QDir hitranDir(QDir(QFileInfo(path_to_co2amp).absolutePath()).filePath("hitran_data"));
    QStringList filelist = hitranDir.entryList(QStringList() << "*.par", QDir::Files);
    int i;
    for(i=0; i<filelist.size(); i++)
        QFile::copy(hitranDir.absoluteFilePath(filelist[i]), workDir.absoluteFilePath(filelist[i]));
}


void MainWindow::InvalidateResults()
{
    ClearPlot();
    ClearWorkDir();
    textBrowser_terminal->clear();

    flag_project_modified = true;

    UpdateConfigurationFiles();
}


bool MainWindow::CalcResultsExist()
{
    return QFile::exists("energy.dat");
}


bool MainWindow::OkToInvalidate()
{
    if(always_ok_to_invalidate)
        return true;

    QMessageBox messageBox(this);
    messageBox.setWindowTitle("co2am+");
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText("This change will invalidate calculation results. "
                       "You will have to re-run calculations.\n");
    QPushButton *yes =
            messageBox.addButton(QMessageBox::Yes);
    QPushButton *no =
            messageBox.addButton(QMessageBox::No);
    QPushButton *dontAskAgain =
            messageBox.addButton(("Yes. Don't ask again."), QMessageBox::ActionRole);
    messageBox.setDefaultButton(no);

    messageBox.exec();

    if(messageBox.clickedButton() == yes)
        return true;
    if(messageBox.clickedButton() == no)
        return false;
    if(messageBox.clickedButton() == dontAskAgain) {
        always_ok_to_invalidate = true;
        return true;
    }
    return false;
}
