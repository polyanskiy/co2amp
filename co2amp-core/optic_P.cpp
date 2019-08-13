#include "co2amp.h"


P::P(std::string id)
{
    this->id = id;
    type = "P";
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

    // Rmax -> Dr
    if(!YamlGetValue(&value, yaml, "Rmax")){
        configuration_error = true;
        return;
    }
    Debug(2, "Rmax = " + toExpString(std::stod(value)) + " m");
    Dr = std::stod(value)/(x0-1);
}


void P::InternalDynamics(double clock_time)
{

}


void P::PulseInteraction(int pulse_n)
{

}

