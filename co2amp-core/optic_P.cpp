#include "co2amp.h"


P::P(std::string id)
{
    this->id = id;
    type = "P";
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
}


void P::InternalDynamics(double)
{

}


void P::PulseInteraction(Pulse*, Plane*, double)
{

}

