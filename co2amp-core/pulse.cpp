#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    this->yaml = id + ".yml";

    this->E0 = -1;
    this->w0 = -1;
    this->tau0 = -1;
    this->vc = -1;
    this->t_inj = -1;

    Debug(2, "Creating pulse from file \'" + this->yaml + "\' ...");

    std::string value="";    
    if(YamlGetValue(&value, this->yaml, "E0"))
        this->E0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "w0"))
        this->w0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "tau0"))
        this->tau0 = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "vc"))
        this->vc = std::stod(value);
    if(YamlGetValue(&value, this->yaml, "t_inj"))
        this->t_inj = std::stod(value);

    Debug(2, "E0 = " + std::to_string(E0) + " J");
    Debug(2, "w0 = " + std::to_string(w0) + " mm");
    Debug(2, "tau0 = " + std::to_string(tau0) + " ps");
    Debug(2, "vc = " + std::to_string(vc) + " THz");
    Debug(2, "t_inj = " + std::to_string(t_inj) + " μs");

    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];

    InitializeE();
}


void Pulse::IntensityNormalization(void) // Field amplitude adjustment (to match initial pulse energy)
{
    int x, n;
    double Energy, af;
    double Dr = optics[0].Dr;
    double Dt = t_pulse_lim/(n0-1);

    Energy = 0;
    for(n=0; n<n0-1; n++){
        for(x=0; x<x0-1; x++)
            Energy += 2.0 * h * vc * pow(abs(E[x][n]+E[x][n+1]+E[x+1][n]+E[x+1][n+1])/4, 2) * 2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J
        }

    af = sqrt(E0/Energy);
    for(n=0; n<n0; n++){
        for(x=0; x<x0; x++)
            E[x][n] *= af;
    }
}


void Pulse::InitializeE()
{
    int x, n;
    //FILE *file;

    double Dr = optics[0].Dr;
    double Dt = t_pulse_lim/(n0-1);

    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] = field(Dr*x, Dt*n);

    //IntensityNormalization();
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
    std::complex<double> pulse = exp(-pow((t-t_pulse_shift)/xx, 2));
    std::complex<double> beam = exp(-pow(r/w0, 2.0));
    return pulse*beam;
}
