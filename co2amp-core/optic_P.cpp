#include "co2amp.h"


P::P(std::string id)
{
    this->id = id;
    type = "P";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max
    if(!YamlGetValue(&value, yaml, "r_max")){
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "r_max = " + toExpString(r_max) + " m");
}


void P::InternalDynamics(double)
{

}


void P::PulseInteraction(Pulse*, Plane*, double)
{

}

