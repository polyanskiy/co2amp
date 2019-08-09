#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    this->yaml = id + ".yml";

    this->E0 = -1;
    this->w0 = -1;
    this->tau0 = -1;
    this->nu0 = -1;
    this->t0 = -1;

    Debug(2, "Creating pulse from file \'" + this->yaml + "\' ...");

    std::string value="";    
    if(YamlGetValue(&value, yaml, "E0"))
        E0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "w0"))
        w0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "tau0"))
        tau0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "nu0"))
        nu0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "t0"))
        t0 = std::stod(value);

    Debug(2, "E0 = " + std::to_string(E0) + " J");
    Debug(2, "w0 = " + std::to_string(w0) + " m");
    Debug(2, "tau0 = " + std::to_string(tau0) + " s");
    Debug(2, "nu0 = " + std::to_string(nu0) + " Hz");
    Debug(2, "t0 = " + std::to_string(t0) + " s");

    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];
}


void Pulse::InitializeE()
{
    Debug(2, "Initializing field array for pulse \'" + this->id + "\' ...");

    int x, n;
    double Energy, af;
    //FILE *file;

    double Dr = layout[0].optic->Dr; // use first optic in the layout
    double Dt = (t_pulse_max-t_pulse_min)/(n0-1);

    // Create 2D array
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] = field(Dr*x, t_pulse_min + Dt*n);

    // frequency shift between central frequency of the pulse (v0) and central frequency of the calculation grig (vc)
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*(Dt*n));

    // Normalize intensity
    Energy = 0;
    for(n=0; n<n0-1; n++)
        for(x=0; x<x0-1; x++)
            Energy += 2.0 * h * nu0 *
                    (pow(abs(E[x][n]),2) + pow(abs(E[x][n+1]),2) +
                    pow(abs(E[x+1][n]),2) + pow(abs(E[x+1][n+1]),2))/4 *
                    2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J

    af = sqrt(E0/Energy);
    for(n=0; n<n0; n++)
        for(x=0; x<x0; x++)
            E[x][n] *= af;


    /*else{
        file = fopen("field_in.bin", "rb");
        for(pulse=0; pulse<n_pulses; pulse++){
            for(x=0; x<x0; x++)
                fread(E[pulse][x], sizeof(std::complex<double>)*n0, 1, file);
        }
        fclose(file);
    }*/
}


std::complex<double> Pulse::field(double r, double t)
{
    double xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
    std::complex<double> pulse = exp(-pow(t/xx, 2));
    std::complex<double> beam = exp(-pow(r/w0, 2));
    return pulse*beam;
}
