#include  "co2amp.h"


void A::InternalDynamics(int m)
{
    if(p_CO2+p_N2+p_He <= 0)
        return;

    StatusDisplay(nullptr, nullptr, m, "molecular dynamics...");

    // Thermalization time for semi-population model
    // k = 3.9e6 (s torr)^{-1}   [Finzi & Moore 1975 - https://doi.org/10.1063/1.431678]
    double tauV = 1e-6 / (3.9*750*p_CO2); // intra-mode vibrational thermalization time
    // Pre-calculate re-usable expressions to accelerate computations
    double vib_relax = 1.0 - exp(-time_tick/tauV);

    double N = 273*(p_CO2+p_N2+p_He)/T0;

    // Relative concentrations: y1: CO2, y2: N2, y3: He
    double y1 = p_CO2/(p_CO2+p_N2+p_He);
    double y2 = p_N2/(p_CO2+p_N2+p_He);
    double y3 = p_He/(p_CO2+p_N2+p_He);

    // Constant used in calculation of translational temperature dynamics ("C_v" in the manual)
    double cv = 2.5*(y1+y2) + 1.5*y3;

    // Discharge power
    double W = 0;

    // populations before interaction
    std::vector<double> N_vib0[NumIso][NumVib];
    for (int is = 0; is < NumIso; ++is)
        for (int vl = 0; vl < NumVib; ++vl)
            N_vib0[is][vl].resize(x0);

    #pragma omp parallel for// multithreaded
    for(int x=0; x<x0; ++x)
    {
        double A, X;
        double K, K31, K32, K33, K21, K22, K23;
        double ra, rc, r2, r3;
        double f2, f3;
        double D_e2, D_e3, D_e4;

        A = T[x]/273 * pow(1+0.5*e2e(T[x]),-3);
        X = pow(T[x],-1.0/3);

        // Collisional relaxation rates, 1/s
        K = 240/pow(T[x],0.5) * 1e6;
        K31 = A * exp(4.138+7.945 *X-631.24*X*X+2239.0 *X*X*X) * 1e6;
        K32 = A * exp(-1.863+213.3*X-2796.2*X*X+9001.9 *X*X*X) * 1e6;
        K33 = A * exp(-3.276+291.4*X-3831.8*X*X+12688.0*X*X*X) * 1e6;
        K21 = 1160 * exp(-59.3*X) * 1e6;
        K22 = 855  * exp(-69.0*X) * 1e6;
        K23 = 1300 * exp(-40.6*X) * 1e6;

        ra = K*N*y1;
        rc = K*N*y2;
        r2 = N*(y1*K21 + y2*K22 + y3*K23);
        r3 = N*(y1*K31 + y2*K32 + y3*K33) ;

        f2 = 2 * pow(1+e2[x],2) / (2+6*e2[x]+3*pow(e2[x],2));
        f3 = e3[x]*pow(1+e2[x]/2,3) - (1+e3[x])*pow(e2[x]/2,3)*exp(-500/T[x]);

        // Pumping rates, 1/s
        // - vibrational modes
        double pump2 = 0;
        double pump3 = 0;
        double pump4 = 0;
        // - groups of vibrational levels
        double pump_vl[NumVib] = {}; // initialized with zeros

        if(pumping == "discharge")
        {
            W = current[m]*voltage[m] / Vd;              // W/m^3
            pump4 = y2!=0.0 ? 0.8e-6*q4[m]/N/y2*W : 0;   // 1/s
            pump3 = y1!=0.0 ? 0.8e-6*q3[m]/N/y1*W : 0;   // 1/s
            pump2 = y1!=0.0 ? 2.8e-6*q2[m]/N/y1*W : 0;   // 1/s
        }

        if(pumping == "optical")
        {
            double photon_flux = normalized_intensity[m] * fluence[x] / (h*c/pump_wl); // photons/(m^2 * s)

            if(pump_level == "001") // direct excitation of (001) level @ ~4.3 um
            {
                pump3 = photon_flux * pump_sigma;
                pump2 = 0;
                pump_vl[0] = pump3;
            }

            if(pump_level == "021") // excitation via combinational vibration (101,021) @ ~2.8 um
            {
                pump3 = photon_flux * pump_sigma;
                pump2 = 2 * pump3;
                pump_vl[14] = 0.25*pump3;
                pump_vl[15] = 0.25*pump3;
                pump_vl[16] = 0.25*pump3;
                pump_vl[17] = 0.25*pump3;
            }

            if(pump_level == "041") // excitation via combinational vibration (201,121,041) @ ~2.0 um
            // ! excitation level is not directly involved in any laser transition
            //   and thus not associated with a "group" of levels
            {
                pump3 = photon_flux * pump_sigma;
                pump2 = 4 * pump3;
            }

            if(pump_level == "002") // excitation via 2nd overtone level @ ~2.2 um
            // ! only possible for non-symmetric molecules
            {
                pump3 = 2 * photon_flux * pump_sigma;
                pump2 = 0;
                pump_vl[13] = pump3/2;
            }

            if(pump_level == "003") // excitation via 3rd overtone level @ ~1.4 um
            {
                pump3 = 3 * photon_flux * pump_sigma;
                pump2 = 0;
            }
        }

        // pumping and collisional relaxation
        D_e2 = 0;
        D_e3 = 0;
        D_e4 = 0;
        if(y1!=0.0)
        {
            D_e2 = f2 *( pump2 + 3*r3*f3 - r2*(e2[x]-e2e(T[x])) ) * time_tick;
            D_e3 = ( pump3 + rc*(e4[x]-e3[x]) - r3*f3 ) * time_tick;
        }
        if(y2!=0.0)
        {
            D_e4 = ( pump4 - ra*(e4[x]-e3[x]) ) * time_tick;
        }
        e2[x] += D_e2;
        e3[x] += D_e3;
        e4[x] += D_e4;

        // Remember populations of vibrational levels before pulse interaction
        for(int is=0; is<NumIso; ++is)
        {
            for(int vl=0; vl<NumVib; ++vl)
            {
                N_vib0[is][vl][x] = N_vib[is][vl][x];
            }
        }

        // In our model some levels can be pumped directly in case of optical pumping
        // In other cases, pump energy first goes to corresponding vibrational modes
        // and then re-distibutes between levels through intramode thermalization
        for(int is=0; is<NumIso; ++is)
        {
            // 001      optical pumping @ ~4.3 um
            N_vib[is][0][x] += N_iso[is] * pump_vl[0] * time_tick;;
            // 002      optical pumping @ ~2.2 um (non-symmetric molecules only)
            N_vib[is][13][x] += N_iso[is] * pump_vl[4] * time_tick;
            // 101+021  optical pumping @ ~2.8 um
            N_vib[is][14][x] += N_iso[is] * pump_vl[14] * time_tick;
            N_vib[is][15][x] += N_iso[is] * pump_vl[15] * time_tick;
            N_vib[is][16][x] += N_iso[is] * pump_vl[16] * time_tick;
            N_vib[is][17][x] += N_iso[is] * pump_vl[17] * time_tick;
        }

        T[x] += y1/cv * (500*r3*f3 + 960*r2*(e2[x]-e2e(T[x]))) * time_tick;

        if(pumping=="discharge")
            T[x] += 2.7e-3*W*qT[m]/N/cv * time_tick;

        // Include non-zero thermalization time
        double T2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double T3 = VibrationalTemperatures(x, 3); // equilibrium vibrational temterature of nu3 mode
        double Q = 1 / ( (1-exp(-1920/T2))*pow(1-exp(-960/T2),2)*(1-exp(-3380/T3)) ); // partition function

        double N_grp;

        double w[NumIso][NumVib], w_sum;

        for(int is=0; is<NumIso; ++is)
        {
            w[is][0]  = 1;                                            // 001
            w[is][1]  = exp( -h *  G[is][1]              / (kB*T2) ); // 100+020
            w[is][2]  = exp( -h *  G[is][2]              / (kB*T2) );
            w[is][3]  = exp( -h *  G[is][3]              / (kB*T2) );
            w[is][4]  = exp( -h *  G[is][4]              / (kB*T2) );
            w[is][5]  = exp( -h * (G[is][5] - G[is][0])  / (kB*T2) ); // 011 (ignore energy in nu3)
            w[is][6]  = exp( -h * (G[is][6] - G[is][0])  / (kB*T2) );
            w[is][7]  = exp( -h *  G[is][7]              / (kB*T2) ); // 110+030
            w[is][8]  = exp( -h *  G[is][8]              / (kB*T2) );
            w[is][9]  = exp( -h *  G[is][9]              / (kB*T2) );
            w[is][10] = exp( -h *  G[is][10]             / (kB*T2) );
            w[is][11] = exp( -h *  G[is][11]             / (kB*T2) );
            w[is][12] = exp( -h *  G[is][12]             / (kB*T2) );
            w[is][13] = 1;                                            // 002
            w[is][14] = exp( -h * (G[is][14] - G[is][0]) / (kB*T2) ); // 101+021 (ignore energy in nu3)
            w[is][15] = exp( -h * (G[is][15] - G[is][0]) / (kB*T2) );
            w[is][16] = exp( -h * (G[is][16] - G[is][0]) / (kB*T2) );
            w[is][17] = exp( -h * (G[is][17] - G[is][0]) / (kB*T2) );
        }

        for(int is=0; is<NumIso; ++is)
        {
            // INTRA-MODE THERMALIZATION
            // 001
            N_grp = N_iso[is]*exp(-3380/T3)/Q;
            N_vib[is][0][x] += (N_grp - N_vib[is][0][x]) * vib_relax;
            // 100 + 020
            N_grp = 4 * N_iso[is]*exp(-2*960/T2)/Q;
            w_sum = w[is][1] + w[is][2] + w[is][3] + w[is][4];
            N_vib[is][1][x]  += (N_grp*w[is][1]/w_sum  - N_vib[is][1][x]) * vib_relax;
            N_vib[is][2][x]  += (N_grp*w[is][2]/w_sum  - N_vib[is][2][x]) * vib_relax;
            N_vib[is][3][x]  += (N_grp*w[is][3]/w_sum  - N_vib[is][3][x]) * vib_relax;
            N_vib[is][4][x]  += (N_grp*w[is][4]/w_sum  - N_vib[is][4][x]) * vib_relax;
            // 011
            N_grp = 2 * N_iso[is]*exp(-960/T2)*exp(-3380/T3)/Q;
            w_sum = w[is][5] + w[is][6];
            N_vib[is][5][x]  += (N_grp*w[is][5]/w_sum  - N_vib[is][5][x]) * vib_relax;
            N_vib[is][6][x]  += (N_grp*w[is][6]/w_sum  - N_vib[is][6][x]) * vib_relax;
            // 110 + 030
            N_grp = 6 * N_iso[is]*exp(-3*960/T2)/Q;
            w_sum = w[is][7] + w[is][8] + w[is][9] + w[is][10] + w[is][11] + w[is][12];
            N_vib[is][7][x]  += (N_grp*w[is][7]/w_sum  - N_vib[is][7][x]) * vib_relax;
            N_vib[is][8][x]  += (N_grp*w[is][8]/w_sum  - N_vib[is][8][x]) * vib_relax;
            N_vib[is][9][x]  += (N_grp*w[is][9]/w_sum  - N_vib[is][9][x]) * vib_relax;
            N_vib[is][10][x] += (N_grp*w[is][10]/w_sum - N_vib[is][10][x]) * vib_relax;
            N_vib[is][11][x] += (N_grp*w[is][11]/w_sum - N_vib[is][11][x]) * vib_relax;
            N_vib[is][12][x] += (N_grp*w[is][12]/w_sum - N_vib[is][12][x]) * vib_relax;
            // 002
            N_grp = N_iso[is]*exp(-2*3380/T3)/Q;
            N_vib[is][13][x] += (N_grp - N_vib[is][13][x]) * vib_relax;       // upper seq
            // 101 + 021
            N_grp = 4 * N_iso[is]*exp(-2*960/T2)*exp(-3380/T3)/Q;
            w_sum = w[is][13] + w[is][14] + w[is][15] + w[is][16];
            N_vib[is][13][x] += (N_grp*w[is][13]/w_sum - N_vib[is][13][x]) * vib_relax;
            N_vib[is][14][x] += (N_grp*w[is][14]/w_sum - N_vib[is][14][x]) * vib_relax;
            N_vib[is][15][x] += (N_grp*w[is][15]/w_sum - N_vib[is][15][x]) * vib_relax;
            N_vib[is][16][x] += (N_grp*w[is][16]/w_sum - N_vib[is][16][x]) * vib_relax;
        }


    }

    // ROTATIONAL LEVELS
    // We only care about energy distribution between rotational sub-level during the amplification process
    // Rotational relaxation is included in the amplification model that uses a faster time scale.
    // Here, we only consider how the change in vibrational energy in the result of pumping and vibrational
    //   relaxation distributes between rotational levels during amplificaiton. Rotational relaxation
    //   is NOT modelled here.
    if(flag_interaction == true)
    {
        #pragma omp parallel for collapse(3)// schedule(static)
        for (int is = 0; is < NumIso; ++is)
        {
            for (int vl = 0; vl < NumVib; ++vl)
            {
                for (int j = 0; j < NumRot; ++j)
                {
                    for (int x = 0; x < x0; ++x)
                    {
                        N_rot[is][vl][j][x] += f_rot[is][vl][j]*(N_vib[is][vl][x] - N_vib0[is][vl][x]);
                    }
                }
            }
        }
    }

    // Update pumping and molecular dynamics files
    int n_ticks = std::llround(save_interval/time_tick); // number of ticks between saves
    if(n_ticks<1)
        n_ticks=1;
    if(m % n_ticks == 0)
    {
        //std::cout << std::endl << "m = " << m  << "\t n_ticks = " << n_ticks << "\t m0 = " << m0 << std::endl << std::flush;
        Update_eT_Files(m);
    }
}



/*double Cv(double T) //T -temperature in K
{
  // Heat capacity calculations (using data from NIST Chemistry WebBook http://webbook.nist.gov/chemistry/)
  double Cv;
  T /= 1000;
  // CO2:
  if(T<=1.2)

	Cv = (24.99735 + 55.18696*T - 33.69137*T*T + 7.948387*T*T*T - 0.136638/T/T - 8.31) * p_CO2*10000/22.4;
  else
	Cv = (58.16639 + 2.720074*T - 0.492289*T*T + 0.038844*T*T*T - 6.447293/T/T - 8.31) * p_CO2*10000/22.4;
  // N2:
  Cv += (26.09200 + 8.218801*T - 1.976141*T*T + 0.159274*T*T*T - 0.044434/T/T - 8.31) * p_N2*10000/22.4;
  // He:
  Cv += (20.78603 + 4.850636e-10*T - 1.582916e-10*T*T + 1.525102e-11*T*T*T - 3.196347e-11/T/T - 8.31) * p_He*10000/22.4;

  return Cv;
}*/


void A::InitializePopulations()
{
    double Q = 1 / ( (1-exp(-1920/T0))*pow(1-exp(-960/T0),2)*(1-exp(-3380/T0)) ); // partition function

    double N_grp;

    double w[NumIso][NumVib], w_sum;

    for(int is=0; is<NumIso; ++is)
    {
        for(int vl=0; vl<NumVib; ++vl)
        {
            w[is][vl] = exp(-h*G[is][vl]/(kB*T0));
        }
    }

    for(int x=0; x<x0; ++x)
    {
        T[x] = T0;
        e2[x] = 2/(exp(960/T0)-1);
        e3[x] = 1/(exp(3380/T0)-1);
        e4[x] = 1/(exp(3350/T0)-1);

        for(int is=0; is<12; ++is)
        {
            // VIBRATIONAL LEVELS
            // 001
            N_grp = N_iso[is]*exp(-3380/T0)/Q;
            N_vib[is][0][x]  = N_grp;
            // 100 + 020
            N_grp = 4 * N_iso[is]*exp(-2*960/T0)/Q;
            w_sum = w[is][1] + w[is][2] + w[is][3] + w[is][4];
            N_vib[is][1][x]  = N_grp * w[is][1] / w_sum;
            N_vib[is][2][x]  = N_grp * w[is][2] / w_sum;
            N_vib[is][3][x]  = N_grp * w[is][3] / w_sum;
            N_vib[is][4][x]  = N_grp * w[is][4] / w_sum;
            // 011
            N_grp = 2 * N_iso[is] * exp(-960/T0) * exp(-3380/T0) / Q;
            w_sum = w[is][5] + w[is][6];
            N_vib[is][5][x]  = N_grp * w[is][5] / w_sum;
            N_vib[is][6][x]  = N_grp * w[is][6] / w_sum;
            // 110 + 030
            N_grp = 6 * N_iso[is] * exp(-3*960/T0) / Q;
            w_sum = w[is][7] + w[is][8] + w[is][9] + w[is][10] + w[is][11] + w[is][12];
            N_vib[is][7][x]  = N_grp * w[is][7]  / w_sum;
            N_vib[is][8][x]  = N_grp * w[is][8]  / w_sum;
            N_vib[is][9][x]  = N_grp * w[is][9]  / w_sum;
            N_vib[is][10][x] = N_grp * w[is][10] / w_sum;
            N_vib[is][11][x] = N_grp * w[is][11] / w_sum;
            N_vib[is][12][x] = N_grp * w[is][12] / w_sum;
            // 002
            N_grp = N_iso[is]*exp(-2*3380/T0)/Q;
            N_vib[is][13][x] = N_grp;
            // 101 + 021
            N_grp = 4 * N_iso[is] * exp(-2*960/T0) * exp(-3380/T0) / Q;
            w_sum = w[is][14] + w[is][15] + w[is][16] + w[is][17];
            N_vib[is][14][x]  = N_grp * w[is][14] / w_sum;
            N_vib[is][15][x]  = N_grp * w[is][15] / w_sum;
            N_vib[is][16][x]  = N_grp * w[is][16] / w_sum;
            N_vib[is][17][x]  = N_grp * w[is][17] / w_sum;

            // ROTATIONAL LEVELS
            for(int vl=0; vl<NumVib; ++vl)
            {
                for(int j=0; j<NumRot; ++j)
                {
                    N_rot[is][vl][j][x] = f_rot[is][vl][j] * N_vib[is][vl][x];
                }
            }

        }
    }
}

double A::e2e(double T)
{
  return 2/(exp(960/T)-1);
}


double A::VibrationalTemperatures(int x, int mode){
    // See Nevdakh 2005 for details: dx.doi.org/10.1007/s10812-005-0034-4

    // zero-approximation (neglect common ground level)
    double T2 = 960/log(2/e2[x]+1);  // equilibrium vibrational temperature of nu1 and nu2 modes
    double T3 = 3380/log(1/e3[x]+1); // equilibrium vibrational temterature of nu3 mode
    /*double X1;
    double X2 = exp(-960.0/T2);
    double X3 = exp(-3380.0/T3);

    // iterations: solve Nevdakhs's equations
    for(int i=0; i<10; i++)
    {
        X1 = exp(-1920.0/T2); // no need to solve 1st equation (e1=...): X1 is known if X2 is known (T1=T2)
        X3 = 1.0 - e2[x]/(1.0-X1)*(1.0-X2)/(2.0*X2); // 2nd equation (e2=...)
        X2 = 1.0 - sqrt( e3[x]/(1.0-X1)*(1.0-X3)/X3 ); // 3rd equation (e3=...)
        T2 = -920.0/log(X2);
    }
    T3 = -3380.0/log(X3);*/

    switch(mode)
    {
        case 1:
            return T2;
        case 2:
            return T2;
        case 3:
            return T3;
    }

    return 0;
}


void A::Update_eT_Files(int m)
{
    double time = time_tick * (0.5+m);

    FILE *file;

    /////////////// e (average number of quanta in vibration modes) ////////////////
    if(m==0)
    {
        file = fopen((id+"_e.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] e1 e2 e3 e4\n");
    }
    else
        file = fopen((id+"_e.dat").c_str(), "a");
    //double Temp2 = 960/log(2/e2[0]+1);
    double T2 = VibrationalTemperatures(0, 2);
    double e1 = 1/(exp(1920/T2)-1);
    fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time, e1, e2[0], e3[0], e4[0]);
    fclose(file);

    ///////////////////////// Temperatures /////////////////////////
    if(m==0)
    {
        file = fopen((id+"_temperatures.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] T2[K] T3[K] T4[K] T[K]\n");
    }
    else
        file = fopen((id+"_temperatures.dat").c_str(), "a");
    fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time, VibrationalTemperatures(0,2),
            VibrationalTemperatures(0,3), 3350/log(1/e4[0]+1), T[0]);
    fclose(file);
}
