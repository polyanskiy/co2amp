#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    this->type = "C";
    this->yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + this->type + "\' from file \'" + this->yaml + "\' ...");

    std::string value="";

    if(YamlGetValue(&value, yaml, "CA"))
        Dr = std::stod(value)/2/(x0-1);
    else
        std::cout << "ERROR: cannot find \'CA\' value in optics config file \'" << yaml << "\'";

    if(YamlGetValue(&value, yaml, "chirp")){
        chirp = std::stof(value);
        Debug(2, "chirp = " + toExpString(chirp) + " s/Hz");
    }
    else
        std::cout << "ERROR: cannot find \'chirp\' value in \'Chirper\' config file \'" << yaml << "\'";
}


void C::InternalDynamics(double clock_time)
{

}


void C::PulseInteraction(int pulse_n)
{
    if(chirp==0.0)
        return;
    Debug(2, "Chirper interaction, chirp = " + toExpString(chirp) + " s/Hz");

    int x;
    double Dt = (t_max-t_min)/(n0-1); // pulse time step, s
    double Dv = 1.0/(Dt*n0);          // frequency step, Hz
    double v_min = vc - Dv*(n0-1)/2;

    #pragma omp parallel for // mulithread
    for(x=0; x<x0; x++){
        int n;
        std::complex<double> *spectrum;
        double delay;
        spectrum = new std::complex<double>[n0];
        FFT(pulses[pulse_n]->E[x], spectrum);
        for(n=0; n<n0; n++){
            delay = (v_min+Dv*n-vc) * chirp; // delay increases with frequency (red chirp) - group delay
            delay *= 0.5; // conversion to phase delay.
            spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*n-vc)*(-delay)); // no "-" in the exponent in frequency domain E(omega)
        }
        IFFT(spectrum, pulses[pulse_n]->E[x]);
        delete spectrum;
    }

}

