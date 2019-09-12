#include "co2i.h"


void MainWindow::on_toolButton_configFile_add_clicked()
{
    QStringList selection_list = {"PULSE",
                                  "LAYOUT",
                                  "OPTIC - Amplifier section (A)",
                                  "OPTIC - Probe (P)",
                                  "OPTIC - Lens (L)",
                                  "OPTIC - Matter (M)",
                                  "OPTIC - Spatial filter (F)",
                                  "OPTIC - Spectral filter (S)",
                                  "OPTIC - Chirper (C)"};
    QStringList type_list = {"PULSE", "LAYOUT", "A", "P", "L", "M", "F", "S", "C"};

    // allow only one layout
    for(int i=0; i<configFile_id.size(); i++){
        if(configFile_type[i] == "LAYOUT"){
            selection_list.removeOne("LAYOUT");
            type_list.removeOne("LAYOUT");
        }
    }

    bool ok_pressed;

    QString type = QInputDialog().getItem(this, "co2i", "Type",
                                          selection_list, 0, false, &ok_pressed);
    if(!ok_pressed)
        return;  
    type = type_list[selection_list.indexOf(type)];

    QString id="";
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2i", " ID",
                                    QLineEdit::Normal, SuggestConfigFileName(type), &ok_pressed);
        if(!ok_pressed)
            return;
        if(!id.contains(QRegExp("^[A-Za-z][A-Za-z0-9_]*$"))){
            QMessageBox().warning(this, "co2i", "Please provide a valid ID");
            continue;
        }
        if(ConfigFileNameExists(id)){
            QMessageBox().warning(this, "co2i", "ID already exists");
            continue;
        }
        good_id_provided = true;
    }

    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else
            return;
    }

    int current_optic = listWidget_configFile_list->currentRow();

    configFile_id.insert(current_optic+1, id);
    configFile_type.insert(current_optic+1, type);
    configFile_content.insert(current_optic+1, "# Configuration YAML file, type " + type + "\n\n");

    flag_project_modified = true;

    listWidget_configFile_list->insertItem(current_optic+1, id);
    listWidget_configFile_list->setCurrentRow(current_optic+1); // this triggers Update() via ...currentRowChanged()
}


void MainWindow::on_toolButton_configFile_up_clicked()
{
    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else
            return;
    }

    int current_optic = listWidget_configFile_list->currentRow();
    if(current_optic == 0)
        return;

    configFile_id.swap(current_optic, current_optic-1);
    configFile_type.swap(current_optic, current_optic-1);
    configFile_content.swap(current_optic, current_optic-1);

    flag_project_modified = true;

    QListWidgetItem *item = listWidget_configFile_list->takeItem(current_optic);

    listWidget_configFile_list->insertItem(current_optic-1, item);
    listWidget_configFile_list->setCurrentRow(current_optic-1); // this triggers Update() via ...currentRowChanged()
}


void MainWindow::on_toolButton_configFile_down_clicked()
{
    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else
            return;
    }

    int current_optic = listWidget_configFile_list->currentRow();
    if(current_optic == listWidget_configFile_list->count()-1)
        return;

    configFile_id.swap(current_optic, current_optic+1);
    configFile_type.swap(current_optic, current_optic+1);
    configFile_content.swap(current_optic, current_optic+1);

    flag_project_modified = true;

    QListWidgetItem *item = listWidget_configFile_list->takeItem(current_optic);

    listWidget_configFile_list->insertItem(current_optic+1, item);
    listWidget_configFile_list->setCurrentRow(current_optic+1); // this triggers Update() via ...currentRowChanged()
}


void MainWindow::on_toolButton_configFile_rename_clicked()
{
    int current_optic = listWidget_configFile_list->currentRow();

    QString oldid =configFile_id[current_optic];

    QString id="";
    bool ok_pressed;
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2i", "ID", QLineEdit::Normal, oldid, &ok_pressed);
        if(!ok_pressed || id == oldid)
            return;
        if(!id.contains(QRegExp("^[A-Za-z][A-Za-z0-9_]*$"))){
            QMessageBox().warning(this, "co2i", "Please provide a valid ID");
            continue;
        }
        if(ConfigFileNameExists(id)){
            QMessageBox().warning(this, "co2i", "ID already exists");
            continue;
        }
        good_id_provided = true;
    }  

    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else
            return;
    }

    configFile_id[current_optic] = id;

    flag_project_modified = true;

    listWidget_configFile_list->currentItem()->setText(id); // this DOESN'T trigger Update()

    Update();
}


void MainWindow::on_toolButton_configFile_remove_clicked()
{
    if(QMessageBox::question(this, "co2i", "Sure?") == QMessageBox::Yes){
        if(CalcResultsExist()){
            if(OkToInvalidate())
                InvalidateResults();
            else
                return;
        }

        int current_optic = listWidget_configFile_list->currentRow();
        configFile_id.removeAt(current_optic);
        configFile_type.removeAt(current_optic);
        configFile_content.removeAt(current_optic);

        flag_project_modified = true;

        delete listWidget_configFile_list->currentItem(); // this triggers Update() via ...currentRowChanged()
    }

}


void MainWindow::on_listWidget_configFile_list_currentRowChanged(int)
{
    Update();
}


void MainWindow::on_plainTextEdit_configFile_content_textChanged()
{
    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else{
            plainTextEdit_configFile_content->blockSignals(true);
            plainTextEdit_configFile_content->undo();
            plainTextEdit_configFile_content->blockSignals(false);
            return;
        }
    }
    int current_optic = listWidget_configFile_list->currentRow();
    configFile_content[current_optic] = plainTextEdit_configFile_content->toPlainText();

    flag_project_modified = true;
    Update();
}


void MainWindow::on_pushButton_configFile_load_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_co2amp);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/optics/");
    }

    QString path = QFileDialog::getOpenFileName(this, QString(), yaml_dir, "Configuration YAML file (*.yml)");
    if(path == QString())
        return;

    if(CalcResultsExist()){
        if(OkToInvalidate())
            InvalidateResults();
        else
            return;
    }

    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        QTextStream yaml(&file);
        configFile_content[listWidget_configFile_list->currentRow()] = yaml.readAll();
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();

        flag_project_modified = true;
        Update();
    }
    else
        QMessageBox().warning(this, "co2i", "File open error");

}


void MainWindow::on_pushButton_configFile_save_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_co2amp);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/optics/");
    }

    QString path = QFileDialog::getSaveFileName(this, QString(), yaml_dir, "Configuration YAML file (*.yml)");
    if(path == QString())
        return;

    QFile file(path);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream yaml(&file);
        yaml << configFile_content[listWidget_configFile_list->currentRow()];
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
    }
    else
        QMessageBox().warning(this, "co2i", "File save error");
}


QString MainWindow::SuggestConfigFileName(QString type)
{
    if(type == "LAYOUT")
        return "LAYOUT"; //only one layout permitted

    int i = 1;
    while(ConfigFileNameExists(type + QString().setNum(i)))
        i++;
    return type + QString().setNum(i);
}


bool MainWindow::ConfigFileNameExists(QString id)
{
    for(int i=0; i<configFile_id.size(); i++)
        if(configFile_id[i] == id)
            return true;

    return false;
}


void MainWindow::PopulateConfigFileList()
{
    listWidget_configFile_list->blockSignals(true);

    listWidget_configFile_list->clear();
    for(int i=0; i<configFile_id.size(); i++)
        listWidget_configFile_list->addItem(configFile_id[i]);
    listWidget_configFile_list->setCurrentRow(0);

    listWidget_configFile_list->blockSignals(false);
}
