#include "co2amp.h"


F::F(std::string id)
{  
    this->id = id;
    type = "F";
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
        double T = std::stod(value);
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
        double Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*(0.5+x) <= Rmin)
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
        double Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*(0.5+x) <= Rmin)
                Transmittance[x] = 1;
            else
                Transmittance[x] = pow(sin(M_PI*(Rmax-Dr*(0.5+x))/(2.0*(Rmax-Rmin))),2);
        }
        return;
    }

    if(kind == "GAUSS"){
        if(!YamlGetValue(&value, yaml, "Rmin")){
            configuration_error = true;
            return;
        }
        double Rmin = std::stod(value);
        Debug(2, "Rmin = " + toExpString(Rmin) + " m");
        if(!YamlGetValue(&value, yaml, "w")){
            configuration_error = true;
            return;
        }
        double w = std::stod(value);
        Debug(2, "w = " + toExpString(w) + " m");
        for(int x=0; x<x0; x++){
            if(Dr*(0.5+x) <= Rmin)
                Transmittance[x] = 1;
            else
                Transmittance[x] = exp(-2.0*pow((Dr*(0.5+x)-Rmin)/w,2));
        }
        return;
    }

    if(kind == "FREEFORM"){
        std::vector<double> pos;
        std::vector<double> transm;
        if(!YamlGetData(&pos, yaml, "form", 0) || !YamlGetData(&transm, yaml, "form", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile [Radial coordinate(m) Transmittance(-)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<pos.size(); i++)
                std::cout << toExpString(pos[i]) <<  " " << toExpString(transm[i]) << std::endl;
        for(int x=0; x<x0; x++)
                Transmittance[x] = Interpolate(&pos, &transm, Dr*(0.5+x));
        return;
    }

    // not a supproted "kind"
    std::cout << "ERROR: wrong \'kind\' in config file \'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void F::InternalDynamics(double)
{

}


void F::PulseInteraction(Pulse *pulse, Plane*, double)
{
    Debug(2, "Interaction with spatial filter");
    for(int x=0; x<x0; x++){
        for(int n=0; n<n0; n++)
            pulse->E[x][n] *=  sqrt(Transmittance[x]);
    }
}

