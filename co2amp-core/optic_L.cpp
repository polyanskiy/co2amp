#include "co2amp.h"


//////////////////////////////// LENS ///////////////////////////////////////


L::L(std::string id)
{
    this->id = id;
    type = "L";
    yaml = id + ".yml";

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

    // F
    if(!YamlGetValue(&value, yaml, "F")){
        configuration_error = true;
        return;
    }
    F = std::stof(value);
    Debug(2, "F = " + toExpString(F) + " m");
}


void L::InternalDynamics(double)
{

}


void L::PulseInteraction(int pulse_n)
{
    if(F==0.0)
        return;
    Debug(2, "Lens interaction, F = " + toExpString(F) + " m");
    int x, n;
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            pulses[pulse_n]->E[x][n] *= exp(I*2.0*M_PI*(vc/c)*pow(Dr*x,2)/(2.0*F));
}
