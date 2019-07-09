#include "mainwindow.h"


void MainWindow::on_toolButton_optic_add_clicked()
{
    QStringList selection_list = {"Amplifier section (A)",
                                  "Probe (P)",
                                  "Lens (L)",
                                  "Matter (M)",
                                  "Spatial filter (ND)",
                                  "Spectral filter (SF)",
                                  "Chirp (C)"};
    QStringList type_list = {"A", "P", "L", "M", "ND", "SF", "C"};
    bool ok_pressed;

    QString type = QInputDialog().getItem(this, "co2amp", "Optic type", selection_list, 0, false, &ok_pressed);
    if(!ok_pressed)
        return;  
    type = type_list[selection_list.indexOf(type)];

    QString id="";
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Optic ID:", QLineEdit::Normal, SuggestOpticID(type), &ok_pressed);
        if(!ok_pressed)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide an optic name");
            continue;
        }
        if(OpticIDExists(id)){
            QMessageBox().warning(this, "co2amp", "Optic name already exists");
            continue;
        }
        good_id_provided = true;
    }

    int current_optic = listWidget_optics->currentRow();

    Memorized.optic_id.insert(current_optic+1, id);
    Memorized.optic_type.insert(current_optic+1, type);
    Memorized.optic_yaml.insert(current_optic+1, "type: "+type);

    listWidget_optics->insertItem(current_optic+1, id+" ("+type+")");
    listWidget_optics->setCurrentRow(current_optic+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_optic_up_clicked()
{
    int current_optic = listWidget_optics->currentRow();
    if(current_optic == 0)
        return;

    Memorized.optic_id.swap(current_optic, current_optic-1);
    Memorized.optic_type.swap(current_optic, current_optic-1);
    Memorized.optic_yaml.swap(current_optic, current_optic-1);

    QListWidgetItem *item = listWidget_optics->takeItem(current_optic);
    listWidget_optics->insertItem(current_optic-1, item);
    listWidget_optics->setCurrentRow(current_optic-1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_optic_down_clicked()
{
    int current_optic = listWidget_optics->currentRow();
    if(current_optic == listWidget_optics->count()-1)
        return;

    Memorized.optic_id.swap(current_optic, current_optic+1);
    Memorized.optic_type.swap(current_optic, current_optic+1);
    Memorized.optic_yaml.swap(current_optic, current_optic+1);

    QListWidgetItem *item = listWidget_optics->takeItem(current_optic);
    listWidget_optics->insertItem(current_optic+1, item);
    listWidget_optics->setCurrentRow(current_optic+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_optic_rename_clicked()
{
    int current_optic = listWidget_optics->currentRow();

    QString oldid = Memorized.optic_id[current_optic];

    QString id="";
    bool ok_pressed;
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Optic ID:", QLineEdit::Normal, oldid, &ok_pressed);
        if(!ok_pressed || id == oldid)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide an optic name");
            continue;
        }
        if(OpticIDExists(id)){
            QMessageBox().warning(this, "co2amp", "Optic name already exists");
            continue;
        }
        good_id_provided = true;
    }

    Memorized.optic_id[current_optic] = id;

    listWidget_optics->currentItem()->setText(id+" ("+Memorized.optic_type[current_optic]+")");
}


void MainWindow::on_toolButton_optic_remove_clicked()
{
    if(listWidget_optics->count() == 0)
        return;
    int current_optic = listWidget_optics->currentRow();

    if(QMessageBox::question(this, "co2amp", "Sure?") == QMessageBox::Yes){
        Memorized.optic_id.removeAt(current_optic);
        Memorized.optic_type.removeAt(current_optic);
        Memorized.optic_yaml.removeAt(current_optic);
        BlockSignals(true);
        //next line would pre-maturely trigger UpdateControls() - raw is changed before item deleted:
        delete listWidget_optics->currentItem();
        UpdateControls();
    }

}


void MainWindow::on_listWidget_optics_currentRowChanged(int)
{
    UpdateControls();
}


void MainWindow::on_plainTextEdit_optic_textChanged()
{
    int current_optic = listWidget_optics->currentRow();
    Memorized.optic_yaml[current_optic] = plainTextEdit_optic->toPlainText();
}


void MainWindow::on_pushButton_optic_load_clicked()
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
        Memorized.optic_yaml[listWidget_optics->currentRow()] = yaml.readAll();
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
        UpdateControls();
    }
    else
        QMessageBox().warning(this, "co2amp", "File open error");

}


void MainWindow::on_pushButton_optic_save_clicked()
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
        yaml << Memorized.optic_yaml[listWidget_optics->currentRow()];
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
    }
    else
        QMessageBox().warning(this, "co2amp", "File save error");
}


QString MainWindow::SuggestOpticID(QString type)
{
    int i = 1;
    while(OpticIDExists(type + QString().setNum(i)))
        i++;
    return type + QString().setNum(i);
}


bool MainWindow::OpticIDExists(QString id)
{
    int optics_count = Memorized.optic_id.size();

    if(optics_count == 0)
        return false;

    for(int i=0; i<optics_count; i++)
        if(Memorized.optic_id[i] == id)
            return true;

    return false;
}

void MainWindow::PopulateOpticsList()
{
    BlockSignals(true); // don't call UpdateOptics() when optics being removed/added
    listWidget_optics->clear();

    int optic_count = Memorized.optic_id.size();
    if(optic_count == 0)
        return;

    for(int i=0; i<optic_count; i++)
        listWidget_optics->addItem(Memorized.optic_id[i]+" ("+Memorized.optic_type[i]+")");

    listWidget_optics->setCurrentRow(0);
}
