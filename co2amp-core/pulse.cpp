#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    this->yaml = id + ".yml";

    this->E0 = -1;
    this->w0 = -1;
    this->tau0 = -1;
    this->vc = -1;
    this->t_inj = -1;

    Debug(2, "Creating pulse from file \'" + this->yaml + "\' ...");

    std::string value="";    
    if(YamlGetValue(&value, this->yaml, "E0"))
        this->E0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "w0"))
        this->w0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "tau0"))
        this->tau0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "vc"))
        this->vc = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "t_inj"))
        this->t_inj = std::stod(value);

    Debug(2, "E0 = " + std::to_string(E0));
    Debug(2, "w0 = " + std::to_string(w0));
    Debug(2, "tau0 = " + std::to_string(tau0));
    Debug(2, "vc = " + std::to_string(vc));
    Debug(2, "t_inj = " + std::to_string(t_inj));

}
