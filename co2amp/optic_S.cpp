#include "co2amp.h"


S::S(std::string id)
{
    this->id = id;
    type = "S";
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

    // filter type
    if(!YamlGetValue(&value, yaml, "filter"))
    {
        configuration_error = true;
        return;
    }
    std::string filter = value;
    Debug(2, "filter = " + filter);

    Transmittance = new double[n0];

    if(filter == "HIGHPASS")
    {
        if(!YamlGetValue(&value, yaml, "cutoff"))
        {
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v0+Dv*(n-n0/2)>=cutoff ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter == "LOWPASS"){
        if(!YamlGetValue(&value, yaml, "cutoff"))
        {
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v0+Dv*(n-n0/2)<cutoff ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter == "BANDPASS"){
        if(!YamlGetValue(&value, yaml, "cutoff_lo"))
        {
            configuration_error = true;
            return;
        }
        double cutoff_lo = std::stod(value);
        Debug(2, "cutoff_lo = " + toExpString(cutoff_lo) + " Hz");

        if(!YamlGetValue(&value, yaml, "cutoff_hi"))
        {
            configuration_error = true;
            return;
        }
        double cutoff_hi = std::stod(value);
        Debug(2, "cutoff_hi = " + toExpString(cutoff_hi) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v0+Dv*(n-n0/2)>=cutoff_lo && v0+Dv*(n-n0/2)<=cutoff_hi ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter == "FREEFORM")
    {
        std::vector<double> nu;
        std::vector<double> transm;
        if(!YamlGetData(&nu, yaml, "form", 0) || !YamlGetData(&transm, yaml, "form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile [Frequency(Hz) Transmittance(-)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<nu.size(); i++)
                std::cout << toExpString(nu[i]) <<  " " << toExpString(transm[i]) << std::endl;

        for(int n=0; n<n0; n++)
            Transmittance[n] = Interpolate(&nu, &transm, v0+Dv*(n-n0/2));
        WriteTransmittanceFile();
        return;
    }

    // not a supproted "filter"
    std::cout << "ERROR: wrong \'filter\' in config file \'" << yaml << "\'" << std::endl;
    configuration_error = true;
}


void S::InternalDynamics(double)
{

}


void S::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    Debug(2, "Interaction with spectral filter");
    StatusDisplay(pulse, plane, time, "spectral filtering...");

    #pragma omp parallel for
    for(int x=0; x<x0; x++)
    {
        std::complex<double> *E1;
        E1 = new std::complex<double>[n0];
        FFT(pulse->E[x], E1);
        for(int n=0; n<n0; n++)
        {
            int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
            E1[n1] *= sqrt(Transmittance[n]);
        }
        IFFT(E1, pulse->E[x]);
        delete[] E1;
    }
}


void S::WriteTransmittanceFile()
{
    double Dv = 1.0/(t_max-t_min); // frequency step, Hz

    FILE *file;

    file = fopen((id+"_transmittance.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] transmittance\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v0+Dv*(n-n0/2), Transmittance[n]);

    fclose(file);
}
