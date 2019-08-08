#include "co2amp.h"


// GLOBAL VARIABLES

// -- PULSES, OPTICS, GEOMETRY ---
std::vector<Pulse> pulses;
std::vector<Optic> optics;
std::vector<LayoutComponent> layout;
//int n_AM, n_propagations;
bool noprop;
//double *alpha;  // temporary - for nonlinear absorption in Ge
// ------- CALCULATION NET -------
double vc;//, v_max, v_min;
double t_pulse_min, t_pulse_max;
double Dt_pump; // "main time" - for pumping/relaxation
int x0, n0; // number of points in radial and time nets and number of pulses in the train
// ------- DEBUGGING -------
int debug_level; // debug output control 0 - nothing; 1 - some; 2 - everything
//int bands;       // SUMM of 1 for regular + 2 for hot + 4 for sequence
bool flag_status_or_debug; // last message displayed: True if status False if debug
// ------- MISC. CONSTANTS -------
double c, h; // spped of light [m/s]; Plank's [J s]
// ------- OUTPUT ARRAY -------
//std::complex<double> ***E;



int main(int argc, char **argv)
{
    // Constants
    c = 2.99792458e8; // m/s
    h = 6.626069e-34; // J*s

    debug_level = 1;
    flag_status_or_debug = true;  

    std::cout << "co2amp-core v.2019-08-07" << std::endl << std::flush;

    #pragma omp parallel // counting processors (for parallel computing)
    if (omp_get_thread_num() == 0)
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
    return 0;
}


void Calculations()
{
    int pulse, k, K, i;
    double t, t_cur=0;

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

    // 1 Fill out spectroscoic arrays &
    // 2 Populations and field initialization
    /*if(n_AM>0 && p_CO2+p_N2+p_He>0){
        AmplificationBand();
        InitializePopulations();
    }*/

    //StatusDisplay(-1, -1, -1, "initial field...");
    //InitializeE();

    // 3 Initial q's
    /*if(n_AM>0 && p_CO2+p_N2+p_He>0){
        StatusDisplay(-1, -1, 0, "pumping and relaxation...");
        if(pumping == "discharge"){
            Boltzmann(0);
            q2_b = q2;
            q3_b = q3;
            q4_b = q4;
            qT_b = qT;
            t_b = 0;
        }
    }*/

    // 4 Calculation of field dynamics:
//    for(t=0; t<=layout_time[n_propagations-1] + Dt_train*(n_pulses-1) + Dt_pump; t+=Dt_pump){

        // molecular dynamics calculations
        /*if(n_AM>0 && p_CO2+p_N2+p_He>0){
            StatusDisplay(-1, -1, t, "pumping and relaxation...");
            PumpingAndRelaxation(t);
        }*/

        for(k=0; k<optics.size(); k++){
            for(pulse=0; pulse<pulses.size(); pulse++){
//                t_cur = layout_time[k] + Dt_train*pulse;
//                if(t-Dt_pump/2<t_cur && t+Dt_pump/2>=t_cur){
//                    K = layout_component[k]; // component number in the "components" list

                    // files written on component input (ex: before amplification)
                    StatusDisplay(pulse, k, t_cur, "saving...");
                    UpdateOutputFiles(pulse, k, t_cur);
            }
        }

                    // amplification
                    /*if(optics[K].type == "AM"){ //active medium
                        int am_section = 0;
                        for(i=0; i<K; i++){
                            if(optics[i].type == "AM")
                                am_section++;
                        }
                        if(p_CO2>0){
                            Debug(1, "amplification: starting...");
                            //Amplification(pulse, k, t_cur, am_section, atof(component_param1[K])*1e-2); //param1: amplification length, cm -> m
                            Debug(1, "amplification: done");
                            SaveGainSpectrum(pulse, k);
                        }
                    }*/

                    // optical components
                    /*if(!strcmp(component_type[K], "MASK"))
                        Mask(pulse, component_Dr[K], atof(component_param1[K])*1e-2); //param1: mask radius, cm -> m
                    if(!strcmp(component_type[K], "ATTENUATOR") || !strcmp(component_type[K], "ABSORBER"))
                        Attenuator(pulse, atof(component_param1[K])); //param1: transmission
                    if(!strcmp(component_type[K], "LENS"))
                        Lens(pulse, component_Dr[K], atof(component_param1[K])*1e-2); //param1: Focal length, cm -> m
                    if(!strcmp(component_type[K], "WINDOW")){
                        StatusDisplay(pulse, k, t_cur, "material...");
                        Window(pulse, k, t, component_param1[K], atof(component_param2[K])*1e-2); //param1: material; param2: thickness, cm -> m
                    }
                    if(!strcmp(component_type[K], "STRETCHER")){
                        StatusDisplay(pulse, k, t_cur, "stretcher...");
                        Stretcher(pulse, atof(component_param1[K])*1e-24); //param1: stretching ps/THz -> s/Hz
                    }
                    if(!strcmp(component_type[K], "BANDPASS")){
                        StatusDisplay(pulse, k, t_cur, "bandpass...");
                        Bandpass(pulse, atof(component_param1[K])*1e12, atof(component_param2[K])*1e12); //param1: band center, THz -> Hz; param2: bandwidth, THz -> Hz,
                    }
                    if(!strcmp(component_type[K], "FILTER")){
                        StatusDisplay(pulse, k, t_cur, "spectral filter...");
                        Filter(pulse, component_param1[K]); //param1: path to configuration file (yaml)
                    }
                    if(!strcmp(component_type[K], "APODIZER")){
                        StatusDisplay(pulse, k, t_cur, "apodizer...");
                        Apodizer(pulse, atof(component_param1[K])); //param1: apodization parameter alpha. Must be in range 0 (no apodization) ... 1 (full-aperture apodization)
                    }
                    if(!strcmp(component_type[K], "AIR")){
                        StatusDisplay(pulse, k, t_cur, "air...");
                        Air(pulse, k, t, atof(component_param1[K]), atof(component_param2[K])*1e-2); //param1: humidity, %; param2: length, cm -> m
                    }*/

                    //propagation to next component
/*                    if(k!=n_propagations-1){ //no propagation after last surface
                        if(!noprop){ //running without -noprop option
                            Debug(1, "propagation: starting...");
                            BeamPropagation(pulse, k, t_cur);
                            Debug(1, "propagation: done");
                        }
                    }
                }
            }
        }
    }
*/
}

void StatusDisplay(int pulse, int k, double t, std::string status)
{
    if(k == -1){
        if(t < 0)
            std::cout << "\r" << status
                      << "                                               ";
        else
            std::cout << "\r" << t*1e6 << " us: " << status
                      << "                                               ";
    }
    else{
        if(pulses.size()==1)
            std::cout << "\r" << t*1e6 << " us; Optic " << layout[k].optic->id << " (" <<  k+1
                      << " of " << layout.size() << "): " << status
                      << "                    ";
        else
            std::cout << "\r" << t*1e6 << " us; Optic " << layout[k].optic->id << " (" <<  k+1
                      << " of " << layout.size() << "); Pulse " << pulse+1 << ": " << status
                      << "                    ";
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
