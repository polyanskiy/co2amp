#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    this->yaml = id + ".yml";

    Debug(2, "Creating pulse from file \'" + this->yaml + "\' ...");
}
