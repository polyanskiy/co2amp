#include "mainwindow.h"


void MainWindow::on_toolButton_component_add_clicked()
{
    QStringList selection_list = {"Active medium (AM)",
                                  "Probe (P)",
                                  "Mask (M)",
                                  "Lens (L)",
                                  "Window (W)",
                                  "Attenuator (AT)",
                                  "Bandpass filter (BF)",
                                  "Spectral filter (SF)",
                                  "Apodizing filter (AF)",
                                  "Air (A)"};
    QStringList type_list = {"AM", "P", "M", "L", "W", "AT", "BF", "SF", "AF", "A"};
    bool ok_pressed;

    QString type = QInputDialog().getItem(this, "co2amp", "Component type", selection_list, 0, false, &ok_pressed);
    if(!ok_pressed)
        return;  
    type = type_list[selection_list.indexOf(type)];

    QString id="";
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Component ID:", QLineEdit::Normal, SuggestComponentID(type), &ok_pressed);
        if(!ok_pressed)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide a component name");
            continue;
        }
        if(ComponentIDExists(id)){
            QMessageBox().warning(this, "co2amp", "Component name already exists");
            continue;
        }
        good_id_provided = true;
    }

    int current_component = listWidget_components->currentRow();

    Memorized.component_id.insert(current_component+1, id);
    Memorized.component_type.insert(current_component+1, type);
    Memorized.component_yaml.insert(current_component+1, "type: "+type);

    listWidget_components->insertItem(current_component+1, id+" ("+type+")");
    listWidget_components->setCurrentRow(current_component+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_component_up_clicked()
{
    int current_component = listWidget_components->currentRow();
    if(current_component == 0)
        return;

    Memorized.component_id.swap(current_component, current_component-1);
    Memorized.component_type.swap(current_component, current_component-1);
    Memorized.component_yaml.swap(current_component, current_component-1);

    QListWidgetItem *item = listWidget_components->takeItem(current_component);
    listWidget_components->insertItem(current_component-1, item);
    listWidget_components->setCurrentRow(current_component-1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_component_down_clicked()
{
    int current_component = listWidget_components->currentRow();
    if(current_component == listWidget_components->count()-1)
        return;

    Memorized.component_id.swap(current_component, current_component+1);
    Memorized.component_type.swap(current_component, current_component+1);
    Memorized.component_yaml.swap(current_component, current_component+1);

    QListWidgetItem *item = listWidget_components->takeItem(current_component);
    listWidget_components->insertItem(current_component+1, item);
    listWidget_components->setCurrentRow(current_component+1); //this also triggers UpdateControls();
}


void MainWindow::on_toolButton_component_rename_clicked()
{
    int current_component = listWidget_components->currentRow();

    QString oldid = Memorized.component_id[current_component];

    QString id="";
    bool ok_pressed;
    bool good_id_provided = false;
    while(!good_id_provided){
        id = QInputDialog().getText(this, "co2amp", "Component ID:", QLineEdit::Normal, oldid, &ok_pressed);
        if(!ok_pressed || id == oldid)
            return;
        id.replace( " ", "" );
        if(id == QString()){
            QMessageBox().warning(this, "co2amp", "Please provide a component name");
            continue;
        }
        if(ComponentIDExists(id)){
            QMessageBox().warning(this, "co2amp", "Component name already exists");
            continue;
        }
        good_id_provided = true;
    }

    Memorized.component_id[current_component] = id;

    listWidget_components->currentItem()->setText(id+" ("+Memorized.component_type[current_component]+")");
}


void MainWindow::on_toolButton_component_remove_clicked()
{
    if(listWidget_components->count() == 0)
        return;
    int current_component = listWidget_components->currentRow();

    if(QMessageBox::question(this, "co2amp", "Sure?") == QMessageBox::Yes){
        Memorized.component_id.removeAt(current_component);
        Memorized.component_type.removeAt(current_component);
        Memorized.component_yaml.removeAt(current_component);
        BlockSignals(true);
        //next line would pre-maturely trigger UpdateControls() - raw is changed before item deleted:
        delete listWidget_components->currentItem();
        UpdateControls();
    }

}


void MainWindow::on_listWidget_components_currentRowChanged(int)
{
    UpdateControls();
}


void MainWindow::on_plainTextEdit_component_textChanged()
{
    int current_component = listWidget_components->currentRow();
    Memorized.component_yaml[current_component] = plainTextEdit_component->toPlainText();
}


void MainWindow::on_pushButton_component_load_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_core);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/components/");
    }

    QString path = QFileDialog::getOpenFileName(this, QString(), yaml_dir, tr("Component specification file (*.yml)"));
    if(path == QString())
        return;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QFile::Text)){
        QTextStream yaml(&file);
        Memorized.component_yaml[listWidget_components->currentRow()] = yaml.readAll();
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
        UpdateControls();
    }
    else
        QMessageBox().warning(this, "co2amp", "File open error");

}


void MainWindow::on_pushButton_component_save_clicked()
{
    QFileInfo fileinfo;

    if(yaml_dir == QString()){
        fileinfo.setFile(path_to_core);
        yaml_dir = QDir::toNativeSeparators(fileinfo.dir().path() + "/library/components/");
    }

    QString path = QFileDialog::getSaveFileName(this, QString(), yaml_dir, tr("Component specification file (*.yml)"));
    if(path == QString())
        return;

    QFile file(path);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream yaml(&file);
        yaml << Memorized.component_yaml[listWidget_components->currentRow()];
        file.close();
        fileinfo.setFile(file);
        yaml_dir = fileinfo.dir().path();
    }
    else
        QMessageBox().warning(this, "co2amp", "File save error");
}


QString MainWindow::SuggestComponentID(QString type)
{
    int i = 1;
    while(ComponentIDExists(type + QString().setNum(i)))
        i++;
    return type + QString().setNum(i);
}


bool MainWindow::ComponentIDExists(QString id)
{
    int components_count = Memorized.component_id.size();

    if(components_count == 0)
        return false;

    for(int i=0; i<components_count; i++)
        if(Memorized.component_id[i] == id)
            return true;

    return false;
}

void MainWindow::PopulateComponentsList()
{
    BlockSignals(true); // don't call UpdateComponents() when components being removed/added
    listWidget_components->clear();

    int component_count = Memorized.component_id.size();
    if(component_count == 0)
        return;

    for(int i=0; i<component_count; i++)
        listWidget_components->addItem(Memorized.component_id[i]+" ("+Memorized.component_type[i]+")");

    listWidget_components->setCurrentRow(0);
}
