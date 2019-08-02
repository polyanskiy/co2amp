#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    this->type = "C";
    this->yaml = id + ".yml";

    Debug(2, "Initialializing " + this->type + " from file " + this->yaml + " ...");

    //std::string value="";
    //YamlGetValue(&value, yaml, "diameter");
    //this->Dr = std::stod(value) / 2 / (x0-1) / 1000; // mm->m
}
