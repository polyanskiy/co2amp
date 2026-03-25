#include  "co2amp.h"

void A::PulseInteraction(Pulse *pulse, Plane *plane, int m, int n_min, int n_max)
{
    if(p_CO2+p_N2+p_He <= 0 || length == 0)
        return;

    //Debug(2, "Amplification");
    StatusDisplay(pulse, plane, m, "amplification...");
    if(debug_level >= 3)
        std::cout << std::endl;
    //std::string tmp_str = "amplification... time steps: " + std::to_string(n_min) + "-"  + std::to_string(n_max);
    //StatusDisplay(pulse, plane, m, tmp_str);

    flag_interaction = n_max==n0-1 ? false : true;

    double tauR = 1e-7 / (750*(1.3*p_CO2+1.2*p_N2+0.6*p_He)); // rotational thermalization time, s
    double rot_relax = 1.0 - exp(-Dt/tauR/2); // half-step (rotational relaxation during Dt/2)



    int num_pulses = pulses.size();
    int pulse_n = pulse->number;

    int num_passes = 0; // how many times each pulse passes this a.m. section
    for(size_t i=0; i<planes.size(); ++i)
        if(planes[i]->optic == this)
            num_passes++;

    // Count pass number through current element
    int pass_n = 0;
    for(int i=0; i<plane->number; ++i)
        if(planes[i]->optic == plane->optic)
            pass_n++;

    int num_tr[NumIso]; // number of transitions to concider
    int offset[NumIso]; // index offset in the rho vector corresponding to given pulse and pass number
    for(int is=0; is<NumIso; ++is)
    {
        num_tr[is] = v[is].size();
        offset[is] = pulse_n*num_passes*x0*num_tr[is] + pass_n*x0*num_tr[is];
    }

    // zero-out arrays when new pulse enters the amplifier section
    if(n_min==0)
    {
        // polarization (only zero-out elements corresponding to the present pulse and pass number
        for(int is=0; is<NumIso; ++is)
            std::fill_n(rho[is].begin()+offset[is], x0*num_tr[is], 0.0);

        // spectrum
        std::fill_n(gainSpectrum.begin(), n0, 0.0);
    }


    // ====================== AMPLIFICATIOIN ======================
    //int count = 0;
    #pragma omp parallel for// multithreaded
    for(int x=0; x<x0; ++x)
    {
        // this block freezes the output at large x0
        /*if(debug_level >= 0)
        {
            std::string tmp_str = "amplification... time " + std::to_string(n_min) + "-"  + std::to_string(n_max) + "; coord ";
            #pragma omp critical
            {
                StatusDisplay(pulse, plane, m, tmp_str + std::to_string(++count));
            }
        }*/

        double N_vib0[NumIso][NumVib];
        std::vector<double> Dn[NumIso];               // Population inversions (rotational transitions)
        std::complex<double> E_in;                    // input field (before ampliifcation)
        double delta;                                 // change in population difference


        for(int is=0; is<NumIso; ++is)
        {
            // Remember populations of vibrational levels before pulse interaction
            for(int vl=0; vl<NumVib; ++vl)
                N_vib0[is][vl] = N_vib[is][vl][x];

            // Initialize populations of rotational sub-leves
            if(n_min==0)
            {
                for(int vl=0; vl<NumVib; ++vl)
                {
                    for(int j=0; j<NumRot; ++j)
                    {
                        N_rot[is][vl][j][x] = f_rot[is][vl][j] * N_vib0[is][vl]; // initial population densities of rotational sub-levels
                    }
                }
            }

            // Initialize transition arrays
            for(int tr=0; tr<num_tr[is]; ++tr)
                Dn[is].push_back(0);

        }

        // Amplification
        for(int n=n_min; n<=n_max; ++n)
        {
            // shift center frequency to pulse->vc
            pulse->E[n0*x+n] *= exp(I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
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
                            gainSpectrum[n1] += sigma[is][tr] * Dn[is][tr] * pow(fwhm[is][tr]/2,2)
                                                / ( pow((v0+Dv*(n1-n0/2)-v[is][tr]),2) + pow(fwhm[is][tr]/2,2) ); // Gain [m-1]
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
                    /*
                    // Eq 2
                    rho[is][offset[is]+num_tr[is]*x+tr] *= dephase_exp[is][tr];                               // polarization dephasing (half-step 1)
                    rho[is][offset[is]+num_tr[is]*x+tr] *= detune_exp[is][num_pulses*tr + pulse_n];           // phase detuning (half-step 1)
                    rho[is][offset[is]+num_tr[is]*x+tr] -= sigma[is][tr]*Dn[is][tr]*E_in/(2*tau2[is][tr])*Dt; // excitation (full step)
                    rho[is][offset[is]+num_tr[is]*x+tr] *= detune_exp[is][num_pulses*tr + pulse_n];           // phase detuning (half-step 2)
                    rho[is][offset[is]+num_tr[is]*x+tr] *= dephase_exp[is][tr];                               // polarization dephasing (half-step 2)
                    */

                    // Eq 2
                    // *** exact solution over Dt for fixed Dn and E_in ***
                    // dρ/dt = -a ρ + b
                    // a = 1/tau2 + i*2π*(vc - v_j)
                    // b = - sigma / (2*tau2) * Dn * E_in
                    // Exact: ρ <- ρ*exp(-a*Dt) + b*(1-exp(-a*Dt))/a

                    // a (and exp) may differ between pulses if vc is different
                    std::complex<double> a   = precalc_a[is][num_pulses*tr + pulse_n];
                    std::complex<double> exp = precalc_exp[is][num_pulses*tr + pulse_n];
                    // b doesn't depend on pulse_n
                    std::complex<double> b   = precalc_b_part[is][tr] * Dn[is][tr] * E_in;

                    auto &rho_ = rho[is][offset[is] + num_tr[is]*x + tr];

                    rho_ = rho_ * exp + b * (1.0 - exp) / a;

                    // Eq 1
                    pulse->E[n0*x+n] -= rho[is][offset[is]+num_tr[is]*x+tr] * length;
                }
            }

            // Eq 3 (populations)
            for(int is=0; is<NumIso; ++is)
            {
                if(N_iso[is]==0.0)
                    continue;

                // ROTATIONAL REFILL (half-step 1)
                for(int vl=0; vl<NumVib; ++vl)
                {
                    for(int j=0; j<NumRot; ++j)
                    {
                        N_rot[is][vl][j][x] += (f_rot[is][vl][j]*N_vib[is][vl][x] - N_rot[is][vl][j][x]) * rot_relax;
                    }
                }

                // STIMULATED TRANSITIONS (full step)
                for(int tr=0; tr<num_tr[is]; ++tr)
                {
                    delta = 4 * real(rho[is][offset[is]+num_tr[is]*x+tr]*conj((E_in+pulse->E[n0*x+n])/2.0)) * Dt;
                    // NOTE: E_in+pulse->E[n0*x+n])/2.0 is the average field (before and after amplification)

                    // upper level
                    N_vib[is][vl_up[is][tr]][x] += delta;
                    N_rot[is][vl_up[is][tr]][j_up[is][tr]][x] +=delta;

                    // lower level
                    N_vib[is][vl_lo[is][tr]][x] -= delta;
                    N_rot[is][vl_lo[is][tr]][j_lo[is][tr]][x] -=delta;
                }

                // ROTATIONAL REFILL (half-step 2)
                for(int vl=0; vl<NumVib; vl++)
                {
                    for(int j=0; j<NumRot; ++j)
                    {
                        N_rot[is][vl][j][x] += (f_rot[is][vl][j]*N_vib[is][vl][x] - N_rot[is][vl][j][x]) * rot_relax;
                    }
                }
            }

            // shift center frequency back to v0 (center of the calculation grid)
            pulse->E[n0*x+n] *= exp(-I*2.0*M_PI*(v0-pulse->vc)*Dt*(0.5+n));
        }

        double DeltaN_nu3 = 0; // change of number of nu_3 quanta
        double DeltaN_nu2 = 0; // change of number of nu_2 quanta (+ double the change of nu_1 quanta)

        for(int is=0; is<NumIso; ++is)
        {
            if(N_iso[is]==0.0)
                continue;

            // change of nuber of vibrational quanta per molecule
            // all isotopologues are added together:

            // nu3
            DeltaN_nu3 +=      N_vib[is][1][x]  - N_vib0[is][1]   // 001
                        + 2 * (N_vib[is][2][x]  - N_vib0[is][2])  // 002
                        +      N_vib[is][14][x] - N_vib0[is][14]  // 011
                        +      N_vib[is][15][x] - N_vib0[is][15]
                        +      N_vib[is][16][x] - N_vib0[is][16]  // 101+021
                        +      N_vib[is][17][x] - N_vib0[is][17]
                        +      N_vib[is][18][x] - N_vib0[is][18]
                        +      N_vib[is][19][x] - N_vib0[is][19];

            // nu2 + 2*nu1 (Fermi-coupled vibrations)
            DeltaN_nu2 += 2 * (N_vib[is][4][x]  - N_vib0[is][4]   // 100+020
                        +      N_vib[is][5][x]  - N_vib0[is][5]
                        +      N_vib[is][6][x]  - N_vib0[is][6]
                        +      N_vib[is][7][x]  - N_vib0[is][7])
                        + 3 * (N_vib[is][8][x]  - N_vib0[is][8]   // 110+030
                        +      N_vib[is][9][x]  - N_vib0[is][9]
                        +      N_vib[is][10][x] - N_vib0[is][10]
                        +      N_vib[is][11][x] - N_vib0[is][11])
                        +      N_vib[is][14][x] - N_vib0[is][14]  // 011
                        +      N_vib[is][15][x] - N_vib0[is][15]
                        + 2 * (N_vib[is][16][x] - N_vib0[is][16]  // 101+021
                        +      N_vib[is][17][x] - N_vib0[is][17]
                        +      N_vib[is][18][x] - N_vib0[is][18]
                        +      N_vib[is][19][x] - N_vib0[is][19]);
        }

        // change of vibrational temerature
        double T2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double e1_0 = 1/(exp(1920/T2)-1);
        double e2_0 = 2/(exp(960/T2)-1);
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
