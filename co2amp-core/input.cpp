#include  "co2amp.h"

bool ReadCommandLine(int argc, char **argv)
{
    int i;

    // ------- OPTICS, GEOMETRY -------
    noprop = false;           // Skip propagation calculations
    // ------- CALCULATION NET -------
    vc = -1;                  // Center frequency, Hz
    x0 = -1;
    n0 = -1;
    t_min = -1;               // Pulse time calculation limit, s
    t_max = -1;               // Pulse shift from 0, s
    clock_tick = -1;         // Time net step for pumping/relaxation calculations, s (fixed!)
    // ---------- DEBUGGING ----------
    debug_level = 0;          // No debugging info output by default

    //Read command line
    int count = 0;
    std::string debug_str = "Command line: ";
    for (i=1; i<argc; i++){
        debug_str += argv[i];
        debug_str += " ";

        // ------- LAYOUT -------
        if (!strcmp(argv[i], "-noprop"))
            noprop = true;

        // ------- CALCULATION GRID -------
        if (!strcmp(argv[i], "-vc")){
            vc = atof(argv[i+1]);
            count = count | 1;
        }

        if (!strcmp(argv[i], "-x0")){
            x0 = atoi(argv[i+1]);
            count = count | 2;
        }
        if (!strcmp(argv[i], "-n0")){
            n0 = atoi(argv[i+1]);
            count = count | 4;
        }
        if (!strcmp(argv[i], "-t_min")){
            t_min = atof(argv[i+1]);
            count = count | 8;
        }
        if (!strcmp(argv[i], "-t_max")){
            t_max = atof(argv[i+1]);
            count = count | 16;
        }
        if (!strcmp(argv[i], "-clock_tick")){
            clock_tick = atof(argv[i+1]);
            count = count | 32;
        }
        // --------- DEBUGGING ---------
        if (!strcmp(argv[i], "-debug"))
            debug_level = atoi(argv[i+1]);
    }
    
    Debug(2, debug_str);

    if(count != 63){ //1+2+4+6+8+16+32
        std::cout << "Input ERROR: Missing command line argument(s)\n";
        return false;
    }

    return true;
}


bool ReadConfigFiles(std::string path)
{
    std::string str, file_content_str, key, value;
    std::ifstream in;
    std::istringstream iss, iss2;

    Debug(2, "Interpreting configuration file list \"" + path + "\"...");
    in = std::ifstream(path, std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading configuration file list \"" + path + "\"\n";
        return false;
    }
    Debug(3, path + " content:\n" + file_content_str);

    std::string id="";
    std::string type="";
    std::string layout_file_name="";
    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        value.erase(remove_if(value.begin(), value.end(), isspace), value.end()); // remove spaces
        if(key == "- id")
            id = value;
        if(key == "  type")
            type = value;
        if(id != "" && type != ""){
            Debug(2, "ID: \"" + id + "\"; Type: \"" + type + "\"");
            if(type=="A")
                optics.push_back(A(id));
            if(type=="C")
                optics.push_back(C(id));
            if(type=="L")
                optics.push_back(L(id));
            if(type=="M")
                optics.push_back(M(id));
            if(type=="F")
                optics.push_back(F(id));
            if(type=="P")
                optics.push_back(P(id));
            if(type=="S")
                optics.push_back(S(id));
            if(type=="LAYOUT")
                layout_file_name = id + ".yml";
            if(type=="PULSE")
                pulses.push_back(Pulse(id));
            id = "";
            type = "";
        }
    }

    // add optic numbers to all optics
    for(int optic_n=0; optic_n<optics.size(); optic_n++)
        optics[optic_n].optic_n =optic_n;

    // When all optics created, create layout...
    if(!ReadLayoutConfigFile(layout_file_name))
        return false;

    // ... and then initialize pulses (CA of first layout element needed for 'InitializeE')
    for(int pulse_n=0; pulse_n<pulses.size(); pulse_n++){
        pulses[pulse_n].pulse_n = pulse_n;
        pulses[pulse_n].InitializeE();
    }

    return true;
}


bool ReadLayoutConfigFile(std::string path){

    std::string str, file_content_str, key, value;
    std::ifstream in;
    std::istringstream iss, iss2;
    std::string propagate = "";
    int times = -1;
    int layout_position = -1;
    int prop_n;

    Debug(2, "Interpreting layout configuration file \"" + path +"\"...");
    in = std::ifstream(path, std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading layout file \'" + path + "\'\n";
        return false;
    }
    Debug(3, path + " content:\n" + file_content_str);

    Debug(2, "Creating layout form file \'" + path + "\'...");
    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        if(key == "- propagate"){
            propagate = value;
            propagate.erase(remove_if(propagate.begin(), propagate.end(), isspace), propagate.end()); // remove spaces
        }
        if(key == "  times")
            times = std::stoi(value);

        if(propagate != "" && times != -1){            
            Debug(2, "propagate = \"" + propagate + "\"; times = " + std::to_string(times));
            Debug(2, "Reading \"propagate\" entries (separated by \'>\'):");
            for(prop_n=0; prop_n<times; prop_n++){
                Debug(2, "Propagation #" + std::to_string(prop_n+1) + " of " + std::to_string(times) + " ...");
                iss2 = std::istringstream(propagate);
                while(std::getline(iss2, value, '>')){
                    if(value == "")
                        continue;
                    if(is_number(value)){
                        Debug(2, "Beam propagation distance: " + value + " mm");
                        if(layout_position==-1){
                            std::cout << "Layout error: first entry must be an optic\n";
                            return false;
                        }
                        bool flag = layout[layout_position].space == 0;
                        layout[layout_position].space += std::stod(value);
                        if(flag)
                            Debug(2, "space after layout component #" + std::to_string(layout_position) +
                                  " set at " + std::to_string(layout[layout_position].space) + " m");
                        else
                            Debug(2, "added " + value + " m space after layout component #" +
                              std::to_string(layout_position) + " (now " + std::to_string(layout[layout_position].space) + " m)");
                    }
                    else{
                        layout_position++;
                        Debug(2, "Optic entry: \"" + value + "\"");
                        Optic *optic = FindOpticByID(value);
                        if(optic == nullptr){
                            std::cout << "Error in layout configuration: cannot find optic \"" << value << "\"\n";
                            return false;
                        }
                        Debug(2, "\"" + optic->id + "\" optic found! Adding as layout component #" + std::to_string(layout_position));
                        layout.push_back(optic);
                    }
                }
            }

            propagate = "";
            times = -1;
        }
    }

    // Print full layout for debugging
    Debug(1, "LAYOUT:");
    if(debug_level>=1){
        for(layout_position=0; layout_position<layout.size(); layout_position++){
            std::cout << layout[layout_position].optic->id;
            if(layout_position != layout.size()-1)
                std::cout << ">>" ;
            if(layout[layout_position].space != 0)
                std::cout << std::to_string(layout[layout_position].space) << ">>";
        }
        std::cout << "\n";
    }

    if(layout[layout.size()-1].optic->type != "P"){
        std::cout << "Layout error: last optic must be type \'P\' (probe)\n";
        return false;
    }

    if(layout[layout.size()-1].space != 0){
        std::cout << "Layout error: there should be no space after the last optic\n";
        return false;
    }

    // calculate layout component's "time" (distance from first surface in seconds)
    double time = 0;
    for(layout_position=0; layout_position<layout.size(); layout_position++){
        layout[layout_position].time = time;
        time += layout[layout_position].space / c;
    }

    return true;
}


/*void ArraysInit(void)
{
    int i, k, K;

    int N_symm = 0;  //Number of symmetric CO2 molecules
    int N_asymm = 0; // --''-- asymmetric --''--

    if(p_626>0)
        N_symm++;
    if(p_628>0)
        N_asymm++;
    if(p_828>0)
        N_symm++;
    if(p_636>0)
        N_symm++;
    if(p_638>0)
        N_asymm++;
    if(p_838>0)
        N_symm++;

    // Read discharge profile
    if(!strcmp(pumping, "discharge")){
        int size;
        char *str, *file_str;
        FILE *file;
        file = fopen("discharge.txt", "r");
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        rewind(file);
        file_str = malloc(size+1);
        fread(file_str, size, 1, file);
        file_str[size] = '\0'; // string terminating character
        str = strtok(file_str," \t\n\r");
        i=0;
        while(str != NULL){
            discharge[0][i] = atof(str)*1e-6; // time, us->s
            str = strtok(NULL," \t\n\r");
            discharge[1][i] = atof(str);      // Current, A
            str = strtok(NULL," \t\n\r");
            discharge[2][i] = atof(str);      // Voltage, V
            str = strtok(NULL," \t\n\r");
            i++;
        }
        free(file_str);
        fclose(file);
        if(debug_level>=2){
            printf("\nDISCHARGE PROFILE:\n");
            printf("t, us\tI, A\tU, V\n");
            for(i=0; i<n_discharge_points; i++)
                printf("%f\t%f\t%f\n", discharge[0][i]*1e6, discharge[1][i], discharge[2][i]);
            fflush(stdout);
        }
    }

    // Read component data from 'optics_components.txt' file
    int size;
    char *str, *file_str, id[64];
    FILE *file;
    file = fopen("optics_components.txt", "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
    file_str = malloc(size+1);
    fread(file_str, size, 1, file);
    file_str[size] = '\0'; // string terminating character
    str = strtok(file_str," \t\n\r");
    K = 0; //component number
    while(str != NULL){
        if(!strcmp(str, "AM") || !strcmp(str, "PROBE") || !strcmp(str, "MASK") || !strcmp(str, "ATTENUATOR")
                || !strcmp(str, "ABSORBER") || !strcmp(str, "LENS") || !strcmp(str, "WINDOW") || !strcmp(str, "STRETCHER")
                || !strcmp(str, "FILTER") || !strcmp(str, "BANDPASS") || !strcmp(str, "APODIZER") || !strcmp(str, "AIR")){
            strcpy(component_id[K], id);
            strcpy(component_type[K], str);
            component_Dr[K] = atof(strtok(NULL," \t\n\r")) / (x0-1) * 1e-2; // cm -> m
            if(noprop) // no propagation calculations
                component_Dr[K] = component_Dr[0];
            strcpy(component_param1[K], "");
            strcpy(component_param2[K], "");
            if(!strcmp(str, "AM") || !strcmp(str, "MASK") || !strcmp(str, "ATTENUATOR") || !strcmp(str, "ABSORBER")
                    || !strcmp(str, "LENS") || !strcmp(str, "WINDOW") || !strcmp(str, "STRETCHER")
                    || !strcmp(str, "FILTER") || !strcmp(str, "BANDPASS") || !strcmp(str, "APODIZER") || !strcmp(str, "AIR"))
                strcpy(component_param1[K], strtok(NULL," \t\n\r")); // 1st parameter
            if(!strcmp(str, "WINDOW") || !strcmp(str, "BANDPASS") || !strcmp(str, "AIR"))
                strcpy(component_param2[K], strtok(NULL," \t\n\r")); // 2nd parameter
            K ++;
        }
        strcpy(id, str);
        str = strtok(NULL," \t\n\r");
    }
    free(file_str);
    fclose(file);

    printf("\nCOMPONENTS (distances in cm):\n");
    printf("[id]\t\[type]\t\[field]\t[param1]\t[param2]\n");
    for(K=0; K<n_components; K++){
        // output: distances in cm
        printf("%s\t%s\t%f\t%s\t%s\n",component_id[K], component_type[K], component_Dr[K]*(x0-1)*100, component_param1[K], component_param2[K]);
        fflush(stdout);
    }

    // Read component data from 'optics_layout.txt' file
    file = fopen("optics_layout.txt", "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
    file_str = malloc(size+1);
    fread(file_str, size, 1, file);
    file_str[size] = '\0'; // string terminating character
    str = strtok (file_str," -\t\n\r");
    double t = t_inj;
    for(k=0; k<n_propagations; k++){
        layout_component[k] = -1; // flag to show that component id is not found
        for(i=0; i<n_components; i++){
            if(!strcmp(str, component_id[i]))
                layout_component[k] = i;
        }
        layout_time[k] = t;
        str = strtok(NULL," -\t\n\r");
        if(str != NULL)
            layout_distance[k] = atof(str) * 1e-2; // cm -> m
        else
            layout_distance[k] = 0;
        t += layout_distance[k] / c; // t[s]
        str = strtok(NULL," -\t\n\r");
    }
    free(file_str);
    fclose(file);

    int flag_error = 0;
    printf("\nLAYOUT:\n");
    for(k=0; k<n_propagations; k++){
        if(k != 0)
            printf("-");
        if(layout_component[k] != -1)
            printf("%s", component_id[layout_component[k]]);
        else{
            printf("[?]");
            flag_error = 1;
        }
        if(k != n_propagations-1)
            printf("-%.1f", layout_distance[k]*1e2); // m->cm
    }
    printf("\n");
    fflush(stdout);

    if(flag_error){
        printf("\nError in optical layout :(\nAbnormal program termination\n");
        FreeMemory();
        abort();
    }
}*/



