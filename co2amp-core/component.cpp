#include "component.h"
#include "co2amp.h"


Component::Component()
{
    this->id = "";
    this->type = "";
    this->yaml = "";
    this->test = "";
    this->Dr = 0;
}


Component::Component(std::string id, std::string type, std::string yaml)
{
    this->id = id;
    this->type = type;
    this->yaml = yaml;
    std::string value="qq";
    YamlGetValue(&value, yaml, "diameter");
    this->test = value;
    this->Dr = std::stod(value) / 2 / (x0-1) / 1000; // mm->m
}
