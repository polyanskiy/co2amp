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

void UnwrapPhase(Pulse* pulse, int x, double* phase)
{
    std::vector<std::complex<double>> E1(n0);
    double Emax = 0;
    int num_steps = 0;
    double chirp = 0;

    for(int n=0; n<n0; ++n)
    {
        // invert shift between v0 and vc
        E1[n] = pulse->E[n0*x+n] * exp(-I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
        // find max field
        Emax = std::max(Emax, std::abs(E1[n]));
    }

    // 1st run: rough-estimate and subtract chirp
    for(int n=1; n<n0; ++n)
    {
        if(std::abs(pulse->E[n0*x+n])/Emax > 1e-2) // only consider intense part of the pulse
        {
            double phase_step = arg(E1[n]) - arg(E1[n-1]);

            if(phase_step < -M_PI)
                phase_step += 2*M_PI;

            if(phase_step > M_PI)
                phase_step -= 2*M_PI;

            double t = t_min+Dt*(0.5+n);
            if(t > Dt)
            {
                chirp += phase_step/(2*M_PI) / t / Dt;
                num_steps += 1;
            }
        }
    }

    if(num_steps>0)
        chirp /= num_steps; // average liniar chirp dν/dt

    for(int n=0; n<n0; ++n)
    {
        double t = t_min+Dt*(0.5+n);
        E1[n] *= exp(-I*M_PI*t*t*chirp); // subtracting chirp
    }

    // Rub 2: get unwrapped phase of de-chirped pulse
    phase[0] = 0;
    for(int n=1; n<n0; ++n)
    {
        double phase_step = arg(E1[n]) - arg(E1[n-1]);

        if(phase_step < -M_PI)
            phase_step += 2*M_PI;

        if(phase_step > M_PI)
            phase_step -= 2*M_PI;

        // calculate the phase taking chirp into account
        double t = t_min+Dt*(0.5+n);
        phase[n] = phase[n-1] + phase_step + 2*M_PI*chirp*t*Dt;
    }

    // Shift phase so that the absolute phase at t=0 is preserved
    double offset;
    if(t_min<-Dt && t_max>Dt) // t=0 is within the pulse time range
    {
        int zerotime_n = int(-t_min/Dt);
        offset = phase[zerotime_n] - std::arg(pulse->E[n0*x+zerotime_n]);
    }
    else // t=0 is not in the pule time range: use middle point (is this scenario realistic???)
    {
        offset = phase[n0/2] - arg(pulse->E[n0*x+n0/2]);
    }

    for(int n=0; n<n0; ++n)
        phase[n] -= offset;
}
