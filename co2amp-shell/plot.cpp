#include "mainwindow.h"


void MainWindow::Plot()
{
    if(!flag_projectloaded)
        return;

    MemorizeSettings();

    int i;
    QProcess *proc;
    QFile file;
    QTextStream out(&file);
    QString newline, font, lmargin;
    #ifdef Q_OS_X11
        newline = "\n";
        font = "small";
        lmargin = "12";
    #endif
    #ifdef Q_OS_WIN
        newline = "\r\n";
        font = "small";
        lmargin = "12";
    #endif

    ////////////////////////////////////////////////// Figure: Energy ////////////////////////////////////////////////
    SelectEnergies();
    file.setFileName("script_energy.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_energy.png\"" << newline;
    out << "set xlabel \"Time, us\"" << newline;
    out << "set ylabel \"Pulse energy, J\"" << newline;
    if(checkBox_grid->isChecked())
        out << "set grid" << newline;
    if(checkBox_log->isChecked())
        out << "set logscale y" << newline;
    else
        out << "set yrange [0:*]" << newline;
    out << "plot \"data_energy_selected.dat\" using 1:2 notitle" << newline;
    file.close();

    int pulse_n = comboBox_pulse->currentIndex();
    int comp_n = comboBox_component->currentIndex();
    int pass_n, plot_n, set_n, am_n;
    //////////////////////////////////////////////// Figure: Fluence ////////////////////////////////////////////////////
    plot_n = 0;
    file.setFileName("script_fluence.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_fluence.png\"" << newline;
    if(checkBox_grid->isChecked())
	out << "set grid" << newline;
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
    out << newline;
    file.close();

    //////////////////////////////////////////////// Figure: Power ////////////////////////////////////////////////////
    plot_n = 0;
    file.setFileName("script_power.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_power.png\"" << newline;
    if(checkBox_grid->isChecked())
        out << "set grid" << newline;
    out << "set xlabel \"Time, ps\"" << newline;
    float tmin = Saved.t_pulse_min.toFloat()/pow(2, comboBox_timeScale->currentIndex());
    float tmax = Saved.t_pulse_max.toFloat()/pow(2, comboBox_timeScale->currentIndex());
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
    out << newline;
    file.close();

    ////////////////////////////////////////////////// Figure: Spectra /////////////////////////////////////////////////
    int n0 = comboBox_precision_t->currentText().toInt(); // number of points in the pulse time calculation net
    float Delta_t = (Saved.t_pulse_max.toFloat()-Saved.t_pulse_min.toFloat()) / (n0-1); // pulse time step
    float Delta_v = 1.0/(Delta_t*n0); // frequency step, THz
    float v_spread = Delta_v*(n0-1)/pow(2, comboBox_freqScale->currentIndex()+1);

    plot_n = 0;
    file.setFileName("script_spectra.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_spectra.png\"" << newline;
    if(checkBox_grid->isChecked())
	out << "set grid" << newline;
    out << "set xlabel \"Frequency, THz\"" << newline;
    out << "set xrange [" << Saved.vc.toFloat()-v_spread << ":" << Saved.vc.toFloat()+v_spread << "]" << newline;
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
    out << newline;
    file.close();

    ////////////////////////////////////////////////////////// Figure: Temperatures //////////////////////////////////////////////////
    file.setFileName("script_temperatures.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    am_n = AmNumber(comp_n);
    if(am_n != -1){
        out << "set terminal png " << font << " size 360,270" << newline;
        out << "set output \"fig_temperatures.png\"" << newline;
        out << "set xlabel \"Time, us\"" << newline;
        out << "set ylabel \"Temperature, K\"" << newline;
        if(checkBox_grid->isChecked())
            out << "set grid" << newline;
        out << "set yrange [0:*]" << newline;
        out << "plot \"data_temperatures.dat\" using 1:($" << 4*am_n+2 << ") with lines ti \"Lower: T2\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+3 << ") with lines ti \"Upper: T3\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+4 << ") with lines ti \"N2: T4\",\\" << newline;
        out << "\"data_temperatures.dat\" using 1:($" << 4*am_n+5 << ") with lines ti \"Transl:  T\"" << newline;
    }
    file.close();

    ////////////////////////////////////////////////////////// Figure: e //////////////////////////////////////////////////
    file.setFileName("script_e.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    am_n = AmNumber(comp_n);
    if(am_n != -1){
        out << "set terminal png " << font << " size 360,270" << newline;
        out << "set output \"fig_e.png\"" << newline;
        out << "set xlabel \"Time, us\"" << newline;
        out << "set ylabel \"e (# of quanta / molecule)\"" << newline;
        if(checkBox_grid->isChecked())
            out << "set grid" << newline;
        out << "set yrange [0:*]" << newline;
        out << "plot \"data_e.dat\" using 1:($" << 4*am_n+2 << ") with lines ls 4 ti \"Lower 10 um (symm stretch): e1\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+3 << ") with lines ls 1 ti \"Lower 9 um (bend): e2\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+4 << ") with lines ls 2 ti \"Upper (asymm stretch): e3\",\\" << newline;
        out << "\"data_e.dat\" using 1:($" << 4*am_n+5 << ") with lines ls 3 ti \"N2: e4\"\\" << newline;
    }
    file.close();

    ///////////////////////////////////////////// Figure: Gain spectrum ////////////////////////////////////////////////
    plot_n = 0;
    file.setFileName("script_band.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_band.png\"" << newline;
    if(checkBox_grid->isChecked())
        out << "set grid" << newline;
    out << "set xlabel \"Frequency, THz\"" << newline;
    out << "set xrange [" << Saved.vc.toFloat()-v_spread << ":" << Saved.vc.toFloat()+v_spread << "]" << newline;
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
    out << newline;
    file.close();

    ////////////////////////////////////////////////////// Figure: Discharge ///////////////////////////////////////////////////
    file.setFileName("script_discharge.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_discharge.png\"" << newline;
    out << "set y2range [0:*]" << newline;
    out << "set ytics nomirror" << newline;
    out << "set y2tics nomirror" << newline;
    out << "set xlabel \"Time, us\"" << newline;
    out << "set ylabel \"Current, kA\"" << newline;
    out << "set y2label \"Voltage, kV\"" << newline;
    if(checkBox_grid->isChecked())
        out << "set grid" << newline;
    out << "plot \"data_discharge.dat\" using 1:($2/1000.0) axis x1y1 with lines ti \"Current\",\\" << newline;
    out << "\"data_discharge.dat\" using 1:($3/1000.0) axis x1y2 with lines ti \"Voltage\"" << newline;;
    file.close();

    /////////////////////////////////////////////////////////// Figure: q ///////////////////////////////////////////////////////
    file.setFileName("script_q.txt");
    file.open(QFile::WriteOnly | QFile::Truncate) ;
    out << "set terminal png " << font << " size 360,270" << newline;
    out << "set output \"fig_q.png\"" << newline;
    out << "set yrange [0:1]" << newline;
    out << "set xlabel \"Time, us\"" << newline;
    out << "set ylabel \"q\"" << newline;
    if(checkBox_grid->isChecked())
        out << "set grid" << newline;
    out << "plot \"data_q.dat\" using 1:2 with lines ti \"Lower: q2\",\\" << newline;
    out << "\"data_q.dat\" using 1:3 with lines ti \"Upper: q3\",\\" << newline;
    out << "\"data_q.dat\" using 1:4 with lines ti \"N2: q4\",\\" << newline;
    out << "\"data_q.dat\" using 1:5 with lines ti \"Transl: qT\"" << newline;
    file.close();

    //////////////////////////////////////////////////////////// Make figures with Gnuplot ////////////////////////////////////////////
    QFile::remove("fig_energy.png");
    QFile::remove("fig_power.png");
    QFile::remove("fig_fluence.png");
    QFile::remove("fig_spectra.png");
    QFile::remove("fig_band.png");
    QFile::remove("fig_discharge.png");
    QFile::remove("fig_q.png");
    QFile::remove("fig_temperatures.png");
    QFile::remove("fig_e.png");
    this->setCursor(Qt::WaitCursor);
    proc = new QProcess( this );
    proc->execute(path_to_gnuplot + " script_energy.txt");
    proc->execute(path_to_gnuplot + " script_power.txt");
    proc->execute(path_to_gnuplot + " script_fluence.txt");
    proc->execute(path_to_gnuplot + " script_spectra.txt");
    if(am_n != -1){ // if active medium
        proc->execute(path_to_gnuplot + " script_band.txt");
        proc->execute(path_to_gnuplot + " script_discharge.txt");
        proc->execute(path_to_gnuplot + " script_q.txt");
        proc->execute(path_to_gnuplot + " script_temperatures.txt");
        proc->execute(path_to_gnuplot + " script_e.txt");
    }
    this->setCursor(Qt::ArrowCursor);

    ////////// Display figures ////////////
    ShowFigures();

    ///////// Save plot settings, update controls /////////
    SaveSettings("plot"); // save only plot settings
    OnModified();
}


void MainWindow::ShowFigures()
{
    QPixmap pixmap;
    bool b;

    label_fig1->clear();
    label_fig2->clear();
    label_fig3->clear();
    label_fig4->clear();
    label_fig5->clear();
    label_fig6->clear();
    label_fig7->clear();
    label_fig8->clear();
    label_fig9->clear();

    b = pixmap.load("fig_energy.png");
    label_fig1->setVisible(b);
    if(b)
        label_fig1->setPixmap(pixmap);

    b = pixmap.load("fig_spectra.png");
    label_fig2->setVisible(b);
    if(b)
        label_fig2->setPixmap(pixmap);

    b = pixmap.load("fig_band.png");
    label_fig3->setVisible(b);
    if(b)
        label_fig3->setPixmap(pixmap);

    b = pixmap.load("fig_fluence.png");
    label_fig4->setVisible(b);
    if(b)
        label_fig4->setPixmap(pixmap);

    b = pixmap.load("fig_power.png");
    label_fig5->setVisible(b);
    if(b)
        label_fig5->setPixmap(pixmap);

    b = pixmap.load("fig_discharge.png");
    label_fig6->setVisible(b);
    if(b)
        label_fig6->setPixmap(pixmap);

    b = pixmap.load("fig_temperatures.png");
    label_fig7->setVisible(b);
    if(b)
        label_fig7->setPixmap(pixmap);

    b = pixmap.load("fig_e.png");
    label_fig8->setVisible(b);
    if(b)
        label_fig8->setPixmap(pixmap);

    b = pixmap.load("fig_q.png");
    label_fig9->setVisible(b);
    if(b)
        label_fig9->setPixmap(pixmap);

    flag_plot_modified = true;
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
