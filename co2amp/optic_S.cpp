#include "co2amp.h"


S::S(std::string id)
{
    this->id = id;
    type = "S";
    yaml = id + ".yml";
    double Dv = 1.0/(t_max-t_min);       // frequency step, Hz
    double v_min = vc - Dv*n0/2;

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max
    if(!YamlGetValue(&value, yaml, "r_max")){
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "r_max = " + toExpString(r_max) + " m");

    // Kind
    if(!YamlGetValue(&value, yaml, "kind")){
        configuration_error = true;
        return;
    }
    std::string kind = value;
    Debug(2, "kind = " + kind);

    Transmittance = new double[n0];

    if(kind == "HIGHPASS"){
        if(!YamlGetValue(&value, yaml, "cutoff")){
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            if((v_min+Dv*(0.5+n)) >= cutoff)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        WriteTransmittanceFile();
        return;
    }

    if(kind == "LOWPASS"){
        if(!YamlGetValue(&value, yaml, "cutoff")){
            configuration_error = true;
            return;
        }
        double cutoff = std::stod(value);
        Debug(2, "cutoff = " + toExpString(cutoff) + " Hz");

        for(int n=0; n<n0; n++)
            if((v_min+Dv*(0.5+n)) < cutoff)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        WriteTransmittanceFile();
        return;
    }

    if(kind == "BANDPASS"){
        if(!YamlGetValue(&value, yaml, "cutoff_lo")){
            configuration_error = true;
            return;
        }
        double cutoff_lo = std::stod(value);
        Debug(2, "cutoff_lo = " + toExpString(cutoff_lo) + " Hz");

        if(!YamlGetValue(&value, yaml, "cutoff_hi")){
            configuration_error = true;
            return;
        }
        double cutoff_hi = std::stod(value);
        Debug(2, "cutoff_hi = " + toExpString(cutoff_hi) + " Hz");

        for(int n=0; n<n0; n++)
            if((v_min+Dv*(0.5+n))>=cutoff_lo && (v_min+Dv*(0.5+n))<=cutoff_hi)
                Transmittance[n] = 1;
            else
                Transmittance[n] = 0;
        WriteTransmittanceFile();
        return;
    }

    if(kind == "FREEFORM"){
        std::vector<double> nu;
        std::vector<double> transm;
        if(!YamlGetData(&nu, yaml, "form", 0) || !YamlGetData(&transm, yaml, "form", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile [Frequency(Hz) Transmittance(-)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<nu.size(); i++)
                std::cout << toExpString(nu[i]) <<  " " << toExpString(transm[i]) << std::endl;

        for(int n=0; n<n0; n++)
            Transmittance[n] = Interpolate(&nu, &transm, v_min+Dv*(0.5+n));
        WriteTransmittanceFile();
        return;
    }

    // not a supproted "kind"
    std::cout << "ERROR: wrong \'kind\' in config file \'" << yaml << "\'" << std::endl;
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
    for(int x=0; x<x0; x++){
        std::complex<double> *spectrum;
        spectrum = new std::complex<double>[n0];
        FFT(pulse->E[x], spectrum);
        for(int n=0; n<n0; n++)
            spectrum[n] *= sqrt(Transmittance[n]);
        IFFT(spectrum, pulse->E[x]);
        delete[] spectrum;
    }
}





void S::WriteTransmittanceFile()
{
    double Dv = 1.0/(t_max-t_min);       // frequency step, Hz
    double v_min = vc - Dv*n0/2;

    FILE *file;

    file = fopen((id+"_transmittance.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] transmittance\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v_min+Dv*(0.5+n), Transmittance[n]);

    fclose(file);
}