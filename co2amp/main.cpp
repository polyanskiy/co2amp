#include "co2amp.h"
#include <iomanip>


// GLOBAL VARIABLES

// --- PULSES, OPTICS, GEOMETRY ----
std::vector<Pulse*> pulses;
std::vector<Optic*> optics;
std::vector<Plane*> planes;
// ------- CALCULATION GRID --------
double v0;                 // central frequency of the calculation grid
double t_min, t_max;       // pulse (fast) time limits
double time_tick;          // main (slow) time step
int x0, n0;                // number of points in radial and time grids
// ---- CALCULATION PARAMETERS -----
int method;                // propagation method
                           // 0: no propagation 1: Fresnel 2: Rayleigh-Sommerfeld
// ----------- DEBUGGING ----------
int debug_level;           // debug output control
                           // -1: less than nothing 0: nothing; 1: some; 2: a lot; 3: everything
bool flag_status_or_debug; // last message displayed: True if status False if debug
// --- MISC CONSTANTS AND FLAGS ----
double c, h;               // spped of light [m/s]; Plank's [J s]
bool configuration_error = false;
std::string search_dir;    // Additional directory for HDF5 pulse files


int main(int argc, char **argv)
{
    std::string version = "2021-05-18";

    std::clock_t stopwatch = std::clock();

    c = 2.99792458e8; // m/s
    h = 6.626069e-34; // J*s

    debug_level = 1;
    flag_status_or_debug = true;

    std::string command = ReadCommandLine(argc, argv);

    if (command == "" )
    {
        std::cout << "Input ERROR: Missing command line argument(s)\n";
        std::cout << "Error in command line. Aborting.\n";
        return EXIT_FAILURE;
    }

    if(command == "version")
    {
        std::cout << version;
        return EXIT_SUCCESS;
    }

    std::cout << "co2amp v." << version << "\n";

    #pragma omp parallel // counting processors (for parallel computing)
    if(omp_get_thread_num() == 0)
        std::cout << "Number of CPU cores: " << omp_get_num_threads() << "\n\n" << std::flush;

    Debug(1, "Command line read done");

    if (!ReadConfigFiles("config_files.yml"))
    {
        std::cout << "Error in configuration file(s). Aborting.\n";
        return EXIT_FAILURE;
    }
    Debug(1, "Processing configuration files done");

    Calculations(); // Main program !!!

    // Save pulses at the output
    for(int i=0; i<pulses.size(); i++)
    {
        pulses[i]->SavePulse();
        pulses[i]->SaveBeam();
    }

    Debug(2,"Success!");
    StatusDisplay(nullptr, nullptr, -1, "All done!");

    // Show run time
    stopwatch = std::clock()-stopwatch;
    float run_s = (float)stopwatch/CLOCKS_PER_SEC;
    int run_m = run_s / 60;
    int run_h = run_s / (60*60);
    int run_d = run_s / (24*60*60);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std:: endl << "Execution time: ";
    std::cout << run_s << " s";
    if(run_m>0)
        std::cout << " (";
    if(run_d>0)
        std::cout << run_d << " d, ";
    if(run_h>0)
        std::cout << run_h - run_d*24 << " h, ";
    if(run_m>0)
    {
        std::cout << run_m - run_h*60 << " m, ";
        std::cout << run_s - run_m*60 << " s)";
    }

    return EXIT_SUCCESS;
}


void Calculations()
{
    // Total pumping energy
    /*if(n_amsections>0 && p_CO2+p_N2+p_He>0)
    {
        double Epump=0;
        double t_lim = t_inj+tbp*(K0-1);
        if(mode==1) // train
            t_lim = t_inj+Dt_train*(K0-1)+tbp;
        for(t=0; t<=t_lim; t+=Dt)
            Epump += Current(t)*Voltage(t) * Dt/1e6;
        printf("\nDischarge energy (withing %f us) = %f J\n", t_lim, Epump);
    }*/

    std::cout << "CALCULATION\n";

    for(double time=0; time<=(planes[planes.size()-1]->time_from_first_plane + pulses[pulses.size()-1]->time_in + time_tick); time+=time_tick)
    {
        for(int optic_n=0; optic_n<optics.size(); optic_n++)
            optics[optic_n]->InternalDynamics(time);

        for(int plane_n=0; plane_n<planes.size(); plane_n++)
        {
            for(int pulse_n=0; pulse_n<pulses.size(); pulse_n++)
            {
                double time_of_arival = planes[plane_n]->time_from_first_plane + pulses[pulse_n]->time_in;
                if(time-time_tick/2 < time_of_arival && time+time_tick/2 >= time_of_arival)
                {
                    // 1: Propagate beam to(!) this plane
                    if(plane_n != 0) // propagate to(!) this palne
                        pulses[pulse_n]->Propagate(planes[plane_n-1], planes[plane_n], time);
                    // 2: Save pulse parameters at plane location (before interaction!!!)
                    StatusDisplay(pulses[pulse_n], planes[plane_n], time, "saving...");
                    UpdateOutputFiles(pulses[pulse_n], planes[plane_n], time);
                    // 3: Do Interaction (amplification etc.)
                    if(plane_n != planes.size()-1) // interact with this palne
                        planes[plane_n]->optic->PulseInteraction(pulses[pulse_n], planes[plane_n], time);
                }
            }
        }
    }
}


void StatusDisplay(Pulse *pulse, Plane *plane, double time, std::string status)
{
    if(pulse == nullptr)
    {
        if(time < 0)
            std::cout << "\r" << status
                      << "                                               ";
        else
            std::cout << "\r" << toExpString(time) << " s: " << status
                      << "                                               ";
    }
    else
    {
        if(pulses.size()==1)
            std::cout << "\r" << toExpString(time) << " s; "
                      << plane->optic->id
                      << " (plane " << plane->number+1 << " of " << planes.size() << "): "
                      << status << "                                     ";
        else
            std::cout << "\r" << toExpString(time) << " s; "
                      << pulse->id << "; "
                      << plane->optic->id
                      << " (plane " << plane->number+1 << " of " << planes.size() << "): "
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
