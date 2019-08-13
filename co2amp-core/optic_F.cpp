#include "co2amp.h"


F::F(std::string id)
{  
    this->id = id;
    type = "F";
    yaml = id + ".yml";

    double Rmin, Rmax, T, w;

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
    Rmax = std::stod(value);
    Debug(2, "Rmax = " + toExpString(Rmax) + " m");
    Dr = Rmax/(x0-1);

    // Kind
    if(!YamlGetValue(&value, yaml, "kind")){
        configuration_error = true;
        return;
    }
    std::string kind = value;

    Transmittance = new double[x0];

    if(kind == "ND"){
        if(!YamlGetValue(&value, yaml, "T")){
            configuration_error = true;
            return;
        }
        T = std::stod(value);
        Debug(2, "T = " + toExpString(T));
        for(int x=0; x<x0; x++)
            Transmittance[x] = T;
        return;
    }

    if(kind == "MASK"){
        if(!YamlGetValue(&value, yaml, "Rmin")){
            configuration_error = true;
            return;
        }
        Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*x <= Rmin)
                Transmittance[x] = 0;
            else
                Transmittance[x] = 1;
        }
        return;
    }

    if(kind == "SIN"){
        if(!YamlGetValue(&value, yaml, "Rmin")){
            configuration_error = true;
            return;
        }
        Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*x <= Rmin)
                Transmittance[x] = 1;
            else
                Transmittance[x] = pow(sin(M_PI*(Rmax-Dr*x)/(2.0*(Rmax-Rmin))),2);
        }
        return;
    }

    if(kind == "GAUSS"){
        if(!YamlGetValue(&value, yaml, "Rmin")){
            configuration_error = true;
            return;
        }
        Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        if(!YamlGetValue(&value, yaml, "w")){
            configuration_error = true;
            return;
        }
        w = std::stod(value);
        Debug(2, "w = " + toExpString(w) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*x <= Rmin)
                Transmittance[x] = 1;
            else
                Transmittance[x] = exp(-2.0*pow((Dr*x-Rmin)/w,2));
        }
        return;
    }


    if(kind == "FREEFORM"){
        std::vector<double> pos;
        std::vector<double> transm;
        if(!YamlGetData(&pos, yaml, "profile", 0) || !YamlGetData(&transm, yaml, "profile", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile:");
        if(debug_level >= 2)
            for(int i=0; i<pos.size(); i++)
                std::cout << toExpString(pos[i]) <<  " " << toExpString(transm[i]) << std::endl;
        for(int x=0; x<x0; x++)
                Transmittance[x] = Interpolate(&pos, &transm, Dr*x);
        return;
    }

    // not a supproted "kind"
    std::cout << "ERROR: wrong \'kind\' in config file \'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void F::InternalDynamics(double clock_time)
{

}


void F::PulseInteraction(int pulse_n)
{
    Debug(2, "Interaction with spatial filter");
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++)
            pulses[pulse_n]->E[x][n] *=  sqrt(Transmittance[x]);
    }
}

