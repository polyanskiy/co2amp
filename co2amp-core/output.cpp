#include  "co2amp.h"


void UpdateDynamicsFiles(double t)
{
/*
    int i;
    FILE *file;

    ////////////////////////// Discharge //////////////////////////
    if(pumping == "discharge"){
        if(t==0.0){
            file = fopen("data_discharge.dat", "w");
            fprintf(file, "# time[us] current[A] voltage[V]\n");
        }
        else
            file = fopen("data_discharge.dat", "a");
        fprintf(file, "%7f\t%.7f\t%.7f\n", t*1e6, Current(t), Voltage(t));
        fclose(file);
    }

    ////////////////////////////// q //////////////////////////////
    if(pumping == "discharge"){
        if(t==0.0){
            file = fopen("data_q.dat", "w");
            fprintf(file, "# time[us] q2 q3 q4 qT\n");
        }
        else
            file = fopen("data_q.dat", "a");
        fprintf(file, "%7f\t%.7f\t%.7f\t%.7f\t%.7f\n", t*1e6, q2, q3, q4, qT);
        fclose(file);
    }

    /////////////// e (average number of quanta in vibration modes) ////////////////
    if(t==0.0){
        file = fopen("data_e.dat", "w");
        fprintf(file, "# time[us] e1(am1) e2(am1) e3(am1) e4(am1) e1(am2) e2(am2) e3(am2) ...\n");
    }
    else
        file = fopen("data_e.dat", "a");
    fprintf(file, "%7f", t*1e6);
    for(i=0; i<n_AM; i++){
        double Temp2 = 960/log(2/e2[i][0]+1);
        double e1 = 1/(exp(1920/Temp2)-1);
        fprintf(file, "\t%.7f\t%.7f\t%.7f\t%.7f", e1, e2[i][0], e3[i][0], e4[i][0]);
    }
    fprintf(file, "\n");
    fclose(file);

    ///////////////////////// Temperatures /////////////////////////
    if(t==0.0){
        file = fopen("data_temperatures.dat", "w");
        fprintf(file, "# time[us] T2(am1) T3(am1) T4(am1) T(am1) T2(am2) T3(am2) ...\n");
    }
    else
        file = fopen("data_temperatures.dat", "a");
    fprintf(file, "%7f", t*1e6);
    for(i=0; i<n_AM; i++)
        //fprintf(file, "\t%.7f\t%.7f\t%.7f\t%.7f", 960/log(2/e2[i][0]+1), 3380/log(1/e3[i][0]+1), 3350/log(1/e4[i][0]+1), T[i][0]);
        fprintf(file, "\t%.7f\t%.7f\t%.7f\t%.7f", VibrationalTemperatures(i,0,2), VibrationalTemperatures(i,0,3), 3350/log(1/e4[i][0]+1), T[i][0]);
    fprintf(file, "\n");
    fclose(file);
*/
}


void UpdateOutputFiles(unsigned int pulse_n, unsigned int layout_position, double t)
{
    Pulse *pulse = &pulses[pulse_n];
    Optic *optic = layout[layout_position].optic;

    int x, n, i;
    FILE *file;

    double *Fluence = new double[x0];
    double *Power = new double[n0];
    double Energy = 0;
    double Dt = (t_pulse_max-t_pulse_min)/(n0-1); // pulse time step, s
    double Dv = 1.0/(Dt*n0);        // frequency step, Hz
    double v_min = vc - Dv*(n0-1)/2;
    //double v_max = vc + Dv*(n0-1)/2;
    double Dr = optic->Dr;

    unsigned int optic_n = 0;
    for(i=0; i<optics.size(); i++)
        if(&optics[i] == optic)
            optic_n = i;

    std::complex<double> **E = pulse->E;

    ///////////////////////////////// Fluence, Power, Energy //////////////////////////////////

    for(x=0; x<x0; x++)
        Fluence[x] = 0;
    for(n=0; n<n0; n++)
        Power[n]=0;

    //#pragma omp parallel for private(x, n) // multithreaded
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++){
            if(x+1<x0 && n+1<n0)
                Energy     += 2.0 * h * pulse->nu0 * pow(abs(E[x][n]+E[x][n+1]+E[x+1][n]+E[x+1][n+1])/4, 2) * 2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J
            if(x+1<x0)
                Power[n]   += 2.0 * h * pulse->nu0 * pow(abs(E[x][n]+E[x+1][n])/2 ,2) * 2*M_PI*(Dr*x+Dr/2)*Dr; // W/m2
            if(n+1<n0)
                Fluence[x] += 2.0 * h * pulse->nu0 * pow(abs(E[x][n]+E[x][n+1])/2, 2) * Dt; // J/m2
        }
    }

    // Count pass number through current element
    int pass_n = 0;
    for(i=0; i<layout_position; i++){
        if(layout[i].optic == layout[layout_position].optic)
            pass_n++;
    }

    // Write fluence file
    if(pulse_n==0 && layout_position==0){
        file = fopen("data_fluence.dat", "w");
        fprintf(file, "#Data format: r[m] fluence[J/m^2]\n");
    }
    else{
        file = fopen("data_fluence.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d, pass_n %d\n", pulse_n, optic_n, pass_n);
    for(x=0; x<x0; x++)
        fprintf(file, "%e\t%e\n", Dr*x, Fluence[x]);
    fclose(file);

    // Write power file
    if(pulse_n==0 && layout_position==0){
        file = fopen("data_power.dat", "w");
        fprintf(file, "#Data format:  time[s] power[W]\n");
    }
    else{
        file = fopen("data_power.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d, pass_n %d\n", pulse_n, optic_n, pass_n);
    for(n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", (t_pulse_min + Dt*n), Power[n]);
    fclose(file);

    // Write energy file
    if(pulse_n==0 && layout_position==0){
        file = fopen("data_energy.dat", "w");
        fprintf(file, "#Data format: time[s] energy[J] pulse_n optic_n pass_number\n");
    }
    else
        file = fopen("data_energy.dat", "a");
    fprintf(file, "%e\t%e\t%d\t%d\t%d\n", t, Energy, pulse_n, optic_n, pass_n);
    fclose(file);

    ////////////////////////////////////// Spectra //////////////////////////////////////////////
    double *average_spectrum;
    average_spectrum = new double[n0];
    std::complex<double> *spectrum;
    spectrum = new std::complex<double>[n0];

    for(i=0; i<n0; i++)
        average_spectrum[i] = 0;

    // FAST: single point spectrum (comment SLOW or FAST)
    //FFT(E[pulse][50], spectrum);
    //for(i=0; i<n0; i++)
    //    average_spectrum[i] = pow(cabs(spectrum[i]), 2);

    // SLOW: averaged across the beam (comment SLOW or FAST)
    //#pragma omp parallel for shared(average_spectrum) private(spectrum, x, i) // multithreaded
    for(x=0; x<x0; x++){
        FFT(E[x], spectrum);
        for(i=0; i<n0; i++)
            average_spectrum[i] += (0.5+x) * pow(abs(spectrum[i]), 2);
    }

    // spectrum normalization
    double max_int=0;
    for(n=0; n<n0; n++){
        if(average_spectrum[n] >= max_int)
            max_int = average_spectrum[n];
    }
    for(n=0; n<n0 && max_int>0; n++)
            average_spectrum[n] /= max_int;

    // Write spectra file
    if(pulse_n==0 && layout_position==0){
        file = fopen("data_spectra.dat", "w");
        fprintf(file, "#Data format: frequency[Hz] intensity[au]\n");
    }
    else{
        file = fopen("data_spectra.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse_n %d optic_n %d, pass_n %d\n", pulse_n, optic_n, pass_n);
    for(n=0; n<=n0-1; n++)
        fprintf(file, "%e\t%e\n", v_min+Dv*n, average_spectrum[n]);
    fclose(file);

    delete[] Power;
    delete[] Fluence;
    delete average_spectrum;
    delete spectrum;
}


void SaveGainSpectrum(int pulse, int k){
/*
    int i, n;
    FILE *file;
    double Dv = (v_max-v_min)/(n0-1); // frequency time step, Hz
    //K = layout_component[k]; // component number in the "components" list;

    bool firstAmSection = true;
    int pass = 0;
    for(i=0; i<k; i++){
        if(optics[layout_component[i]].type == "AM" || pulse>0)
            firstAmSection = false;
        if(optics[layout_component[i]].id==optics[layout_component[k]].id)
            pass++;
    }

    if(firstAmSection){
        file = fopen("data_band.dat", "w");
        fprintf(file, "# frequency[THz] gain[au]\n");
    }
    else{
        file = fopen("data_band.dat", "a");
        fprintf(file, "\n\n"); // data set separator
    }
    fprintf(file, "#pulse %d component %d pass %d\n", pulse, layout_component[k], pass);
    for(n=0; n<n0; n++)
        fprintf(file, "%.7f\t%.7f\n", (v_min+Dv*n)*1e-12, gainSpectrum[n]); //frequency in THz, gain in m-1 (<=> %/cm)
    fclose(file);
*/
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
