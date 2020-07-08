#include  "co2amp.h"


void UpdateOutputFiles(Pulse *pulse, Plane *plane, double clock_time)
{
    int pulse_n = pulse->number;
    int plane_n = plane->number;
    int optic_n = plane->optic->number;
    double *Fluence = new double[x0];
    double *Power = new double[n0];
    double Energy;
    double Dt = (t_max-t_min)/n0;     // pulse time step, s
    double Dv = 1.0/(t_max-t_min);    // frequency step, Hz
    double Dr = plane->optic->r_max/x0;
    FILE *file;

    std::complex<double> **E = pulse->E;

    ///////////////////////////////// Fluence, Power, Energy //////////////////////////////////

    Energy = 0;
    for(int x=0; x<x0; x++)
        Fluence[x] = 0;
    for(int n=0; n<n0; n++)
        Power[n]=0;

    //#pragma omp parallel for reduction(+:Energy)
    for(int x=0; x<x0; x++)
    {
        for(int n=0; n<n0; n++)
        {
            Energy += 2.0 * h * pulse->vc
                    * pow(abs(E[x][n]),2)
                    * M_PI*pow(Dr,2)*(2*x+1) //ring area dS = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
                    * Dt; // J

            Power[n] += 2.0 * h * pulse->vc
                    * pow(abs(E[x][n]),2)
                    * M_PI*pow(Dr,2)*(2*x+1);

            Fluence[x] += 2.0 * h * pulse->vc * pow(abs(E[x][n]),2) * Dt; // J/m2
        }
    }

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
    for(int x=0; x<x0; x++)
        fprintf(file, "%e\t%e\n", Dr*(0.5+x), Fluence[x]);
    fclose(file);

    // Write power file
    file = fopen((basename+"_power.dat").c_str(), "w");
    fprintf(file, "#Data format:  time[s] power[W]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", (t_min + Dt*(0.5+n)), Power[n]);
    fclose(file);

    // Write energy file
    if(pulse_n==0 && plane_n==0)
    {
        file = fopen("energy.dat", "w");
        fprintf(file, "#Data format: time[s] energy[J] pulse# optic# pass#\n");
    }
    else
        file = fopen("energy.dat", "a");
    fprintf(file, "%e\t%e\t%d\t%d\t%d\n", clock_time, Energy, pulse_n, optic_n, pass_n);
    fclose(file);

    ////////////////////////////////////// Spectra //////////////////////////////////////////////
    double *average_spectrum;
    average_spectrum = new double[n0];
    std::complex<double> *spectrum;
    spectrum = new std::complex<double>[n0];

    for(int n=0; n<n0; n++)
        average_spectrum[n] = 0;

    // FAST: single point spectrum (comment SLOW or FAST)
    //FFT(E[0], spectrum);
    //for(int n=0; n<n0; n++)
    //    average_spectrum[n] = pow(cabs(spectrum[n]), 2);

    // SLOW: averaged across the beam (comment SLOW or FAST)
    //#pragma omp parallel for shared(average_spectrum)// multithreaded
    for(int x=0; x<x0; x++)
    {
        FFT(E[x], spectrum);
        for(int n=0; n<n0; n++)
            average_spectrum[n] += pow(abs(spectrum[n]), 2) * (2*x+1); //(2*x+1) is proportional to ring area:
    }                                                                  //dS = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)

    // spectrum normalization
    double max_int=0;
    for(int n=0; n<n0; n++)
        if(average_spectrum[n] >= max_int)
            max_int = average_spectrum[n];
    for(int n=0; n<n0 && max_int>0; n++)
            average_spectrum[n] /= max_int;

    // Write spectrum file
    file = fopen((basename+"_spectrum.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] intensity[au]\n");
    for(int n=0; n<n0; n++)
    {
        int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
        fprintf(file, "%e\t%e\n", v0+Dv*(n-n0/2), average_spectrum[n1]);
    }
    fclose(file);


    ////////////////////////////////////// Phase //////////////////////////////////////////////
    double *phase;
    phase = new double[n0];

    // Phase in the center of the beam!
    for(int n=0; n<n0; n++){
        if(abs(E[0][n])<1000) // remove noise
            phase[n] = 0;
        else
            phase[n] = arg(E[0][n]);
    }

    // Write phase file
    file = fopen((basename+"_phase.dat").c_str(), "w");
    fprintf(file, "#Data format: Time[s] Phase[rad]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", t_min+Dt*(0.5+n), phase[n]);
    fclose(file);


    ////////////////////////////////////// Free memory //////////////////////////////////////////////

    delete[] Power;
    delete[] Fluence;
    delete[] average_spectrum;
    delete[] spectrum;
    delete[] phase;
}
