#include "co2amp.h"


S::S(std::string id)
{
    this->id = id;
    type = "S";
    yaml_path = id + ".yml";
    std::string value="";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml_path + "\'");

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
    Dr = r_max/x0;

    // filter type
    if(!YamlGetValue(&value, &yaml_content, "filter"))
    {
        configuration_error = true;
        return;
    }
    std::string filter_type = value;
    Debug(2, "filter (filter type) = " + filter_type);

    //Transmittance = new double[n0];
    std::vector<double> Transmittance(n0);

    if(filter_type == "HIGHPASS")
    {
        if(!YamlGetValue(&value, &yaml_content, "cutoff"))
        {
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v_min+Dv*(0.5+n)>=cutoff ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "LOWPASS"){
        if(!YamlGetValue(&value, &yaml_content, "cutoff"))
        {
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v_min+Dv*(0.5+n)<cutoff ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "BANDPASS"){
        if(!YamlGetValue(&value, &yaml_content, "cutoff_lo"))
        {
            configuration_error = true;
            return;
        }
        double cutoff_lo = std::stod(value);
        Debug(2, "cutoff_lo = " + toExpString(cutoff_lo) + " Hz");

        if(!YamlGetValue(&value, &yaml_content, "cutoff_hi"))
        {
            configuration_error = true;
            return;
        }
        double cutoff_hi = std::stod(value);
        Debug(2, "cutoff_hi = " + toExpString(cutoff_hi) + " Hz");

        for(int n=0; n<n0; n++)
            Transmittance[n] = v_min+Dv*(0.5+n)>=cutoff_lo && v_min+Dv*(0.5+n)<=cutoff_hi ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "FREEFORM")
    {
        std::vector<double> nu;
        std::vector<double> transm;
        if(!YamlGetData(&nu, &yaml_content, "form", 0) || !YamlGetData(&transm, &yaml_content, "form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile loaded (use debug level 3 to display)");
        Debug(2, "Transmittance profile [Frequency(Hz) Transmittance(-)]");
        if(debug_level >= 3)
            for(size_t i=0; i<nu.size(); i++)
                std::cout << "  " << toExpString(nu[i]) <<  " " << toExpString(transm[i]) << std::endl;

        for(int n=0; n<n0; n++)
            Transmittance[n] = Interpolate(&nu, &transm, v_min+Dv*(0.5+n));
        WriteTransmittanceFile();
        return;
    }

    // not a supproted "filter"
    std::cout << "ERROR: wrong \'filter\' in config file \'" << yaml_path << "\'" << std::endl;
    configuration_error = true;
}


void S::InternalDynamics(double)
{

}


void S::PulseInteraction(Pulse *pulse, Plane* plane, double time, int n_min, int)
{
    if(n_min!=0)
        return;

    Debug(2, "Interaction with spectral filter");
    StatusDisplay(pulse, plane, time, "spectral filtering...");

    #pragma omp parallel for
    for(int x=0; x<x0; ++x)
    {
        std::vector<std::complex<double>> E1(n0);
        FFT(&pulse->E[n0*x], E1.data());
        for(int n=0; n<n0; n++)
        {
            int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
            E1[n1] *= sqrt(Transmittance[n]);
        }
        IFFT(E1.data(), &pulse->E[x*n0]);
    }
}


void S::WriteTransmittanceFile()
{
    FILE *file;

    file = fopen((id+"_transmittance.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] transmittance\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v_min+Dv*(0.5+n), Transmittance[n]);

    fclose(file);
}
