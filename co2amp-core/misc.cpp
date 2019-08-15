#include  "co2amp.h"


Optic* FindOpticByID(std::string id){
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


bool YamlGetValue(std::string *value, std::string path, std::string key)
{
    std::string str, file_content_str;
    std::ifstream in;
    std::istringstream iss;

    in = std::ifstream(path, std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading YAML file \'" + path + "\'\n";
        return false;
    }
    Debug(3, path + " content:\n" + file_content_str);

    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        std::size_t found = 1;
        found = str.find(key + ": ");
        if(found == 0){ //key found at the beginnig of the string
            *value = str.substr(std::string(key + ": ").length());
            Debug(3, key + ": " + *value);
            value->erase(remove_if(value->begin(), value->end(), isspace), value->end()); // remove spaces
            return true;
        }
    }

    std::cout << "Key \"" + key + "\" not found in YAML file \'" + path + "\'\n";
    return false;
}


bool YamlGetData(std::vector<double> *data, std::string path, std::string key, int column_n)
{
    std::string str, str2, str3, file_content_str;
    std::ifstream in;
    std::istringstream iss, iss2;

    in = std::ifstream(path, std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading YAML file \'" + path + "\'\n";
        return false;
    }
    Debug(3, path + " content:\n" + file_content_str);

    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        std::size_t found = 1;
        found = str.find(key + ": |");
        if(found == 0){ //key found at the beginnig of the string
            while(std::getline(iss, str2) && str2.substr(0,4) == "    "){
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

    std::cout << "Key \"" + key + "\" not found in YAML file \'" + path + "\'\n";
    return false;
}


double Interpolate(std::vector<double> *X, std::vector<double> *Y, double x)
{
    if(X->size()<=1)
        return 0;
    if(x < (*X)[0])
        return (*Y)[0];
    if(x > (*X)[X->size()-1])
        return (*Y)[Y->size()-1];

    int i;
    for(i=0; i<X->size()-1; i++){
        if(x >= (*X)[i] && x < (*X)[i+1])
            return (*Y)[i] + ((*Y)[i+1]-(*Y)[i]) * (x-(*X)[i])/((*X)[i+1]-(*X)[i]);
    }

    return 0;
}
