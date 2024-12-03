#include "co2amp.h"


A::A(std::string id)
{
    this->id = id;
    type = "A";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max (semiDia)
    if(!YamlGetValue(&value, yaml, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");

    // Length (L)
    if(!YamlGetValue(&value, yaml, "L"))
    {
        configuration_error = true;
        return;
    }
    length = std::stod(value);
    Debug(2, "L = " + toExpString(std::stod(value)) + " m");


    // ------- PUMPING -------
    // pumping type (must be "discharge" or "optical")
    if(!YamlGetValue(&value, yaml, "pumping"))
    {
        configuration_error = true;
        return;
    }
    pumping = value;
    Debug(2, "pumping = " + pumping);
    if(pumping != "discharge" && pumping != "optical")
    {
        configuration_error = true;
        std::cout << "ERROR: Wrong \'pumping\' parameter (must be \"discharge\" or \"optical\")\n";
        return;
    }

    if(pumping == "discharge")
    {
        if(!YamlGetValue(&value, yaml, "Vd"))
        {
            configuration_error = true;
            return;
        }
        Vd = std::stod(value);
        Debug(2, "Vd (discharge volume) = " + toExpString(Vd) + " m^3");

        if(!YamlGetValue(&value, yaml, "D"))
        {
            configuration_error = true;
            return;
        }
        D = std::stod(value);  // m
        Debug(2, "D (inter-electrode diatance) = " + toExpString(D) + " m");

        // Discharge profile: time(s) Current(A) Voltage(V)
        if(!YamlGetData(&discharge_time, yaml, "discharge", 0)
                || !YamlGetData(&discharge_current, yaml, "discharge", 1)
                || !YamlGetData(&discharge_voltage, yaml, "discharge", 2))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Discharge profile [Time(s) Current(A) Voltage(V)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<discharge_time.size(); i++)
                std::cout << toExpString(discharge_time[i]) <<  " "
                << toExpString(discharge_current[i]) <<  " "
                << toExpString(discharge_voltage[i])
                << std::endl;
    }

    if(pumping == "optical")
    {
        if(!YamlGetValue(&value, yaml, "pump_wl"))
        {
            configuration_error = true;
            return;
        }
        pump_wl = std::stod(value); // m
        Debug(2, "pump_wl (wavelength) = " + toExpString(pump_wl) + " m");

        if(!YamlGetValue(&value, yaml, "pump_sigma"))
        {
            configuration_error = true;
            return;
        }
        pump_sigma = std::stod(value); // m^2
        Debug(2, "pump_sigma (abs. cross-section) = " + toExpString(pump_wl) + " m^2");


        // Pumping pulse profile: time(s) Intensity(W/m^2)
        if(!YamlGetData(&pumping_pulse_time, yaml, "pumping_pulse", 0)
                || !YamlGetData(&pumping_pulse_intensity, yaml, "pumping_pulse", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Pumping pulse profile [Time(s) Intensity(W/m^2))]");
        if(debug_level >= 2)
        {
            for(int i=0; i<pumping_pulse_time.size(); i++)
            {
                std::cout << toExpString(pumping_pulse_time[i]) <<  " "
                << toExpString(pumping_pulse_intensity[i]) << std::endl;
            }
        }
    }

    // ------- GAS MIXTURE -------

    // defaults
    for(int i=0; i<12; i++)
    {
        p_iso[i] = 0;
    }
    double O16 = 1; // Oxygen-16 content (0..1)
    double O17 = 0; // Oxygen-17 content (0..1)
    double O18 = 0; // Oxygen-18 content (0..1)
    double C12 = 1; // Carbon-12 content (0..1)
    double C13 = 0; // Carbon-13 content (0..1)

    if(YamlGetValue(&value, yaml, "p_CO2", false))
    {
        p_CO2 = std::stod(value);
        Debug(2, "p_CO2 = " + std::to_string(p_CO2) + " bar");
        Debug(2, "Because p_CO2 is specified, partial pressures of isotopologues will be");
        Debug(2, "calculated at statistical equilibrium for specified isotope content");

        // Check if content of rare isotopes provided
        if(YamlGetValue(&value, yaml, "O17", false))
        {
            O17 = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "O18", false))
        {
            O18 = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "C13", false))
        {
            C13 = std::stod(value);
        }

        // Check if content of natural isotopes provided. If not - calculate as a balance
        if(YamlGetValue(&value, yaml, "O16", false))
        {
            O16 = std::stod(value);
        }
        else
        {
            O16 = 1 - (O17 + O18);
        }

        if(YamlGetValue(&value, yaml, "C12", false))
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

        if(YamlGetValue(&value, yaml, "p_626", false))
        {
            p_iso[0] = std::stod(value);
        }
        if(YamlGetValue(&value, yaml, "p_727", false))
        {
            p_iso[1] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_828", false))
        {
            p_iso[2] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_636", false))
        {
            p_iso[3] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_737", false))
        {
            p_iso[4] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_838", false))
        {
            p_iso[5] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_627", false))
        {
            p_iso[6] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_628", false))
        {
            p_iso[7] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_728", false))
        {
            p_iso[8] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_637", false))
        {
            p_iso[9] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_638", false))
        {
            p_iso[10] = std::stod(value);
        }

        if(YamlGetValue(&value, yaml, "p_738", false))
        {
            p_iso[11] = std::stod(value);
        }

        // Total CO2 pressure, bar
        p_CO2 = 0;
        for(int i=0; i<12; ++i)
        {
            p_CO2 += p_iso[i];
        }
        Debug(2, "(p_CO2 is calculated as a sum of isotopologues)");
    }

    if(!YamlGetValue(&value, yaml, "p_N2"))
    {
        configuration_error = true;
        return;
    }
    p_N2 = std::stod(value);

    if(!YamlGetValue(&value, yaml, "p_He"))
    {
        configuration_error = true;
        return;
    }
    p_He = std::stod(value);

    Debug(1, id + " gas composition:");
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

    if(!YamlGetValue(&value, yaml, "T0"))
    {
        configuration_error = true;
        return;
    }
    T0 = std::stod(value);
    Debug(1, "T0 = " + std::to_string(T0) + " K");

    if(p_CO2+p_N2+p_He <=0)
    {
        std::cout << "Total pressure in amplifier section " + this->id + " is 0. No interaction.\n";
        return;
    }

    // ------- BANDS TO CONSIDER -------

    // defaults
    band_reg = true;
    band_seq = true;
    band_hot = true;
    band_4um = false;

    if(YamlGetValue(&value, yaml, "band_reg", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_reg\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_reg = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, yaml, "band_seq", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_seq\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_seq = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, yaml, "band_hot", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_hot\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_hot = value=="true" ? true : false;
    }

    if(YamlGetValue(&value, yaml, "band_4um", false))
    {
        if(value!="true" && value!="false")
        {
            configuration_error = true;
            std::cout << "ERROR: Wrong \'band_4um\' parameter (must be \"true\" or \"false\")\n";
            return;
        }
        band_4um = value=="true" ? true : false;
    }

    Debug(1, id + " bands to consider:");
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
    }

    // ------- MISC INITIALISATIONS -------

    // Convert pressures to number densities where needed
    N_CO2 = 2.7e25*p_CO2;
    for(int i=0; i<12; ++i)
    {
        N_iso[i] = 2.7e25*p_iso[i];
    }

    // allocate memory
    e2 = new double [x0];
    e3 = new double [x0];
    e4 = new double [x0];
    T  = new double [x0];

    for(int i=0; i<12; ++i) // isotopologues
    {
        for(int gr=0; gr<6; ++gr) // groups of vib. levels
        {
            N_gr[i][gr] = new double[x0];
        }
    }
    gainSpectrum  = new double [n0];


    // Fill out spectroscoic arrays &
    AmplificationBand();

    // Populations and field initialization
    InitializePopulations();

    // Initial q's
    if(pumping == "discharge")
    {
        Debug(2, "Initializing q's");
        Boltzmann(0); // initialize q's
        q2_b = q2;
        q3_b = q3;
        q4_b = q4;
        qT_b = qT;
        time_b = 0;
    }



}
