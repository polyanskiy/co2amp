#include "co2amp.h"


F::F(std::string id, std::string yaml)
{
    this->id = id;
    this->type = "F";
    this->yaml = yaml;
    std::string value="";
    //YamlGetValue(&value, yaml, "diameter");
    //this->Dr = std::stod(value) / 2 / (x0-1) / 1000; // mm->m
}
