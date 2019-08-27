#include "co2amp.h"


void MainWindow::NewProject()
{
    ClearPlot();
    ClearWorkDir();
    project_file = QString();
    ReadConfigurationFiles(); //loads default values
    UpdateConfigurationFiles();
    MainWindow::setWindowTitle("untitled - co2amp");
    flag_project_modified    = false;
    flag_calculating         = false;
    flag_plot_postponed      = false;
    Update();
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

    if(!flag_project_modified) // nothing changed
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


void MainWindow::SaveProject()
{
    QFileInfo fileinfo(project_file);
    if(QFile::exists(project_file) && !QFile::remove(project_file)){// cannot remove project file (e.g. it's open in another program)
        QMessageBox().warning(this, "co2amp", "Cannot save to the existing file (file may be open in another program)");
        return;
    }

    UpdateConfigurationFiles();

    QProcess *proc;
    proc = new QProcess(this);
    proc->start("\"" + path_to_7zip + "\" a -tzip \"" + project_file + "\" *.dat ; *.txt ; *.ini ; *.yml");
    proc->waitForFinished();
    delete proc;
    flag_project_modified = false;
    Update();
}


void MainWindow::LoadProject()
{
    MainWindow::setWindowTitle("co2amp");

    if(project_file == QString())
        return;

    this->setCursor(Qt::WaitCursor);

    ClearPlot();
    ClearWorkDir();
    QProcess *proc;
    proc = new QProcess(this);
    proc->start("\"" + path_to_7zip + "\" e -y \"" + project_file + "\"");
    proc->waitForFinished();
    delete proc;
    ReadConfigurationFiles();
    QFileInfo fileinfo(project_file);
    def_dir = QDir::toNativeSeparators(fileinfo.dir().absolutePath());

    MainWindow::setWindowTitle(fileinfo.fileName() + " - co2amp");
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
    QDir dir;
    dir.setPath(work_dir);
    QStringList filelist = dir.entryList();
    int i;
    for(i=0; i<filelist.size(); i++)
        dir.remove(filelist[i]);
}


void MainWindow::InvalidateResults()
{
    ClearPlot();
    ClearWorkDir();

    flag_project_modified = true;

    UpdateConfigurationFiles();
}


bool MainWindow::CalcResultsExist()
{
    return QFile::exists("energy.dat");
}


bool MainWindow::OkToInvalidate()
{
    return QMessageBox::question(this, "co2amp", "This change will invalidate calculation results. "
                             "You will have to re-run calculations.\n"
                             "Proceed?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes;
}
