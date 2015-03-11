#include "mainwindow.h"


int MainWindow::FigureMenu()
{
    QMenu menu("");
    menu.addAction("Copy image");
    menu.addAction("Copy data");
    QAction *action = menu.exec(QCursor::pos());
    if(action){
	if(action->text()=="Copy image")
	    return 1;
	if(action->text()=="Copy data")
	    return 2;
    }
    return 0;
}


/////////////////////////////// COPY FIGURE OR DATA /////////////////////////////////////


void MainWindow::on_label_fig1_customContextMenuRequested() // Energy
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig1->pixmap());
    if(m==2){ //data
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
}


void MainWindow::on_label_fig2_customContextMenuRequested() // Spectra
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig2->pixmap());
    if(m==2) //data
        CopyMultipassData("data_spectra.dat");
}


void MainWindow::on_label_fig3_customContextMenuRequested() // Gain spectrum
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig3->pixmap());
    if(m==2) //data
        CopyMultipassData("data_band.dat");
}


void MainWindow::on_label_fig4_customContextMenuRequested() // Fluence
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig4->pixmap());
    if(m==2) //data
        CopyMultipassData("data_fluence.dat");
}


void MainWindow::on_label_fig5_customContextMenuRequested() // Power
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig5->pixmap());
    if(m==2) //data
        CopyMultipassData("data_power.dat");
}


void MainWindow::on_label_fig6_customContextMenuRequested() // Discharge
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig6->pixmap());
    if(m==2){ //data
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
}


void MainWindow::on_label_fig7_customContextMenuRequested() // Temperatures
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig7->pixmap());
    if(m==2){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        int comp_n = comboBox_component->currentIndex();
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
}


void MainWindow::on_label_fig8_customContextMenuRequested() // e (# of quanta / molequle)
{
    int m = FigureMenu();
    if(m==1) //image
        QApplication::clipboard()->setPixmap(*label_fig8->pixmap());
    if(m==2){ //data
        QString line, out;
        QRegExp separators("[\t\n]");
        int comp_n = comboBox_component->currentIndex();
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
}


void MainWindow::on_label_fig9_customContextMenuRequested() // q
{
    int m = FigureMenu();
    if(m==1) //image
	QApplication::clipboard()->setPixmap(*label_fig9->pixmap());
    if(m==2){ //data
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
}



void MainWindow::CopyMultipassData(QString filename)
{
    QString line, out;
    QStringList x, y[10];
    QRegExp separators("[ \t\n\r]");
    QFile file(filename);
    file.open(QFile::ReadOnly);
    int pass_n;
    int comp_n = comboBox_component->currentIndex();
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
               && line.section(separators, 3, 3).toInt() == comp_n
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

