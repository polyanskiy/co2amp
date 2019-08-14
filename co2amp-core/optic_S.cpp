#include "co2amp.h"


S::S(std::string id)
{
    this->id = id;
    type = "S";
    yaml = id + ".yml";
    double nu_lo, nu_hi;
    double Dt = (t_max-t_min)/(n0-1); // pulse time step, s
    double Dv = 1.0/(Dt*n0);          // frequency step, Hz
    double v_min = vc - Dv*(n0-1)/2;

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // type
    if(!YamlGetValue(&value, yaml, "type")){
        configuration_error = true;
        return;
    }
    if(value != type){
        std::cout << "ERROR: wrong \'type\' in config file \'" << yaml
                  << "\' (must be \'" << type << "\')" << std::endl;
        configuration_error = true;
        return;
    }

    // Rmax, Dr
    if(!YamlGetValue(&value, yaml, "Rmax")){
        configuration_error = true;
        return;
    }
    Debug(2, "Rmax = " + toExpString(std::stod(value)) + " m");
    Dr = std::stod(value)/(x0-1);

    // Kind
    if(!YamlGetValue(&value, yaml, "kind")){
        configuration_error = true;
        return;
    }
    std::string kind = value;

    Transmittance = new double[n0];

    if(kind == "HIGHPASS"){
        if(!YamlGetValue(&value, yaml, "nu_hi")){
            configuration_error = true;
            return;
        }
        nu_hi = std::stod(value);
        Debug(2, "nu_hi = " + toExpString(nu_hi) + " Hz");
        for(int n=0; n<n0; n++)
            if((v_min+Dv*n) >= nu_hi)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        return;
    }

    if(kind == "LOWPASS"){
        if(!YamlGetValue(&value, yaml, "nu_lo")){
            configuration_error = true;
            return;
        }
        nu_lo = std::stod(value);
        Debug(2, "nu_lo = " + toExpString(nu_lo) + " Hz");
        for(int n=0; n<n0; n++)
            if((v_min+Dv*n) < nu_lo)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        return;
    }

    if(kind == "BANDPASS"){
        if(!YamlGetValue(&value, yaml, "nu_lo")){
            configuration_error = true;
            return;
        }
        nu_lo = std::stod(value);
        Debug(2, "nu_lo = " + toExpString(nu_lo) + " Hz");

        if(!YamlGetValue(&value, yaml, "nu_hi")){
            configuration_error = true;
            return;
        }
        nu_hi = std::stod(value);
        Debug(2, "nu_hi = " + toExpString(nu_hi) + " Hz");

        for(int n=0; n<n0; n++)
            if((v_min+Dv*n)>=nu_lo && (v_min+Dv*n)<=nu_hi)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        return;
    }


    if(kind == "FREEFORM"){
        std::vector<double> nu;
        std::vector<double> transm;
        if(!YamlGetData(&nu, yaml, "profile", 0) || !YamlGetData(&transm, yaml, "profile", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile:");
        if(debug_level >= 2)
            for(int i=0; i<nu.size(); i++)
                std::cout << toExpString(nu[i]) <<  " " << toExpString(transm[i]) << std::endl;
        for(int n=0; n<n0; n++)
                Transmittance[n] = Interpolate(&nu, &transm, v_min+Dv*n);
        return;
    }

    // not a supproted "kind"
    std::cout << "ERROR: wrong \'kind\' in config file \'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void S::InternalDynamics(double)
{

}


void S::PulseInteraction(int pulse_n)
{

}
