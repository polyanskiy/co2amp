#include  "co2amp.h"


void A::InternalDynamics(double time)
{
    if(p_CO2+p_N2+p_He <= 0)
        return;

    StatusDisplay(nullptr, nullptr, time, "molecular dynamics...");

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

    // pumping of vibrational modes
    double pump2 = 0;
    double pump3 = 0;
    double pump4 = 0;

    // pumping of groups of vibrational levels
    double pump_gr[NumGrp] = {}; // initialized with zeros

    // populations before interaction
    std::vector<double> N_vib0[NumIso][NumVib];
    for (int is = 0; is < NumIso; ++is)
        for (int vl = 0; vl < NumVib; ++vl)
            N_vib0[is][vl].resize(x0);

    if(pumping == "discharge")
    {
        // re-solve Boltzmann equation periodically, use linear interpolation otherwise
        // 'Slow' calculations of discharge energy deposition coefficients q
        // are done for time_a and time_b separated by solve_interval.
        // time_a is before the current calculation time and time_b is after
        // Linear interpolation is used to approximate energy deposition in any time between time_a and time_b
        // When time moves to after time_b, time_b is moved forward by solve_interval and time_a becomes time_b
        // q's are then calculated for the new time_b
        //double step = 25e-9;
        if(time >= time_b) // MUST be called at time==0 !!!
        {
            q2_a = q2_b;
            q3_a = q3_b;
            q4_a = q4_b;
            qT_a = qT_b;
            time_a = time_b;
            time_b += solve_interval;
            Boltzmann(time_b);
            q2_b = q2;
            q3_b = q3;
            q4_b = q4;
            qT_b = qT;
        }

        q2 = q2_a+(q2_b-q2_a)*(time-time_a)/(time_b-time_a);
        q3 = q3_a+(q3_b-q3_a)*(time-time_a)/(time_b-time_a);
        q4 = q4_a+(q4_b-q4_a)*(time-time_a)/(time_b-time_a);
        qT = qT_a+(qT_b-qT_a)*(time-time_a)/(time_b-time_a);

        W = Current(time)*Voltage(time) / Vd;     // W/m^3
        pump4 = y2!=0.0 ? 0.8e-6*q4/N/y2*W : 0;   // 1/s
        pump3 = y1!=0.0 ? 0.8e-6*q3/N/y1*W : 0;   // 1/s
        pump2 = y1!=0.0 ? 2.8e-6*q2/N/y1*W : 0;   // 1/s
    }

    if(pumping == "optical")
    {
        double photon_flux = PumpPulseIntensity(time) / (h*c/pump_wl); // photons/(m^2 * s)

        if(pump_level == "001") // direct excitation of (001) level @ ~4.3 um
        {
            pump3 = photon_flux * pump_sigma;
            pump2 = 0;
            pump_gr[0] = pump3;
        }

        if(pump_level == "021") // excitation via combinational vibration (101,021) @ ~2.8 um
        {
            pump3 = photon_flux * pump_sigma;
            pump2 = 2 * pump3;
            pump_gr[5] = pump3;
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
            pump_gr[4] = pump3;
        }

        if(pump_level == "003") // excitation via 3rd overtone level @ ~1.4 um
        {
            pump3 = 3 * photon_flux * pump_sigma;
            pump2 = 0;
            pump_gr[8] = pump3;
        }

    }

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
            for(int vl=0; vl<NumVib; ++vl)
                N_vib0[is][vl][x] = N_vib[is][vl][x];

        // In our model 3 groups can be pumped directly in case of optical pumping
        // In othr cases, pump energy first goes to corresponding vibrational modes
        // and then re-distibutes between levels through intramode thermalization
        for(int is=0; is<NumIso; ++is)
        {
            N_grp[is][0][x] += N_iso[is] * pump_gr[0] * time_tick; // 4.3 um - 001
            N_grp[is][4][x] += N_iso[is] * pump_gr[4] * time_tick; // 2.2 um - 002 (non-symmetric molecules only)
            N_grp[is][5][x] += N_iso[is] * pump_gr[5] * time_tick; // 2.8 um - 101+021
            N_grp[is][8][x] += N_iso[is] * pump_gr[8] * time_tick; // 1.4 um - 003
            // No group for 2.0 um (201+121+041): corresponding levels are not involved in laser transitions (?)
        }

        T[x] += ( y1/cv * (500*r3*f3 + 960*r2*(e2[x]-e2e(T[x]))) + 2.7e-3*W*qT/N/cv ) * time_tick;

        // Include non-zero thermalization time
        double Temp2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double Temp3 = VibrationalTemperatures(x, 3); // equilibrium vibrational temterature of nu3 mode
        double Q = 1 / ( (1-exp(-1920/Temp2))*pow(1-exp(-960/Temp2),2)*(1-exp(-3380/Temp3)) ); // partition function
        double N_grp0; // population of a group of vibrational levels in thermal equilibrium

        for(int is=0; is<NumIso; ++is)
        {
            // INTRA-MODE THERMALIZATION
            N_grp0 =     N_iso[is]*exp(-3380/Temp3)/Q;                    // 001
            N_grp[is][0][x] += (N_grp0 - N_grp[is][0][x]) * vib_relax;

            N_grp0 = 2 * N_iso[is]*exp(-2*960/Temp2)/Q;                   // 100 + 020
            N_grp[is][1][x] += (N_grp0 - N_grp[is][1][x]) * vib_relax;

            N_grp0 =     N_iso[is]*exp(-960/Temp2)*exp(-3380/Temp3)/Q;    // 011
            N_grp[is][2][x] += (N_grp0 - N_grp[is][2][x]) * vib_relax;

            N_grp0 = 2 * N_iso[is]*exp(-3*960/Temp2)/Q;                   // 110 + 030
            N_grp[is][3][x] += (N_grp0 - N_grp[is][3][x]) * vib_relax;

            N_grp0 =     N_iso[is]*exp(-2*3380/Temp3)/Q;                  // 002
            N_grp[is][4][x] += (N_grp0 - N_grp[is][4][x]) * vib_relax;

            N_grp0 = 2 * N_iso[is]*exp(-2*960/Temp2)*exp(-3380/Temp3)/Q;  // 101 + 021
            N_grp[is][5][x] += (N_grp0 - N_grp[is][5][x]) * vib_relax;

            N_grp0 =     N_iso[is]*exp(-960/Temp2)*exp(-2*3380/Temp3)/Q;  // 012
            N_grp[is][6][x] += (N_grp0 - N_grp[is][6][x]) * vib_relax;

            N_grp0 = 2 * N_iso[is]*exp(-3*960/Temp2)*exp(-3380/Temp3)/Q;  // 111 + 031
            N_grp[is][7][x] += (N_grp0 - N_grp[is][7][x]) * vib_relax;

            N_grp0 =     N_iso[is]*exp(-3*3380/Temp3)/Q;                  // 003
            N_grp[is][8][x] += (N_grp0 - N_grp[is][8][x]) * vib_relax;

            N_grp0 = 2 * N_iso[is]*exp(-2*960/Temp2)*exp(-2*3380/Temp3)/Q;// 102 + 022
            N_grp[is][9][x] += (N_grp0 - N_grp[is][9][x]) * vib_relax;

            // UPDATE VIBRATIONAL LEVELS
            // 001 (Group 0)
            N_vib[is][0][x]  = N_grp[is][0][x];          // upper reg
            // 100 + 020 (Group 1)
            N_vib[is][1][x]  = N_grp[is][1][x] / 3;      // lower reg 10 um; lower 4um
            N_vib[is][2][x]  = N_grp[is][1][x] / 3;      // lower reg 9 um;  lower 4um
            N_vib[is][3][x]  = N_grp[is][1][x] / 6;      //                  lower 4um
            N_vib[is][4][x]  = N_grp[is][1][x] / 6;      //                  lower 4um
            // 011 (Group 2)
            N_vib[is][5][x]  = N_grp[is][2][x] / 2;      // upper hot-e
            N_vib[is][6][x]  = N_grp[is][2][x] / 2;      // upper hot-f
            // 110 + 030 (Group 3)
            N_vib[is][7][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-e 10 um
            N_vib[is][8][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-e 9 um
            N_vib[is][9][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-f 10 um
            N_vib[is][10][x] = N_grp[is][3][x] * 3 / 16; // lower hot-f 9 um
            N_vib[is][11][x] = N_grp[is][3][x] / 8;
            N_vib[is][12][x] = N_grp[is][3][x] / 8;
            // 002 (Group 4)
            N_vib[is][13][x] = N_grp[is][4][x];          // upper seq
            // 101 + 021 (Group 5)
            N_vib[is][14][x] = N_grp[is][5][x] / 3;      // lower seq 10 um; upper 4um
            N_vib[is][15][x] = N_grp[is][5][x] / 3;      // lower seq 9 um;  upper 4um
            N_vib[is][16][x] = N_grp[is][5][x] / 6;      //                  upper 4um
            N_vib[is][17][x] = N_grp[is][5][x] / 6;      //                  upper 4um

        }
    }

    // ROTATIONAL LEVELS
    // we only care about energy distribution between rotational sub-level during the amplification process
    // (when a pulse is interacting witht the amplifier section)
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


    // Update pumping and molecular dynamics files if time is around an integer number of save intervals
    int save_number = llround(time / save_interval); // number of point to be saved around the current time
    if( time-time_tick/2 <= save_interval*save_number && save_interval*save_number < time+time_tick/2 )
    {
        UpdateDynamicsFiles(time);
    }
}


double A::Current(double time)
{
    return Interpolate(&discharge_time, &discharge_current, time);
}


double A::Voltage(double time)
{
    return Interpolate(&discharge_time, &discharge_voltage, time);
}


double A::PumpPulseIntensity(double time)
{
    //return Interpolate(&pump_pulse_time, &pump_pulse_intensity, time);
    int i = llround(time/time_tick-0.5);
    return normalized_intensity[i] * fluence[x];
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

    for(int x=0; x<x0; ++x)
    {
        T[x] = T0;
        e4[x] = 1/(exp(3350/T0)-1);
        e2[x] = 2/(exp(960/T0)-1);
        e3[x] = 1/(exp(3380/T0)-1);

        for(int is=0; is<12; ++is)
        {
            // GROUPS
            N_grp[is][0][x] =     N_iso[is]*exp(-3380/T0)/Q;                 // 001
            N_grp[is][1][x] = 2 * N_iso[is]*exp(-2*960/T0)/Q;                // 100 + 020
            N_grp[is][2][x] =     N_iso[is]*exp(-960/T0)*exp(-3380/T0)/Q;    // 011
            N_grp[is][3][x] = 2 * N_iso[is]*exp(-3*960/T0)/Q;                // 110 + 030
            N_grp[is][4][x] =     N_iso[is]*exp(-2*3380/T0)/Q;               // 002
            N_grp[is][5][x] = 2 * N_iso[is]*exp(-2*960/T0)*exp(-3380/T0)/Q;  // 101 + 021
            N_grp[is][6][x] =     N_iso[is]*exp(-4*960/T0)*exp(-3380/T0)/Q;  // 012
            N_grp[is][7][x] = 2 * N_iso[is]*exp(-3*960/T0)*exp(-3380/T0)/Q;  // 111 + 031
            N_grp[is][8][x] =     N_iso[is]*exp(-3*3380/T0)/Q;               // 003
            N_grp[is][9][x] = 2 * N_iso[is]*exp(-2*960/T0)*exp(-2*3380/T0)/Q;// 102 + 022

            // VIBRATIONAL LEVELS
            // 001 (Group 0)
            N_vib[is][0][x]  = N_grp[is][0][x];          // upper reg
            // 100 + 020 (Group 1)
            N_vib[is][1][x]  = N_grp[is][1][x] / 3;      // lower reg 10 um; lower 4um
            N_vib[is][2][x]  = N_grp[is][1][x] / 3;      // lower reg 9 um;  lower 4um
            N_vib[is][3][x]  = N_grp[is][1][x] / 6;      //                  lower 4um
            N_vib[is][4][x]  = N_grp[is][1][x] / 6;      //                  lower 4um
            // 011 (Group 2)
            N_vib[is][5][x]  = N_grp[is][2][x] / 2;      // upper hot-e
            N_vib[is][6][x]  = N_grp[is][2][x] / 2;      // upper hot-f
            // 110 + 030 (Group 3)
            N_vib[is][7][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-e 10 um
            N_vib[is][8][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-e 9 um
            N_vib[is][9][x]  = N_grp[is][3][x] * 3 / 16; // lower hot-f 10 um
            N_vib[is][10][x] = N_grp[is][3][x] * 3 / 16; // lower hot-f 9 um
            N_vib[is][11][x] = N_grp[is][3][x] / 8;
            N_vib[is][12][x] = N_grp[is][3][x] / 8;
            // 002 (Group 4)
            N_vib[is][13][x] = N_grp[is][4][x];          // upper seq
            // 101 + 021 (Group 5)
            N_vib[is][14][x] = N_grp[is][5][x] / 3;      // lower seq 10 um; upper 4um
            N_vib[is][15][x] = N_grp[is][5][x] / 3;      // lower seq 9 um;  upper 4um
            N_vib[is][16][x] = N_grp[is][5][x] / 6;      //                  upper 4um
            N_vib[is][17][x] = N_grp[is][5][x] / 6;      //                  upper 4um

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
    double Temp2 = 960/log(2/e2[x]+1);  // equilibrium vibrational temperature of nu1 and nu2 modes
    double Temp3 = 3380/log(1/e3[x]+1); // equilibrium vibrational temterature of nu3 mode
    /*double X1;
    double X2 = exp(-960.0/Temp2);
    double X3 = exp(-3380.0/Temp3);

    // iterations: solve Nevdakhs's equations
    for(int i=0; i<10; i++)
    {
        X1 = exp(-1920.0/Temp2); // no need to solve 1st equation (e1=...): X1 is known if X2 is known (T1=T2)
        X3 = 1.0 - e2[x]/(1.0-X1)*(1.0-X2)/(2.0*X2); // 2nd equation (e2=...)
        X2 = 1.0 - sqrt( e3[x]/(1.0-X1)*(1.0-X3)/X3 ); // 3rd equation (e3=...)
        Temp2 = -920.0/log(X2);
    }
    Temp3 = -3380.0/log(X3);*/

    switch(mode)
    {
        case 1:
            return Temp2;
        case 2:
            return Temp2;
        case 3:
            return Temp3;
    }

    return 0;
}


void A::UpdateDynamicsFiles(double time)
{
    FILE *file;

    if(pumping == "discharge"){
        //////////////////////// Discharge ////////////////////////
        if(time==0)
        {
            file = fopen((id+"_discharge.dat").c_str(), "w");
            fprintf(file, "#Data format: time[s] current[A] voltage[V]\n");
        }
        else
            file = fopen((id+"_discharge.dat").c_str(), "a");
        fprintf(file, "%e\t%e\t%e\n", time, Current(time), Voltage(time));
        fclose(file);

        //////////////////////////// q ////////////////////////////
        if(time==0)
        {
            file = fopen((id+"_q.dat").c_str(), "w");
            fprintf(file, "#Data format: time[s] q2 q3 q4\n");
        }
        else
            file = fopen((id+"_q.dat").c_str(), "a");
        fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time, q2, q3, q4, qT);
        fclose(file);
    }

    if(pumping == "optical"){
        ////////////////////// Pumping pulse ///////////////////////
        if(time==0)
        {
            file = fopen((id+"_pumping_pulse.dat").c_str(), "w");
            fprintf(file, "#Data format: time[s] intensity[W/m^2]\n");
        }
        else
            file = fopen((id+"_pumping_pulse.dat").c_str(), "a");
        fprintf(file, "%e\t%e\n", time, PumpPulseIntensity(time));
        fclose(file);
    }

    /////////////// e (average number of quanta in vibration modes) ////////////////
    if(time==0)
    {
        file = fopen((id+"_e.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] e1 e2 e3 e4\n");
    }
    else
        file = fopen((id+"_e.dat").c_str(), "a");
    //double Temp2 = 960/log(2/e2[0]+1);
    double Temp2 = VibrationalTemperatures(0, 2);
    double e1 = 1/(exp(1920/Temp2)-1);
    fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time, e1, e2[0], e3[0], e4[0]);
    fclose(file);

    ///////////////////////// Temperatures /////////////////////////
    if(time==0)
    {
        file = fopen((id+"_temperatures.dat").c_str(), "w");
        fprintf(file, "#Data format: time[s] T[K] T2[K] T3[K] T4[K]\n");
    }
    else
        file = fopen((id+"_temperatures.dat").c_str(), "a");
    fprintf(file, "%e\t%e\t%e\t%e\t%e\n", time, VibrationalTemperatures(0,2),
            VibrationalTemperatures(0,3), 3350/log(1/e4[0]+1), T[0]);
    fclose(file);
}
