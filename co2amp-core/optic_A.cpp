#include "co2amp.h"


A::A(std::string id, std::string type, std::string yaml)
{
    this->id = id;
    this->type = type;
    this->yaml = yaml;

    pumping = "";
    Vd = 0;
    D = 0;
    pump_wl = 0;
    pump_sigma = 0;
    pump_fluence = 0;
    p_626 = 0;
    p_628 = 0;
    p_828 = 0;
    p_636 = 0;
    p_638 = 0;
    p_838 = 0;
    p_CO2 = 0;
    double O18 = 0; // Oxygen-18 content (0..1)
    double C13 = 0; // Carbon-13 content (0..1)
    p_N2 = 0;
    p_He = 0;
    T0 = 0;

    // Read optic parameters from YAML specification file
    std::string value="";

    // ------- SIZE -------
    YamlGetValue(&value, yaml, "diameter");
    Dr = std::stod(value) / 2 / (x0-1) / 1000; // diameter->radius; mm->m

    // ------- PUMPING -------
    YamlGetValue(&value, yaml, "pumping");
    pumping = value; // "discharge" or "optical"

    if(pumping == "discharge"){
        YamlGetValue(&value, yaml, "Vd");
        Vd = std::stod(value)*1e-6; // cm^3->m^3
        YamlGetValue(&value, yaml, "D");
        D = std::stod(value)*1e-3; // mm->m
    }

    if(pumping == "optical"){
        YamlGetValue(&value, yaml, "pump_wl");
        pump_wl = std::stod(value)*1e-6; // um -> m
        YamlGetValue(&value, yaml, "pump_sigma");
        pump_sigma = std::stod(value)*1e-4; // cm^2 -> m^2
        YamlGetValue(&value, yaml, "pump_fluence");
        pump_fluence = std::stod(value)*1e4; // J/cm^2 -> J/m^2
    }

    if(pumping != "optical" && pumping == "discharge")
        Core::Abort("Wrong pumpinge type (must be \"discharge\" or \"optical\"");

    // ------- GAS MIXTURE -------
    YamlGetValue(&value, yaml, "p_626");
    p_626 = std::stod(value);
    YamlGetValue(&value, yaml, "p_628");
    p_628 = std::stod(value);
    YamlGetValue(&value, yaml, "p_828");
    p_828 = std::stod(value);
    YamlGetValue(&value, yaml, "p_636");
    p_636 = std::stod(value);
    YamlGetValue(&value, yaml, "p_638");
    p_638 = std::stod(value);
    YamlGetValue(&value, yaml, "p_838");
    p_838 = std::stod(value);
    YamlGetValue(&value, yaml, "p_N2");
    p_N2 = std::stod(value);
    YamlGetValue(&value, yaml, "p_He");
    p_He = std::stod(value);
    YamlGetValue(&value, yaml, "T0");
    T0 = std::stod(value);

    p_CO2 = p_626+p_628+p_828+p_636+p_638+p_838; // Total CO2 pressure, bar

    if(p_CO2 == 0){ // equilibrium isotope distribution from Oxygen-18 and Carbon-13 content
        YamlGetValue(&value, yaml, "p_CO2");
        p_CO2 = std::stod(value);
        YamlGetValue(&value, yaml, "O18");
        O18 = std::stod(value);
        YamlGetValue(&value, yaml, "C13");
        C13 = std::stod(value);
        p_626 = p_CO2 * pow(1-O18,2) * (1-C13);
        p_628 = p_CO2 * 2*(1-O18)*O18 * (1-C13);
        p_828 = p_CO2 * pow(O18,2) * (1-C13);
        p_636 = p_CO2 * pow(1-O18,2) * C13;
        p_638 = p_CO2 * 2*(1-O18)*O18 * C13;
        p_838 = p_CO2 * pow(O18,2) * C13;
    }

    std::cout << std::endl << "CALCULATING:" << std::endl;

    // 1 Fill out spectroscoic arrays &
    // 2 Populations and field initialization
    if(p_CO2+p_N2+p_He>0){
        AmplificationBand();
        InitializePopulations();
    }

    StatusDisplay(-1, -1, -1, "initial field...");
    InitializeE();

    // 3 Initial q's
    if(p_CO2+p_N2+p_He>0){
        StatusDisplay(-1, -1, 0, "pumping and relaxation...");
        if(pumping == "discharge"){
            Boltzmann(0);
            q2_b = q2;
            q3_b = q3;
            q4_b = q4;
            qT_b = qT;
            t_b = 0;
        }
    }
}


void A::InternalDynamics(double t)
{
    Core::StatusDisplay(-1, -1, t, "pumping and relaxation...");
    PumpingAndRelaxation(t);
}


void PulseInteraction(int)
{

}
