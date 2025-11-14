#include "co2amp.h"


//P::P(std::string id)
void P::Initialize()
{
    /*this->id = id;
    type = "P";
    yaml_path = id + ".yml";
    std::string value="";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml_path + "\'");

    if(!YamlReadFile(yaml_path, &yaml_content))
    {
        configuration_error = true;
        return;
    }

    // r_max (semiDia)
    if(!YamlGetValue(&value, &yaml_content, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");
    Dr = r_max/x0;*/
}


void P::InternalDynamics(double)
{

}


void P::PulseInteraction(Pulse*, Plane*, double, int, int)
{

}

