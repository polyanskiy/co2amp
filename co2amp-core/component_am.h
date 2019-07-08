#ifndef COMPONENT_AM_H
#define COMPONENT_AM_H

#include<string>
#include <component.h>

class AM: public Component
{
public:
    AM();
    AM(std::string id, std::string type, std::string yaml);
};

#endif // COMPONENT_AM_H
