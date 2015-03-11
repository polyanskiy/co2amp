#include "co2amp.h"


int main(int argc, char **argv)
{
    debug_level = 1;
    flag_status_or_debug = true;
    printf("co2amp-core v.2015-03-11\n\n");
    fflush(stdout);
    #pragma omp parallel // counting processors (for parallel computing)
	if (omp_get_thread_num() == 0)
        printf("number of CPU cores: %d\n\n", omp_get_num_threads());
    fflush(stdout);
    ReadCommandLine(argc, argv);
    Debug(1, "Command line read done!");
    ConstantsInit();
    Debug(1, "Constants init done!");
    AllocateMemory();
    Debug(1, "Allocate memory done!");
    ArraysInit();
    Debug(1, "Arrays init done!");
    Calculations(); // Main program !!!
    SaveOutputField();
    StatusDisplay(-1, -1, -1, "done!");
    FreeMemory();
    return 0;
}


void Calculations()
{
    int pulse, k, K, i;
    double t, t_cur;

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

    printf("\nCALCULATING:\n");
    fflush(stdout);

    // 1 Fill out spectroscoic arrays &
    // 2 Populations and field initialization
    if(n_amsections>0 && p_CO2+p_N2+p_He>0){
        AmplificationBand();
        InitializePopulations();
    }

    StatusDisplay(-1, -1, -1, "initial field...");
    InitializeE();

    // 3 Initial q's
    if(n_amsections>0 && p_CO2+p_N2+p_He>0){
        StatusDisplay(-1, -1, 0, "pumping and relaxation...");
        if(!strcmp(pumping, "discharge")){
            Boltzmann(0);
            q2_b = q2;
            q3_b = q3;
            q4_b = q4;
            qT_b = qT;
            t_b = 0;
        }
    }

    // 4 Calculation of field dynamics:
    for(t=0; t<=layout_time[n_propagations-1] + Dt_train*(n_pulses-1) + Dt_pump; t+=Dt_pump){

        // molecular dynamics calculations
        if(n_amsections>0 && p_CO2+p_N2+p_He>0){
            StatusDisplay(-1, -1, t, "pumping and relaxation...");
            PumpingAndRelaxation(t);
        }

        for(k=0; k<n_propagations; k++){
            for(pulse=0; pulse<n_pulses; pulse++){
                t_cur = layout_time[k] + Dt_train*pulse;
                if(t-Dt_pump/2<t_cur && t+Dt_pump/2>=t_cur){
                    K = layout_component[k]; // component number in the "components" list

                    // files written on component input (ex: before amplification)
                    StatusDisplay(pulse, k, t_cur, "saving...");
                    UpdateOutputFiles(pulse, k, t_cur);

                    // amplification
                    if(!strcmp(component_type[K], "AM")){ //active medium
                        int am_section = 0;
                        for(i=0; i<K; i++){
                            if(!strcmp(component_type[i], "AM"))
                                am_section++;
                        }
                        if(p_CO2>0){
                            Debug(1, "amplification: starting...");
                            Amplification(pulse, k, t_cur, am_section, atof(component_param1[K])*1e-2); //param1: amplification length, cm -> m
                            Debug(1, "amplification: done");
                            SaveGainSpectrum(pulse, k);
                        }
                    }

                    // optical components
                    if(!strcmp(component_type[K], "MASK"))
                        Mask(pulse, component_Dr[K], atof(component_param1[K])*1e-2); //param1: mask radius, cm -> m
                    if(!strcmp(component_type[K], "ABSORBER"))
                        Absorber(pulse, atof(component_param1[K])); //param1: transmission
                    if(!strcmp(component_type[K], "LENS"))
                        Lens(pulse, component_Dr[K], atof(component_param1[K])*1e-2); //param1: Focal length, cm -> m
                    if(!strcmp(component_type[K], "WINDOW")){
                        StatusDisplay(pulse, k, t_cur, "material...");
                        Window(pulse, component_param1[K], atof(component_param2[K])*1e-2); //param1: material, param2: thickness, cm -> m
                    }
                    if(!strcmp(component_type[K], "STRETCHER")){
                        StatusDisplay(pulse, k, t_cur, "stretcher...");
                        Stretcher(pulse, atof(component_param1[K])*1e-24); //param1: stretching ps/THz -> s/Hz
                    }
                    if(!strcmp(component_type[K], "BANDPASS")){
                        StatusDisplay(pulse, k, t_cur, "bandpass...");
                        Bandpass(pulse, atof(component_param1[K])*1e12, atof(component_param2[K])*1e12); //param1: band center THz -> Hz, param2: bandwidth THz -> Hz,
                    }

                    //propagation to next component
                    if(k!=n_propagations-1){ //no propagation after last surface
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
}

void StatusDisplay(int pulse, int k, double t, char *status)
{
    int K;
    if(k == -1){
        if(t < 0)
            printf("\r%s                                               ", status);
        else
            printf("\r%f us: %s                                               ", t*1e6, status);
    }
    else{
	K = layout_component[k];
        if(n_pulses==1)
            printf("\r%f us; Component \"%s\" (%d of %d): %s                    ", t*1e6, component_id[K], k+1, n_propagations, status);
        else
            printf("\r%f us; Component \"%s\" (%d of %d); Pulse %d: %s                    ", t*1e6, component_id[K], k+1, n_propagations, pulse+1, status);
    }
    fflush(stdout);
    flag_status_or_debug = true;
}


void Debug(int level, char *str)
{
    if(level > debug_level)
        return;
    if(flag_status_or_debug) //if last displayed message is status
        printf("\n");
    printf("DEBUG (level %d): %s\n", level, str);
    fflush(stdout);
    flag_status_or_debug = false;
}
