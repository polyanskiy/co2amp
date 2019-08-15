#include "co2amp.h"


//////////////////////////////// LENS ///////////////////////////////////////


L::L(std::string id)
{
    this->id = id;
    type = "L";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // Rmax
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


void L::PulseInteraction(Pulse *pulse, Plane*, double)
{
    Debug(2, "Interaction with lens, F = " + toExpString(F) + " m");
    if(F==0.0)
        return;
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            pulse->E[x][n] *= exp(I*2.0*M_PI*(vc/c)*pow(Dr*x,2)/(2.0*F));
}
