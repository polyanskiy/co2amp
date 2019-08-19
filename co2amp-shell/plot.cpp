#include "mainwindow.h"



void MainWindow::Plot()
{
    if(tabWidget_main->currentIndex()!=2){
        flag_plot_postponed = true;
        return;
    }

    //if(flag_plot_postponed_modified)
    //    flag_plot_modified = true;

    if(flag_plot_modified)
        MemorizeSettings();

    this->setCursor(Qt::WaitCursor);

    int i;
    QFile file;
    QTextStream out(&file);
    out.setCodec("UTF-8");

    int optic_n = comboBox_optic->currentIndex();
    int am_n = AmNumber(optic_n);
    int pulse_n = comboBox_pulse->currentIndex();
    int pass_n, plot_n, set_n;

    ClearPlot();

    QFile::remove("fig_energy.svg");
    QFile::remove("fig_fluence.svg");
    QFile::remove("fig_power.svg");
    QFile::remove("fig_spectra.svg");
    QFile::remove("fig_temperatures.svg");
    QFile::remove("fig_e.svg");
    QFile::remove("fig_gain.svg");
    QFile::remove("fig_discharge.svg");
    QFile::remove("fig_q.svg");

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

    double zoom = doubleSpinBox_zoom->value();

    QSize size(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    switch(comboBox_size->currentIndex()){
    case 0: // "Auto"
        size = QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        break;
    case 1: // "360x270"
        size = QSize(floor(360.0*zoom),(int)floor(270.0*zoom));
        break;
    case 2: // "480x360"
        size = QSize(floor(480.0*zoom),(int)floor(360.0*zoom));
        break;
    case 3: // "640x480"
        size = QSize(floor(640.0*zoom),(int)floor(480.0*zoom));
        break;
    case 4: // "800x600"
        size = QSize(floor(800.0*zoom),(int)floor(600.0*zoom));
        break;
    case 5: // "1024x768"
        size = QSize(floor(1024.0*zoom),(int)floor(768.0*zoom));
        break;
    case 6: // "Custom"
        size = QSize(floor(spinBox_width->value()*zoom),(int)floor(spinBox_height->value()*zoom));
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

    /////////////// wait for widgets to update sizes (IMPORTANT TO DO IT HERE!!!) ////////////////
    QCoreApplication::processEvents(QEventLoop::AllEvents,1000);


    //////////////////////////////////// Write GnuPlot scripts ///////////////////////////////////
    // multipliers
    double time_mult       = pow(10, comboBox_timeUnit->currentIndex()*3);
    double energy_mult     = pow(10, comboBox_energyUnit->currentIndex()*3 - 6);
    double length_mult     = pow(10, comboBox_lengthUnit->currentIndex()*3 - 3);
    double fluence_mult    = pow(10, floor(comboBox_fluenceUnit->currentIndex()/3)*3 - 3);
    if(comboBox_fluenceUnit->currentIndex()%3 == 1)
        fluence_mult *= pow(10,-4);
    if(comboBox_fluenceUnit->currentIndex()%3 == 2)
        fluence_mult *= pow(10,-6);
    double t_mult          = pow(10, comboBox_tUnit->currentIndex()*3);
    double power_mult      = pow(10, comboBox_powerUnit->currentIndex()*3 - 18);
    double discharge_mult  = pow(10, comboBox_dischargeUnits->currentIndex()*3 - 6);
    // units
    QString time_unit      = comboBox_timeUnit->currentText();
    QString energy_unit    = comboBox_energyUnit->currentText();
    QString length_unit    = comboBox_lengthUnit->currentText();
    QString fluence_unit   = comboBox_fluenceUnit->currentText();
    QString t_unit         = comboBox_tUnit->currentText();
    QString power_unit     = comboBox_powerUnit->currentText();
    QString voltage_unit   = comboBox_dischargeUnits->currentText().split(", ")[0];
    QString current_unit   = comboBox_dischargeUnits->currentText().split(", ")[1];

    // pulse time limits
    double t_min = Saved.t_min.toDouble()*t_mult/pow(2, comboBox_timeScale->currentIndex());
    double t_max = Saved.t_max.toDouble()*t_mult/pow(2, comboBox_timeScale->currentIndex());

    // frequency limits
    double v_range = comboBox_precision_t->currentText().toDouble()
            / (Saved.t_max.toDouble()-Saved.t_min.toDouble())
            / pow(2, comboBox_freqScale->currentIndex()+1);
    double v_min = Saved.vc.toDouble() - v_range/2;
    double v_max = Saved.vc.toDouble() + v_range/2;

    // frequency / wavelength / wavenumber axis
    double c = 2.99792458e8;// m/s
    QString frequency_xlabel;
    QString frequency_using;
    if(comboBox_frequencyUnit->currentIndex() == 0){ // THz
        frequency_xlabel = "Frequency, THz";
        frequency_using = " using ($1/1e12):($2)";
        v_min /= 1e12;
        v_max /= 1e12;
    }
    if(comboBox_frequencyUnit->currentIndex() == 1){ // 1/cm
        frequency_xlabel = "Wavenumber, 1/cm";
        frequency_using = " using ($1/2.99792458e10):($2)";
        v_min = v_min/c/100;
        v_max = v_max/c/100;
    }
    if(comboBox_frequencyUnit->currentIndex() == 2){ // um
        frequency_xlabel = "Wavelength, µm";
        frequency_using = " using (2.99792458e14/$1):($2)";
        double tmp = v_min;
        v_min = c/v_max*1e6;
        v_max = c/tmp*1e6;
    }
    if(comboBox_frequencyUnit->currentIndex() == 3){ // nm
        frequency_xlabel = "Wavelength, nm";
        frequency_using = " using (2.99792458e17/$1):($2)";
        double tmp = v_min;
        v_min = c/v_max*1e9;
        v_max = c/tmp*1e9;
    }

    QString common_file_head = "set terminal svg size "
            + QString::number(svg_fig1->width()/zoom) + ","
            + QString::number(svg_fig1->height()/zoom) +"\n"
            + "set encoding utf8\n";
    if(!checkBox_labels->isChecked())
        common_file_head += "unset key\n";
    if(checkBox_grid->isChecked())
        common_file_head += "set grid\n";

    // GnuPlot script: Energy
    SelectEnergies();
    file.setFileName("script_energy.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head;
    out << "set output \"fig_energy.svg\"\n";
    out << "set xlabel \"Time, " << time_unit << "\"\n";
    out << "set ylabel \"Pulse energy, " << energy_unit << "\"\n";
    checkBox_log->isChecked() ? out << "set logscale y\n" : out << "set yrange [0:*]\n";
    out << "plot \"data_energy_selected.dat\" using ($1*" << time_mult << "):($2*" << energy_mult << ") notitle\n";
    file.close();
    QProcess *proc1 = new QProcess(this);
    proc1->start(path_to_gnuplot + " script_energy.gp");
    proc1->waitForFinished();;

    // GnuPlot script: Fluence
    plot_n = 0;
    file.setFileName("script_fluence.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head;
    out << "set output \"fig_fluence.svg\"\n";
    out << "set xlabel \"r, " << length_unit << "\"\n";
    out << "set ylabel \"Fluence, " << fluence_unit << "\"\n";
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, optic_n, pass_n, "data_fluence.dat");
        if(set_n != -1){
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\\n";
            out << "\"data_fluence.dat\" index " << set_n
                << " using ($1*" << length_mult << "):($2*" << fluence_mult << ")"
                << " with lines ti \"";
            if(plot_n==0)
                out << "Passes: ";
            out << pass_n+1 << "\"";
            plot_n ++;
        }
    }
    out << "\n";
    file.close();
    QProcess *proc2 = new QProcess(this);
    proc2->start(path_to_gnuplot + " script_fluence.gp");

    // GnuPlot script: Power
    plot_n = 0;
    file.setFileName("script_power.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head;
    out << "set output \"fig_power.svg\"\n";
    out << "set xlabel \"Time, " << t_unit << "\"\n";
    out << "set xrange [" << t_min << ":" << t_max << "]\n";
    out << "set ylabel \"Power, " << power_unit << "\"\n";
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, optic_n, pass_n, "data_power.dat");
        if(set_n != -1){
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\\n";
            out << "\"data_power.dat\" index " << set_n
                << " using ($1*" << t_mult << "):($2*" << power_mult << ")"
                << " with lines ti \"";
            if(plot_n==0)
                out << "Passes: ";
            out << pass_n+1 << "\"";
            plot_n ++;
        }
    }
    out << "\n";
    file.close();
    QProcess *proc3 = new QProcess(this);
    proc3->start(path_to_gnuplot + " script_power.gp");

    // GnuPlot script: Spectra
    plot_n = 0;
    file.setFileName("script_spectra.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head;
    out << "set output \"fig_spectra.svg\"\n";
    out << "set xlabel \"" << frequency_xlabel << "\"\n";
    out << "set xrange [" << v_min << ":" << v_max << "]\n";
    out << "set ylabel \"Intensity, a.u.\"\n";
    out << "set yrange [0:*]\n";
    for(i=0; i<=9; i++){
        pass_n = PassNumber(i);
        set_n = DatasetNumber(pulse_n, optic_n, pass_n, "data_spectra.dat");
        if(set_n != -1){
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\\n";
            out << "\"data_spectra.dat\" index " << set_n;
            out << frequency_using;
            out << " with lines ti \"";
            if(plot_n==0)
                out << "Passes: ";
            out << pass_n+1 << "\"";
            plot_n ++;
        }
    }
    out << "\n";
    file.close();
    QProcess *proc4 = new QProcess(this);
    proc4->start(path_to_gnuplot + " script_spectra.gp");

    if(am_n != -1){
        // GnuPlot script: Temperatures
        file.setFileName("script_temperatures.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head;
        out << "set output \"fig_temperatures.svg\"\n";
        out << "set xlabel \"Time, " << time_unit << "\"\n";
        out << "set ylabel \"Temperature, K\"\n";
        out << "set yrange [0:*]\n";
        out << "plot \"data_temperatures.dat\" using ($1*" << time_mult << "):($" << 4*am_n+2 << ") with lines ti \"Lower: T2\",\\\n";
        out << "\"data_temperatures.dat\" using ($1*" << time_mult << "):($" << 4*am_n+3 << ") with lines ti \"Upper: T3\",\\\n";
        out << "\"data_temperatures.dat\" using ($1*" << time_mult << "):($" << 4*am_n+4 << ") with lines ti \"N2: T4\",\\\n";
        out << "\"data_temperatures.dat\" using ($1*" << time_mult << "):($" << 4*am_n+5 << ") with lines ti \"Transl:  T\"\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot + " script_temperatures.gp");

        // GnuPlot script: e
        file.setFileName("script_e.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head;
        out << "set output \"fig_e.svg\"\n";
        out << "set xlabel \"Time, " << time_unit << "\"\n";
        out << "set ylabel \"e (# of quanta / molecule)\"\n";
        out << "set yrange [0:*]\n";
        out << "plot \"data_e.dat\" using ($1*" << time_mult << "):($" << 4*am_n+2 << QString(") with lines ls 4 ti \"Lower 10 μm (symm stretch): e1\",\\\n");
        out << "\"data_e.dat\" using ($1*" << time_mult << "):($" << 4*am_n+3 << QString(") with lines ls 1 ti \"Lower 9 μm (bend): e2\",\\\n");
        // (Qstring(...) used to solve a unicode problem - "μ" is not written to file correctly)
        out << "\"data_e.dat\" using ($1*" << time_mult << "):($" << 4*am_n+4 << ") with lines ls 2 ti \"Upper (asymm stretch): e3\",\\\n";
        out << "\"data_e.dat\" using ($1*" << time_mult << "):($" << 4*am_n+5 << ") with lines ls 3 ti \"N2: e4\"\n";
        file.close();
        QProcess *proc6 = new QProcess(this);
        proc6->start(path_to_gnuplot + " script_e.gp");

        // GnuPlot script: Gain spectrum
        plot_n = 0;
        file.setFileName("script_gain.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head;
        out << "set output \"fig_gain.svg\"\n";
        out << "set xlabel \"" << frequency_xlabel << "\"\n";
        out << "set xrange [" << v_min << ":" << v_max << "]\n";
        out << "set ylabel \"Gain, %/cm\"\n";
        for(i=0; i<10; i++){
            pass_n = PassNumber(i);
            set_n = DatasetNumber(pulse_n, optic_n, pass_n, "data_gain.dat");
            if(set_n != -1){
                if(plot_n==0)
                    out << "plot ";
                else
                    out << ",\\\n";
                out << "\"data_gain.dat\" index " << set_n;
                out << frequency_using;
                out << " with lines ti \"";
                if(plot_n==0)
                    out << "Passes: ";
                out << pass_n+1 << "\"";
                plot_n ++;
            }
        }
        out << "\n";
        file.close();
        QProcess *proc7 = new QProcess(this);
        proc7->start(path_to_gnuplot + " script_gain.gp");

        // GnuPlot script: Discharge
        file.setFileName("script_discharge.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head;
        out << "set output \"fig_discharge.svg\"\n";
        out << "set y2range [0:*]\n";
        out << "set ytics nomirror\n";
        out << "set y2tics nomirror\n";
        out << "set xlabel \"Time, " << time_unit << "\"\n";
        out << "set ylabel \"Current, " << current_unit << "\"\n";
        out << "set y2label \"Voltage, " << voltage_unit << "\"\n";
        out << "plot \"data_discharge.dat\" using ($1*" << time_mult << "):($" << 2*am_n+2 << "*" << discharge_mult << ") axis x1y1 with lines ti \"Current\",\\\n";
        out << "\"data_discharge.dat\" using ($1*"      << time_mult << "):($" << 2*am_n+3 << "*" << discharge_mult << ") axis x1y2 with lines ti \"Voltage\"\n";
        file.close();
        QProcess *proc8 = new QProcess(this);
        proc8->start(path_to_gnuplot + " script_discharge.gp");

        // GnuPlot script: q
        file.setFileName("script_q.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head;
        out << "set output \"fig_q.svg\"\n";
        out << "set yrange [0:1]\n";
        out << "set xlabel \"Time, " << time_unit << "\"\n";
        out << "set ylabel \"q\"\n";
        out << "plot \"data_q.dat\" using ($1*" << time_mult << "):($" << 4*am_n+2 << ") with lines ti \"Lower: q2\",\\\n";
        out << "\"data_q.dat\" using ($1*" << time_mult << "):($" << 4*am_n+3 << ") with lines ti \"Upper: q3\",\\\n";
        out << "\"data_q.dat\" using ($1*" << time_mult << "):($" << 4*am_n+4 << ") with lines ti \"N2: q4\",\\\n";
        out << "\"data_q.dat\" using ($1*" << time_mult << "):($" << 4*am_n+5 << ") with lines ti \"Transl: qT\"\n";
        file.close();
        QProcess *proc9 = new QProcess(this);
        proc9->start(path_to_gnuplot + " script_q.gp");

        proc5->waitForFinished();
        proc6->waitForFinished();
        proc7->waitForFinished();
        proc8->waitForFinished();
        proc9->waitForFinished();
    }
    proc1->waitForFinished();
    proc2->waitForFinished();
    proc3->waitForFinished();
    proc4->waitForFinished();

    /////////////////////////////////////////////////////////////// Display figures ///////////////////////////////////////////////////

    svg_fig1->load(QString("fig_energy.svg"));
    svg_fig2->load(QString("fig_spectra.svg"));
    svg_fig4->load(QString("fig_fluence.svg"));
    svg_fig5->load(QString("fig_power.svg"));
    if(am_n != -1){ //active medium
        svg_fig3->load(QString("fig_gain.svg"));
        svg_fig6->load(QString("fig_discharge.svg"));
        svg_fig7->load(QString("fig_temperatures.svg"));
        svg_fig8->load(QString("fig_e.svg"));
        svg_fig9->load(QString("fig_q.svg"));
    }

    ///////////////////////////////////////////////////// update flags and controls ////////////////////////////////////////////////
    OnModified();
    if(flag_plot_modified)
        SaveSettings("plot"); // save only plot settings
    flag_plot_postponed = false;
    //flag_plot_postponed_modified = false;
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
    svg_fig1->setHidden(true);
    svg_fig2->setHidden(true);
    svg_fig3->setHidden(true);
    svg_fig4->setHidden(true);
    svg_fig5->setHidden(true);
    svg_fig6->setHidden(true);
    svg_fig7->setHidden(true);
    svg_fig8->setHidden(true);
    svg_fig9->setHidden(true);
}



void MainWindow::SelectEnergies()
{
    QString line;
    QRegExp separators("[\t\n]");
    int pulse_n = comboBox_pulse->currentIndex();
    int optic_n = comboBox_optic->currentIndex();

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
        case 1: // optic
        if(line.section(separators, 3, 3).toInt() == optic_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
	    case 2: // pulse
		if(line.section(separators, 2, 2).toInt() == pulse_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
        case 3: // optic + pulse
        if(line.section(separators, 2, 2).toInt() == pulse_n && line.section(separators, 3, 3).toInt() == optic_n)
		    out << line.section(separators, 0, 1) << "\n";
		break;
	    }
	}
	line = file_all.readLine();
    }

    file_sel.close();
    file_all.close();
}
