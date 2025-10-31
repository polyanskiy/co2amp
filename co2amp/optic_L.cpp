#include "co2amp.h"


//////////////////////////////// LENS ///////////////////////////////////////


L::L(std::string id)
{
    this->id = id;
    type = "L";
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

    // F
    if(!YamlGetValue(&value, &yaml_content, "F"))
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
            std::vector<std::complex<double>> E1(n0);
            FFT(&pulse->E[n0*x], E1.data());
            for(int n=0; n<n0; n++)
            {
                k_wave=2.0*M_PI*(v_min+Dv*(0.5+n))/c;
                int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                E1[n1] *= exp(-I*k_wave*pow(Dr*(0.5+x),2)/(2.0*F));
            }
            IFFT(E1.data(), &pulse->E[n0*x]);
        //}
    }
}
