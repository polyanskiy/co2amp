#include "co2amp.h"


//C::C(std::string id)
void C::Initialize()
{
    //this->id = id;
    //type = "C";
    //yaml_path = id + ".yml";
    std::string value="";

    /*Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml_path + "\'");

    if(!YamlReadFile(yaml_path, &yaml_content))
    {
        configuration_error = true;
        return;
    }

    // r_max (semiDia)
    if(!YamlGetValue(&value, &yaml_content, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");
    Dr = r_max/x0;*/

    // Chirp type
    if(!YamlGetValue(&value, &yaml_content, "chirp"))
    {
        configuration_error = true;
        return;
    }
    std::string chirp_type = value;
    Debug(2, "chirp (chirp type) = " + chirp_type);

    Chirp.resize(n0);

    if(chirp_type == "LINEAR")
    {
        if(!YamlGetValue(&value, &yaml_content, "c"))
        {
            configuration_error = true;
            return;
        }
        for(int n=0; n<n0; n++)
            Chirp[n] = std::stof(value);
        Debug(2, "c (chirp rate, dν/dt) = " + toExpString(Chirp[n0]) + " Hz/s");
        WriteChirpFile();
        return;
    }

    if(chirp_type == "FREEFORM")
    {
        std::vector<double> nu;
        std::vector<double> chrp;
        if(!YamlGetData(&nu, &yaml_content, "form", 0) || !YamlGetData(&chrp, &yaml_content, "form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Chirp profile [Frequency(Hz) Chirp_rate(Hz/s)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(size_t i=0; i<nu.size(); i++)
                std::cout << "  " << toExpString(nu[i]) << " " << toExpString(chrp[i]) << std::endl;

        for(int n=0; n<n0; n++)
            Chirp[n] = Interpolate(&nu, &chrp, v_min+Dv*(0.5+n));
        WriteChirpFile();
        return;
    }

    // not supproted chirp type
    std::cout << "ERROR: wrong \'chirp\' in config file (must be LINEAR or FREEFORM)\'" << yaml_path << "\'" << std::endl;
    configuration_error = true;
}


void C::InternalDynamics(int)
{

}


void C::PulseInteraction(Pulse *pulse, Plane* plane, int m, int n_min, int)
{
    if(n_min!=0)
        return;

    Debug(2, "Interaction with chirper");
    StatusDisplay(pulse, plane, m, "chirping/de-chirping...");

    #pragma omp parallel for // mulithread
    for(int x=0; x<x0; ++x)
    {
        // this can be used with any chirp profile
        double v;
        double phase_shift = 0;
        int n1;
        std::vector<std::complex<double>> E1(n0); // field in frequency domain
        FFT(&pulse->E[n0*x], E1.data());
        for(int n=0; n<n0; n++)
        {
            v = v_min+Dv*(0.5+n);
            phase_shift += 2*M_PI*(v-pulse->vc) / Chirp[n] * Dv;
            n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*phase_shift);
        }
        IFFT(E1.data(), &pulse->E[n0*x]);

        // liniar chirp only
        /*double v;
        double chirp = Chirp[0];
        int n1;
        std::vector<std::complex<double>> E1(n0); // field in frequency domain
        FFT(&pulse->E[n0*x], E1.data());
        for(int n=0; n<n0; n++)
        {
            v = v_min+Dv*(0.5+n);
            v -= pulse->vc; // relative frequency (v-vc)
            n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
            E1[n1] *= exp(I*M_PI*v*v/chirp);
        }
        IFFT(E1.data(), &pulse->E[n0*x]);*/

        // this only imposes a chirp on the phase in time domain - pulse is not stretched
        /*double t;
        double chirp = Chirp[0];
        for(int n=0; n<n0; n++)
        {
            t = t_min+Dt*(0.5+n);
            pulse->E[n0*x+n] *= exp(I*M_PI*chirp*t*t);
        }*/
    }

}


void C::WriteChirpFile()
{
    FILE *file;

    file = fopen((id+"_chirp.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] chirp[Hz/s]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v_min+Dv*(0.5+n), Chirp[n]);
    fclose(file);
}

