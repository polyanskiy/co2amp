#include  "co2amp.h"

//void YamlGetValue(char *value, char* path, char* key)
void YamlGetValue(std::string value, std::string path, std::string key)
{
    yaml_parser_t parser;
    yaml_token_t  token;
    FILE *file = fopen(path.c_str(), "r");
    //int size;
    //char *str;
    std::string str;

    //fseek(file, 0, SEEK_END);
    //size = ftell(file);
    //rewind(file);
    //str = malloc(size+1);
    //str = new char[size+1];

    //strcpy(value, "");
    value = "";

    if(!file){
        //sprintf(str, "YAML file %s NOT FOUND", path);
        str = "YAML file " + path + " NOT FOUND";
        Debug(2, str);
        //free(str);
        return;
    }
    Debug(2, "Yaml file found");


    if(!yaml_parser_initialize(&parser)){
        Debug(2, "Yaml parser initialization FAILED");
        //free(str);
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
            //sprintf(str, "%s", token.data.scalar.value);
            str = reinterpret_cast<char*>(token.data.scalar.value);
            //if(!strcmp(str, key)){
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
                //sprintf(value, "%s", token.data.scalar.value);
                str = reinterpret_cast<char*>(token.data.scalar.value);
            }
        }
        if(token.type != YAML_STREAM_END_TOKEN)
            yaml_token_delete(&token);
    } while(token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    //free(str);
    //delete str;
    fclose(file);
    return;
}
