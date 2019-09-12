#include "co2i.h"


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

    if(m==1) // data
        CopyDataFromFile("energy_selected.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig1);
    if(m==3) // SVG
        SaveSVG("fig_energy.svg");
}


void MainWindow::on_svg_fig2_customContextMenuRequested() // Spectra
{
    int m = FigureMenu();

    if(m==1) //data
        CopyMultipassData("_spectrum.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig2);
    if(m==3) // SVG
        SaveSVG("fig_spectra.svg");
}


void MainWindow::on_svg_fig3_customContextMenuRequested() // Gain spectrum
{
    QString optic_id = comboBox_optic->currentText();
    int m = FigureMenu();   

    if(Type(optic_id)=="A"){
        if(m==1) //data
            CopyMultipassData("_gain.dat");
        if(m==2) // pixmap
            CopyPixmap(svg_fig3);
        if(m==3) // SVG
            SaveSVG("fig_gain.svg");
    }

    if(Type(optic_id)=="S"){
        if(m==1) //data
            CopyDataFromFile(optic_id + "_transmittance.dat");
        if(m==2) // pixmap
            CopyPixmap(svg_fig3);
        if(m==3) // SVG
            SaveSVG("fig_transmittance.svg");
    }
}


void MainWindow::on_svg_fig4_customContextMenuRequested() // Fluence
{
    int m = FigureMenu();

    if(m==1) //data
        CopyMultipassData("_fluence.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig4);
    if(m==3) // SVG
        SaveSVG("fig_fluence.svg");

}


void MainWindow::on_svg_fig5_customContextMenuRequested() // Power
{
    int m = FigureMenu();

    if(m==1) //data
        CopyMultipassData("_power.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig5);
    if(m==3) // SVG
        SaveSVG("fig_power.svg");
}


void MainWindow::on_svg_fig6_customContextMenuRequested() // Discharge
{
    QString optic_id = comboBox_optic->currentText();
    int m = FigureMenu();

    if(Type(optic_id)=="A"){
        if(m==1) //data
            CopyDataFromFile(optic_id + "_discharge.dat");
        if(m==2) // pixmap
            CopyPixmap(svg_fig6);
        if(m==3) // SVG
            SaveSVG("fig_discharge.svg");
    }

    if(Type(optic_id)=="F"){
        if(m==1) //data
            CopyDataFromFile(optic_id + "_transmittance.dat");
        if(m==2) // pixmap
            CopyPixmap(svg_fig6);
        if(m==3) // SVG
            SaveSVG("fig_transmittance.svg");
    }
}


void MainWindow::on_svg_fig7_customContextMenuRequested() // Temperatures
{
    QString optic_id = comboBox_optic->currentText();
    int m = FigureMenu(); 

    if(m==1) //data
        CopyDataFromFile(optic_id + "_temperatures.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig7);
    if(m==3) // SVG
        SaveSVG("fig_temperatures.svg");
}


void MainWindow::on_svg_fig8_customContextMenuRequested() // e (# of quanta / molequle)
{
    QString optic_id = comboBox_optic->currentText();
    int m = FigureMenu();

    if(m==1) //data
        CopyDataFromFile(optic_id + "_e.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig8);
    if(m==3) // SVG
        SaveSVG("fig_e.svg");
}


void MainWindow::on_svg_fig9_customContextMenuRequested() // q
{
    QString optic_id = comboBox_optic->currentText();
    int m = FigureMenu();

    if(m==1) //data
        CopyDataFromFile(optic_id + "_q.dat");
    if(m==2) // pixmap
        CopyPixmap(svg_fig9);
    if(m==3) // SVG
        SaveSVG("fig_q.svg");
}


void MainWindow::CopyDataFromFile(QString filename)
{
    QString out = "";

    QFile file(filename);
    file.open(QFile::ReadOnly);

    QString line = file.readLine();
    while(line != QString()){
        if(line[0]!='#') // skip comments
            out += line;
        line = file.readLine();
    }

    file.close();

    QApplication::clipboard()->setText(out);

}


void MainWindow::CopyMultipassData(QString longext)
{
    //QString line;
    QStringList out;
    QRegExp separators("[ \t\n\r]");

    QString optic_id = comboBox_optic->currentText();
    QString pulse_id = comboBox_pulse->currentText();

    int plot_n = 0;

    for(int i=0; i<10; i++){

        int pass_n = PassNumber(i);
        if(pass_n == -1)
            continue;

        QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + longext;
        if(!QFile::exists(filename))
            continue;

        QFile file(filename);
        file.open(QFile::ReadOnly);

        QString line = file.readLine();

        if(plot_n==0){ // first plot: write first column (argument) and second columns (value for first plot)
            while(line != QString()){
                if(line[0]!='#') // skip comments
                    out.push_back(line.section(separators, 0, 1));
                line = file.readLine();
            }
        }

        else{ // write additional columns
            int line_n = 0;
            while(line != QString() && line_n<out.size()){
                if(line[0]!='#'){ // skip comments
                    out[line_n] += "\t" + line.section(separators, 1, 1);
                    line_n++;
                }
                line = file.readLine();
            }
        }

        file.close();

        plot_n ++;
    }

    QApplication::clipboard()->setText(out.join("\n"));
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

