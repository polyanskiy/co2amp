#include "co2amp.h"


P::P(std::string id)
{
    this->id = id;
    type = "P";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\' ...");

    std::string value="";
    if(YamlGetValue(&value, yaml, "CA"))
        Dr = std::stod(value)/2/(x0-1);
    else
        std::cout << "ERROR: cannot find \'CA\' value in optics config file \'" << yaml << "\'";
}


void P::InternalDynamics(double clock_time)
{

}


void P::PulseInteraction(int pulse_n)
{

}

