#include "mainwindow.h"



void MainWindow::PlotAndSetModifiedFlag()
{
    if(!flag_projectloaded || !flag_calculation_success)
        return;
    MemorizeSettings();
    flag_plot_modified = true; //set before Plot() to properly update settings
    Plot();
    SaveSettings("plot"); // save only plot settings
}

void MainWindow::Plot()
{
    if(!flag_projectloaded || !flag_calculation_success)
        return;

    if(tabWidget_main->currentIndex()!=1){
        flag_replot_needed = true;
        return;
    }

    this->setCursor(Qt::WaitCursor);

    int i;
    QProcess *proc;
    QFile file;
    QTextStream out(&file);
    QString newline, common_plot_param;
    #ifdef Q_OS_LINUX
        newline = "\n";
    #endif
    #ifdef Q_OS_WIN
        newline = "\r\n";
    #endif
    checkBox_labels->isChecked() ? common_plot_param = "set key" + newline : common_plot_param = "unset key" + newline;
    if(checkBox_grid->isChecked())
        common_plot_param += "set grid" + newline;
    int comp_n = comboBox_component->currentIndex();
    int am_n = AmNumber(comp_n);
    int pulse_n = comboBox_pulse->currentIndex();
    int pass_n, plot_n, set_n;

    ClearPlot();

    svg_fig1->setStyleSheet("background-color:white;");
    svg_fig2->setStyleSheet("background-color:white;");
    svg_fig3->setStyleSheet("background-color:white;");
    svg_fig4->setStyleSheet("background-color:white;");
    svg_fig5->setStyleSheet("background-color:white;");
    svg_fig6->setStyleSheet("background-color:white;");
    svg_fig7->setStyleSheet("background-color:white;");
    svg_fig8->setStyleSheet("background-color:white;");
    svg_fig9->setStyleSheet("background-color:white;");

    svg_fig1->setHidden(false);
    svg_fig2->setHidden(false);
    svg_fig3->setHidden(am_n == -1);
    svg_fig4->setHidden(false);
    svg_fig5->setHidden(false);
    svg_fig6->setHidden(am_n == -1);
    svg_fig7->setHidden(am_n == -1);
    svg_fig8->setHidden(am_n == -1);
    svg_fig9->setHidden(am_n == -1);

    QSize size(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    switch(comboBox_plotSize->currentIndex()){
    case 0:
        size = QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        break;
    case 1:
        size = QSize(360,270);
        break;
    case 2:
        size = QSize(480,360);
        break;
    case 3:
        size = QSize(640,480);
        break;
    case 4:
        size = QSize(800,600);
        break;
    case 5:
        size = QSize(1024,768);
        break;
    case 6:
        size = QSize(1280,900);
        break;
    case 7:
        size = QSize(1600,1200);
        break;
    case 8:
        size = QSize(1920,1080);
        break;
    }

    svg_fig1->setFixedSize(size);
    svg_fig2->setFixedSize(size);
    svg_fig3->setFixedSize(size);
    svg_fig4->setFixedSize(size);
    svg_fig5->setFixedSize(size);
    svg_fig6->setFixedSize(size);
    svg_fig7->setFixedSize(size);
    svg_fig8->setFixedSize(size);
    svg_fig9->setFixedSize(size);

    /////////////////////// wait for widgets to update sizes (IMPORTANT TO DO IT HERE!!!) ////////////////////////////
    QCoreApplication::processEvents(QEventLoop::AllEvents,1000);


    ///////////////////////////////////////////// Create GnuPlot script //////////////////////////////////////////
    file.setFileName("gnuplot_script");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << "set terminal svg size " << svg_fig1->width() << "," << svg_fig1->height() << " font 'arial," << spinBox_fontSize->text() << "'"  << newline  << newline;

    // GnuPlot script: Energy
    SelectEnergies();
    out << "reset" << newline;
    out << "set output \"fig_energy.svg\"" << newline;
    out << "set xlabel \"Time, µs\"" << newline;
    out << "set ylabel \"Pulse energy, J\"" << newline;
    checkBox_log->isChecked() ? out << "set logscale y" << newline : out << "set yrange [0:*]" << newline;
    out << common_plot_param;
    out << "plot \"data_energy_selected.dat\" using 1:2 notitle" << newline << newline;

    // GnuPlot script: Fluence
    plot_n = 0;
    out << "reset" << newline;
    out << "set output \"fig_fluence.svg\"" << newline;
    out << common_plot_param;
    out << "set xlabel \"r, cm\"" << newline;
    out << "set ylabel \"Fluence, mJ/cm^2\"" << newline;
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, comp_n, pass_n, "data_fluence.dat");
            if(set_n != -1){
                if(plot_n==0)
                    out << "plot ";
                else
                    out << ",\\" << newline;
                out << "\"data_fluence.dat\" index " << set_n << " with lines ti \"";
                if(plot_n==0)
                    out << "Passes: ";
                out << pass_n+1 << "\"";
                plot_n ++;
            }
    }
    out << newline << newline;;

    // GnuPlot script: Power
    plot_n = 0;
    out << "reset" << newline;
    out << "set output \"fig_power.svg\"" << newline;
    out << common_plot_param;
    out << "set xlabel \"Time, ps\"" << newline;
    double tmin = Saved.t_pulse_min.toDouble()/pow(2, comboBox_timeScale->currentIndex());
    double tmax = Saved.t_pulse_max.toDouble()/pow(2, comboBox_timeScale->currentIndex());
    out << "set xrange [" << tmin << ":" << tmax << "]" << newline;
    out << "set ylabel \"Power, GW\"" << newline;
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, comp_n, pass_n, "data_power.dat");
            if(set_n != -1){
                if(plot_n==0)
                    out << "plot ";
                else
                    out << ",\\" << newline;
                out << "\"data_power.dat\" index " << set_n << " with lines ti \"";
                if(plot_n==0)
                    out << "Passes: ";
                out << pass_n+1 << "\"";
                plot_n ++;
            }
    }
    out << newline << newline;

    // GnuPlot script: Spectra
    int n0 = comboBox_precision_t->currentText().toInt(); // number of points in the pulse time calculation net
    double Delta_t = (Saved.t_pulse_max.toDouble()-Saved.t_pulse_min.toDouble()) / (n0-1); // pulse time step
    double Delta_v = 1.0/(Delta_t*n0); // frequency step, THz
    double v_spread = Delta_v*(n0-1)/pow(2, comboBox_freqScale->currentIndex()+1);
    plot_n = 0;
    out << "reset" << newline;
    out << "set output \"fig_spectra.svg\"" << newline;
    out << common_plot_param;
    out << "set xlabel \"Frequency, THz\"" << newline;
    out << "set xrange [" << Saved.vc.toDouble()-v_spread << ":" << Saved.vc.toDouble()+v_spread << "]" << newline;
    out << "set ylabel \"Intensity, a.u.\"" << newline;
    out << "set yrange [0:*]" << newline;
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, comp_n, pass_n, "data_spectra.dat");
        if(set_n != -1){
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\" << newline;
            out << "\"data_spectra.dat\" index " << set_n << " with lines ti \"";
            if(plot_n==0)
                out << "Passes: ";
            out << pass_n+1 << "\"";
            plot_n ++;
        }
    }
    out << newline << newline;

    if(am_n != -1){
        // GnuPlot script: Temperatures
        out << "reset" << newline;
        out << "set output \"fig_temperatures.svg\"" << newline;
        out << "set xlabel \"Time, µs\"" << newline;
        out << "set ylabel \"Temperature, K\"" << newline;
        out << common_plot_param;
        out << "set yrange [0:*]" << newline;
        out << "plot \"data_temperatures.dat\" using 1:($" << 4*am_n+2 << ") with lines ti \"Lower: T2\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+3 << ") with lines ti \"Upper: T3\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+4 << ") with lines ti \"N2: T4\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+5 << ") with lines ti \"Transl:  T\"" << newline << newline;

        // GnuPlot script: e
        out << "reset" << newline;
        out << "set output \"fig_e.svg\"" << newline;
        out << "set xlabel \"Time, µs\"" << newline;
        out << "set ylabel \"e (# of quanta / molecule)\"" << newline;
        out << common_plot_param;
        out << "set yrange [0:*]" << newline;
        out << "plot \"data_e.dat\" using 1:($" << 4*am_n+2 << ") with lines ls 4 ti \"Lower 10 µm (symm stretch): e1\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+3 << ") with lines ls 1 ti \"Lower 9 µm (bend): e2\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+4 << ") with lines ls 2 ti \"Upper (asymm stretch): e3\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+5 << ") with lines ls 3 ti \"N2: e4\"" << newline << newline;

        // GnuPlot script: Gain spectrum
        plot_n = 0;
        out << "reset" << newline;
        out << "set output \"fig_band.svg\"" << newline;
        out << common_plot_param;
        out << "set xlabel \"Frequency, THz\"" << newline;
        out << "set xrange [" << Saved.vc.toDouble()-v_spread << ":" << Saved.vc.toDouble()+v_spread << "]" << newline;
        out << "set ylabel \"Gain, %/cm\"" << newline;
        for(i=0; i<10; i++){
            pass_n = PassNumber(i);
            set_n = DatasetNumber(pulse_n, comp_n, pass_n, "data_band.dat");
            if(set_n != -1){
                if(plot_n==0)
                    out << "plot ";
                else
                    out << ",\\" << newline;
                out << "\"data_band.dat\" index " << set_n << " with lines ti \"";
                if(plot_n==0)
                    out << "Passes: ";
                out << pass_n+1 << "\"";
                plot_n ++;
            }
        }
        out << newline << newline;

        // GnuPlot script: Discharge
        out << "reset" << newline;
        out << "set output \"fig_discharge.svg\"" << newline;
        out << "set y2range [0:*]" << newline;
        out << "set ytics nomirror" << newline;
        out << "set y2tics nomirror" << newline;
        out << "set xlabel \"Time, µs\"" << newline;
        out << "set ylabel \"Current, kA\"" << newline;
        out << "set y2label \"Voltage, kV\"" << newline;
        out << common_plot_param;
        out << "plot \"data_discharge.dat\" using 1:($2/1000.0) axis x1y1 with lines ti \"Current\",\\" << newline;
        out << "\"data_discharge.dat\" using 1:($3/1000.0) axis x1y2 with lines ti \"Voltage\"" << newline << newline;;

        // GnuPlot script: q
        out << "reset" << newline;
        out << "set output \"fig_q.svg\"" << newline;
        out << "set yrange [0:1]" << newline;
        out << "set xlabel \"Time, µs\"" << newline;
        out << "set ylabel \"q\"" << newline;
        out << common_plot_param;
        out << "plot \"data_q.dat\" using 1:2 with lines ti \"Lower: q2\",\\" << newline;
        out << "\"data_q.dat\" using 1:3 with lines ti \"Upper: q3\",\\" << newline;
        out << "\"data_q.dat\" using 1:4 with lines ti \"N2: q4\",\\" << newline;
        out << "\"data_q.dat\" using 1:5 with lines ti \"Transl: qT\"" << newline;
    }

    file.close();

    //////////////////////////////////////////////////////////// Make figures with GnuPlot ////////////////////////////////////////////
    QFile::remove("fig_energy.svg");
    QFile::remove("fig_power.svg");
    QFile::remove("fig_fluence.svg");
    QFile::remove("fig_spectra.svg");
    QFile::remove("fig_band.svg");
    QFile::remove("fig_discharge.svg");
    QFile::remove("fig_q.svg");
    QFile::remove("fig_temperatures.svg");
    QFile::remove("fig_e.svg");
    proc = new QProcess( this );
    proc->execute(path_to_gnuplot + " gnuplot_script");
    /*proc->execute(path_to_gnuplot + " script_energy.txt");
    proc->execute(path_to_gnuplot + " script_power.txt");
    proc->execute(path_to_gnuplot + " script_fluence.txt");
    proc->execute(path_to_gnuplot + " script_spectra.txt");
    if(am_n != -1){ // if active medium
        proc->execute(path_to_gnuplot + " script_band.txt");
        proc->execute(path_to_gnuplot + " script_discharge.txt");
        proc->execute(path_to_gnuplot + " script_q.txt");
        proc->execute(path_to_gnuplot + " script_temperatures.txt");
        proc->execute(path_to_gnuplot + " script_e.txt");
    }*/

    /////////////////////////////////////////////////////////////// Display figures ///////////////////////////////////////////////////
    svg_fig1->load(QString("fig_energy.svg"));
    svg_fig2->load(QString("fig_spectra.svg"));
    svg_fig4->load(QString("fig_fluence.svg"));
    svg_fig5->load(QString("fig_power.svg"));
    if(am_n != -1){ //active medium
        svg_fig3->load(QString("fig_band.svg"));
        svg_fig6->load(QString("fig_discharge.svg"));
        svg_fig7->load(QString("fig_temperatures.svg"));
        svg_fig8->load(QString("fig_e.svg"));
        svg_fig9->load(QString("fig_q.svg"));
    }

    //////////////////////////////////////////////////////////// update controls ////////////////////////////////////////////////
    OnModified();
    flag_replot_needed = false;
    this->setCursor(Qt::ArrowCursor);
}


void MainWindow::ClearPlot()
{
    QByteArray ba;
    ba = QByteArray();
    svg_fig1->load(ba);
    svg_fig2->load(ba);
    svg_fig3->load(ba);
    svg_fig4->load(ba);
    svg_fig5->load(ba);
    svg_fig6->load(ba);
    svg_fig7->load(ba);
    svg_fig8->load(ba);
    svg_fig9->load(ba);
    svg_fig1->setStyleSheet("background-color:transparent;");
    svg_fig2->setStyleSheet("background-color:transparent;");
    svg_fig3->setStyleSheet("background-color:transparent;");
    svg_fig4->setStyleSheet("background-color:transparent;");
    svg_fig5->setStyleSheet("background-color:transparent;");
    svg_fig6->setStyleSheet("background-color:transparent;");
    svg_fig7->setStyleSheet("background-color:transparent;");
    svg_fig8->setStyleSheet("background-color:transparent;");
    svg_fig9->setStyleSheet("background-color:transparent;");
}



void MainWindow::SelectEnergies()
{
    QString line;
    QRegExp separators("[\t\n]");
    int pulse_n = comboBox_pulse->currentIndex();
    int comp_n = comboBox_component->currentIndex();

    QFile file_all("data_energy.dat");
    file_all.open(QFile::ReadOnly);
    QFile file_sel("data_energy_selected.dat");
    QTextStream out(&file_sel);
    file_sel.open(QFile::WriteOnly);

    line = file_all.readLine();
    while(line != QString()){
	if(line[0]!='#'){ // skip comments
	    switch(comboBox_energyPlot->currentIndex()){
	    case 0: // all energies
		out << line.section(separators, 0, 1) << "\n";
		break;
	    case 1: // component
		if(line.section(separators, 3, 3).toInt() == comp_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
	    case 2: // pulse
		if(line.section(separators, 2, 2).toInt() == pulse_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
	    case 3: // comp + pulse
		if(line.section(separators, 2, 2).toInt() == pulse_n && line.section(separators, 3, 3).toInt() == comp_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
	    }
	}
	line = file_all.readLine();
    }

    file_sel.close();
    file_all.close();
}
