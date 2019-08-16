#include  "co2amp.h"


void UpdateOutputFiles(Pulse *pulse, Plane *plane, double clock_time)
{
    int pulse_n = pulse->number;
    int plane_n = plane->number;
    int optic_n = plane->optic->number;
    double *Fluence = new double[x0];
    double *Power = new double[n0];
    double Energy;
    double Dt = (t_max-t_min)/(n0-1); // pulse time step, s
    double Dv = 1.0/(t_max-t_min);    // frequency step, Hz
    double v_min = vc - Dv*(n0-1)/2;
    //double v_max = vc + Dv*(n0-1)/2;
    double Dr = plane->optic->Dr;
    FILE *file;

    std::complex<double> **E = pulse->E;

    ///////////////////////////////// Fluence, Power, Energy //////////////////////////////////

    Energy = 0;
    for(int x=0; x<x0; x++)
        Fluence[x] = 0;
    for(int n=0; n<n0; n++)
        Power[n]=0;

    //#pragma omp parallel for reduction(+:Energy)
    for(int x=0; x<x0; x++){
        for(int n=0; n<n0; n++){
            if(x+1<x0 && n+1<n0)
            Energy += 2.0 * h * pulse->nu0 *
                        (pow(abs(E[x][n]),2) + pow(abs(E[x][n+1]),2) +
                        pow(abs(E[x+1][n]),2) + pow(abs(E[x+1][n+1]),2))/4 *
                        2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J
            if(x+1<x0)
                Power[n] += 2.0 * h * pulse->nu0 *
                        (pow(abs(E[x][n]),2) + pow(abs(E[x+1][n]),2))/2 *
                        2*M_PI*(Dr*x+Dr/2)*Dr; // W/m2
            if(n+1<n0)
                Fluence[x] += 2.0 * h * pulse->nu0 *
                        (pow(abs(E[x][n]),2) + pow(abs(E[x][n+1]),2))/2 *
                        Dt; // J/m2
        }
    }

    // Count pass number through current element
    int pass_n = 0;
    for(int i=0; i<plane_n; i++)
        if(layout[i]->optic == layout[plane_n]->optic)
            pass_n++;

    // Write fluence file
    if(pulse_n==0 && plane_n==0){
        file = fopen("data_fluence.dat", "w");
        fprintf(file, "#Data format: r[m] fluence[J/m^2]\n");
    }
    else{
        file = fopen("data_fluence.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d pass_n %d\n", pulse_n, optic_n, pass_n);
    for(int x=0; x<x0; x++)
        fprintf(file, "%e\t%e\n", Dr*x, Fluence[x]);
    fclose(file);

    // Write power file
    if(pulse_n==0 && plane_n==0){
        file = fopen("data_power.dat", "w");
        fprintf(file, "#Data format:  time[s] power[W]\n");
    }
    else{
        file = fopen("data_power.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d pass_n %d\n", pulse_n, optic_n, pass_n);
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", (t_min + Dt*n), Power[n]);
    fclose(file);

    // Write energy file
    if(pulse_n==0 && plane_n==0){
        file = fopen("data_energy.dat", "w");
        fprintf(file, "#Data format: time[s] energy[J] pulse_n optic_n pass_number\n");
    }
    else
        file = fopen("data_energy.dat", "a");
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
    //FFT(E[pulse][50], spectrum);
    //for(i=0; i<n0; i++)
    //    average_spectrum[i] = pow(cabs(spectrum[i]), 2);

    // SLOW: averaged across the beam (comment SLOW or FAST)
    //#pragma omp parallel for shared(average_spectrum)// multithreaded
    for(int x=0; x<x0; x++){
        FFT(E[x], spectrum);
        for(int n=0; n<n0; n++)
            average_spectrum[n] += (0.5+x) * pow(abs(spectrum[n]), 2);
    }

    // spectrum normalization
    double max_int=0;
    for(int n=0; n<n0; n++)
        if(average_spectrum[n] >= max_int)
            max_int = average_spectrum[n];
    for(int n=0; n<n0 && max_int>0; n++)
            average_spectrum[n] /= max_int;

    // Write spectra file
    if(pulse_n==0 && plane_n==0){
        file = fopen("data_spectra.dat", "w");
        fprintf(file, "#Data format: frequency[Hz] intensity[au]\n");
    }
    else{
        file = fopen("data_spectra.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d pass_n %d\n", pulse_n, optic_n, pass_n);
    for(int n=0; n<=n0-1; n++)
        fprintf(file, "%e\t%e\n", v_min+Dv*n, average_spectrum[n]);
    fclose(file);

    delete[] Power;
    delete[] Fluence;
    delete[] average_spectrum;
    delete[] spectrum;
}





void SaveOutputField()
{
/*
    int pulse, x;
    FILE *file;

    StatusDisplay(-1, -1, 0, "saving field data");

    file = fopen("field.bin", "wb");
    for(pulse=0; pulse<n_pulses; pulse++){
        for(x=0; x<x0; x++)
            fwrite(E[pulse][x], sizeof(std::complex<double>)*n0, 1, file);
    }
    fclose(file);
*/

    // Gaussian beam diffraction in empty space: analytical solution for test purposes
    /*double ld = 0.5*2*M_PI*vc*pow(r0,2);
    double z=Lr;
    double intens;
    file = fopen("diffraction_test.dat", "w");
    for(x=0; x<x0; x++){
        intens = Fluence[0][0]*1000 *ld*ld/(ld*ld+z*z) * exp(-2*pow(r[x]/r0,2)*ld*ld/(ld*ld+z*z));
        fprintf(file, "%.7f\t%.7f\n", r[x], intens);
    }
    fclose(file);*/
}
