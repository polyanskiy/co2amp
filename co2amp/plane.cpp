#include  "co2amp.h"

Plane::Plane(Optic *optic)
{
    this->optic = optic;
    this->space = 0;

    input_fluence.assign(x0, 0.0);
    input_power.assign(n0, 0.0);
}
