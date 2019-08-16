#include "co2amp.h"


// GLOBAL VARIABLES

// --- PULSES, OPTICS, GEOMETRY ----
std::vector<Pulse*> pulses;
std::vector<Optic*> optics;
std::vector<Plane*> layout;
// ------- CALCULATION GRID --------
double vc;                 // central frequency
double t_min, t_max;       // pulse (fast) time limits
double time_tick;          // main (slow) time step
int x0, n0;                // number of points in radial and time grids
// ----------- DEBUGGING -----------
int debug_level;           // debug output control 0: nothing; 1: some; 2: a lot; 3: everything
bool noprop;
bool flag_status_or_debug; // last message displayed: True if status False if debug
// --- MISC CONSTANTS AND FLAGS ----
double c, h;               // spped of light [m/s]; Plank's [J s]
bool configuration_error = false;


int main(int argc, char **argv)
{
    std::clock_t start_time = std::clock();

    // Constants
    c = 2.99792458e8; // m/s
    h = 6.626069e-34; // J*s

    debug_level = 1;
    flag_status_or_debug = true;

    std::cout << "co2amp-core v.2019-08-16" << std::endl << std::flush;

    #pragma omp parallel // counting processors (for parallel computing)
    if(omp_get_thread_num() == 0)
        std::cout << "number of CPU cores: " << omp_get_num_threads() << std::endl << std::endl << std::flush;

    //co2amp = new CO2AMP();

    if (!ReadCommandLine(argc, argv)){
        std::cout << "Error in command line. Aborting.\n";
        return -1;
    }
    Debug(1, "Command line read done");
    if (!ReadConfigFiles("config_files.yml")){
        std::cout << "Error in configuration file(s). Aborting.\n";
        return -1;
    }
    Debug(1, "Processing configuration files done");

    Calculations(); // Main program !!!
    /*AllocateMemory();
    Debug(1, "Allocate memory done!");
    ArraysInit();
    Debug(1, "Arrays init done!");
    Calculations(); // Main program !!!
    SaveOutputField();
    StatusDisplay(-1, -1, -1, "done!");
    FreeMemory();*/

    Debug(2,"Success!");
    StatusDisplay(nullptr, nullptr, -1, "All done!");

    std::cout << std:: endl << "Execution time: "  << (std::clock()-start_time)/CLOCKS_PER_SEC << " s";

    return 0;
}


void Calculations()
{
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

    std::cout << std::endl << "CALCULATION" << std::endl;

    for(double time=0; time<=(layout[layout.size()-1]->time_from_first_plane + pulses[pulses.size()-1]->time_inj + time_tick); time+=time_tick){

        for(int optic_n=0; optic_n<optics.size(); optic_n++)
            optics[optic_n]->InternalDynamics(time);

        for(int plane_n=0; plane_n<layout.size(); plane_n++){
            for(int pulse_n=0; pulse_n<pulses.size(); pulse_n++){
                double time_of_arival = layout[plane_n]->time_from_first_plane + pulses[pulse_n]->time_inj;
                if(time-time_tick/2 < time_of_arival && time+time_tick/2 >= time_of_arival){
                    // 1: Propagate beam to(!) this plane
                    if(plane_n != 0) // propagate to(!) this palne
                        pulses[pulse_n]->Propagate(layout[plane_n-1], layout[plane_n], time);
                    // 2: Save pulse parameters at plane location (before interaction!!!)
                    StatusDisplay(pulses[pulse_n], layout[plane_n], time, "saving...");
                    UpdateOutputFiles(pulses[pulse_n], layout[plane_n], time);
                    // 3: Do Interaction (amplification etc.)
                    if(plane_n != layout.size()-1){ // interact with this palne
                        layout[plane_n]->optic->PulseInteraction(pulses[pulse_n], layout[plane_n], time);
                    }
                }
            }
        }
    }
}


void StatusDisplay(Pulse *pulse, Plane *plane, double time, std::string status)
{
    if(pulse == nullptr){
        if(time < 0)
            std::cout << "\r" << status
                      << "                                               ";
        else
            std::cout << "\r" << toExpString(time) << " s: " << status
                      << "                                               ";
    }
    else{
        if(pulses.size()==1)
            std::cout << "\r" << toExpString(time) << " s; "
                      << "Plane " << plane->optic->id
                      << " (" << plane->number+1 << " of " << layout.size() << "): "
                      << status << "                                     ";
        else
            std::cout << "\r" << toExpString(time) << " s; "
                      << "Pulse " << pulse->id << "; "
                      << "Plane " << plane->optic->id
                      << " (" << plane->number+1 << " of " << layout.size() << "): "
                      << status << "                                     ";
    }
    std::cout << std::flush;
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
