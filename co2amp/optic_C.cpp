#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    type = "C";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max (R)
    if(!YamlGetValue(&value, yaml, "R"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "R = " + toExpString(r_max) + " m");

    // Chirp
    if(!YamlGetValue(&value, yaml, "chirp"))
    {
        configuration_error = true;
        return;
    }   
    chirp = std::stof(value);
    Debug(2, "chirp = " + toExpString(chirp) + " s/Hz");
}


void C::InternalDynamics(double)
{

}


void C::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    if(chirp == 0)
        return;

    Debug(2, "Interaction with chirper, chirp = " + toExpString(chirp) + " s/Hz");
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
            v = Dv*(n-n0/2); // relative frequency (v-v0)
            shift += v * chirp * Dv;
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*2.0*M_PI*shift);
        }
        */
        // liniar chirp
        for(int n=0; n<n0; n++)
        {
            v = Dv*(n-n0/2); // relative frequency (v-v0)
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*M_PI*v*v*chirp);
        }

        IFFT(E1, pulse->E[x]);
        delete[] E1;
    }

}

