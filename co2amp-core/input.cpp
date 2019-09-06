#include  "co2amp.h"

std::string ReadCommandLine(int argc, char **argv)
{       
    // CALCULATION ARGUMeNTS
    vc = -1;         // Center frequency, Hz
    x0 = -1;         // # of bins in radial coordinate grid
    n0 = -1;         // # of bins in time & frequency grids
    t_min = -1;      // Pulse time calculation limit, s
    t_max = -1;      // Pulse shift from 0, s
    time_tick = -1;  // Time step for main (slow) time, s
    noprop = false;  // Skip propagation calculations
    debug_level = 0; // No debugging info output by default

    //Read command line
    int count = 0;
    std::string debug_str = "Command line: ";
    for (int i=1; i<argc; i++){
        debug_str += argv[i];
        debug_str += " ";

        // -------- VERSION --------
        if (!strcmp(argv[i], "-version")) // version info requested
            return "version";

        // - CALCULATION ARGUMENTS -
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
        if (!strcmp(argv[i], "-time_tick")){
            time_tick = atof(argv[i+1]);
            count = count | 32;
        }
        if (!strcmp(argv[i], "-noprop")){ // -optional-
            noprop = true;
        }
        if (!strcmp(argv[i], "-debug")){  // -optional-
            debug_level = atoi(argv[i+1]);
        }

    }
    
    Debug(1, debug_str);

    if(count == 63) //1+2+4+6+8+16+32 <=> all required calculation parameters provided
        return "calc_arguments";


    return ""; // something is missing from the command line
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
        pulses[pulse_n]->Initialize();
        if(configuration_error)
            return false;
        if(pulse_n>0 && pulses[pulse_n]->time_inj < pulses[pulse_n-1]->time_inj){
            std::cout << "Arrange pulses in order of injection (smaller \'t_inj\' first)!\n";
            return false;
        }
    }

    // add plane numbers to all layout planes
    for(int plane_n=0; plane_n<planes.size(); plane_n++)
        planes[plane_n]->number =plane_n;

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
    bool flag_gotimesfound = false;
    while(std::getline(iss, str)){
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        if(key == "- go"){
            if(go != "" || times != -1) // missing 'times' or previous 'go'
                break;
            go = value;
            go.erase(remove_if(go.begin(), go.end(), isspace), go.end()); // remove spaces
        }
        if(key == "  times"){
            if(times != -1) // missing 'go'
                break;
            times = std::stoi(value);
        }

        if(go != "" && times != -1){
            flag_gotimesfound = true;
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
                        bool flag = planes[plane_n]->space == 0;
                        planes[plane_n]->space += std::stod(value);
                        if(flag)
                            Debug(2, "Space after plane #" + std::to_string(plane_n) +
                                  " set at " + std::to_string(planes[plane_n]->space) + " m");
                        else
                            Debug(2, "Added " + value + " m space after plane #" +
                              std::to_string(plane_n) + " (now " + std::to_string(planes[plane_n]->space) + " m)");
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
                        planes.push_back(new Plane(optic));
                    }
                }
            }

            go = "";
            times = -1;
        }
    }

    // Error handlers
    if(go != ""){
        std::cout << "missing \'times\' key in layout configuration file \'" << path << "\'\n";
        std::cout << "go: " << go << "\n";
        return false;
    }
    if(times != -1){
        std::cout << "missing \'go\' key in layout configuration file \'" << path << "\'\n";
        std::cout << "times: " << times << "\n";
        return false;
    }
    if(!flag_gotimesfound){
        std::cout << "Layout error: cannot find a pair of \'go\' and \'times\' values in layout configuration file \'" << path << "\'\n";
        return false;
    }

    // Print full layout for debugging
    Debug(1, "LAYOUT:");
    if(debug_level>=1){
        for(plane_n=0; plane_n<planes.size(); plane_n++){
            std::cout << planes[plane_n]->optic->id;
            if(plane_n != planes.size()-1)
                std::cout << ">>" ;
            if(planes[plane_n]->space != 0)
                std::cout << std::to_string(planes[plane_n]->space) << ">>";
        }
        std::cout << "\n";
    }

    if(planes[planes.size()-1]->optic->type != "P"){
        std::cout << "Layout error: last plane must be optic type \'P\' (probe)\n";
        return false;
    }

    if(planes[planes.size()-1]->space != 0){
        std::cout << "Layout error: there should be no space after the last plane\n";
        return false;
    }

    // calculate layout plane's "time" (distance from first surface in seconds)
    double time = 0;
    for(plane_n=0; plane_n<planes.size(); plane_n++){
        planes[plane_n]->time_from_first_plane = time;
        time += planes[plane_n]->space / c;
    }

    return true;
}
