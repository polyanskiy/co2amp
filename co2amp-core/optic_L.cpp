#include "co2amp.h"


//////////////////////////////// LENS ///////////////////////////////////////


L::L(std::string id)
{
    this->id = id;
    this->type = "L";
    this->yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + this->type + "\' from file \'" + this->yaml + "\' ...");

    std::string value="";

    if(YamlGetValue(&value, yaml, "CA"))
        Dr = std::stod(value)/2/(x0-1);
    else
        std::cout << "ERROR: cannot find \'CA\' value in optics config file \'" << yaml << "\'";

    if(YamlGetValue(&value, yaml, "F")){
        F = std::stof(value);
        Debug(2, "F = " + toExpString(F) + " m");
    }
    else
        std::cout << "ERROR: cannot find \'F\' value in \'Lens\' config file \'" << yaml << "\'";
}


void L::InternalDynamics(double clock_time)
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
