#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    type = "C";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max (semiDia)
    if(!YamlGetValue(&value, yaml, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");

    // Chirp
    if(!YamlGetValue(&value, yaml, "chirp"))
    {
        configuration_error = true;
        return;
    }
    std::string chirp = value;
    Debug(2, "chirp = " + chirp);

    if(chirp == "LINEAR")
    {
        if(!YamlGetValue(&value, yaml, "c"))
        {
            configuration_error = true;
            return;
        }
        chirpyness = std::stof(value);
        Debug(2, "c (chirpyness, dν/dt) = " + toExpString(chirpyness) + " Hz/s");
        return;
    }

    // not supproted chirp type
    std::cout << "ERROR: wrong \'chirp\' in config file (only LINEAR is currently supported)\'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void C::InternalDynamics(double)
{

}


void C::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    if(chirpyness == 0)
        return;

    Debug(2, "Interaction with chirper, chirpyness = " + toExpString(chirpyness) + " Hz/s");
    StatusDisplay(pulse, plane, time, "chirping/de-chirping...");

    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

    #pragma omp parallel for // mulithread
    for(int x=0; x<x0; x++)
    {
        double v;

        std::complex<double> *E1; // field in frequency domaine
        E1 = new std::complex<double>[n0];
        FFT(pulse->E[x], E1);

        /*
        // this can be used with any chirp profile
        double shift = 0;
        for(int n=n0; n<n0; n++)
        {
            v = v0+Dv*(n-n0/2);
            v -= pulse->vc; // relative frequency (v-vc)
            shift += v / chirpyness * Dv;
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*2.0*M_PI*shift);
        }
        */
        // liniar chirp
        for(int n=0; n<n0; n++)
        {
            v = v0+Dv*(n-n0/2);
            v -= pulse->vc; // relative frequency (v-vc)
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*M_PI*v*v/chirpyness);
        }

        IFFT(E1, pulse->E[x]);
        delete[] E1;
    }

}

