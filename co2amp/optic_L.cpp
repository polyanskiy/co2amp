#include "co2amp.h"


//////////////////////////////// LENS ///////////////////////////////////////


L::L(std::string id)
{
    this->id = id;
    type = "L";
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

    // F
    if(!YamlGetValue(&value, yaml, "F"))
    {
        configuration_error = true;
        return;
    }
    F = std::stof(value);
    Debug(2, "F = " + toExpString(F) + " m");
}


void L::InternalDynamics(double)
{

}


void L::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    if(F==0.0)
        return;

    Debug(2, "Interaction with lens, F = " + toExpString(F) + " m");
    StatusDisplay(pulse, plane, time, "lens...");

    double Dr = r_max/x0;
    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

    #pragma omp parallel for
    for(int x=0; x<x0; x++)
    {
        /*if(method==0) // no propagation
        {
            for(int n=0; n<n0; n++)
                pulse->E[x][n] *= exp(-I*2.0*M_PI*(pulse->vc/c)*pow(Dr*(0.5+x),2)/(2.0*F));
        }

        else
        {*/
            // frequency domain
            double k_wave;
            std::complex<double> *E1;
            E1 = new std::complex<double>[n0];
            FFT(pulse->E[x], E1);
            for(int n=0; n<n0; n++)
            {
                k_wave=2.0*M_PI*(v0+Dv*(n-n0/2))/c;
                int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                E1[n1] *= exp(-I*k_wave*pow(Dr*(0.5+x),2)/(2.0*F));
            }
            IFFT(E1, pulse->E[x]);
            delete[] E1;
        //}
    }
}
