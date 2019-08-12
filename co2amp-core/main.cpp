#include "co2amp.h"


// GLOBAL VARIABLES

// -- PULSES, OPTICS, GEOMETRY ---
std::vector<Pulse> pulses;
std::vector<Optic> optics;
std::vector<LayoutComponent> layout;
bool noprop;
// ------- CALCULATION NET -------
double vc;
double t_min, t_max;       // fast ("pulse") time
double clock_tick;         // slow "layout" time - e.g. for pumping/relaxation
int x0, n0; // number of points in radial and time nets and number of pulses in the train
// ------- DEBUGGING -------
int debug_level;           // debug output control 0: nothing; 1: some; 2: a lot; 3: everything
bool flag_status_or_debug; // last message displayed: True if status False if debug
// ------- MISC. CONSTANTS -------
double c, h;               // spped of light [m/s]; Plank's [J s]


int main(int argc, char **argv)
{
    // Constants
    c = 2.99792458e8; // m/s
    h = 6.626069e-34; // J*s

    debug_level = 1;
    flag_status_or_debug = true;

    std::cout << "co2amp-core v.2019-08-08" << std::endl << std::flush;

    #pragma omp parallel // counting processors (for parallel computing)
    if(omp_get_thread_num() == 0)
        std::cout << "number of CPU cores: " << omp_get_num_threads() << std::endl << std::endl << std::flush;

    //co2amp = new CO2AMP();

    if (!ReadCommandLine(argc, argv)){
        std::cout << "Error in command line. Aborting...\n";
        return -1;
    }
    Debug(1, "Command line read done!");
    if (!ReadConfigFiles("config_files.yml")){
        std::cout << "Error in configuration file(s). Aborting...\n";
        return -1;
    }
    Debug(1, "Processing configuration files done!");

    Calculations(); // Main program !!!
    /*AllocateMemory();
    Debug(1, "Allocate memory done!");
    ArraysInit();
    Debug(1, "Arrays init done!");
    Calculations(); // Main program !!!
    SaveOutputField();
    StatusDisplay(-1, -1, -1, "done!");
    FreeMemory();*/

    StatusDisplay(-1, -1, -1, "all done!");
    return 0;
}


void Calculations()
{
    int optic_n, pulse_n, layout_position;
    double clock_time;

     // Total pumping energy
    /*if(n_amsections>0 && p_CO2+p_N2+p_He>0){
        double Epump=0;
        double t_lim = t_inj+tbp*(K0-1);
        if(mode==1) // train
            t_lim = t_inj+Dt_train*(K0-1)+tbp;
        for(t=0; t<=t_lim; t+=Dt)
            Epump += Current(t)*Voltage(t) * Dt/1e6;
        printf("\nDischarge energy (withing %f us) = %f J\n", t_lim, Epump);
    }*/

    std::cout << std::endl << "CALCULATING:" << std::endl;

    for(clock_time=0; clock_time<=(layout[layout.size()-1].time + pulses[pulses.size()-1].t0 + clock_tick); clock_time+=clock_tick){

        for(optic_n=0; optic_n<optics.size(); optic_n++)
            optics[optic_n].InternalDynamics(clock_time);

        for(layout_position=0; layout_position<layout.size(); layout_position++){
            for(pulse_n=0; pulse_n<pulses.size(); pulse_n++){
                double time_of_arival = layout[layout_position].time + pulses[pulse_n].t0;
                if(clock_time-clock_tick/2 < time_of_arival && clock_time+clock_tick/2 >= time_of_arival){
                    // output files are written on component input (before interaction)!!!
                    StatusDisplay(pulse_n, layout_position, clock_time, "saving...");
                    UpdateOutputFiles(pulse_n, layout_position, clock_time);
                    // now do interaction and propagation (but not with the last layout component)
                    if(layout_position != layout.size()-1){
                        layout[layout_position].optic->PulseInteraction(pulse_n);
                        pulses[pulse_n].Propagate(layout_position, layout_position+1, clock_time);
                    }
                }
            }
        }
    }

}


void StatusDisplay(int pulse_n, int layout_position, double clock_time, std::string status)
{
    std::string str;
    if(layout_position == -1){
        if(clock_time < 0)
            str = "\r" + status
                    + "                                               ";
        else
            str = "\r" + std::to_string(clock_time) + " s: " + status
                    + "                                               ";
    }
    else{
        if(pulses.size()==1)
            str = "\r" + std::to_string(clock_time) + " s; Optic "
                    + layout[layout_position].optic->id + " ("
                    + std::to_string(layout_position+1) + " of "
                    + std::to_string(layout.size()) + "): " + status
                    + "                    ";
        else
            str = "\r" + std::to_string(clock_time) + " s; Optic "
                    + layout[layout_position].optic->id + " ("
                    + std::to_string(layout_position+1) + " of "
                    + std::to_string(layout.size()) + "); Pulse "
                    + pulses[pulse_n].id + " ("
                    + std::to_string(pulse_n+1) + " of "
                    + std::to_string(pulses.size()) + "): " + status
                    + "                    ";
    }
    std::cout << str << std::flush;
    flag_status_or_debug = true;
}


void Debug(int level, std::string str)
{
    if(level > debug_level)
        return;
    if(flag_status_or_debug) //if last displayed message is status
        std::cout << std::endl;
    std::cout << "DEBUG (level " << level << "): " << str << std::endl << std::flush;
    flag_status_or_debug = false;
}
