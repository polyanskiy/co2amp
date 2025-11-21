#include "co2amp.h"


void A::Initialize()
{
    std::string value="";

    Debug(1, "*** AMPLIFIER SECTION \'" + id + "\' ***");

    // Length (L)
    if(!YamlGetValue(&value, &yaml_content, "L"))
    {
        configuration_error = true;
        return;
    }
    length = std::stod(value);
    Debug(2, "L = " + toExpString(std::stod(value)) + " m");


    // ------- GAS MIXTURE -------

    // defaults
    for(int is=0; is<NumIso; ++is)
    {
        p_iso[is] = 0;
    }
    double O16 = 1; // Oxygen-16 content (0..1)
    double O17 = 0; // Oxygen-17 content (0..1)
    double O18 = 0; // Oxygen-18 content (0..1)
    double C12 = 1; // Carbon-12 content (0..1)
    double C13 = 0; // Carbon-13 content (0..1)

    if(YamlGetValue(&value, &yaml_content, "p_CO2", false))
    {
        p_CO2 = std::stod(value);
        Debug(2, "p_CO2 = " + std::to_string(p_CO2) + " bar");
        Debug(2, "Because p_CO2 is specified, partial pressures of isotopologues will be");
        Debug(2, "calculated at statistical equilibrium for specified isotope content");

        // Check if content of rare isotopes provided
        if(YamlGetValue(&value, &yaml_content, "O17", false))
        {
            O17 = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "O18", false))
        {
            O18 = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "C13", false))
        {
            C13 = std::stod(value);
        }

        // Check if content of natural isotopes provided. If not - calculate as a balance
        if(YamlGetValue(&value, &yaml_content, "O16", false))
        {
            O16 = std::stod(value);
        }
        else
        {
            O16 = 1 - (O17 + O18);
        }

        if(YamlGetValue(&value, &yaml_content, "C12", false))
        {
            C12 = std::stod(value);
        }
        else
        {
            C12 = 1 - C13;
        }

        Debug(2, "O16 = " + std::to_string(O16) + " (" + std::to_string(O16*100) + " %)");
        Debug(2, "O17 = " + std::to_string(O17) + " (" + std::to_string(O17*100) + " %)");
        Debug(2, "O18 = " + std::to_string(O18) + " (" + std::to_string(O18*100) + " %)");
        Debug(2, "C12 = " + std::to_string(C12) + " (" + std::to_string(C12*100) + " %)");
        Debug(2, "C13 = " + std::to_string(C13) + " (" + std::to_string(C13*100) + " %)");

        p_iso[0]  =    p_CO2 * O16 * C12 * O16;
        p_iso[1]  =    p_CO2 * O17 * C12 * O17;
        p_iso[2]  =    p_CO2 * O18 * C12 * O18;
        p_iso[3]  =    p_CO2 * O16 * C13 * O16;
        p_iso[4]  =    p_CO2 * O17 * C13 * O17;
        p_iso[5]  =    p_CO2 * O18 * C13 * O18;
        p_iso[6]  = 2* p_CO2 * O16 * C12 * O17;
        p_iso[7]  = 2* p_CO2 * O16 * C12 * O18;
        p_iso[8]  = 2* p_CO2 * O17 * C12 * O18;
        p_iso[9]  = 2* p_CO2 * O16 * C13 * O17;
        p_iso[10] = 2* p_CO2 * O16 * C13 * O18;
        p_iso[11] = 2* p_CO2 * O17 * C13 * O18;
        Debug(2, "Isotopic composition (calculated for statistical equilibrium):");
    }
    else
    {
        Debug(2, "Because p_CO2 is NOT specified, partial pressures of isotopologues will be");
        Debug(2, "read from the configuration (or set to zero if not specified)");

        if(YamlGetValue(&value, &yaml_content, "p_626", false))
        {
            p_iso[0] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_727", false))
        {
            p_iso[1] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_828", false))
        {
            p_iso[2] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_636", false))
        {
            p_iso[3] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_737", false))
        {
            p_iso[4] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_838", false))
        {
            p_iso[5] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_627", false))
        {
            p_iso[6] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_628", false))
        {
            p_iso[7] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_728", false))
        {
            p_iso[8] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_637", false))
        {
            p_iso[9] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_638", false))
        {
            p_iso[10] = std::stod(value);
        }

        if(YamlGetValue(&value, &yaml_content, "p_738", false))
        {
            p_iso[11] = std::stod(value);
        }

        // Total CO2 pressure, bar
        p_CO2 = 0;
        for(int is=0; is<NumIso; ++is)
        {
            p_CO2 += p_iso[is];
        }
        Debug(2, "(p_CO2 is calculated as a sum of isotopologues)");
    }

    if(!YamlGetValue(&value, &yaml_content, "p_N2"))
    {
        configuration_error = true;
        return;
    }
    p_N2 = std::stod(value);

    if(!YamlGetValue(&value, &yaml_content, "p_He"))
    {
        configuration_error = true;
        return;
    }
    p_He = std::stod(value);

    Debug(1, "Gas composition:");
    if(debug_level>=1)
    {
        std::cout << "  p_CO2 = " + std::to_string(p_CO2) + " bar\n" ;
        if(p_iso[0]>0)  std::cout << "    p_626 = " + std::to_string(p_iso[0])  + " bar\n" ;
        if(p_iso[1]>0)  std::cout << "    p_727 = " + std::to_string(p_iso[1])  + " bar\n" ;
        if(p_iso[2]>0)  std::cout << "    p_828 = " + std::to_string(p_iso[2])  + " bar\n" ;
        if(p_iso[3]>0)  std::cout << "    p_636 = " + std::to_string(p_iso[3])  + " bar\n" ;
        if(p_iso[4]>0)  std::cout << "    p_737 = " + std::to_string(p_iso[4])  + " bar\n" ;
        if(p_iso[5]>0)  std::cout << "    p_838 = " + std::to_string(p_iso[5])  + " bar\n" ;
        if(p_iso[6]>0)  std::cout << "    p_627 = " + std::to_string(p_iso[6])  + " bar\n" ;
        if(p_iso[7]>0)  std::cout << "    p_628 = " + std::to_string(p_iso[7])  + " bar\n" ;
        if(p_iso[8]>0)  std::cout << "    p_728 = " + std::to_string(p_iso[8])  + " bar\n" ;
        if(p_iso[9]>0)  std::cout << "    p_637 = " + std::to_string(p_iso[9])  + " bar\n" ;
        if(p_iso[10]>0) std::cout << "    p_638 = " + std::to_string(p_iso[10]) + " bar\n" ;
        if(p_iso[11]>0) std::cout << "    p_738 = " + std::to_string(p_iso[11]) + " bar\n" ;
        std::cout << "  p_N2 = " + std::to_string(p_N2) + " bar\n" ;
        std::cout << "  p_He = " + std::to_string(p_He) + " bar\n" ;
    }

    if(!YamlGetValue(&value, &yaml_content, "T0"))
    {
        configuration_error = true;
        return;
    }
    T0 = std::stod(value);
    Debug(2, "T0 = " + std::to_string(T0) + " K");

    if(p_CO2+p_N2+p_He <=0)
    {
        std::cout << "Total pressure in amplifier section " + this->id + " is 0. No interaction.\n";
        return;
    }


    // ------- PUMPING -------

    // interval between saving e and T data points (to be rounded to neareat whole number of time_ticks)
    save_interval = 1e-9;
    if(YamlGetValue(&value, &yaml_content, "save_interval", false))
    {
        save_interval = std::stod(value);
    }
    Debug(2, "save_interval = " + toExpString(save_interval) + " s (time interval between saving e and T data points)");
    if(save_interval <= 0)
    {
        configuration_error = true;
        std::cout << "ERROR: \'save_interval\' must be > 0\n";
        return;
    }

    // pumping type
    if(!YamlGetValue(&value, &yaml_content, "pumping"))
    {
        configuration_error = true;
        return;
    }
    pumping = value;
    Debug(1, "pumping = " + pumping);
    if(pumping != "discharge" && pumping != "optical")
    {
        configuration_error = true;
        std::cout << "ERROR: Wrong \'pumping\' parameter (must be \"discharge\" or \"optical\")\n";
        return;
    }

    //================================ DISCHARGE PUMPING ====================================
    if(pumping == "discharge")
    {
        // Time between re-solving Boltzmann equation for determining pump energy distribution
        // between solutions, linear interpolation is applied to estimate 'q' coefficients
        solve_interval = 25e-9;
        if(YamlGetValue(&value, &yaml_content, "solve_interval", false))
        {
            solve_interval = std::stod(value);
        }
        Debug(2, "solve_interval = " + toExpString(solve_interval) + " s (time interval between re-solving Boltzmann equation)");
        if(solve_interval <= 0)
        {
            configuration_error = true;
            std::cout << "ERROR: \'solve_interval\' must be > 0\n";
            return;
        }

        if(!YamlGetValue(&value, &yaml_content, "Vd"))
        {
            configuration_error = true;
            return;
        }
        Vd = std::stod(value);
        Debug(2, "Vd (discharge volume) = " + toExpString(Vd) + " m^3");

        if(!YamlGetValue(&value, &yaml_content, "D"))
        {
            configuration_error = true;
            return;
        }
        D = std::stod(value);  // m
        Debug(2, "D (inter-electrode diatance) = " + toExpString(D) + " m");

        std::vector<double> coarse_t;
        std::vector<double> coarse_U;
        std::vector<double> coarse_I;

        // Discharge profile: time(s) Current(A) Voltage(V)
        if(!YamlGetData(&coarse_t, &yaml_content, "discharge", 0)
            || !YamlGetData(&coarse_I, &yaml_content, "discharge", 1)
            || !YamlGetData(&coarse_U, &yaml_content, "discharge", 2))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Discharge profile loaded (use debug level 3 to display)");
        Debug(3, "Discharge profile [Time(s) Current(A) Voltage(V)]");
        if(debug_level >= 3)
        {
            for(size_t i=0; i<coarse_t.size(); i++)
            {
                std::cout << "  "
                          << toExpString(coarse_t[i]) <<  " "
                          << toExpString(coarse_I[i]) <<  " "
                          << toExpString(coarse_U[i])
                          << std::endl;
            }
        }

        current.resize(m0);
        voltage.resize(m0);

        for(int m=0; m<m0; ++m)
        {
            current[m] = Interpolate(&coarse_t, &coarse_I, time_tick*(0.5+m));
            voltage[m] = Interpolate(&coarse_t, &coarse_U, time_tick*(0.5+m));
        }

        // populate q arrays (includes solving Boltzmann equations)
        // solve Botzman on coarse time grid first, then interpolate
        int n_ticks = std::llround(solve_interval/time_tick); // number of time_ticks between solves
        if(n_ticks<1)
            n_ticks = 1;
        int n_solves = m0/n_ticks;
        // make sure that solution at m = m0-1 is included
        // Examples mor m0=8 (ensure solution at m=7)
        // n_ticks=1 => n_solves=8 (solve at m = 0, 1, 2, 3, 4, 5, 6, 7)
        // n_ticks=2 => n_solves=5 (solve at m = 0, 2, 4, 6, 7)
        // n_ticks=3 => n_solves=4 (solve at m = 0, 3, 6, 7)
        while(n_ticks*(n_solves-1) < m0-1)
        {
            if(n_solves==m0) // n_ticks==1
                break;
            n_solves++;
        }

        /*int n_solves = m0;             // e.g., m0=8; n_ticks=1 => n_solves=8 (solve at m = 0, 1, 2, 3, 4, 5, 6, 7)
        if(n_ticks>1)
        {
            n_solves = m0/n_ticks + 1; // e.g., m0=8; n_ticks=2 => n_solves=5 (solve at m = 0, 2, 4, 6, 7)
                                       // !!! solution at m = m0-1 (7 in this example) must be present for covering whole time range !!!
            if(m0 % n_ticks > 0)
                n_solves += 1;         // e.g., m0=8; n_ticks=3 => n_solves=4 (solve at m = 0, 3, 6, 7)
        }*/

        std::vector<double> coarse_q2(n_solves);
        std::vector<double> coarse_q3(n_solves);
        std::vector<double> coarse_q4(n_solves);
        std::vector<double> coarse_qT(n_solves);
        coarse_t.resize(n_solves);

        int count = 0;

        #pragma omp parallel for
        for(int i=0; i<n_solves; ++i)
        {
            #pragma omp critical
            {
                StatusDisplay(nullptr, nullptr, -1, this->id +
                              ": solving Boltzmann equations: " + std::to_string(++count) + " of " + std::to_string(n_solves));
            }

            //std::vector<std::vector<double>> M(b0, std::vector<double>(b0, 0.0)); // zero-fill

            int m = i * n_ticks;
            if(m>m0-1)
                m=m0-1;
            coarse_t[i] = time_tick*(0.5+m);
            double q[5] = {};
            Boltzmann(m, q);
            coarse_qT[i] = q[1];
            coarse_q2[i] = q[2];
            coarse_q3[i] = q[3];
            coarse_q4[i] = q[4];
        }

        q2.resize(m0);
        q3.resize(m0);
        q4.resize(m0);
        qT.resize(m0);

        for(int m=0; m<m0; ++m)
        {
            q2[m] = Interpolate(&coarse_t, &coarse_q2, time_tick*(0.5+m));
            q3[m] = Interpolate(&coarse_t, &coarse_q3, time_tick*(0.5+m));
            q4[m] = Interpolate(&coarse_t, &coarse_q4, time_tick*(0.5+m));
            qT[m] = Interpolate(&coarse_t, &coarse_qT, time_tick*(0.5+m));
        }
    }

    //================================= OPTICAL PUMPING =====================================
    if(pumping == "optical")
    {
        //------------------------ Basic optical pumping parameters -------------------------
        if(!YamlGetValue(&value, &yaml_content, "pump_level"))
        {
            configuration_error = true;
            return;
        }
        pump_level = value;
        Debug(1, "pump_level = " + pump_level);
        if(pump_level != "001" && pump_level != "021" && pump_level != "041" && pump_level != "002" && pump_level != "003")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'pump_level\' parameter (must be \"001\", \"021\", \"041\", \"002\", or \"003\")\n";
            return;
        }

        if(!YamlGetValue(&value, &yaml_content, "pump_wl"))
        {
            configuration_error = true;
            return;
        }
        pump_wl = std::stod(value); // m
        Debug(2, "pump_wl (optical pumping wavelength) = " + toExpString(pump_wl) + " m");

        if(!YamlGetValue(&value, &yaml_content, "pump_sigma"))
        {
            configuration_error = true;
            return;
        }
        pump_sigma = std::stod(value); // m^2
        Debug(2, "pump_sigma (optical pumping absorption cross-section) = " + toExpString(pump_sigma) + " m^2");

        if(!YamlGetValue(&value, &yaml_content, "pump_E"))
        {
            configuration_error = true;
            return;
        }
        double pump_E = std::stod(value);
        Debug(2, "pump_E = " + toExpString(pump_E) + " J");

        //--------------------------- Pump beam spatial profile -----------------------------
        if(!YamlGetValue(&value, &yaml_content, "pump_beam"))
        {
            configuration_error = true;
            return;
        }
        std::string pump_beam = value;
        Debug(2, "pump_beam = " + pump_beam);

        fluence.resize(x0);
        if(pump_beam == "GAUSS" || pump_beam == "SUPERGAUSS4" || pump_beam == "SUPERGAUSS6"
            || pump_beam == "SUPERGAUSS8" || pump_beam == "SUPERGAUSS10" || pump_beam == "FLATTOP")
        {
            if(!YamlGetValue(&value, &yaml_content, "w"))
            {
                configuration_error = true;
                return;
            }
            double w0 = std::stod(value);
            Debug(2, "w = " + toExpString(w0) + " m");
            if(pump_beam == "GAUSS")
                for(int x=0; x<x0; ++x)
                    fluence[x] = exp(-2*pow(Dr*(0.5+x)/w0, 2));
            if(pump_beam == "SUPERGAUSS4")
                for(int x=0; x<x0; ++x)
                    fluence[x] = exp(-2*pow(Dr*(0.5+x)/w0, 4));
            if(pump_beam == "SUPERGAUSS6")
                for(int x=0; x<x0; ++x)
                    fluence[x] = exp(-2*pow(Dr*(0.5+x)/w0, 6));
            if(pump_beam == "SUPERGAUSS8")
                for(int x=0; x<x0; ++x)
                    fluence[x] = exp(-2*pow(Dr*(0.5+x)/w0, 8));
            if(pump_beam == "SUPERGAUSS10")
                for(int x=0; x<x0; ++x)
                    fluence[x] = exp(-2*pow(Dr*(0.5+x)/w0, 10));
            if(pump_beam == "FLATTOP")
                for(int x=0; x<x0; ++x)
                    Dr*(0.5+x)<=w0 ? fluence[x]=1 : fluence[x]=0;
        }
        else if(pump_beam == "FREEFORM")
        {
            std::vector<double> r;
            std::vector<double> A;
            if(!YamlGetData(&r, &yaml_content, "beam_form", 0) || !YamlGetData(&A, &yaml_content, "beam_form", 1))
            {
                configuration_error = true;
                return;
            }
            Debug(2, "Beam profile loaded (use debug level 3 to display)");
            Debug(3, "Beam profile [r(m) amplitude(a.u)]");
            if(debug_level >= 3)
                for(size_t i=0; i<r.size(); i++)
                    std::cout << toExpString(r[i]) <<  " " << toExpString(A[i]) << std::endl;
            for(int x=0; x<x0; ++x)
                fluence[x] = Interpolate(&r, &A, Dr*(0.5+x));
        }
        else
        {
            std::cout << "ERROR: wrong \'pump_beam\' in config file \'" << yaml_path << "\'" << std::endl;
            configuration_error = true;
            return;
        }

        // convert beam profile to absolute fluence
        double E_au = 0; // beam energy for given profile in arbitraty units
        for(int x=0; x<x0; ++x)
            E_au += fluence[x] * M_PI*pow(Dr,2)*(2*x+1); //ring area = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
        for(int x=0; x<x0; ++x)
            fluence[x] *= pump_E/E_au;



        //-------------------------- Pump pulse temporal profile ----------------------------
        if(!YamlGetValue(&value, &yaml_content, "pump_pulse"))
        {
            configuration_error = true;
            return;
        }
        std::string pump_pulse = value;
        Debug(2, "pump_pulse = " + pump_pulse);

        normalized_intensity.resize(m0);

        double pump_pulse_integral = 0;
        if(pump_pulse == "GAUSS" || pump_pulse == "FLATTOP")
        {
            if(!YamlGetValue(&value, &yaml_content, "t0"))
            {
                configuration_error = true;
                return;
            }
            double t0 = std::stod(value);
            Debug(2, "t0 = " + toExpString(t0) + " s");

            if(!YamlGetValue(&value, &yaml_content, "fwhm"))
            {
                configuration_error = true;
                return;
            }
            double fwhm = std::stod(value);
            Debug(2, "fwhm = " + toExpString(fwhm) + " s");

            if(pump_pulse == "GAUSS")
            {
                double tau = fwhm/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
                pump_pulse_integral = tau*sqrt(M_PI/2);
                for(int m=0; m<m0; ++m)
                    normalized_intensity[m] = exp(-2*pow((time_tick*(0.5+m)-t0)/tau, 2));

            }
            if(pump_pulse == "FLATTOP")
            {
                pump_pulse_integral = fwhm;
                for(int m=0; m<m0; ++m)
                {
                    if( t0-fwhm/2 <= time_tick*(0.5+m) || time_tick*(0.5+m) <= t0+fwhm/2)
                        normalized_intensity[m] = 1;
                    else
                        normalized_intensity[m] = 0;
                }
            }
        }
        else if(pump_pulse == "FREEFORM")
        {
            std::vector<double> t;
            std::vector<double> A;
            if(!YamlGetData(&t, &yaml_content, "pulse_form", 0) || !YamlGetData(&A, &yaml_content, "pulse_form", 1))
            {
                configuration_error = true;
                return;
            }
            Debug(2, "Pulse profile loaded (use debug level 3 to display)");
            Debug(3, "Pulse profile [t(s) amplitude(a.u)]");
            if(debug_level >= 3)
            {
                for(size_t m=0; m<t.size(); m++)
                    std::cout << toExpString(t[m]) <<  " " << toExpString(A[m]) << std::endl;
            }

            for(int m=0; m<m0; ++m)
                normalized_intensity[m] = Interpolate(&t, &A, time_tick*(0.5+m));

            for(int m=0; m<m0; ++m)
                pump_pulse_integral += normalized_intensity[m] * time_tick;
        }
        else
        {
            std::cout << "ERROR: wrong \'pump_pulse\' in config file \'" << yaml_path << "\'" << std::endl;
            configuration_error = true;
            return;
        }

        // normalize intensity profile so that normalized_intensity[t] * fluence[x] gives absloute intensity in point x at time t
        if(pump_pulse_integral != 0)
        {
            for(int m=0; m<m0; ++m)
                normalized_intensity[m] /= pump_pulse_integral;
        }

    }


    // ------- BANDS TO CONSIDER -------

    // defaults
    band_reg = true;
    band_seq = true;
    band_hot = true;
    band_4um = false;

    if(YamlGetValue(&value, &yaml_content, "band_reg", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_reg\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_reg = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, &yaml_content, "band_seq", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_seq\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_seq = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, &yaml_content, "band_hot", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_hot\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_hot = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, &yaml_content, "band_4um", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_4um\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_4um = value=="true" ? true : false;
    }

    Debug(1, "Bands to consider:");
    if(debug_level>=1)
    {
        std::string truefalse;
        truefalse = (band_reg ? +"true" : +"false");
        std::cout << "  band_reg: " + truefalse + "\n" ;
        truefalse = (band_seq ? +"true" : +"false");
        std::cout << "  band_seq: " + truefalse + "\n" ;
        truefalse = (band_hot ? +"true" : +"false");
        std::cout << "  band_hot: " + truefalse + "\n" ;
        truefalse = (band_4um ? +"true" : +"false");
        std::cout << "  band_4um: " + truefalse + "\n" ;
        std::cout << std::flush;
    }

    // ------- MISC INITIALISATIONS -------

    // Convert pressures to number densities where needed
    N_CO2 = 2.7e25*p_CO2;
    for(int is=0; is<NumIso; ++is)
    {
        N_iso[is] = 2.7e25*p_iso[is];
    }

    // allocate memory
    e2.resize(x0);
    e3.resize(x0);
    e4.resize(x0);
    T.resize(x0);

    for(int is=0; is<NumIso; ++is) // isotopologues
    {
        for(int gr=0; gr<NumGrp; ++gr) // groups of vib. levels
        {
            N_grp[is][gr].resize(x0);
        }

        for(int vl=0; vl<NumVib; ++vl) // vibrational levels
        {
            N_vib[is][vl].resize(x0);
            for(int j=0; j<NumRot; ++j) // rotational levels
            {
                N_rot[is][vl][j].resize(x0);
            }

        }
    }

    gainSpectrum.resize(n0);

    // Fill out spectroscoic arrays &
    AmplificationBand();

    // Create polarization arrays (must be called after transitions are counted in AmplificationBand()
    for(int is=0; is<NumIso; ++is)
    {
        int n_tr = v[is].size();
        rho[is].resize(n_tr*x0);
    }

    // zero-out all rho arrays
    for (auto& v : rho)
    {
        std::fill(v.begin(), v.end(), 0.0);
    }

    // Populations and field initialization
    InitializePopulations();

    // No pulse interacting with the amplifier yet (thus, no need to concider pump energy distribution betweenrotational levels)
    flag_interaction = false;

    Debug(2, "Writing pumping files...");
    WritePumpingFiles();
    Debug(2, "Writing pumping files done");
}



void A::WritePumpingFiles()
{
    FILE *file;

    if(pumping == "optical")
    {
        // Write fluence file
        file = fopen((id+"_pumping_fluence.dat").c_str(), "w");
        fprintf(file, "#Data format: r[m] fluence[J/m^2]\n");
        for(int x=0; x<x0; ++x)
        {
            fprintf(file, "%e\t%e\n", Dr*(0.5+x), fluence[x]);
        }
        fclose(file);

        // Write power file
        file = fopen((id+"_pumping_power.dat").c_str(), "w");
        fprintf(file, "#Data format:  time[s] power[W]\n");
        double energy = 0;
        for(int x=0; x<x0; ++x)
        {
            energy += fluence[x] *  M_PI*pow(Dr,2)*(2*x+1); //ring area = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
        }
        for(int m=0; m<m0; ++m)
        {
            double power = energy * normalized_intensity[m];
            fprintf(file, "%.8E\t%e\n", time_tick*(0.5+m), power);
        }
        fclose(file);
    }

    if(pumping == "discharge")
    {
        // Write discharge (current & voltage) file
        file = fopen((id+"_discharge.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] current[A] voltage[V]\n");
        for(int m=0; m<m0; ++m)
        {
            fprintf(file, "%e\t%e\t%e\n", time_tick*(0.5+m), current[m], voltage[m]);
        }
        fclose(file);

        // Write q (pump energy distribution) file
        file = fopen((id+"_q.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] q2 q3 q4\n");
        for(int m=0; m<m0; ++m)
        {
            fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time_tick*(0.5+m), q2[m], q3[m], q4[m], qT[m]);
        }
        fclose(file);
    }
}
