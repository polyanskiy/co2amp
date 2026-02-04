#include  "co2amp.h"

Plane::Plane(Optic *optic)
{
    this->optic = optic;
    this->space = 0;

    int num_pulses = pulses.size();

    input_fluence.assign(num_pulses*x0, 0.0);
    input_power.assign(num_pulses*n0, 0.0);
    input_E_center.assign(num_pulses*n0, 0.0);    // Field in the beam center (for spectrum calculation)
}
