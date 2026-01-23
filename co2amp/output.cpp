#include  "co2amp.h"


void UpdateOutputFiles(Pulse *pulse, Plane *plane, int n_min, int n_max)
{
    // In case of non-amplifier optics interaction is done at once when pulse time frame reaches the optic
    // therefore, we only run this function ones at n_min=0
    if(plane->optic->type != "A")
    {
        if(n_min != 0)
            return;
        n_max = n0-1;
    }

    double time = pulse->time_in + plane->time_from_first_plane;
    int pulse_n = pulse->number;
    int plane_n = plane->number;
    int optic_n = plane->optic->number;
    double energy;
    double Dr = plane->optic->Dr;
    FILE *file;

    auto& E = pulse->E;

    ///////////////////////////////// Fluence, Power, Energy //////////////////////////////////

    for(int x=0; x<x0; ++x)
    {
        for(int n=n_min; n<=n_max; ++n)
        {
            double intensity = 2 * h * pulse->vc * std::norm(E[n0*x+n]);
            plane->input_power[n] += intensity * M_PI*pow(Dr,2)*(2*x+1); //ring area dS = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
            plane->input_fluence[x] += intensity * Dt; // J/m^2
        }
    }

    // Write files only when pulse interaction is completed
    if(n_max != n0-1)
        return;

    energy = 0;
    for(int n=0; n<n0; ++n)
        energy += plane->input_power[n] * Dt; // J

    // Count pass number through current element
    int pass_n = 0;
    for(int i=0; i<plane_n; i++)
        if(planes[i]->optic == planes[plane_n]->optic)
            pass_n++;

    std::string basename = planes[plane_n]->optic->id
            + "_" + pulses[pulse_n]->id
            + "_pass" + std::to_string(pass_n);

    // Write fluence file
    file = fopen((basename+"_fluence.dat").c_str(), "w");
    fprintf(file, "#Data format: r[m] fluence[J/m^2]\n");
    for(int x=0; x<x0; ++x)
        fprintf(file, "%e\t%e\n", Dr*(0.5+x), plane->input_fluence[x]);
    fclose(file);

    // Write power file
    file = fopen((basename+"_power.dat").c_str(), "w");
    fprintf(file, "#Data format:  time[s] power[W]\n");
    for(int n=0; n<n0; ++n)
        fprintf(file, "%.8E\t%e\n", (t_min + Dt*(0.5+n)), plane->input_power[n]);
    fclose(file);

    // Write energy file
    if(pulse_n==0 && plane_n==0)
    {
        file = fopen("energy.dat", "w");
        fprintf(file, "#Data format: time[s] energy[J] pulse# optic# pass#\n");
    }
    else
        file = fopen("energy.dat", "a");
    fprintf(file, "%e\t%e\t%d\t%d\t%d\n", time, energy, pulse_n, optic_n, pass_n);
    fclose(file);

    ////////////////////////////////////// Spectra //////////////////////////////////////////////
    std::vector<double> average_spectrum(n0);
    std::vector<std::complex<double>> spectrum(n0);

    for(int n=0; n<n0; ++n)
        average_spectrum[n] = 0;

    // FAST: single point spectrum (comment SLOW or FAST)
    //FFT(E[0], spectrum);
    //for(int n=0; n<n0; ++n)
    //    average_spectrum[n] = pow(cabs(spectrum[n]), 2);

    // SLOW: averaged across the beam (comment SLOW or FAST)
    //#pragma omp parallel for shared(average_spectrum)// multithreaded
    for(int x=0; x<x0; ++x)
    {
        FFT(&E[n0*x], spectrum.data());
        for(int n=0; n<n0; ++n)
        {
            average_spectrum[n] += std::norm(spectrum[n]) * (2*x+1);    // norm() returns the squared magnitude
                                                                        //(2*x+1) is proportional to ring area:
                                                                        //dS = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
        }
    }

    // spectrum normalization
    /*double max_int=0;
    for(int n=0; n<n0; ++n)
        if(average_spectrum[n] >= max_int)
            max_int = average_spectrum[n];
    for(int n=0; n<n0 && max_int>0; ++n)
            average_spectrum[n] /= max_int;*/

    // convert spectrum to absolute units (J/Hz)
    double integrated_spectrum = 0;
    for(int n=0; n<n0; ++n)
    {
        integrated_spectrum += average_spectrum[n];
    }
    for(int n=0; n<n0; ++n)
    {
        average_spectrum[n] *= energy/integrated_spectrum/Dv;
    }

    // Write spectrum file
    file = fopen((basename+"_spectrum.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] spectral_energy_density[J/Hz]\n");
    for(int n=0; n<n0; ++n)
    {
        int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
        fprintf(file, "%.8E\t%e\n", v_min+Dv*(0.5+n), average_spectrum[n1]);
    }
    fclose(file);


    ////////////////////////////////////// Phase //////////////////////////////////////////////

    // Phase in the center of the beam!
    std::vector<double> phase(n0);

    UnwrapPhase(pulse, 0, phase.data()); // 0 is x value in the center

    // Write phase file
    file = fopen((basename+"_phase.dat").c_str(), "w");
    fprintf(file, "#Data format: Time[s] Phase[rad]\n");
    for(int n=0; n<n0; ++n)
        fprintf(file, "%e\t%e\n", t_min+Dt*(0.5+n), phase[n]);
    fclose(file);

}
