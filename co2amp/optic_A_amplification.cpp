#include  "co2amp.h"

void A::PulseInteraction(Pulse *pulse, Plane *plane, double time, int n_min, int n_max)
{
    if(p_CO2+p_N2+p_He <= 0 || length == 0)
        return;

    Debug(2, "Amplification");
    StatusDisplay(pulse, plane, time, "amplification...");

    double T2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.733*p_N2+0.64*p_He)); // transition dipole dephasing time, s
    //double tauR = 1e-7 / (750*(1.3*p_CO2+1.2*p_N2+0.6*p_He));        // rotational thermalization time, s
    double gamma = 1 / T2;   // Lorentzian HWHM (for gain spectrum calculation)

    // number of ro-vibrational transitions extracted from HITRAN files
    int num_tr[NumIso];
    for(int is=0; is<NumIso; ++is)
        num_tr[is] = v[is].size();

    // zero-out arrays when new pulse enters the amplifier section
    if(n_min==0)
    {
        // polarization
        for (auto& v : rho)
            std::fill(v.begin(), v.end(), 0.0);

        // spectrum
        std::fill(gainSpectrum.begin(), gainSpectrum.end(), 0.0);
    }

    // Pre-calculate re-usable expressios to accelerate computations
    //double exp_tauR = exp(-Dt/tauR/2); // half-step
    double exp_T2 = exp(-Dt/T2/2); // half-step
    std::vector<std::complex<double>> exp_phase[NumIso]; // half-step
    for(int is=0; is<NumIso; ++is)
    {
        for(int tr=0; tr<num_tr[is]; ++tr)
            exp_phase[is].push_back(exp(I*M_PI*(pulse->vc-v[is][tr])*Dt)); //half-step: note factor 2.0 in front of "PI" removed
    }

    // ====================== AMPLIFICATIOIN ======================
    int count = 0;
    #pragma omp parallel for// multithreaded
    for(int x=0; x<x0; ++x)
    {
        if(debug_level >= 0)
        {
            #pragma omp critical
            {
                StatusDisplay(pulse, plane, time,
                          "amplification: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
        }

        double N_vib0[NumIso][NumVib];
        std::vector<double> Dn[NumIso];                   // Population inversions (rotational transitions)
        //std::vector<std::complex<double>> rho[NumIso];    // Polarizations
        std::complex<double> E_in;                    // input field (before ampliifcation)
        double delta;                                 // change in population difference


        for(int is=0; is<NumIso; ++is)
        {
            // Remember populations of vibrational levels before pulse interaction
            for(int vl=0; vl<NumVib; ++vl)
                N_vib0[is][vl] = N_vib[is][vl][x];

            /*
            for(vl=0; vl<NumVib; ++vl)
            {
                N_vib[is][vl][x] = N_vib0[is][vl]; // initial population densities of vibrational levels
                for(j=0; j<NumRot; ++j)
                {
                    N_rot[is][vl][j][x] = f_rot[is][vl][j] * N_vib[is][vl][x]; // initial population densities of rotational sub-levels
                }
            }
            */

            // Initialize transition arrays
            for(int tr=0; tr<num_tr[is]; ++tr)
            {
                //rho[is].push_back(0);
                Dn[is].push_back(0);
            }

        }

        // Amplification
        //for(int n=0; n<n0; n++)
        for(int n=n_min; n<=n_max; ++n)
        {
            // shift center frequency to pulse->vc
            pulse->E[n0*x+n] *= exp(-I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
            // population inversions
            for(int is=0; is<NumIso; ++is) // for each isotopologue
            {
                if(N_iso[is]==0.0)
                    continue;
                for(int tr=0; tr<num_tr[is]; ++tr)
                    Dn[is][tr] = N_rot[is][vl_up[is][tr]][j_up[is][tr]][x] - N_rot[is][vl_lo[is][tr]][j_lo[is][tr]][x];
            }

            // gain spectrum
            if(x==0 && n==0) // in the beam center(!) before amplification
            {
                for(int is=0; is<NumIso; ++is)
                {
                    if(N_iso[is]==0.0)
                        continue;

                    for(int tr=0; tr<num_tr[is]; ++tr)
                    {
                        for(int n1=0; n1<n0; n1++)
                        {
                            gainSpectrum[n1] += sigma[is][tr]*(M_PI*gamma) * Dn[is][tr]
                                                * gamma/M_PI/(pow(2*M_PI*(v0+Dv*(n1-n0/2)-v[is][tr]),2)+pow(gamma,2)); // Gain [m-1]
                        }

                    }
                }
            }

            // Eq 1 (field) & Eq 2 (polarization)
            E_in = pulse->E[n0*x+n];
            for(int is=0; is<NumIso; ++is)
            {
                if(N_iso[is]==0)
                    continue;

                for(int tr=0; tr<num_tr[is]; ++tr)
                {
                    // Eq 2
                    /*rho[is][tr] *= exp_T2; // relaxation (half-step 1)
                    rho[is][tr] *= exp_phase[is][tr]; // phase relaxation (half-step 1)
                    rho[is][tr] -= sigma[is][tr]*Dn[is][tr]*E_in/(2*T2)*Dt; // excitation (full step)
                    rho[is][tr] *= exp_phase[is][tr]; // phase relaxation (half-step 2)
                    rho[is][tr] *= exp_T2; // relaxation (half-step 2)*/
                    rho[is][num_tr[is]*x+tr] *= exp_T2; // relaxation (half-step 1)
                    rho[is][num_tr[is]*x+tr] *= exp_phase[is][tr]; // phase relaxation (half-step 1)
                    rho[is][num_tr[is]*x+tr] -= sigma[is][tr]*Dn[is][tr]*E_in/(2*T2)*Dt; // excitation (full step)
                    rho[is][num_tr[is]*x+tr] *= exp_phase[is][tr]; // phase relaxation (half-step 2)
                    rho[is][num_tr[is]*x+tr] *= exp_T2; // relaxation (half-step 2)
                    // Eq 1
                    //pulse->E[n0*x+n] -= rho[is][tr] * length;
                    pulse->E[n0*x+n] -= rho[is][num_tr[is]*x+tr] * length;
                }
            }

            // Eq 3 (populations)
            for(int is=0; is<NumIso; ++is)
            {
                if(N_iso[is]==0.0)
                    continue;

                // ROTATIONAL REFILL (half-step 1)
                /*for(vl=0; vl<NumVib; ++vl)
                {
                    for(j=0; j<NumRot; ++j)
                    {
                        N_rot[is][vl][j][x] += (f_rot[is][vl][j]*N_vib[is][vl][x] - N_rot[is][vl][j][x]) * (1-exp_tauR);
                    }
                }*/

                // STIMULATED TRANSITIONS (full step)
                for(int tr=0; tr<num_tr[is]; ++tr)
                {
                    //delta = 4 * real(rho[is][tr]*conj((E_in+pulse->E[n0*x+n])/2.0)) * Dt;
                    delta = 4 * real(rho[is][num_tr[is]*x+tr]*conj((E_in+pulse->E[n0*x+n])/2.0)) * Dt;
                    // NOTE: E_in+pulse->E[n0*x+n])/2.0 is the average field (before and after amplification)

                    // upper level
                    N_vib[is][vl_up[is][tr]][x] += delta;
                    N_rot[is][vl_up[is][tr]][j_up[is][tr]][x] +=delta;

                    // lower level
                    N_vib[is][vl_lo[is][tr]][x] -= delta;
                    N_rot[is][vl_lo[is][tr]][j_lo[is][tr]][x] -=delta;
                }

                // ROTATIONAL REFILL (half-step 2)
                /*for(vl=0; vl<NumVib; vl++)
                {
                    for(j=0; j<NumRot; ++j)
                    {
                        N_rot[is][vl][j][x] += (f_rot[is][vl][j]*N_vib[is][vl][x] - N_rot[is][vl][j][x]) * (1-exp_tauR);
                    }
                }*/
            }

            // shift center frequency back to v0 (center of the calculation grid)
            pulse->E[n0*x+n] *= exp(I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
        }

        double DeltaN_nu3 = 0; // change of number of nu_3 quanta
        double DeltaN_nu2 = 0; // change of number of nu_2 quanta (+ double the change of nu_1 quanta)
        double DeltaN_grp[NumIso][NumGrp] = {}; // change of populations of groups of vibrational levels

        for(int is=0; is<NumIso; ++is)
        {
            if(N_iso[is]==0.0)
                continue;

            // change of populations of groups of vibrational levels
            // each isotopologue counted separately:
            DeltaN_grp[is][0] += N_vib[is][0][x]  - N_vib0[is][0];    // 00^01(1)
            DeltaN_grp[is][1] += N_vib[is][1][x]  - N_vib0[is][1]   + // 10^00(1)
                                 N_vib[is][2][x]  - N_vib0[is][2]   + // 10^00(2)
                                 N_vib[is][3][x]  - N_vib0[is][3]   + // 02^20(1)e
                                 N_vib[is][4][x]  - N_vib0[is][4];    // 02^20(1)f
            DeltaN_grp[is][2] += N_vib[is][5][x]  - N_vib0[is][5]   + // 01^11(1)e
                                 N_vib[is][6][x]  - N_vib0[is][6];    // 01^11(1)f
            DeltaN_grp[is][3] += N_vib[is][7][x]  - N_vib0[is][7]   + // 11^10(1)e
                                 N_vib[is][8][x]  - N_vib0[is][8]   + // 11^10(2)e
                                 N_vib[is][9][x]  - N_vib0[is][9]   + // 11^10(1)f
                                 N_vib[is][10][x] - N_vib0[is][10]  + // 11^10(2)f
                                 N_vib[is][11][x] - N_vib0[is][11]  + // 03^30(1)e
                                 N_vib[is][12][x] - N_vib0[is][12];   // 03^30(1)f
            DeltaN_grp[is][4] += N_vib[is][13][x] - N_vib0[is][13];   // 00^02(1)
            DeltaN_grp[is][5] += N_vib[is][14][x] - N_vib0[is][14]  + // 10^01(1)
                                 N_vib[is][15][x] - N_vib0[is][15]  + // 10^01(2)
                                 N_vib[is][16][x] - N_vib0[is][16]  + // 02^21(1)e
                                 N_vib[is][17][x] - N_vib0[is][17];   // 02^21(1)f

            for(int gr=0; gr<NumGrp; ++gr)
                N_grp[is][gr][x] += DeltaN_grp[is][gr];

            // change of nuber of vibrational quanta per molecule
            // all isotopologues are added together:

            // nu3
            DeltaN_nu3 += DeltaN_grp[is][0] + DeltaN_grp[is][2] + 2*DeltaN_grp[is][4] + DeltaN_grp[is][5];
            //              + 2*DeltaN_grp[is][6] + DeltaN_grp[is][7] + 3*DeltaN_grp[is][8] + 2*DeltaN_grp[is][9];

            // nu2 + 2*nu1 (Fermi-coupled vibrations)
            DeltaN_nu2 += 2*DeltaN_grp[is][1] + DeltaN_grp[is][2] + 3*DeltaN_grp[is][3] + 2*DeltaN_grp[is][5];
            //              + DeltaN_grp[is][6] + 3*DeltaN_grp[is][7] + 2*DeltaN_grp[is][9];
        }

        // change of vibrational temerature
        double Temp2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double e1_0 = 1/(exp(1920/Temp2)-1);
        double e2_0 = 2/(exp(960/Temp2)-1);
        e3[x] += DeltaN_nu3/N_CO2;
        e2[x] += DeltaN_nu2/N_CO2 * e2_0/(2*e1_0+e2_0); // last term describes distribution between nu1 and nu2
    }

    if(n_min==0)
        SaveGainSpectrum(pulse, plane);
}


void A::SaveGainSpectrum(Pulse *pulse, Plane *plane){
    FILE *file;

    int pass = 0;
    for(int i=0; i<plane->number; i++)
    {
        if(plane->optic->id == planes[i]->optic->id)
            pass++;
    }

    std::string basename = plane->optic->id
            + "_" + pulse->id
            + "_pass" + std::to_string(pass);

    file = fopen((basename+"_gain.dat").c_str(), "w");
    fprintf(file, "#Data format: frequency[Hz] gain[m^-1 = %%/cm]\n");
    for(int n=0; n<n0; n++)
        fprintf(file, "%.8E\t%e\n", v_min+Dv*(0.5+n), gainSpectrum[n]); //frequency in Hz, gain in m-1 (<=> %/cm)

    fclose(file);
}
