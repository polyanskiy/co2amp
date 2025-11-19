#include "co2amp.h"


//F::F(std::string id)
void F::Initialize()
{  
    //this->id = id;
    //type = "F";
    //yaml_path = id + ".yml";
    std::string value="";

    /*Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml_path + "\'");

    if(!YamlReadFile(yaml_path, &yaml_content))
    {
        configuration_error = true;
        return;
    }

    // r_max (semiDia) - note the difference between the user interface- and internal- notation
    if(!YamlGetValue(&value, &yaml_content, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");
    Dr = r_max/x0;*/

    // filter type
    if(!YamlGetValue(&value, &yaml_content, "filter"))
    {
        configuration_error = true;
        return;
    }
    std::string filter_type = value;
    Debug(2, "filter (filter type) = " + filter_type);

    Transmittance = new double[x0];

    if(filter_type == "ND")
    {
        if(!YamlGetValue(&value, &yaml_content, "T"))
        {
            configuration_error = true;
            return;
        }
        double T = std::stod(value);
        Debug(2, "T = " + toExpString(T));
        for(int x=0; x<x0; ++x)
            Transmittance[x] = T;
        WriteTransmittanceFile();
        return;
    }

    // APERTURE is useful when we don't want to resample the beam (keep semiDia)
    if(filter_type == "APERTURE")
    {
        if(!YamlGetValue(&value, &yaml_content, "R"))
        {
            configuration_error = true;
            return;
        }
        double R = std::stod(value);
        Debug(2, "R = " + toExpString(R) + " m");
        for(int x=0; x<x0; ++x)
            Transmittance[x] = Dr*(0.5+x)<=R ? 1 : 0;
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "MASK")
    {
        if(!YamlGetValue(&value, &yaml_content, "R"))
        {
            configuration_error = true;
            return;
        }
        double R = std::stod(value);
        Debug(2, "R = " + toExpString(R) + " m");
        for(int x=0; x<x0; ++x)
            Transmittance[x] = Dr*(0.5+x)<=R ? 0 : 1;
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "SIN")
    {
        if(!YamlGetValue(&value, &yaml_content, "R"))
        {
            configuration_error = true;
            return;
        }
        double R = std::stod(value);
        Debug(2, "R = " + toExpString(R) + " m");

        double w = r_max-R;
        if(YamlGetValue(&value, &yaml_content, "w"))
            w = std::stod(value);
        Debug(2, "w = " + toExpString(w) + " m");

        for(int x=0; x<x0; ++x)
        {
            //Transmittance[x] = Dr*(0.5+x)<=r_min ? 1 : pow(sin(M_PI*(r_max-Dr*(0.5+x))/(2.0*(r_max-r_min))),2);
            if(Dr*(0.5+x)<=R)
                Transmittance[x] = 1;
            else
            {
                if(Dr*(0.5+x)>R+w)
                    Transmittance[x] = 0;
                else
                    Transmittance[x] = pow(sin(M_PI*(R+w-Dr*(0.5+x))/(2.0*w)),2);
            }


        }
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "GAUSS")
    {
        if(!YamlGetValue(&value, &yaml_content, "R"))
        {
            configuration_error = true;
            return;
        }
        double R = std::stod(value);
        Debug(2, "R = " + toExpString(R) + " m");
        if(!YamlGetValue(&value, &yaml_content, "w"))
        {
            configuration_error = true;
            return;
        }
        double w = std::stod(value);
        Debug(2, "w = " + toExpString(w) + " m");
        for(int x=0; x<x0; ++x)
            Transmittance[x] = Dr*(0.5+x)<=R ? 1 : exp(-2.0*pow((Dr*(0.5+x)-R)/w,2));
        WriteTransmittanceFile();
        return;
    }

    if(filter_type == "FREEFORM")
    {
        std::vector<double> pos;
        std::vector<double> transm;
        if(!YamlGetData(&pos, &yaml_content, "form", 0) || !YamlGetData(&transm, &yaml_content, "form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Transmittance profile loaded (use debug level 3 to display)");
        Debug(3, "Transmittance profile [Radial coordinate(m) Transmittance(-)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
        {
            for(size_t i=0; i<pos.size(); i++)
            {
                std::cout << "  " << toExpString(pos[i]) <<  " " << toExpString(transm[i]) << std::endl;
            }
        }
        for(int x=0; x<x0; ++x)
        {
                Transmittance[x] = Interpolate(&pos, &transm, Dr*(0.5+x));
        }
        WriteTransmittanceFile();
        return;
    }

    // not a supproted "filter"
    std::cout << "ERROR: wrong \'filter\' in config file \'" << yaml_path << "\'" << std::endl;
    configuration_error = true;
}


void F::InternalDynamics(int)
{

}


void F::PulseInteraction(Pulse *pulse, Plane* plane, int m, int n_min, int)
{    
    if(n_min!=0)
        return;

    Debug(2, "Interaction with spatial filter");
    StatusDisplay(pulse, plane, m, "spatial filtering...");

    for(int x=0; x<x0; ++x)
        for(int n=0; n<n0; n++)
            pulse->E[n0*x+n] *=  sqrt(Transmittance[x]);
}


void F::WriteTransmittanceFile()
{
    FILE *file;

    file = fopen((id+"_transmittance.dat").c_str(), "w");
    fprintf(file, "#Data format: r[m] transmittance\n");
    for(int x=0; x<x0; ++x)
        fprintf(file, "%e\t%e\n", Dr*(0.5+x), Transmittance[x]);

    fclose(file);
}

