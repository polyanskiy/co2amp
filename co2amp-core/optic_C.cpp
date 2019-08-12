#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    this->type = "C";
    this->yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + this->type + "\' from file \'" + this->yaml + "\' ...");

    //std::string value="";
    //YamlGetValue(&value, yaml, "diameter");
    //this->Dr = std::stod(value) / 2 / (x0-1) / 1000; // mm->m
}


void C::InternalDynamics(double clock_time)
{

}


void C::PulseInteraction(int pulse_n)
{

}

