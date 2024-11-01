#include  "co2amp.h"

void A::PulseInteraction(Pulse *pulse, Plane *plane, double time)
{
    if(p_CO2+p_N2+p_He <= 0 || length == 0)
        return;

    Debug(2, "Amplification");
    StatusDisplay(pulse, plane, time, "amplification...");

    double Dt = (t_max-t_min)/n0;    // pulse time step, s
    double Dv = 1.0/(t_max-t_min);   // frequency step, Hz

    double N[6] = {2.7e25*p_626, 2.7e25*p_628, 2.7e25*p_828, 2.7e25*p_636, 2.7e25*p_638,2.7e25*p_838}; // CO2 number densities, 1/m^3
    double Nco2 = 2.7e25*p_CO2;

    double T2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.733*p_N2+0.64*p_He)); // transition dipole dephasing time, s
    double tauR = 1e-7 / (750*(1.3*p_CO2+1.2*p_N2+0.6*p_He));        // rotational termalisation time, s
    double gamma = 1.0/T2;   // Lorentzian HWHM (for gain spectrum calculation)

    // number of ro-vibrational transitions extracted from HITRAN files
    int n_transitions[6];
    for(int i=0; i<6; ++i)
    {
        n_transitions[i] = v[i].size();
    }

    // Pre-calculate re-usable expressios to accelerate computations
    double exp_tauR = exp(-Dt/tauR/2.0); // half-step
    double exp_T2 = exp(-Dt/T2/2.0); // half-step
    std::vector<std::complex<double>> exp_phase[6]; // half-step
    for(int i=0; i<6; i++)
    {
        for(int tr=0; tr<n_transitions[i]; ++tr)
        {
            exp_phase[i].push_back(exp(I*M_PI*(pulse->vc-v[i][tr])*Dt)); //half-step: note factor 2.0 in front of "PI" removed
        }
    }

    // initialize/clear gain spectrum array
    for(int n=0; n<n0; n++)
        gainSpectrum[n] = 0;

    // ====================== AMPLIFICATIOIN ======================
    int count = 0;
    #pragma omp parallel for// multithreaded
    for(int x=0; x<x0; x++)
    {
        if(debug_level >= 0)
        {
            #pragma omp critical
            {
                StatusDisplay(pulse, plane, time,
                          "amplification: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
        }
        int i;  // isotopologue  number 0 - 626; 1 - 628; 2 - 828; 3 - 636; 4 - 638; 5 - 838
        int vl; // vibrational level number
        int j;  // rotational quantum number
        double Nvib[6][18], Nvib0[6][18];             // Population densities of vibrational lelvels (actual and equilibrium)
        double Nrot[6][18][60];                       // Population densities of rotational lelvels (actual)
        std::vector<double> Dn[6];                    // Population inversions (rotational transitions)
        std::vector<std::complex<double>> rho[6];     // Polarizations
        std::complex<double> E_in;                    // input field (before ampliifcation)
        double Temp2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double Temp3 = VibrationalTemperatures(x, 3); // equilibrium vibrational temterature of nu3 mode
        double delta;                                 // change in population difference

        // Initial populations and polarizations
        for(i=0; i<6; i++)
        {
            // Vibrational level population densities in thermal equilibrium - see Witteman p. 71 and Nevdakh 2007
            double Q = 1 / ( (1-exp(-1920/Temp2))*pow(1-exp(-960/Temp2),2)*(1-exp(-3380/Temp3)) ); // partition function
            // 001
            Nvib0[i][0]  = N[i]*exp(-3380/Temp3)/Q; // upper reg
            // 100 + 020
            Nvib0[i][1]  = 2* N[i]*exp(-1920/Temp2)/Q;
            Nvib0[i][2]  = Nvib0[i][1];
            Nvib0[i][3]  = Nvib0[i][1];
            Nvib0[i][4]  = Nvib0[i][1];
            // 011
            Nvib0[i][5]  = N[i]*exp(-(1920+960)/Temp2)/Q;  // upper hot-e
            Nvib0[i][6]  = Nvib0[i][5];                    // upper hot-f
            // 110 + 030
            Nvib0[i][7]  = 2* N[i]*exp(-3380/Temp3)*exp(-960/Temp2)/Q;
            Nvib0[i][8]  = Nvib0[i][7];
            Nvib0[i][9]  = Nvib0[i][7];
            Nvib0[i][10] = Nvib0[i][7];
            Nvib0[i][11] = Nvib0[i][7];
            Nvib0[i][12] = Nvib0[i][7];
            // 002
            Nvib0[i][13] = N[i]*exp(-2*3380/Temp3)/Q; // upper seq
            // 101 + 021
            Nvib0[i][14] = 2* N[i]*exp(-1920/Temp2)*exp(-3380/Temp3)/Q;
            Nvib0[i][15] = Nvib0[i][14];
            Nvib0[i][16] = Nvib0[i][14];
            Nvib0[i][17] = Nvib0[i][14];

            for(vl=0; vl<18; ++vl)
            {
                Nvib[i][vl] = Nvib0[i][vl]; // initial population densities of vibrational levels
                for(j=0; j<60; ++j)
                    Nrot[i][vl][j] = nop[i][vl][j] * Nvib[i][vl]; // initial population densities of rotational sub-levels
            }


            for(int tr=0; tr<n_transitions[i]; ++tr)
            {
                rho[i].push_back(0);
                Dn[i].push_back(0);
            }
        }

        // Amplification
        for(int n=0; n<n0; n++)
        {
            // shift center frequency to pulse->vc
            pulse->E[x][n] *= exp(-I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
            // population inversions
            for(i=0; i<6; i++) // for each isotopologue
            {
                if(N[i]==0.0) continue;
                for(int tr=0; tr<n_transitions[i]; ++tr)
                {
                    Dn[i][tr] = Nrot[i][vl_up[i][tr]][j_up[i][tr]] - Nrot[i][vl_lo[i][tr]][j_lo[i][tr]];
                }
            }

            // gain spectrum
            if(x==0 && n==0) // in the beam center(!) before amplification
            {
                for(i=0; i<6; i++)
                {
                    if(N[i]==0.0)
                        continue;

                    for(int tr=0; tr<n_transitions[i]; ++tr)
                    {
                        for(int n1=0; n1<n0; n1++)
                        {
                            gainSpectrum[n1] += sigma[i][tr]*(M_PI*gamma) * Dn[i][tr]
                                                * gamma/M_PI/(pow(2*M_PI*(v0+Dv*(n1-n0/2)-v[i][tr]),2)+pow(gamma,2)); // Gain [m-1]
                        }

                    }
                }
            }

            // Eq 1 (field) & Eq 2 (polarization)
            E_in = pulse->E[x][n];
            for(i=0; i<6; i++)
            {
                if(N[i]==0)
                    continue;

                for(int tr=0; tr<n_transitions[i]; ++tr)
                {
                    // Eq 2
                    rho[i][tr] *= exp_T2; // relaxation (half-step 1)
                    rho[i][tr] *= exp_phase[i][tr]; // phase relaxation (half-step 1)
                    rho[i][tr] -= sigma[i][tr]*Dn[i][tr]*E_in/(2*T2)*Dt; // excitation (full step)
                    rho[i][tr] *= exp_phase[i][tr]; // phase relaxation (half-step 2)
                    rho[i][tr] *= exp_T2; // relaxation (half-step 2)
                    // Eq 1
                    pulse->E[x][n] -= rho[i][tr] * length;
                }
            }

            // Eq 3 (populations)
            for(i=0; i<6; i++)
            {
                if(N[i]==0.0)
                    continue;

                // ROTATIONAL REFILL (half-step 1)
                for(vl=0; vl<18; ++vl)
                {
                    for(j=0; j<60; ++j)
                    {
                        Nrot[i][vl][j] += (nop[i][vl][j]*Nvib[i][vl] - Nrot[i][vl][j]) * (1-exp_tauR);
                    }
                }

                // STIMULATED TRANSITIONS (full step)
                for(int tr=0; tr<n_transitions[i]; ++tr)
                {
                    delta = 4 * real(rho[i][tr]*conj((E_in+pulse->E[x][n])/2.0)) * Dt;
                    // NOTE: E_in+pulse->E[x][n])/2.0 is the average field (before and after amplification)

                    // upper level
                    Nvib[i][vl_up[i][tr]] += delta;
                    Nrot[i][vl_up[i][tr]][j_up[i][tr]]+=delta;

                    // lower level
                    Nvib[i][vl_lo[i][tr]] -= delta;
                    Nrot[i][vl_lo[i][tr]][j_lo[i][tr]]-=delta;
                }

                // ROTATIONAL REFILL (half-step 2)
                for(vl=0; vl<18; vl++)
                {
                    for(j=0; j<60; ++j)
                    {
                        Nrot[i][vl][j] += (nop[i][vl][j]*Nvib[i][vl] - Nrot[i][vl][j]) * (1.0-exp_tauR);
                    }
                }
            }

            // shift center frequency back to v0 (center of the calculation grid)
            pulse->E[x][n] *= exp(I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
        }

        double DeltaN_nu3 = 0; // change of number of nu_3 quanta
        double DeltaN_nu2 = 0; // change of number of nu_2 quanta (+ double the change of nu_1 quanta)
        for(i=0; i<6; i++)
        {
            if(N[i]==0.0) continue;

            // change of number of nu_3 quanta
            DeltaN_nu3 +=    Nvib[i][0]  - Nvib0[i][0]   + // 00^01(1)
                             Nvib[i][5]  - Nvib0[i][5]   + // 01^11(1)e
                             Nvib[i][6]  - Nvib0[i][6]   + // 01^11(1)f
                          2*(Nvib[i][13] - Nvib0[i][13]) + // 00^02(1)
                             Nvib[i][14] - Nvib0[i][14]  + // 10^01(1)
                             Nvib[i][15] - Nvib0[i][15]  + // 10^01(2)
                             Nvib[i][16] - Nvib0[i][16]  + // 02^21(1)e
                             Nvib[i][17] - Nvib0[i][17];   // 02^21(1)f

            // change of number of nu_2 quanta (+ double the change of nu_1 quanta)
            DeltaN_nu2 += 2*(Nvib[i][1]  - Nvib0[i][1])  + // 10^00(1)
                          2*(Nvib[i][2]  - Nvib0[i][2])  + // 10^00(2)
                          2*(Nvib[i][3]  - Nvib0[i][3])  + // 02^20(1)e
                          2*(Nvib[i][4]  - Nvib0[i][4])  + // 02^20(1)f
                             Nvib[i][5]  - Nvib0[i][5]   + // 01^11(1)e
                             Nvib[i][6]  - Nvib0[i][6]   + // 01^11(1)f
                          3*(Nvib[i][7]  - Nvib0[i][7])  + // 11^10(1)e
                          3*(Nvib[i][8]  - Nvib0[i][8])  + // 11^10(2)e
                          3*(Nvib[i][9]  - Nvib0[i][9])  + // 11^10(1)f
                          3*(Nvib[i][10] - Nvib0[i][10]) + // 11^10(2)f
                          3*(Nvib[i][11] - Nvib0[i][11]) + // 03^30(1)e
                          3*(Nvib[i][12] - Nvib0[i][12]) + // 03^30(1)f
                          2*(Nvib[i][14] - Nvib0[i][14]) + // 10^01(1)
                          2*(Nvib[i][15] - Nvib0[i][15]) + // 10^01(2)
                          2*(Nvib[i][16] - Nvib0[i][16]) + // 02^21(1)e
                          2*(Nvib[i][17] - Nvib0[i][17]);  // 02^21(1)f
        }

        // change of vibrational temerature
        double e1_0 = 1/(exp(1920/Temp2)-1);
        double e2_0 = 2/(exp(960/Temp2)-1);
        e3[x] += DeltaN_nu3/Nco2;
        e2[x] += DeltaN_nu2/Nco2 * e2_0/(2*e1_0+e2_0); // last term describes distribution between nu_1 and nu_2
    }

    SaveGainSpectrum(pulse, plane);
}


void A::SaveGainSpectrum(Pulse *pulse, Plane *plane){
    FILE *file;
    double Dv = 1.0/(t_max-t_min);    // frequency step, Hz

    int pass = 0;
    for(int i=0; i<plane->number; i++)
        if(plane->optic->id == planes[i]->optic->id)
            pass++;

    std::string basename = plane->optic->id
            + "_" + pulse->id
            + "_pass" + std::to_string(pass);

    file = fopen((basename+"_gain.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] gain[m^-1 = %%/cm]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%e\t%e\n", v0+Dv*(n-n0/2), gainSpectrum[n]); //frequency in Hz, gain in m-1 (<=> %/cm)

    fclose(file);
}
