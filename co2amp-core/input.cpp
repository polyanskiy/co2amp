#include  "co2amp.h"

bool ReadCommandLine(int argc, char **argv)
{
    int i;

    // ------- INITIAL PULSE -------
    //from_file = 0;          // 1: use input field from files 'field_in_re.dat' and 'field_in_im.dat'
    //E0 = 1;                 // Initial pulse energy, J
    //w0 = 0.01;              // Initial beam radius (w), m
    //tau0 = 5e-12;           // Initial pulse duration (FWHM), s
    //vc = -1;      // Carrying frequency, Hz; default 10P(20) line
    //t_inj = 0;              // Injection moment, s
    //n_pulses = 1;           // Number of pulses in the train
    // ------- OPTICS, GEOMETRY -------
    noprop = false;           // Skip propagation calculations
    // ------- CALCULATION NET -------
    vc = -1;                  // Center frequency, Hz
    x0 = -1;
    n0 = -1;
    t_pulse_lim = -1;         // Pulse time calculation limit, s
    t_pulse_shift = -1;       // Pulse shift from 0, s
    Dt_pump = 2.0e-9;         // Time net step for pumping/relaxation calculations, s (fixed!)
    // ---------- DEBUGGING ----------
    debug_level = 0;          // No debugging info output by default

    //Read command line
    std::string debug_str = "Command line: ";
    for (i=1; i<argc; i++){
        debug_str += argv[i];
        debug_str += " ";
        // ------- INITIAL PULSE -------
        if (!strcmp(argv[i], "-vc"))
            vc = atof(argv[i+1])*1e12;      // THz->Hz;
        // ------- LAYOUT -------
        if (!strcmp(argv[i], "-noprop"))
            noprop = true;
        // ------- CALCULATION NET -------
        if (!strcmp(argv[i], "-x0"))
            x0 = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-n0"))
            n0 = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-t_pulse_lim"))
            t_pulse_lim = atof(argv[i+1])*1e-12;   // ps->s
        if (!strcmp(argv[i], "-t_pulse_shift"))
            t_pulse_shift = atof(argv[i+1])*1e-12; // ps->s
        // --------- DEBUGGING ---------
        if (!strcmp(argv[i], "-debug"))
            debug_level = atoi(argv[i+1]);
    }
    
    Debug(2, debug_str);

    if(vc < 0 || x0 < 0 || n0 < 0 || t_pulse_lim < 0 || t_pulse_shift < 0){
        std::cout << "Input ERROR: Missing command line argument(s)\n";
        return false;
    }

    return true;
}


bool ConstantsInit(void)
{
    // Constants
    c = 2.99792458e8; // m/s
    h = 6.626069e-34; // J*s

    //double Dt = t_pulse_lim/(n0-1); // pulse time step, s
    //double Dv = 1.0/(Dt*n0);        // frequency step, Hz
    //double v_min = vc - Dv*(n0-1)/2;
    //double v_max = vc + Dv*(n0-1)/2;


    std::string str, file_content_str, key, value;
    std::ifstream in;
    std::istringstream iss, iss2;

    ////////////////// PROCESS 'config_files.yml', INITIALIZE PULSES AND PULSES /////////////////////////////

    Debug(2, "Interpreting config_files.yml...");
    in = std::ifstream("config_files.yml", std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading config_files.yml)\n";
        return false;
    }
    Debug(2, "config_files.yml content:\n" + file_content_str);

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
            if(type=="PULSE")
                pulses.push_back(Pulse(id));
            if(type=="LAYOUT")
                layout_file_name =  id + ".yml";
            id = "";
            type = "";
        }
    }

    /////////////// PROCESS LAYOUT CONFIGURATION FILE, INITIALIZE LAYOUT CONFIGURATION ///////////////

    if(layout_file_name == ""){
        std::cout << "Input ERROR: No Layout file (?)\n";
        return false;
    }

    Debug(2, "Interpreting layout configuration file (" + layout_file_name +")...");
    in = std::ifstream(layout_file_name, std::ios::in);
    if(in){
        file_content_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    else{
        std::cout << "Error reading " + layout_file_name + "\n";
        return false;
    }
    Debug(2, layout_file_name + " content:\n" + file_content_str);

    Debug(2, "Creating layout form file \'" + layout_file_name + "\'...");
    std::string propagate = "";
    int and_back = -1;
    int times = -1;
    int i = -1;
    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str)){
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        if(key == "- propagate"){
            propagate = value;
            propagate.erase(remove_if(propagate.begin(), propagate.end(), isspace), propagate.end()); // remove spaces
        }
        if(key == "  and_back"){
            if(value.find("true") != std::string::npos)
                and_back = 1;
            if(value.find("false") != std::string::npos)
                and_back = 0;
        }
        if(key == "  times")
            times = std::stoi(value);

        if(propagate != "" && and_back != -1 && times != -1){
            Debug(2, "propagate = \"" + propagate + "\"; and_back = " + std::to_string(and_back) +
                  "; times = " + std::to_string(times));

            Debug(2, "Reading \"propagate\" entries (separated by \'>\'):");

            iss2 = std::istringstream(propagate);
            while(std::getline(iss2, value, '>')){
                if(value != ""){
                    if(is_number(value)){
                        Debug(2, "Beam propagation distance: " + value + " mm");
                        if(i==-1){
                            std::cout << "Layout error: first entry must be an optic\n";
                            return false;
                        }
                        bool flag = layout[i].distance == 0;
                        layout[i].distance += std::stod(value);
                        if(flag)
                            Debug(2, "propagation after layout component #" + std::to_string(i) +
                                  " set at " + std::to_string(layout[i].distance) + " mm");
                        else
                            Debug(2, "added " + value + " mm propagation after layout component #" +
                              std::to_string(i) + " (now " + std::to_string(layout[i].distance) + " mm)");
                    }
                    else{
                        i++;
                        Debug(2, "Optic entry: \"" + value + "\"");
                        Optic *optic = FindOpticByID(value);
                        if(optic == nullptr){
                            std::cout << "Error in layout configuration: cannot find optic \"" << value << "\"\n";
                            return false;
                        }
                        Debug(2, "\"" + optic->id + "\" optic found! Adding as layout component #" + std::to_string(i));
                        layout.push_back(optic);
                    }
                }
            }

            propagate = "";
            and_back = -1;
            times = -1;
        }
    }

    if(layout[i].optic->type != "P"){
        std::cout << "Layout error: last optic must be type \'P\' (probe)\n";
        return false;
    }

    if(layout[i].distance != 0){
        std::cout << "Layout error: there should be no propagation after the last optic\n";
        return false;
    }


    /*// Optical layout: count number of propagations
    n_propagations = 0;
    in = std::ifstream(layout_file_name, std::ios::in);
    if (in){
        file_str = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
    }
    Debug(2, "Layout configuration file content:\n" + file_str);

    std::regex rgx("[\\s,\\-\\(\\)]+");
    std::sregex_token_iterator iter(file_str.begin(), file_str.end(), rgx, -1);
    std::sregex_token_iterator end;
    for ( ; iter != end; ++iter){
        //std::cout << *iter << '\n';
        n_propagations ++;
    }
    n_propagations = (n_propagations+1)/2;

    std::cout << "pulses: " << pulses.size()
              << "; optics: " << optics.size()
              << "; propagations: " << n_propagations
              << std::endl;

    //if(n_propagations ==0){
    //    std::cout
    //}*/

    /*std::vector<Optic>::iterator itr;
    for(itr=optics.begin(); itr!=optics.end(); itr++){
        std::cout << itr->id << "\t"
                  << itr->type << "\t"
                  << itr->yaml << "\t"
                  << itr->Dr << "\t" << std::endl;
    }*/

    return true;
}


void ArraysInit(void)
{
    /*int i, k, K;

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
    }*/
}


void IntensityNormalization(void) // Field amplitude adjustment (to match initial pulse energy)
{
    /*int pulse, x, n;
    double Energy, af;
    double Dr = optics[layout_component[0]].Dr;
    double Dt = t_pulse_lim/(n0-1);

    for(pulse=0; pulse<=n_pulses-1; pulse++){
        Energy = 0;
        for(n=0; n<n0-1; n++){
            for(x=0; x<x0-1; x++)
                Energy += 2.0 * h * vc * pow(abs(E[pulse][x][n]+E[pulse][x][n+1]+E[pulse][x+1][n]+E[pulse][x+1][n+1])/4, 2) * 2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J
            }

        af = sqrt(E0/Energy);
        for(n=0; n<n0; n++){
            for(x=0; x<x0; x++)
                E[pulse][x][n] *= af;
        }
    }*/
}


void InitializeE()
{
    /*int pulse, x, n;
    FILE *file;

    double Dr = optics[layout_component[0]].Dr;
    double Dt = t_pulse_lim/(n0-1);

    if(!from_file){
        for(pulse=0; pulse<n_pulses; pulse++){
            for(x=0; x<x0; x++){
                for(n=0; n<n0; n++)
                    E[pulse][x][n] = field(Dr*x, Dt*n);
            }
        }
        IntensityNormalization();
    }
    else{
        file = fopen("field_in.bin", "rb");
        for(pulse=0; pulse<n_pulses; pulse++){
            for(x=0; x<x0; x++)
                fread(E[pulse][x], sizeof(std::complex<double>)*n0, 1, file);
        }
        fclose(file);
    }*/
}


std::complex<double> field(double r, double t)
{
    /*double xx;
    xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
    std::complex<double> pulse = exp(-pow((t-t_pulse_shift)/xx, 2));
    //xx = r0/sqrt(log(2.0)/2.0);   //(hwhm -> half-width @ 1/e^2)
    //double beam = exp(-pow(r/xx, 2.0));
    std::complex<double> beam = exp(-pow(r/w0, 2.0));
    return pulse*beam;*/
}
