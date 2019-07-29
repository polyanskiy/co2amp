#include "mainwindow.h"


void MainWindow::on_toolButton_configFile_add_clicked()
{
    QStringList selection_list = {"PULSE",
                                  "Amplifier section (A)",
                                  "Probe (P)",
                                  "Lens (L)",
                                  "Matter (M)",
                                  "Spatial filter (ND)",
                                  "Spectral filter (SF)",
                                  "Chirp (C)",
                                  "LAYOUT",
                                  "COMMENT"};
    QStringList type_list = {"PULSE", "A", "P", "L", "M", "ND", "SF", "C", "LAYOUT", "COMMENT"};
    bool ok_pressed;

    QString type = QInputDialog().getItem(this, "co2amp", "Type of configuration file", selection_list, 0, false, &ok_pressed);
    if(!ok_pressed)
        return;  
    type = type_list[selection_list.indexOf(type)];

    QString id="";
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Base name of the file (element ID)", QLineEdit::Normal, SuggestConfigFileName(type), &ok_pressed);
        if(!ok_pressed)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide a unique name");
            continue;
        }
        if(ConfigFileNameExists(id)){
            QMessageBox().warning(this, "co2amp", "Name already exists");
            continue;
        }
        good_id_provided = true;
    }

    int current_optic = listWidget_configFile_list->currentRow();

    Memorized.configFile_basename.insert(current_optic+1, id);
    Memorized.configFile_type.insert(current_optic+1, type);
    Memorized.configFile_content.insert(current_optic+1, "type: "+type);

    listWidget_configFile_list->insertItem(current_optic+1, id+" ("+type+")");
    listWidget_configFile_list->setCurrentRow(current_optic+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_configFile_up_clicked()
{
    int current_optic = listWidget_configFile_list->currentRow();
    if(current_optic == 0)
        return;

    Memorized.configFile_basename.swap(current_optic, current_optic-1);
    Memorized.configFile_type.swap(current_optic, current_optic-1);
    Memorized.configFile_content.swap(current_optic, current_optic-1);

    QListWidgetItem *item = listWidget_configFile_list->takeItem(current_optic);
    listWidget_configFile_list->insertItem(current_optic-1, item);
    listWidget_configFile_list->setCurrentRow(current_optic-1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_configFile_down_clicked()
{
    int current_optic = listWidget_configFile_list->currentRow();
    if(current_optic == listWidget_configFile_list->count()-1)
        return;

    Memorized.configFile_basename.swap(current_optic, current_optic+1);
    Memorized.configFile_type.swap(current_optic, current_optic+1);
    Memorized.configFile_content.swap(current_optic, current_optic+1);

    QListWidgetItem *item = listWidget_configFile_list->takeItem(current_optic);
    listWidget_configFile_list->insertItem(current_optic+1, item);
    listWidget_configFile_list->setCurrentRow(current_optic+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_configFile_rename_clicked()
{
    int current_optic = listWidget_configFile_list->currentRow();

    QString oldid = Memorized.configFile_basename[current_optic];

    QString id="";
    bool ok_pressed;
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Base name:", QLineEdit::Normal, oldid, &ok_pressed);
        if(!ok_pressed || id == oldid)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide a unique name");
            continue;
        }
        if(ConfigFileNameExists(id)){
            QMessageBox().warning(this, "co2amp", "Name already exists");
            continue;
        }
        good_id_provided = true;
    }

    Memorized.configFile_basename[current_optic] = id;

    listWidget_configFile_list->currentItem()->setText(id+" ("+Memorized.configFile_type[current_optic]+")");
}


void MainWindow::on_toolButton_configFile_remove_clicked()
{
    if(listWidget_configFile_list->count() == 0)
        return;
    int current_optic = listWidget_configFile_list->currentRow();

    if(QMessageBox::question(this, "co2amp", "Sure?") == QMessageBox::Yes){
        Memorized.configFile_basename.removeAt(current_optic);
        Memorized.configFile_type.removeAt(current_optic);
        Memorized.configFile_content.removeAt(current_optic);
        BlockSignals(true);
        //next line would pre-maturely trigger UpdateControls() - raw is changed before item deleted:
        delete listWidget_configFile_list->currentItem();
        UpdateControls();
    }

}


void MainWindow::on_listWidget_configFile_list_currentRowChanged(int)
{
    UpdateControls();
}


void MainWindow::on_plainTextEdit_configFile_content_textChanged()
{
    int current_optic = listWidget_configFile_list->currentRow();
    Memorized.configFile_content[current_optic] = plainTextEdit_configFile_content->toPlainText();
}


void MainWindow::on_pushButton_configFile_load_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_core);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/optics/");
    }

    QString path = QFileDialog::getOpenFileName(this, QString(), yaml_dir, tr("Optic specification file (*.yml)"));
    if(path == QString())
        return;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        QTextStream yaml(&file);
        Memorized.configFile_content[listWidget_configFile_list->currentRow()] = yaml.readAll();
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
        UpdateControls();
    }
    else
        QMessageBox().warning(this, "co2amp", "File open error");

}


void MainWindow::on_pushButton_configFile_save_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_core);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/optics/");
    }

    QString path = QFileDialog::getSaveFileName(this, QString(), yaml_dir, tr("Optic specification file (*.yml)"));
    if(path == QString())
        return;

    QFile file(path);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream yaml(&file);
        yaml << Memorized.configFile_content[listWidget_configFile_list->currentRow()];
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
    }
    else
        QMessageBox().warning(this, "co2amp", "File save error");
}


QString MainWindow::SuggestConfigFileName(QString type)
{
    int i = 1;
    while(ConfigFileNameExists(type + QString().setNum(i)))
        i++;
    return type + QString().setNum(i);
}


bool MainWindow::ConfigFileNameExists(QString id)
{
    int optics_count = Memorized.configFile_basename.size();

    if(optics_count == 0)
        return false;

    for(int i=0; i<optics_count; i++)
        if(Memorized.configFile_basename[i] == id)
            return true;

    return false;
}

void MainWindow::PopulateConfigFileList()
{
    BlockSignals(true); // don't call UpdateFile() when optics being removed/added
    listWidget_configFile_list->clear();

    int optic_count = Memorized.configFile_basename.size();
    if(optic_count == 0)
        return;

    for(int i=0; i<optic_count; i++)
        listWidget_configFile_list->addItem(Memorized.configFile_basename[i]+" ("+Memorized.configFile_type[i]+")");

    listWidget_configFile_list->setCurrentRow(0);
}
