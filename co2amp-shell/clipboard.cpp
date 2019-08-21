#include "mainwindow.h"


int MainWindow::FigureMenu()
{
    QMenu menu("");
    menu.addAction("Copy raw data");
    menu.addAction("Copy pixmap");
    menu.addAction("Save SVG");
    QAction *action = menu.exec(QCursor::pos());
    if(action){
        if(action->text()=="Copy raw data")
            return 1;
        if(action->text()=="Copy pixmap")
            return 2;
        if(action->text()=="Save SVG")
            return 3;
    }
    return 0;
}


/////////////////////////////// COPY FIGURE OR DATA /////////////////////////////////////


void MainWindow::on_svg_fig1_customContextMenuRequested() // Energy
{
    int m = FigureMenu();
    if(m==1){ // data
        QString line, out;
        QRegExp separators("[\t\n]");
        QFile file;
        file.setFileName("data_energy_selected.dat");
        file.open(QFile::ReadOnly);

        line = file.readLine();
        while(line != QString()){
            out += line.section(separators, 0, 1) + "\n";
            line = file.readLine();
        }
        QApplication::clipboard()->setText(out);
        file.close();
    }
    if(m==2) // pixmap
        CopyPixmap(svg_fig1);
    if(m==3) // SVG
        SaveSVG("fig_energy.svg");
}


void MainWindow::on_svg_fig2_customContextMenuRequested() // Spectra
{
    int m = FigureMenu();
    if(m==1) //data
        CopyMultipassData("data_spectra.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig2);
    if(m==3) // SVG
        SaveSVG("fig_spectra.svg");
}


void MainWindow::on_svg_fig3_customContextMenuRequested() // Gain spectrum
{
    int m = FigureMenu();
    if(m==1) //data
        CopyMultipassData("data_gain.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig3);
    if(m==3) // SVG
        SaveSVG("fig_gain.svg");
}


void MainWindow::on_svg_fig4_customContextMenuRequested() // Fluence
{
    int m = FigureMenu();
    if(m==1) //data
        CopyMultipassData("data_fluence.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig4);
    if(m==3) // SVG
        SaveSVG("fig_fluence.svg");

}


void MainWindow::on_svg_fig5_customContextMenuRequested() // Power
{
    int m = FigureMenu();
    if(m==1) //data
        CopyMultipassData("data_power.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig5);
    if(m==3) // SVG
        SaveSVG("fig_power.svg");
}


void MainWindow::on_svg_fig6_customContextMenuRequested() // Discharge
{
    int m = FigureMenu();
    if(m==1){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        QFile file("data_discharge.dat");
        file.open(QFile::ReadOnly);
        line = file.readLine();
        while(line != QString()){
            if(line[0]!='#') // skip comments
                out += line.section(separators, 0, 2) + "\n";
            line = file.readLine();
        }
        QApplication::clipboard()->setText(out);
        file.close();
    }
    if(m==2) // pixmap
        CopyPixmap(svg_fig6);
    if(m==3) // SVG
        SaveSVG("fig_discharge.svg");
}


void MainWindow::on_svg_fig7_customContextMenuRequested() // Temperatures
{
    int m = FigureMenu();
    if(m==1){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        int comp_n = comboBox_optic->currentIndex();
        int am_n = AmNumber(comp_n);
        QFile file("data_temperatures.dat");
        file.open(QFile::ReadOnly);

        line = file.readLine();
        while(line != QString()){
            if(line[0]!='#') // skip comments
                out += line.section(separators, 0, 0) + "\t" + line.section(separators, 4*am_n+1, 4*am_n+4) + "\n";
            line = file.readLine();
        }
        QApplication::clipboard()->setText(out);
        file.close();
    }
    if(m==2) // pixmap
        CopyPixmap(svg_fig7);
    if(m==3) // SVG
        SaveSVG("fig_temperatures.svg");
}


void MainWindow::on_svg_fig8_customContextMenuRequested() // e (# of quanta / molequle)
{
    int m = FigureMenu();
    if(m==1){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        int comp_n = comboBox_optic->currentIndex();
        int am_n = AmNumber(comp_n);
        QFile file("data_e.dat");
        file.open(QFile::ReadOnly);

        line = file.readLine();
        while(line != QString()){
            if(line[0]!='#') // skip comments
                out += line.section(separators, 0, 0) + "\t" + line.section(separators, 4*am_n+1, 4*am_n+4) + "\n";
            line = file.readLine();
        }
        QApplication::clipboard()->setText(out);
        file.close();
    }
    if(m==2) // pixmap
        CopyPixmap(svg_fig8);
    if(m==3) // SVG
        SaveSVG("fig_e.svg");
}


void MainWindow::on_svg_fig9_customContextMenuRequested() // q
{
    int m = FigureMenu();
    if(m==1){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        QFile file("data_q.dat");
        file.open(QFile::ReadOnly);

        line = file.readLine();
        while(line != QString()){
            if(line[0]!='#') // skip comments
            out += line.section(separators, 0, 4) + "\n";
            line = file.readLine();
        }
        QApplication::clipboard()->setText(out);
        file.close();
    }
    if(m==2) // pixmap
        CopyPixmap(svg_fig9);
    if(m==3) // SVG
        SaveSVG("fig_q.svg");
}



void MainWindow::CopyMultipassData(QString filename)
{
    QString line;
    QString out = "";
    QStringList x, y[10];
    QRegExp separators("[ \t\n\r]");
    QFile file(filename);
    file.open(QFile::ReadOnly);
    int pass_n;
    int optic_n = comboBox_optic->currentIndex();
    int pulse_n = comboBox_pulse->currentIndex();
    bool flag_xfilled = false;

    for(int i=0; i<10; i++){
        pass_n = PassNumber(i);
        if(pass_n==-1)
            continue;
        file.seek(0);
        line = file.readLine();
        while(!file.atEnd()){
            if(line.section(separators, 0, 0) == "#pulse"
               && line.section(separators, 1, 1).toInt() == pulse_n
               && line.section(separators, 3, 3).toInt() == optic_n
               && line.section(separators, 5, 5).toInt() == pass_n){
                line = file.readLine();
                while(line!=QString() && line[0]!='#' && line[0]!='\n'){
                    if(!flag_xfilled)
                        x << line.section(separators,0,0); // copy x data only once
                    y[i] << line.section(separators,1,1);
                    line = file.readLine();
                }
                flag_xfilled = true;
            }
            line = file.readLine();
        }

    }

    file.close();

    for(int j=0; j<x.count(); j++){
        out += x[j];
        for(int i=0; i<10; i++){
            if(y[i].count() > j) // extra check - to avoid program crash
            out += "\t" + y[i][j];
        }
        out += "\n";
    }

    QApplication::clipboard()->setText(out);
}


void MainWindow::CopyPixmap(QSvgWidget *svg)
{
    QPixmap pixmap(svg->size());
    svg->render(&pixmap);
    QApplication::clipboard()->setPixmap(pixmap);
}


void MainWindow::SaveSVG(QString svg_path)
{
    QString start_dir, save_path;
    project_file != QString() ? start_dir = project_file : start_dir = def_dir;
    save_path = QFileDialog::getSaveFileName(this, QString(), def_dir, "SVG (*.svg)");
    if(save_path != QString()){
        QFile::remove(save_path);
        QFile::copy(svg_path, save_path);
    }
}

