#include "co2amp.h"


A::A(std::string id)
{
    this->id = id;
    type = "A";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // Rmax
    if(!YamlGetValue(&value, yaml, "Rmax")){
        configuration_error = true;
        return;
    }
    double Rmax = std::stod(value);
    Debug(2, "Rmax = " + toExpString(Rmax) + " m");
    Dr = Rmax/x0;

    // Length
    if(!YamlGetValue(&value, yaml, "length")){
        configuration_error = true;
        return;
    }
    length = std::stod(value);
    Debug(2, "length = " + toExpString(std::stod(value)) + " m");


    // ------- PUMPING -------
    // pumping type (must be "discharge" or "optical")
    if(!YamlGetValue(&value, yaml, "pumping")){
        configuration_error = true;
        return;
    }
    pumping = value;
    Debug(2, "pumping = " + pumping);
    if(pumping != "discharge" && pumping != "optical"){
        configuration_error = true;
        std::cout << "ERROR: Wrong \'pumping\' parameter (must be \"discharge\" or \"optical\")\n";
        return;
    }

    if(pumping == "discharge"){
        if(!YamlGetValue(&value, yaml, "Vd")){
            configuration_error = true;
            return;
        }
        Vd = std::stod(value);
        Debug(2, "Vd (discharge volume) = " + toExpString(Vd) + " m^3");

        if(!YamlGetValue(&value, yaml, "D")){
            configuration_error = true;
            return;
        }
        D = std::stod(value);  // m
        Debug(2, "D (inter-electrode diatance) = " + toExpString(D) + " m");

        // Discharge profile: time(s) Current(A) Voltage(V)
        if(!YamlGetData(&discharge_time, yaml, "discharge", 0)
                || !YamlGetData(&discharge_current, yaml, "discharge", 1)
                || !YamlGetData(&discharge_voltage, yaml, "discharge", 2)){
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

    if(pumping == "optical"){
        if(!YamlGetValue(&value, yaml, "pump_wl")){
            configuration_error = true;
            return;
        }
        pump_wl = std::stod(value); // m
        Debug(2, "pump_wl (wavelength) = " + toExpString(pump_wl) + " m");

        if(!YamlGetValue(&value, yaml, "pump_sigma")){
            configuration_error = true;
            return;
        }
        pump_sigma = std::stod(value); // m^2
        Debug(2, "pump_sigma (abs. cross-section) = " + toExpString(pump_wl) + " m^2");

        if(!YamlGetValue(&value, yaml, "pump_fluence")){
            configuration_error = true;
            return;
        }
        pump_fluence = std::stod(value); // J/m^2
    }

    // ------- GAS MIXTURE -------
    p_626 = 0;
    p_628 = 0;
    p_828 = 0;
    p_636 = 0;
    p_638 = 0;
    p_838 = 0;
    p_CO2 = 0;
    double O18 = 0; // Oxygen-18 content (0..1)
    double C13 = 0; // Carbon-13 content (0..1)

    if(YamlGetValue(&value, yaml, "p_CO2")){
        p_CO2 = std::stod(value);
        Debug(2, "p_CO2 = " + std::to_string(p_CO2) + " bar");

        if(!YamlGetValue(&value, yaml, "O18")){
            configuration_error = true;
            return;
        }
        O18 = std::stod(value);
        Debug(2, "O18 = " + std::to_string(O18) + " (" + std::to_string(O18*100) + " %)");

        if(!YamlGetValue(&value, yaml, "C13")){
            configuration_error = true;
            return;
        }
        C13 = std::stod(value);
        Debug(2, "C13 = " + std::to_string(C13) + " (" + std::to_string(C13*100) + " %)");

        p_626 = p_CO2 * pow(1-O18,2) * (1-C13);
        p_628 = p_CO2 * 2*(1-O18)*O18 * (1-C13);
        p_828 = p_CO2 * pow(O18,2) * (1-C13);
        p_636 = p_CO2 * pow(1-O18,2) * C13;
        p_638 = p_CO2 * 2*(1-O18)*O18 * C13;
        p_838 = p_CO2 * pow(O18,2) * C13;
        Debug(2, "Isotopic composition (calculated for statistical equilibrium):");
    }
    else{
        std::cout << "Let's check if pressures of six CO2 isotopologues provided...\n";

        if(!YamlGetValue(&value, yaml, "p_626")){
            configuration_error = true;
            return;
        }
        p_626 = std::stod(value);

        if(!YamlGetValue(&value, yaml, "p_628")){
            configuration_error = true;
            return;
        }
        p_628 = std::stod(value);

        if(!YamlGetValue(&value, yaml, "p_828")){
            configuration_error = true;
            return;
        }
        p_828 = std::stod(value);

        if(!YamlGetValue(&value, yaml, "p_636")){
            configuration_error = true;
            return;
        }
        p_636 = std::stod(value);

        if(!YamlGetValue(&value, yaml, "p_638")){
            configuration_error = true;
            return;
        }
        p_638 = std::stod(value);

        if(!YamlGetValue(&value, yaml, "p_838")){
            configuration_error = true;
            return;
        }
        p_838 = std::stod(value);

        // Total CO2 pressure, bar
        p_CO2 = p_626+p_628+p_828+p_636+p_638+p_838;
        Debug(2, "p_CO2 = " + std::to_string(p_CO2) + " bar");
        Debug(2, "(Calculated as a sum of isotopologues):");
    }

    Debug(2, "p_626 = " + std::to_string(p_626) + " bar");
    Debug(2, "p_628 = " + std::to_string(p_628) + " bar");
    Debug(2, "p_626 = " + std::to_string(p_828) + " bar");
    Debug(2, "p_636 = " + std::to_string(p_636) + " bar");
    Debug(2, "p_638 = " + std::to_string(p_638) + " bar");
    Debug(2, "p_636 = " + std::to_string(p_838) + " bar");

    if(!YamlGetValue(&value, yaml, "p_N2")){
        configuration_error = true;
        return;
    }
    p_N2 = std::stod(value);
    Debug(2, "p_N2 = " + std::to_string(p_N2) + " bar");

    if(!YamlGetValue(&value, yaml, "p_He")){
        configuration_error = true;
        return;
    }
    p_He = std::stod(value);
    Debug(2, "p_He = " + std::to_string(p_He) + " bar");

    if(!YamlGetValue(&value, yaml, "T0")){
        configuration_error = true;
        return;
    }
    T0 = std::stod(value);
    Debug(2, "T0 = " + std::to_string(T0) + " K");

    if(p_CO2+p_N2+p_He <=0){
        std::cout << "Total pressure in amplifier section " + this->id + " is 0. No interaction.\n";
        return;
    }

    // ------- BANDS TO CONSIDER -------
    band_reg = true;
    band_seq = true;
    band_hot = true;

    if(!YamlGetValue(&value, yaml, "band_reg")){
        configuration_error = true;
        return;
    }
    if(value!="true" && value!="false"){
        configuration_error = true;
        std::cout << "ERROR: Wrong \'band_reg\' parameter (must be \"true\" or \"false\")\n";
        return;
    }
    if(value=="false")
        band_reg = false;

    if(!YamlGetValue(&value, yaml, "band_seq")){
        configuration_error = true;
        return;
    }
    if(value!="true" && value!="false"){
        configuration_error = true;
        std::cout << "ERROR: Wrong \'band_seq\' parameter (must be \"true\" or \"false\")\n";
        return;
    }
    if(value=="false")
        band_seq = false;

    if(!YamlGetValue(&value, yaml, "band_hot")){
        configuration_error = true;
        return;
    }
    if(value!="true" && value!="false"){
        configuration_error = true;
        std::cout << "ERROR: Wrong \'band_hot\' parameter (must be \"true\" or \"false\")\n";
        return;
    }
    if(value=="false")
        band_hot = false;

    Debug(2, "Bands included in calculations:");
    std::string truefalse;
    truefalse = (band_reg ? +"true" : +"false");
    Debug(2, "band_reg: " + truefalse);
    truefalse = (band_seq ? +"true" : +"false");
    Debug(2, "band_seq: " + truefalse);
    truefalse = (band_hot ? +"true" : +"false");
    Debug(2, "band_hot: " + truefalse);

    // ------- MISC INITIALISATIONS -------
    // allocate memory
    e2 = new double [x0];
    e3 = new double [x0];
    e4 = new double [x0];
    T  = new double [x0];
    gainSpectrum  = new double [n0];

    // 1 Fill out spectroscoic arrays &
    AmplificationBand();

    // 2 Populations and field initialization
    InitializePopulations();

    // 3 Initial q's
    if(pumping == "discharge"){
        Debug(2, "Initializing q's");
        Boltzmann(0);
        q2_b = q2;
        q3_b = q3;
        q4_b = q4;
        qT_b = qT;
        time_b = 0;
    }

}
