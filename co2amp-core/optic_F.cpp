#include "co2amp.h"


F::F(std::string yaml)
{  
    std::getline(std::istringstream(yaml), this->id, '.');
    this->type = "F";
    this->yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + this->type + "\' from file \'" + this->yaml + "\' ...");

    std::string value="";
    if(YamlGetValue(&value, yaml, "CA"))
        Dr = std::stod(value)/2/(x0-1);
    else
        std::cout << "ERROR: cannot find \'CA\' value in optics config file \'" << yaml << "\'";
}


void F::InternalDynamics(double clock_time)
{

}


void F::PulseInteraction(int pulse_n)
{

}

