#include "co2amp.h"


S::S(std::string id, std::string yaml)
{
    this->id = id;
    this->type = "S";
    this->yaml = yaml;
    std::string value="";
    //YamlGetValue(&value, yaml, "diameter");
    //this->Dr = std::stod(value) / 2 / (x0-1) / 1000; // mm->m
}
