#include "co2am+.h"


void MainWindow::FlagModifiedAndPostponePlot()
{
    flag_project_modified  = true;
    flag_plot_postponed = true;
}


void MainWindow::PostponePlot()
{
    flag_plot_postponed = true;
}


void MainWindow::PlotIfPostponed()
{
    if(flag_plot_postponed)
        Plot();
}


void MainWindow::FlagModifiedAndPlot()
{
    flag_project_modified  = true;
    Plot();
}


void MainWindow::Plot()
{
    if(!QFile::exists("energy.dat") && !flag_calculating)
        return;

    if(tabWidget_main->currentIndex()!=2)
    {
        flag_plot_postponed = true;
        return;
    }

    this->setCursor(Qt::WaitCursor);

    QFile file;
    QTextStream out(&file);
    //out.setCodec("UTF-8");

    QString optic_id = comboBox_optic->currentText();
    QString pulse_id = comboBox_pulse ->currentText();
    QString optic_type = Type(optic_id);
    int pass_n, plot_n;

    ClearPlot();

    // remove svg files
    QStringList filelist = QDir(work_dir).entryList();
    for(int i=0; i<filelist.size(); i++)
        if(QFileInfo(filelist[i]).suffix()=="svg")
            QFile::remove(filelist[i]);

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
    svg_fig3->setHidden(optic_type != "A" && optic_type != "C" && optic_type != "S");
    svg_fig4->setHidden(false);
    svg_fig5->setHidden(false);
    //svg_fig6->setHidden(optic_type != "A" && optic_type != "F" && optic_type != "P");
    svg_fig6->setHidden(optic_type != "A" && optic_type != "F");
    svg_fig7->setHidden(optic_type != "A");
    svg_fig8->setHidden(optic_type != "A");
    svg_fig9->setHidden(optic_type != "A");

    //double zoom = doubleSpinBox_zoom->value();

    QSize size(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    switch(comboBox_size->currentIndex())
    {
        case 0: // "Auto"
            size = QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
            break;
        case 1: // "360x270"
            size = QSize(floor(360.0),(int)floor(270.0));
            break;
        case 2: // "480x360"
            size = QSize(floor(480.0),(int)floor(360.0));
            break;
        case 3: // "640x480"
            size = QSize(floor(640.0),(int)floor(480.0));
            break;
        case 4: // "800x600"
            size = QSize(floor(800.0),(int)floor(600.0));
            break;
        case 5: // "1024x768"
            size = QSize(floor(1024.0),(int)floor(768.0));
            break;
        case 6: // "Custom"
            size = QSize(floor(spinBox_width->value()),(int)floor(spinBox_height->value()));
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
    double fluence_mult    = pow(10, floor(comboBox_fluenceUnit->currentIndex()/3.0)*3 - 3);
    if(comboBox_fluenceUnit->currentIndex()%3 == 1)
        fluence_mult *= pow(10,-4);
    if(comboBox_fluenceUnit->currentIndex()%3 == 2)
        fluence_mult *= pow(10,-6);
    double intensity_mult  = pow(10, floor(comboBox_intensityUnit->currentIndex()/3.0)*3 - 3);
    if(comboBox_intensityUnit->currentIndex()%3 == 1)
        intensity_mult *= pow(10,-4);
    if(comboBox_intensityUnit->currentIndex()%3 == 2)
        intensity_mult *= pow(10,-6);
    double spectrum_mult  = pow(10, comboBox_spectrumUnit->currentIndex() + 4 * (3*ceil(comboBox_spectrumUnit->currentIndex()/3.0) - comboBox_spectrumUnit->currentIndex()) );
    double t_mult          = pow(10, comboBox_tUnit->currentIndex()*3);
    double power_mult      = pow(10, comboBox_powerUnit->currentIndex()*3 - 18);
    double discharge_mult  = pow(10, comboBox_dischargeUnits->currentIndex()*3 - 6);
    // units
    QString time_unit      = comboBox_timeUnit      -> currentText();
    QString energy_unit    = comboBox_energyUnit    -> currentText();
    QString length_unit    = comboBox_lengthUnit    -> currentText();
    QString fluence_unit   = comboBox_fluenceUnit   -> currentText();
    QString t_unit         = comboBox_tUnit         -> currentText();
    QString power_unit     = comboBox_powerUnit     -> currentText();
    QString intensity_unit = comboBox_intensityUnit -> currentText();
    QString spectrum_unit  = comboBox_spectrumUnit  -> currentText();
    QString voltage_unit   = comboBox_dischargeUnits-> currentText().split(", ")[0];
    QString current_unit   = comboBox_dischargeUnits-> currentText().split(", ")[1];

    // pulse time limits
    double t_min = lineEdit_t_min->text().toDouble()*t_mult/pow(2, comboBox_timeScale->currentIndex());
    double t_max = lineEdit_t_max->text().toDouble()*t_mult/pow(2, comboBox_timeScale->currentIndex());

    // frequency limits
    double v_range = comboBox_precision_t->currentText().toDouble()
            / (lineEdit_t_max->text().toDouble()-lineEdit_t_min->text().toDouble())
            / pow(2, comboBox_freqScale->currentIndex());
    double v_min = lineEdit_v0->text().toDouble() - v_range/2;
    double v_max = lineEdit_v0->text().toDouble() + v_range/2;

    // frequency / wavelength / wavenumber axis
    double c = 2.99792458e8;// m/s
    QString frequency_xlabel;
    QString frequency_using;
    if(comboBox_frequencyUnit->currentIndex() == 0) // Hz
    {
        frequency_xlabel = "Frequency (Hz)";
        frequency_using = " using ($1)";
    }
    if(comboBox_frequencyUnit->currentIndex() == 1) // GHz
    {
        frequency_xlabel = "Frequency (GHz)";
        frequency_using = " using ($1/1e9)";
        v_min /= 1e9;
        v_max /= 1e9;
    }
    if(comboBox_frequencyUnit->currentIndex() == 2) // THz
    {
        frequency_xlabel = "Frequency (THz)";
        frequency_using = " using ($1/1e12)";
        v_min /= 1e12;
        v_max /= 1e12;
    }
    if(comboBox_frequencyUnit->currentIndex() == 3) // 1/cm
    {
        frequency_xlabel = "Wavenumber (1/cm)";
        frequency_using = " using ($1/2.99792458e10)";
        v_min = v_min/c/100;
        v_max = v_max/c/100;
    }
    if(comboBox_frequencyUnit->currentIndex() == 4) // µm
    {
        frequency_xlabel = "Wavelength (µm)";
        frequency_using = " using (2.99792458e14/$1)";
        double tmp = v_min;
        v_min = c/v_max*1e6;
        v_max = c/tmp*1e6;
    }
    if(comboBox_frequencyUnit->currentIndex() == 5) // nm
    {
        frequency_xlabel = "Wavelength (nm)";
        frequency_using = " using (2.99792458e17/$1)";
        double tmp = v_min;
        v_min = c/v_max*1e9;
        v_max = c/tmp*1e9;
    }

    QString common_file_head = "set terminal svg size "
            + QString::number(svg_fig1->width()) + ","
            + QString::number(svg_fig1->height()) +"\n"
            + "set encoding utf8\n";
    if(!checkBox_labels->isChecked())
        common_file_head += "unset key\n";
    if(checkBox_grid->isChecked())
        common_file_head += "set grid\n";

    // GnuPlot script: Energy
    SelectEnergies();
    file.setFileName("script_energy.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head
        << "set output \"fig_energy.svg\"\n"
        << "set xlabel \"Time (" << time_unit << ")\"\n"
        << "set ylabel \"Pulse energy (" << energy_unit << ")\"\n";
    checkBox_log->isChecked() ? out << "set logscale y\n" : out << "set yrange [0:*]\n";
    out << "plot \"energy_selected.dat\" using ($1*" << time_mult << "):($2*" << energy_mult << ") with points pt 7 ps 0.75 notitle\n";
    file.close();
    QProcess *proc1 = new QProcess(this);
    proc1->start(path_to_gnuplot, QStringList("script_energy.gp"));
    //proc1->waitForFinished();;

    // GnuPlot script: Fluence
    plot_n = 0;
    file.setFileName("script_fluence.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head
        << "set output \"fig_fluence.svg\"\n"
        << "set xlabel \"r (" << length_unit << ")\"\n"
        << "set ylabel \"Fluence (" << fluence_unit << ")\"\n";
    for(int i=0; i<=9; i++)
    {
        pass_n = PassNumber(i);
        if(pass_n == -1)
            continue;
        QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + "_fluence.dat";
        if(!QFile::exists(filename))
            continue;
        if(plot_n==0)
            out << "plot ";
        else
            out << ",\\\n";
        out << "\"" << filename << "\""
            << " using ($1*" << length_mult << "):($2*" << fluence_mult << ")"
            << " with lines ti \"";
        if(plot_n==0)
            out << "Pass# ";
        out << pass_n << "\"";
        plot_n ++;
    }
    out << "\n";
    file.close();
    QProcess *proc2 = new QProcess(this);
    proc2->start(path_to_gnuplot, QStringList("script_fluence.gp"));

    // GnuPlot script: Power
    plot_n = 0;
    file.setFileName("script_power.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head
        << "set output \"fig_power.svg\"\n"
        << "set xlabel \"Time (" << t_unit << ")\"\n"
        << "set xrange [" << t_min << ":" << t_max << "]\n"
        << "set ylabel \"Power (" << power_unit << ")\"\n";
    for(int i=0; i<=9; i++)
    {
        pass_n = PassNumber(i);
        if(pass_n == -1)
            continue;
        QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + "_power.dat";
        if(!QFile::exists(filename))
            continue;
        if(plot_n==0)
            out << "plot ";
        else
            out << ",\\\n";
        out << "\"" << filename << "\""
            << " using ($1*" << t_mult << "):($2*" << power_mult << ")"
            << " with lines ti \"";
        if(plot_n==0)
            out << "Pass# ";
        out << pass_n << "\"";
        plot_n ++;
    }
    out << "\n";
    file.close();
    QProcess *proc3 = new QProcess(this);
    proc3->start(path_to_gnuplot, QStringList("script_power.gp"));

    // GnuPlot script: Spectra
    plot_n = 0;
    file.setFileName("script_spectra.gp");
    file.open(QFile::WriteOnly | QFile::Truncate);
    out << common_file_head
        << "set output \"fig_spectra.svg\"\n"
        << "set xlabel \"" << frequency_xlabel << "\"\n"
        << "set xrange [" << v_min << ":" << v_max << "]\n"
        << "set ylabel \"Spectral energy density (" << spectrum_unit << ")\"\n"
        << "set yrange [0:*]\n";
    for(int i=0; i<=9; i++)
    {
        pass_n = PassNumber(i);
        if(pass_n == -1)
            continue;
        QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + "_spectrum.dat";
        if(!QFile::exists(filename))
            continue;
        if(plot_n==0)
            out << "plot ";
        else
            out << ",\\\n";
        out << "\"" << filename << "\""
            << frequency_using
            << ":($2*" << spectrum_mult << ")"
            << " with lines ti \"";
        if(plot_n==0)
            out << "Pass# ";
        out << pass_n << "\"";
        plot_n ++;
    }
    out << "\n";
    file.close();
    QProcess *proc4 = new QProcess(this);
    proc4->start(path_to_gnuplot, QStringList("script_spectra.gp"));

    if(optic_type == "A")
    {
        // GnuPlot script: Temperatures
        file.setFileName("script_temperatures.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_temperatures.svg\"\n"
            << "set xlabel \"Time (" << time_unit << ")\"\n"
            << "set ylabel \"Temperature (K)\"\n"
            << "set yrange [0:*]\n"
            << "plot \"" << optic_id << "_temperatures.dat\" using ($1*" << time_mult << "):($2) with lines ti \"Lower: T2\",\\\n"
            <<      "\"" << optic_id << "_temperatures.dat\" using ($1*" << time_mult << "):($3) with lines ti \"Upper: T3\",\\\n"
            <<      "\"" << optic_id << "_temperatures.dat\" using ($1*" << time_mult << "):($4) with lines ti \"N2: T4\",\\\n"
            <<      "\"" << optic_id << "_temperatures.dat\" using ($1*" << time_mult << "):($5) with lines ti \"Transl:  T\"\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot, QStringList("script_temperatures.gp"));

        // GnuPlot script: e
        file.setFileName("script_e.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_e.svg\"\n"
            << "set xlabel \"Time (" << time_unit << ")\"\n"
            << "set ylabel \"e (quanta / molecule)\"\n"
            << "set yrange [0:*]\n"
            // (Qstring(...) used below to solve a unicode problem - "μ" is not written to file correctly)
            << "plot \"" << optic_id << "_e.dat\" using ($1*" << time_mult << QString("):($2) with lines ls 4 ti \"Lower 10 μm (symm stretch): e1\",\\\n")
            <<      "\"" << optic_id << "_e.dat\" using ($1*" << time_mult << QString("):($3) with lines ls 1 ti \"Lower 9 μm (bend): e2\",\\\n")
            <<      "\"" << optic_id << "_e.dat\" using ($1*" << time_mult <<         "):($4) with lines ls 2 ti \"Upper (asymm stretch): e3\",\\\n"
            <<      "\"" << optic_id << "_e.dat\" using ($1*" << time_mult <<         "):($5) with lines ls 3 ti \"N2: e4\"\n";
        file.close();
        QProcess *proc6 = new QProcess(this);
        proc6->start(path_to_gnuplot, QStringList("script_e.gp"));

        // GnuPlot script: Gain spectrum
        plot_n = 0;
        file.setFileName("script_gain.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_gain.svg\"\n"
            << "set xlabel \"" << frequency_xlabel << "\"\n"
            << "set xrange [" << v_min << ":" << v_max << "]\n"
            << "set ylabel \"Gain, %/cm\"\n";
        for(int i=0; i<10; i++)
        {
            pass_n = PassNumber(i);
            if(pass_n == -1)
                continue;
            QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + "_gain.dat";
            if(!QFile::exists(filename))
                continue;
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\\n";
            out << "\"" << filename << "\""
                << frequency_using
                << ":($2) with lines ti \"";
            if(plot_n==0)
                out << "Pass# ";
            out << pass_n << "\"";
            plot_n ++;
        }
        out << "\n";
        file.close();
        QProcess *proc7 = new QProcess(this);
        proc7->start(path_to_gnuplot, QStringList("script_gain.gp"));

        // GnuPlot script: Discharge
        QProcess *proc8 = new QProcess(this);
        if(QFile::exists(optic_id + "_discharge.dat"))
        {
            file.setFileName("script_discharge.gp");
            file.open(QFile::WriteOnly | QFile::Truncate);
            out << common_file_head
                << "set output \"fig_discharge.svg\"\n"
                << "set y2range [0:*]\n"
                << "set ytics nomirror\n"
                << "set y2tics nomirror\n"
                << "set xlabel \"Time (" << time_unit << ")\"\n"
                << "set ylabel \"Current (" << current_unit << ")\"\n"
                << "set y2label \"Voltage (" << voltage_unit << ")\"\n"
                << "plot \"" << optic_id << "_discharge.dat\" using ($1*" << time_mult << "):($2*" << discharge_mult << ") axis x1y1 with lines ti \"Current\",\\\n"
                <<      "\"" << optic_id << "_discharge.dat\" using ($1*" << time_mult << "):($3*" << discharge_mult << ") axis x1y2 with lines ti \"Voltage\"\n";
            file.close();
            proc8->start(path_to_gnuplot, QStringList("script_discharge.gp"));
        }

        // GnuPlot script: q
        QProcess *proc9 = new QProcess(this);
        if(QFile::exists(optic_id + "_q.dat")){
            file.setFileName("script_q.gp");
            file.open(QFile::WriteOnly | QFile::Truncate);
            out << common_file_head
                << "set output \"fig_q.svg\"\n"
                << "set yrange [0:1]\n"
                << "set xlabel \"Time (" << time_unit << ")\"\n"
                << "set ylabel \"q\"\n"
                << "plot \"" << optic_id << "_q.dat\" using ($1*" << time_mult << "):($2) with lines ti \"Lower: q2\",\\\n"
                <<      "\"" << optic_id << "_q.dat\" using ($1*" << time_mult << "):($3) with lines ti \"Upper: q3\",\\\n"
                <<      "\"" << optic_id << "_q.dat\" using ($1*" << time_mult << "):($4) with lines ti \"N2: q4\",\\\n"
                <<      "\"" << optic_id << "_q.dat\" using ($1*" << time_mult << "):($5) with lines ti \"Transl: qT\"\n";
            file.close();
            proc9->start(path_to_gnuplot, QStringList("script_q.gp"));
        }

        // GnuPlot script: pumping pulse
        QProcess *proc10 = new QProcess(this);
        if(QFile::exists(optic_id + "_pumping_pulse.dat"))
        {
            file.setFileName("script_pumping_pulse.gp");
            file.open(QFile::WriteOnly | QFile::Truncate);
            out << common_file_head
                << "set output \"fig_pumping_pulse.svg\"\n"
                << "set xlabel \"Time (" << time_unit << ")\"\n"
                << "set ylabel \"Intensity (" << intensity_unit << ")\"\n"
                << "plot \"" << optic_id << "_pumping_pulse.dat\" "
                << "using ($1*" << time_mult << "):($2*" << intensity_mult << ")"
                << "with lines ti \"Pumping pulse\"\n";
            file.close();
            proc10->start(path_to_gnuplot, QStringList("script_pumping_pulse.gp"));
        }

        proc5->waitForFinished();
        proc6->waitForFinished();
        proc7->waitForFinished();
        proc8->waitForFinished();
        proc9->waitForFinished();
        proc10->waitForFinished();
    }

    if(optic_type == "C")
    {
        // GnuPlot script: Chirpyness (chirper)
        file.setFileName("script_chirpyness.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_chirpyness.svg\"\n"
            << "set xlabel \"" << frequency_xlabel << "\"\n"
            << "set xrange [" << v_min << ":" << v_max << "]\n"
            << "set ylabel \"Chirpyness, Hz/s\"\n"
            << "plot \"" << optic_id << "_chirpyness.dat\"" << frequency_using << ":($2) with lines notitle\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot, QStringList("script_chirpyness.gp"));
        proc5->waitForFinished();
    }

    if(optic_type == "F")
    {
        // GnuPlot script: Transmittance (spatial filter)
        file.setFileName("script_transmittance.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_transmittance.svg\"\n"
            << "set xlabel \"r (" << length_unit << ")\"\n"
            << "set ylabel \"Transmittance\"\n"
            << "set yrange [0:1]\n"
            << "plot \"" << optic_id << "_transmittance.dat\" using ($1*" << length_mult << "):($2) with lines notitle\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot, QStringList("script_transmittance.gp"));
        proc5->waitForFinished();
    }

    if(optic_type == "S")
    {
        // GnuPlot script: Transmittance (spectral filter)
        file.setFileName("script_transmittance.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_transmittance.svg\"\n"
            << "set xlabel \"" << frequency_xlabel << "\"\n"
            << "set xrange [" << v_min << ":" << v_max << "]\n"
            << "set ylabel \"Transmittance\"\n"
            << "set yrange [0:1]\n"
            << "plot \"" << optic_id << "_transmittance.dat\"" << frequency_using << ":($2) with lines notitle\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot, QStringList("script_transmittance.gp"));
        proc5->waitForFinished();
    }

    /*if(optic_type == "P")
    {
        // GnuPlot script: Phase
        plot_n = 0;
        file.setFileName("script_phase.gp");
        file.open(QFile::WriteOnly | QFile::Truncate);
        out << common_file_head
            << "set output \"fig_phase.svg\"\n"
            << "set xlabel \"Time (" << t_unit << ")\"\n"
            << "set xrange [" << t_min << ":" << t_max << "]\n"
            << "set ylabel \"Phase (rad)\"\n"
            << "set yrange [-3.15:3.15]\n";
        for(int i=0; i<=9; i++)
        {
            pass_n = PassNumber(i);
            if(pass_n == -1)
                continue;
            QString filename = optic_id + "_" + pulse_id + "_pass" + QString::number(pass_n) + "_phase.dat";
            if(!QFile::exists(filename))
                continue;
            if(plot_n==0)
                out << "plot ";
            else
                out << ",\\\n";
            out << "\"" << filename << "\""
                << " using ($1*" << t_mult << "):($2)"
                << " with lines ti \"";
            if(plot_n==0)
                out << "Pass# ";
            out << pass_n << "\"";
            plot_n ++;
        }
        out << "\n";
        file.close();
        QProcess *proc5 = new QProcess(this);
        proc5->start(path_to_gnuplot, QStringList("script_phase.gp"));
        proc5->waitForFinished();
    }*/

    proc1->waitForFinished();
    proc2->waitForFinished();
    proc3->waitForFinished();
    proc4->waitForFinished();

    /////////////////////////////////////// Display figures //////////////////////////////////////////

    svg_fig1->load(QString("fig_energy.svg"));
    svg_fig2->load(QString("fig_spectra.svg"));
    svg_fig4->load(QString("fig_fluence.svg"));
    svg_fig5->load(QString("fig_power.svg"));
    if(optic_type == "A") //active medium
    {
        svg_fig3->load(QString("fig_gain.svg"));
        if(QFile::exists("fig_discharge.svg"))
            svg_fig6->load(QString("fig_discharge.svg"));
        if(QFile::exists("fig_pumping_pulse.svg"))
            svg_fig6->load(QString("fig_pumping_pulse.svg"));
        svg_fig7->load(QString("fig_temperatures.svg"));
        svg_fig8->load(QString("fig_e.svg"));
        if(QFile::exists("fig_q.svg"))
            svg_fig9->load(QString("fig_q.svg"));
    } 
    if(optic_type == "C") // chirper
        svg_fig3->load(QString("fig_chirpyness.svg"));
    if(optic_type == "S") // spectral filter
        svg_fig3->load(QString("fig_transmittance.svg"));
    if(optic_type == "F") // spatial filter
        svg_fig6->load(QString("fig_transmittance.svg"));
    //if(optic_type == "P") // probe, show phase
    //    svg_fig6->load(QString("fig_phase.svg"));

    ////////////////////////////////// Update flags and controls ///////////////////////////////////
    Update();
    if(flag_project_modified)
        UpdateConfigurationFiles();
    flag_plot_postponed = false;
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
