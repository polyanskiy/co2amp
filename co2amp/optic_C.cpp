#include "co2amp.h"


C::C(std::string id)
{
    this->id = id;
    type = "C";
    yaml = id + ".yml";
    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

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

    Chirpyness = new double[n0];

    if(chirp == "LINEAR")
    {
        if(!YamlGetValue(&value, yaml, "c"))
        {
            configuration_error = true;
            return;
        }
        for(int n=0; n<n0; n++)
            Chirpyness[n] = std::stof(value);
        Debug(2, "c (chirpyness, dν/dt) = " + toExpString(Chirpyness[n0]) + " Hz/s");
        WriteChirpynessFile();
        return;
    }

    if(chirp == "FREEFORM")
    {
        std::vector<double> nu;
        std::vector<double> chrp;
        if(!YamlGetData(&nu, yaml, "form", 0) || !YamlGetData(&chrp, yaml, "form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Chirpyness profile [Frequency(Hz) Chirpyness(Hz/s)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<nu.size(); i++)
                std::cout << toExpString(nu[i]) <<  " " << toExpString(chrp[i]) << std::endl;

        for(int n=0; n<n0; n++)
            Chirpyness[n] = Interpolate(&nu, &chrp, v0+Dv*(n-n0/2));
        WriteChirpynessFile();
        return;
    }

    // not supproted chirp type
    std::cout << "ERROR: wrong \'chirp\' in config file (must be LINEAR or FREEFORM)\'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void C::InternalDynamics(double)
{

}


void C::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    //if(chirpyness == 0)
    //    return;

    //Debug(2, "Interaction with chirper, chirpyness = " + toExpString(chirpyness) + " Hz/s");
    Debug(2, "Interaction with chirper");
    StatusDisplay(pulse, plane, time, "chirping/de-chirping...");

    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

    #pragma omp parallel for // mulithread
    for(int x=0; x<x0; x++)
    {
        double v, shift;

        std::complex<double> *E1; // field in frequency domaine
        E1 = new std::complex<double>[n0];
        FFT(pulse->E[x], E1);

        // this can be used with any chirp profile
        shift = 0;
        for(int n=0; n<n0; n++)
        {
            v = v0+Dv*(n-n0/2);
            shift += (v-pulse->vc) / Chirpyness[n] * Dv;
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*2.0*M_PI*shift);
        }

        // liniar chirp only
        /*for(int n=0; n<n0; n++)
        {
            v = v0+Dv*(n-n0/2);
            v -= pulse->vc; // relative frequency (v-vc)
            int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*M_PI*v*v/chirpyness);
        }*/

        IFFT(E1, pulse->E[x]);
        delete[] E1;
    }

}


void C::WriteChirpynessFile()
{
    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

    FILE *file;

    file = fopen((id+"_chirpyness.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] chirpyness[Hz/s]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v0+Dv*(n-n0/2), Chirpyness[n]);
    fclose(file);
}

