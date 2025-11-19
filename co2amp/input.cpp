#include  "co2amp.h"

std::string ReadCommandLine(int argc, char **argv)
{       
    // CALCULATION ARGUMeNTS
    v0 = -1;           // Center frequency, Hz
    x0 = -1;           // # of bins in radial coordinate grid
    n0 = -1;           // # of bins in time & frequency grids
    t_min = -1;        // Pulse time calculation limit, s
    t_max = -1;        // Pulse shift from 0, s
    time_tick = -1;    // Time step for main (slow) time, s
    method = 1;        // Fresnell diffraction with cylindrical symmetry
    debug_level = 1;   // Only the debugging info relevant for user
    search_dir = "";   // Additional directory for HDF5 pulse files

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
        if (!strcmp(argv[i], "-v0") || !strcmp(argv[i], "-vc")) //vc: temporary for backwards compatibility
        {
            v0 = atof(argv[i+1]);
            count = count | 1;
        }
        if (!strcmp(argv[i], "-x0"))
        {
            x0 = atoi(argv[i+1]);
            count = count | 2;
        }
        if (!strcmp(argv[i], "-n0"))
        {
            n0 = atoi(argv[i+1]);
            count = count | 4;
        }
        if (!strcmp(argv[i], "-t_min"))
        {
            t_min = atof(argv[i+1]);
            count = count | 8;
        }
        if (!strcmp(argv[i], "-t_max"))
        {
            t_max = atof(argv[i+1]);
            count = count | 16;
        }
        if (!strcmp(argv[i], "-time_tick"))
        {
            time_tick = atof(argv[i+1]);
            count = count | 32;
        }
        // Optional parameters
        if (!strcmp(argv[i], "-method"))
            method = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-debug"))
            debug_level = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-search_dir"))
            search_dir = argv[i+1];
    }
    
    Debug(1, debug_str);

    if(count != 63) //1+2+4+6+8+16+32 <=> not all required calculation parameters provided
    {
        std::cout << "Input ERROR: Missing command line argument(s)\n";
        return "";
    }

    Dt = (t_max-t_min)/n0;
    Dv = 1.0/(t_max-t_min);
    v_min = v0 - 0.5/Dt;
    //v_max = v0 + 0.5/Dt; // not used - keep for future reference

    if(time_tick <= Dt)
    {
        std::cout << "Lab time tick must be longer than pulse time step (" << Dt << " s)\n";
        return "";
    }

    return "calc_arguments";
}


bool ReadConfigFiles(std::string path)
{
    std::string str, file_content_str, key, value;
    std::istringstream iss, iss2;

    Debug(2, "Reading configuration file list \'" + path + "\'");

    if(!YamlReadFile(path, &file_content_str))
    {
        std::cout << "Error reading configuration file list \"" << path << "\"\n";
        configuration_error = true;
        return false;
    }

    std::string id="";
    std::string type="";
    std::string layout_file_name="";
    iss = std::istringstream(file_content_str);
    while(std::getline(iss, str))
    {
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        value.erase(remove_if(value.begin(), value.end(), isspace), value.end()); // remove spaces
        if(key == "- id")
            id = value;
        if(key == "  type")
            type = value;
        if(id != "" && type != "")
        {
            Debug(2, "Found entry: ID \"" + id + "\", Type \"" + type + "\"");
            if(type=="A")
                optics.push_back(new A(id, type));
            if(type=="C")
                optics.push_back(new C(id, type));
            if(type=="F")
                optics.push_back(new F(id, type));
            if(type=="L")
                optics.push_back(new L(id, type));
            if(type=="M")
                optics.push_back(new M(id, type));
            if(type=="P")
                optics.push_back(new P(id, type));
            if(type=="S")
                optics.push_back(new S(id, type));
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
    for(size_t i=0; i<optics.size(); i++)
        optics[i]->number = i;

    // When all optics created, create layout...
    if(!ReadLayoutConfigFile(layout_file_name))
        return false;

    // add plane numbers to all layout planes
    for(size_t i=0; i<planes.size(); ++i)
        planes[i]->number = i;

    // Once basic geometry is established, finalize initialization of pulses and optics

    // Initialize pulses
    for(size_t i=0; i<pulses.size(); ++i)
    {
        pulses[i]->number = i;
        pulses[i]->Initialize();
        if(configuration_error)
            return false;
        if(i>0)
        {
            if(pulses[i]->time_in < pulses[i-1]->time_in)
            {
                std::cout << "Arrange pulses in order of injection (smaller \'t_in\' first)!\n";
                return false;
            }
            if(pulses[i]->time_in - pulses[i-1]->time_in < (t_max-t_min))
            {
                std::cout << "Error: Delay between pulses \"" << pulses[i-1]->id << "\" and \"" << pulses[i]->id << "\" must be longer than pulse time range (" << (t_max-t_min) << " s)\n";
                return false;
            }
        }
    }

    // now we can calculate number of points in the lab (slow) time grid
    // (t_max-t_min) is the duration of the pulse (slow) time grid
    m0 = (pulses.back()->time_in + planes.back()->time_from_first_plane + (t_max-t_min)) / time_tick + 1; //rounding toward 0

    Debug(1, "m0 = " + std::to_string(m0));

    // Initialize optics
    for(size_t i=0; i<optics.size(); ++i)
    {
        Debug(2, "Initializing optic \'" + optics[i]->id + "\' using content of \'" + optics[i]->yaml_path + "\'");
        optics[i]->Initialize();
        if(configuration_error)
            return false;
    }

    return true;
}


bool ReadLayoutConfigFile(std::string path)
{
    std::string str, yaml_file_content, key, value;
    std::istringstream iss, iss2;
    std::string go = "";
    int times = -1;
    int plane_n = -1;
    int prop_n;

    Debug(1, "*** LAYOUT ***");

    Debug(2, "Reading layout configuration file \'" + path +"\'");

    if(!YamlReadFile(path, &yaml_file_content))
    {
        std::cout << "Error reading layout configuration file \"" + path + "\"\n";
        configuration_error = true;
        return false;
    }

    Debug(2, "Creating layout form file \'" + path + "\'");
    iss = std::istringstream(yaml_file_content);
    bool flag_gotimesfound = false;
    while(std::getline(iss, str))
    {
        iss2 = std::istringstream(str);
        std::getline(iss2, key, ':');
        std::getline(iss2, value);
        if(key == "- go")
        {
            if(go != "" || times != -1) // missing 'times' or previous 'go'
                break;
            go = value;
            go.erase(remove_if(go.begin(), go.end(), isspace), go.end()); // remove spaces
        }
        if(key == "  times")
        {
            if(times != -1) // missing 'go'
                break;
            times = std::stoi(value);
        }

        if(go != "" && times != -1)
        {
            flag_gotimesfound = true;
            Debug(2, "go = \"" + go + "\"; times = " + std::to_string(times));
            Debug(2, "Reading \'go\' entries (separated by \'>\'):");
            for(prop_n=0; prop_n<times; prop_n++)
            {
                Debug(2, "Propagation #" + std::to_string(prop_n+1) + " of " + std::to_string(times));
                iss2 = std::istringstream(go);
                while(std::getline(iss2, value, '>'))
                {
                    if(value == "")
                        continue;
                    if(is_number(value))
                    {
                        Debug(2, "Propagation distance: " + value + " m");
                        if(plane_n==-1)
                        {
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
                    else
                    {
                        plane_n++;
                        Debug(2, "Plane entry: optic \"" + value + "\"");
                        Optic *optic = FindOpticByID(value);
                        if(optic == nullptr)
                        {
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
    if(go != "")
    {
        std::cout << "missing \'times\' key in layout configuration file \'" << path << "\'\n";
        std::cout << "go: " << go << "\n";
        return false;
    }
    if(times != -1)
    {
        std::cout << "missing \'go\' key in layout configuration file \'" << path << "\'\n";
        std::cout << "times: " << times << "\n";
        return false;
    }
    if(!flag_gotimesfound)
    {
        std::cout << "Layout error: cannot find a pair of \'go\' and \'times\' values in layout configuration file \'" << path << "\'\n";
        return false;
    }

    // Print full layout for debugging
    if(debug_level>=1)
    {
        for(size_t i=0; i<planes.size(); i++)
        {
            std::cout << planes[i]->optic->id;
            if(i != planes.size()-1)
                std::cout << ">>" ;
            if(planes[i]->space != 0)
                std::cout << std::to_string(planes[i]->space) << ">>";
        }
        std::cout << "\n";
    }

    if(planes.back()->optic->type != "P")
    {
        std::cout << "Layout error: last plane must be optic type \'P\' (probe)\n";
        return false;
    }

    if(planes.back()->space != 0)
    {
        std::cout << "Layout error: there should be no space after the last plane\n";
        return false;
    }

    for(size_t i=0; i<planes.size(); ++i)
    {
        if(planes[i]->optic->type=="A" && planes[i]->space < (t_max-t_min)*c )
        {
            if(planes[i+1]->optic->type != "A")
            {
                std::cout << "Layout error: Distance after amplifier optic \"" << planes[i]->optic->id
                          << "\" must be longer than pulse time range (" << (t_max-t_min)*c << " m)\n";
                return false;
            }
            std::cout << "Warning: Distance between amplifier sections \"" << planes[i]->optic->id
                      << "\" and " << planes[i+1]->optic->id << "\" is less than pulse time range ("
                      << (t_max-t_min)*c << " m)\n";
            std::cout << "         pulse at \"" << planes[i+1]->optic->id << "\" will not be saved for corresponding pass\n";
        }
    }

    // calculate layout plane's "time" (distance from first surface in seconds)
    double time = 0;
    for(size_t i=0; i<planes.size(); ++i)
    {
        planes[i]->time_from_first_plane = time;
        time += planes[i]->space / c;
    }

    return true;
}
