#include  "co2amp.h"


Optic* FindOpticByID(std::string id){
    for(unsigned int i = 0; i<optics.size(); i++){
        if(optics[i].id == id)
            return &optics[i];
    }
    return nullptr;
}


bool is_number(std::string s)
{
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && val != HUGE_VAL;
}


bool YamlGetValue(std::string *value, std::string path, std::string key)
{
    std::string str, file_content_str;
    std::ifstream in;
    std::istringstream iss;//, iss2;

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
            return true;
        }
    }

    std::cout << "Key \"" + key + "\" not found in YAML file \'" + path + "\'\n";
    return false;


    /*yaml_parser_t parser;
    yaml_token_t  token;
    FILE *file = fopen(path.c_str(), "r");
    std::string str;
    *value = "";

    if(!file){
        str = "YAML file " + path + " NOT FOUND";
        Debug(2, str);
        return;
    }
    Debug(2, "Yaml file found");


    if(!yaml_parser_initialize(&parser)){
        Debug(2, "Yaml parser initialization FAILED");
        fclose(file);
        return;
    }
    yaml_parser_set_input_file(&parser, file);

    Debug(2, "Yaml parser initialized");

    do {
        yaml_parser_scan(&parser, &token);
        if(token.type == YAML_KEY_TOKEN){
            do{
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
            }while(token.type != YAML_SCALAR_TOKEN && token.type != YAML_STREAM_END_TOKEN);
            if(token.type == YAML_STREAM_END_TOKEN)
                break;
            str = reinterpret_cast<char*>(token.data.scalar.value);
            if(str == key){
                Debug(2, "Key found");
                do{
                    yaml_token_delete(&token);
                    yaml_parser_scan(&parser, &token);
                }while(token.type != YAML_VALUE_TOKEN && token.type != YAML_STREAM_END_TOKEN);
                if(token.type == YAML_STREAM_END_TOKEN)
                    break;
                do{
                    yaml_token_delete(&token);
                    yaml_parser_scan(&parser, &token);
                }while(token.type != YAML_SCALAR_TOKEN && token.type != YAML_STREAM_END_TOKEN);
                if(token.type == YAML_STREAM_END_TOKEN)
                    break;
                Debug(2, "Value found");
                //std::stringstream ss;
                //ss << token.data.scalar.value;
                //ss >> *value;
                *value = reinterpret_cast<char*>(token.data.scalar.value);
                //str = reinterpret_cast<char*>(token.data.scalar.value);
            }
        }
        if(token.type != YAML_STREAM_END_TOKEN)
            yaml_token_delete(&token);
    } while(token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(file);
    return;*/
}
