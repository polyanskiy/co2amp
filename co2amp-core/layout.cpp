#include  "co2amp.h"

LayoutComponent::LayoutComponent(Optic *optic, double distance, double time)
{
    this->optic = optic;
    this->distance = distance;
    this->time = time;
}
