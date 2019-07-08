#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>

class Component
{
public:
    std::string id;
    std::string type;
    std::string yaml;
    std::string test;
    double Dr; //m
    Component();
    Component(std::string id, std::string type, std::string yaml);
};

extern std::vector<Component> components;

#endif // COMPONENT_H
