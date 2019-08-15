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

    Debug(2, "Interpreting configuration file list \'" + path + "\'");
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
            Debug(2, "Found entry: ID \"" + id + "\", Type \"" + type + "\"");
            if(type=="A")
                optics.push_back(new A(id));
            if(type=="C")
                optics.push_back(new C(id));
            if(type=="F")
                optics.push_back(new F(id));
            if(type=="L")
                optics.push_back(new L(id));
            if(type=="M")
                optics.push_back(new M(id));
            if(type=="P")
                optics.push_back(new P(id));
            if(type=="S")
                optics.push_back(new S(id));
            if(type=="LAYOUT")
                layout_file_name = id + ".yml";
            if(type=="PULSE")
                pulses.push_back(new Pulse(id));
            id = "";
            type = "";
        }
        if(configuration_error)
            return false;
    }

    // add optic numbers to all optics
    for(int optic_n=0; optic_n<optics.size(); optic_n++)
        optics[optic_n]->number =optic_n;

    // When all optics created, create layout...
    if(!ReadLayoutConfigFile(layout_file_name))
        return false;

    // ... and then initialize pulses (Rmin of first layout element needed for 'InitializeE')
    for(int pulse_n=0; pulse_n<pulses.size(); pulse_n++){
        pulses[pulse_n]->number = pulse_n;
        pulses[pulse_n]->InitializeE();
    }

    // add plane numbers to all layout planes
    for(int plane_n=0; plane_n<layout.size(); plane_n++)
        layout[plane_n]->number =plane_n;

    return true;
}


bool ReadLayoutConfigFile(std::string path){

    std::string str, file_content_str, key, value;
    std::ifstream in;
    std::istringstream iss, iss2;
    std::string go = "";
    int times = -1;
    int plane_n = -1;
    int prop_n;

    Debug(2, "Interpreting layout configuration file \'" + path +"\'");
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

    Debug(2, "Creating layout form file \'" + path + "\'");
    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        if(key == "- go"){
            go = value;
            go.erase(remove_if(go.begin(), go.end(), isspace), go.end()); // remove spaces
        }
        if(key == "  times")
            times = std::stoi(value);

        if(go != "" && times != -1){
            Debug(2, "go = \"" + go + "\"; times = " + std::to_string(times));
            Debug(2, "Reading \'go\' entries (separated by \'>\'):");
            for(prop_n=0; prop_n<times; prop_n++){
                Debug(2, "Propagation #" + std::to_string(prop_n+1) + " of " + std::to_string(times));
                iss2 = std::istringstream(go);
                while(std::getline(iss2, value, '>')){
                    if(value == "")
                        continue;
                    if(is_number(value)){
                        Debug(2, "Propagation distance: " + value + " m");
                        if(plane_n==-1){
                            std::cout << "Layout error: first entry must be an optic\n";
                            return false;
                        }
                        bool flag = layout[plane_n]->space == 0;
                        layout[plane_n]->space += std::stod(value);
                        if(flag)
                            Debug(2, "Space after plane #" + std::to_string(plane_n) +
                                  " set at " + std::to_string(layout[plane_n]->space) + " m");
                        else
                            Debug(2, "Added " + value + " m space after plane #" +
                              std::to_string(plane_n) + " (now " + std::to_string(layout[plane_n]->space) + " m)");
                    }
                    else{
                        plane_n++;
                        Debug(2, "Plane entry: optic \"" + value + "\"");
                        Optic *optic = FindOpticByID(value);
                        if(optic == nullptr){
                            std::cout << "Error in layout configuration: cannot find optic \"" << value << "\"\n";
                            return false;
                        }
                        Debug(2, "\"" + optic->id + "\" optic found. Adding as plane #" + std::to_string(plane_n));
                        layout.push_back(new Plane(optic));
                    }
                }
            }

            go = "";
            times = -1;
        }
    }

    // Print full layout for debugging
    Debug(1, "LAYOUT:");
    if(debug_level>=1){
        for(plane_n=0; plane_n<layout.size(); plane_n++){
            std::cout << layout[plane_n]->optic->id;
            if(plane_n != layout.size()-1)
                std::cout << ">>" ;
            if(layout[plane_n]->space != 0)
                std::cout << std::to_string(layout[plane_n]->space) << ">>";
        }
        std::cout << "\n";
    }

    if(layout[layout.size()-1]->optic->type != "P"){
        std::cout << "Layout error: last plane must be optic type \'P\' (probe)\n";
        return false;
    }

    if(layout[layout.size()-1]->space != 0){
        std::cout << "Layout error: there should be no space after the last plane\n";
        return false;
    }

    // calculate layout plane's "time" (distance from first surface in seconds)
    double time = 0;
    for(plane_n=0; plane_n<layout.size(); plane_n++){
        layout[plane_n]->time = time;
        time += layout[plane_n]->space / c;
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



