#include "co2amp.h"


Optic::Optic(std::string id, std::string type)
{
    this->id = id;
    this->type = type;

    yaml_path = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml_path + "\'");

    if(!YamlReadFile(yaml_path, &yaml_content))
    {
        configuration_error = true;
        return;
    }

    std::string value="";
    if(!YamlGetValue(&value, &yaml_content, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");
    Dr = r_max/x0;
}
