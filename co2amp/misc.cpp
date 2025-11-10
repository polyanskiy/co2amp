#include  "co2amp.h"


Optic* FindOpticByID(std::string id)
{
    for(unsigned int i = 0; i<optics.size(); i++)
        if(optics[i]->id == id)
            return optics[i];
    return nullptr;
}


bool is_number(std::string s)
{
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && val != HUGE_VAL;
}


std::string toExpString(double num)
{
    std::ostringstream out;
    out << std::scientific << num;
    return out.str();
}

std::string toString(int num)
{
    std::ostringstream out;
    out << num;
    return out.str();
}


std::string toString(double num)
{
    std::ostringstream out;
    out << num;
    return out.str();
}


bool YamlReadFile(std::string path, std::string *yaml_file_content)
{
    std::ifstream in;
    in = std::ifstream(path, std::ios::in);
    if(in)
    {
        *yaml_file_content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
        Debug(3, path + " content:\n-------\n" + *yaml_file_content + "\n-------");
    }
    else
    {
        std::cout << "Error reading YAML file \'" + path + "\'\n";
        return false;
    }
    return true;
}


bool YamlGetValue(std::string *value, std::string *yaml_file_content, std::string key, bool required)
{
    std::string str;
    std::istringstream iss;

    iss = std::istringstream(*yaml_file_content);
    while(std::getline(iss, str))
    {
        std::size_t found = 1;
        found = str.find(key + ": ");
        if(found == 0) //key found at the beginnig of the string
        {
            *value = str.substr(std::string(key + ": ").length());

            //std::regex rgx("\"([^\"]*)\"");
            std::smatch match;
            if(std::regex_search(*value, match, std::regex("\"(.*)\""))) // if value is a quoted string
                *value = match[1];
            else
                value->erase(remove_if(value->begin(), value->end(), isspace), value->end()); // remove spaces
            Debug(3, key + ": " + *value + " (as read from file) interpreting...");
            return true;
        }
    }

    if(required)
        std::cout << "Key \"" + key + "\" not found in YAML file\n";

    return false;
}


bool YamlGetData(std::vector<double> *data, std::string *yaml_file_content, std::string key, int column_n)
{
    std::string str, str2, str3;
    std::istringstream iss, iss2;

    iss = std::istringstream(*yaml_file_content);
    while(std::getline(iss, str))
    {
        std::size_t found = 1;
        found = str.find(key + ": |");
        if(found == 0) //key found at the beginnig of the string
        {
            while(std::getline(iss, str2) && str2.substr(0,4) == "    ")
            {
                iss2 = std::istringstream(str2.substr(4));
                for(int i=0; i<=column_n; i++)
                    std::getline(iss2, str3, ' ');
                if(str3 != "")
                    data->push_back(std::stod(str3));
            }
            //Debug(3, key + ": " + *data);
            return true;
        }
    }

    std::cout << "Key \"" + key + "\" not found in YAML file\n";
    return false;
}


double Interpolate(std::vector<double> *X, std::vector<double> *Y, double x)
{
    if(X->size()<1) // require at least one point
        return 0;

    if(x <= (*X)[0])
        return (*Y)[0];

    if(x >= (*X)[X->size()-1])
        return (*Y)[Y->size()-1];

    for(size_t i=0; i<X->size()-1; i++)
    {
        if(x >= (*X)[i] && x < (*X)[i+1])
            return (*Y)[i] + ((*Y)[i+1]-(*Y)[i]) * (x-(*X)[i])/((*X)[i+1]-(*X)[i]);
    }

    return 0;
}

void UnwrapPhase(const std::complex<double>* E, double* phase)
{
    double Emax = 0;
    for(int n=0; n<n0; ++n)
    {
        Emax = std::max(Emax, std::abs(E[n]));
    }

    double phase_step;
    phase[0] = 0;

    //double av_phase_step = 0;
    int num_steps = 0;

    double chirp = 0;

    for(int n=1; n<n0; ++n)
    {
        if(std::abs(E[n])/Emax < 1e-4) // remove noise
        {
            phase_step = 0;
        }
        else
        {
            phase_step = arg(E[n]) - arg(E[n-1]);

            phase[n] = phase[n-1] + phase_step;
            if(phase_step < -M_PI)
            {
                phase_step += 2*M_PI;
            }

            if(phase_step > M_PI)
            {
                phase_step -= 2*M_PI;
            }

            if((t_min+(0.5+n)*Dt) > Dt)
            {
                chirp += phase_step/(2*M_PI)/(t_min+(0.5+n)*Dt);
                num_steps += 1;
            }


            //av_phase_step += phase_step;

        }
    }

    if(num_steps>0)
        chirp /= num_steps; // average liniar chirp dν/dt
    //double chirp = av_phase_step/Dv; //dν/dt*/


    std::vector<std::complex<double>> E1;
    for(int n=0; n<n0; ++n)
        E1[n] = E[n];


    std::vector<std::complex<double>> E2(n0); // field in frequency domain
    FFT(E1.data(), E2.data());
    for(int n=0; n<n0; n++)
    {
        double v = v_min+Dv*(0.5+n);
        v -= v0;//pulse->vc; // relative frequency (v-vc)
        int n1 = (n<n0/2 ? n+n0/2 : n-n0/2);
        E2[n1] *= exp(-I*M_PI*v*v/chirp); // remove chirp
    }
    IFFT(E2.data(), E1.data());



    for(int n=1; n<n0; ++n)
    {
            phase_step = arg(E1[n]) - arg(E1[n-1]);

            phase[n] = phase[n-1] + phase_step;
            if(phase_step < -M_PI)
            {
                phase[n] += 2*M_PI;
            }

            if(phase_step > M_PI)
            {
                phase[n] -= 2*M_PI;
            }

            phase[n] += (t_min+(0.5+n)*Dt)*chirp;
    }



    // Phase relative to central time point
    double offset;
    if(t_min<-Dt && t_max>Dt)
    {
        int zerotime_n = int(-t_min/Dt);
        offset = phase[zerotime_n] - std::arg(E[zerotime_n]);
    }
    else
    {
        offset = phase[n0/2] - arg(E[n0/2]);
    }
    for(int n=0; n<n0; ++n)
    {
        phase[n] -= offset;
    }
}
