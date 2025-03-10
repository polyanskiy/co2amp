#include  "co2amp.h"


void A::InternalDynamics(double time)
{
    if(p_CO2+p_N2+p_He <= 0)
        return;

    #pragma omp critical
    {
        StatusDisplay(nullptr, nullptr, time, "pumping and relaxation...");
    }

    double A, X, W;
    double K, K31, K32, K33, K21, K22, K23;
    double ra, rc, r2, r3;
    double f2, f3;
    double cv;
    double pump2, pump3, pump4; // vib. modes
    double pump_gr[6];          // groups of vib levels (partial alternative to "modes" model)
    double D_e2, D_e3, D_e4;

    double N = 273.0*(p_CO2+p_N2+p_He)/T0;

    // Thermalization time for semi-population model
    // k = 3.9e6 (s torr)^{-1}   [Finzi & Moore 1975 - https://doi.org/10.1063/1.431678]
    double tauV = 1 / (3.9e6*750*p_CO2); // intra-mode vibrational thermalization time
    // Pre-calculate re-usable expressios to accelerate computations
    double exp_tauV = exp(-time_tick/tauV); //

    // Relative concentrations: y1: CO2, y2: N2, y3: He
    double y1 = p_CO2/(p_CO2+p_N2+p_He);
    double y2 = p_N2/(p_CO2+p_N2+p_He);
    double y3 = p_He/(p_CO2+p_N2+p_He);


    W = 0;
    pump2 = 0;
    pump3 = 0;
    pump4 = 0;
    for(int gr=0; gr<6; ++gr)
    {
        pump_gr[gr] = 0;
    }

    if(pumping == "discharge")
    {
        // re-solve Boltzmann equation every 25 ns, use linear interpolation otherwise
        double step = 25e-9;
        if(time-time_tick/2<=time_b && time+time_tick/2>time_b)
        {
            q2_a = q2_b;
            q3_a = q3_b;
            q4_a = q4_b;
            qT_a = qT_b;
            time_a = time_b;
            time_b = time+step;
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

        // assume nu3 excitation goes to (001) and nu2 goes to (010)
        pump_gr[0] = pump3;
    }
    if(pumping == "optical")
    {
        double photon_flux = PumpingPulseIntensity(time) / (h*c/pump_wl); // photons/(m^2 * s)



        if(pump_wl>3.5e-6 && pump_wl<=5.0e-6) // direct excitation of (001) level
        {
            pump3 = photon_flux * pump_sigma;
            pump2 = 0;
            pump_gr[0] = pump3;
        }

        if(pump_wl>2.5e-6 && pump_wl<=3.5e-6) // excitation through combinational vibration (101,021)
        {
            pump3 = photon_flux * pump_sigma;
            pump2 = 2 * pump3;
            pump_gr[5] = pump3;
        }

        if(pump_wl>1.8e-6 && pump_wl<=2.5e-6) // excitation through combinational vibration (201,121,041)
        {
            pump3 = photon_flux * pump_sigma;
            pump2 = 4 * pump3;
        }

        if(pump_wl>1.3e-6 && pump_wl<=1.8e-6) // excitation through (003) level
        {
            // need to fix later: add group for 3nu3 level: pump_gr[X] = pump3/3
            // for now assuming each photons exctes 3 molecules to (001)
            pump3 = 3 * photon_flux * pump_sigma;
            pump2 = 0;
            pump_gr[0] = pump3;
        }

    }

    // time of travel from input plane to first interaction with this AM section
    double time_from_first_plane = 0;
    for(Plane* plane : planes)
    {
        if(plane->optic->id == id)
        {
            time_from_first_plane = plane->time_from_first_plane;
            break;
        }
    }

    // time of arraival of first pulse to this AM section
    double time_of_first_pulse_arrival = 1e12;
    for(Pulse* pulse : pulses)
        if(pulse->time_in + time_from_first_plane < time_of_first_pulse_arrival)
            time_of_first_pulse_arrival = pulse->time_in + time_from_first_plane;

    for(int x=0; x<x0; x++)
    {
        if( time > time_of_first_pulse_arrival || x==0 ) // population is same everythere if no pulse interaction yet occured
        {
            A = T[x]/273 * pow(1+0.5*e2e(T[x]),-3);
            X = pow(T[x],-1.0/3);

            // Collisional relaxation rates, 1/s
            K = 240.0/pow(T[x],0.5) * 1e6;
            K31 = A * exp(4.138+7.945 *X-631.24*X*X+2239.0 *X*X*X) * 1e6;
            K32 = A * exp(-1.863+213.3*X-2796.2*X*X+9001.9 *X*X*X) * 1e6;
            K33 = A * exp(-3.276+291.4*X-3831.8*X*X+12688.0*X*X*X) * 1e6;
            K21 = 1160.0 * exp(-59.3*X) * 1e6;
            K22 = 855.0  * exp(-69.0*X) * 1e6;
            K23 = 1300.0 * exp(-40.6*X) * 1e6;

            ra = K*N*y1;
            rc = K*N*y2;
            r2 = N*(y1*K21 + y2*K22 + y3*K23);
            r3 = N*(y1*K31 + y2*K32 + y3*K33) ;

            f2 = 2.0 * pow (1.0+e2[x],2) / (2.0+6.0*e2[x]+3.0*pow(e2[x],2));
            f3 = e3[x]*pow(1+e2[x]/2.0,3) - (1.0+e3[x])*pow(e2[x]/2.0,3)*exp(-500.0/T[x]);

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

            // Assuming that only two groups are pumped directly (others - via thermalization)
            for(int i=0; i<12; ++i)
            {
                N_gr[i][0][x] += N_iso[i] * pump_gr[0] * time_tick;
                N_gr[i][5][x] += N_iso[i] * pump_gr[5] * time_tick;
            }

            cv = 2.5*(y1+y2) + 1.5*y3;
            T[x] += ( y1/cv * (500.0*r3*f3 + 960.0*r2*(e2[x]-e2e(T[x]))) + 2.7e-3*W*qT/N/cv ) * time_tick;
        }
        else
        {
            e2[x] = e2[0];
            e3[x] = e3[0];
            e4[x] = e4[0];
            T[x] = T[0];
            for(int i=0; i<12; ++i)
            {
                N_gr[i][0][x] = N_gr[i][0][0];
                N_gr[i][5][x] = N_gr[i][5][0];
            }
        }

        // Include non-zero thermalization time
        double Temp2 = VibrationalTemperatures(x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double Temp3 = VibrationalTemperatures(x, 3); // equilibrium vibrational temterature of nu3 mode
        double Q = 1 / ( (1-exp(-1920/Temp2))*pow(1-exp(-960/Temp2),2)*(1-exp(-3380/Temp3)) ); // partition function
        double N_gr0; // population of a group of vibrational levels in thermal equilibrium

        for(int i=0; i<12; ++i)
        {
            N_gr0 =     N_iso[i]*exp(-3380/Temp3)/Q;                  // 001
            N_gr[i][0][x] += (N_gr0 - N_gr[i][0][x]) * (1-exp_tauV);

            N_gr0 = 2 * N_iso[i]*exp(-2*960/Temp2)/Q;                 // 100 + 020
            N_gr[i][1][x] += (N_gr0 - N_gr[i][1][x]) * (1-exp_tauV);

            N_gr0 =     N_iso[i]*exp(-960/Temp2)*exp(-3380/Temp3)/Q;  // 011
            N_gr[i][2][x] += (N_gr0 - N_gr[i][2][x]) * (1-exp_tauV);

            N_gr0 = 2 * N_iso[i]*exp(-3*960/Temp2)/Q;                 // 110 + 030
            N_gr[i][3][x] += (N_gr0 - N_gr[i][3][x]) * (1-exp_tauV);

            N_gr0 =     N_iso[i]*exp(-2*3380/Temp3)/Q;                // 002
            N_gr[i][4][x] += (N_gr0 - N_gr[i][4][x]) * (1-exp_tauV);

            N_gr0 = 2 * N_iso[i]*exp(-2*960/Temp2)*exp(-3380/Temp3)/Q;// 101 + 021
            N_gr[i][5][x] += (N_gr0 - N_gr[i][5][x]) * (1-exp_tauV);
        }
    }

    UpdateDynamicsFiles(time);
}


double A::Current(double time)
{
    return Interpolate(&discharge_time, &discharge_current, time);
}


double A::Voltage(double time)
{
    return Interpolate(&discharge_time, &discharge_voltage, time);
}


double A::PumpingPulseIntensity(double time)
{
    return Interpolate(&pumping_pulse_time, &pumping_pulse_intensity, time);
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

    for(int x=0; x<=x0-1; x++)
    {
        T[x] = T0;
        e4[x] = 1.0/(exp(3350.0/T0)-1.0);
        e2[x] = 2.0/(exp(960.0/T0)-1.0);
        e3[x] = 1.0/(exp(3380.0/T0)-1.0);

        for(int i=0; i<12; ++i)
        {
            N_gr[i][0][x] =     N_iso[i]*exp(-3380/T0)/Q;               // 001
            N_gr[i][1][x] = 2 * N_iso[i]*exp(-2*960/T0)/Q;              // 100 + 020
            N_gr[i][2][x] =     N_iso[i]*exp(-960/T0)*exp(-3380/T0)/Q;  // 011
            N_gr[i][3][x] = 2 * N_iso[i]*exp(-3*960/T0)/Q;              // 110 + 030
            N_gr[i][4][x] =     N_iso[i]*exp(-2*3380/T0)/Q;             // 002
            N_gr[i][5][x] = 2 * N_iso[i]*exp(-2*960/T0)*exp(-3380/T0)/Q;// 101 + 021
        }
    }
}

double A::e2e(double T)
{
  return 2.0/(exp(960.0/T)-1.0);
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
        //////////////////////// Discharge ////////////////////////
        if(time==0)
        {
            file = fopen((id+"_pumping_pulse.dat").c_str(), "w");
            fprintf(file, "#Data format: time[s] intensity[W/m^2]\n");
        }
        else
            file = fopen((id+"_pumping_pulse.dat").c_str(), "a");
        fprintf(file, "%e\t%e\n", time, PumpingPulseIntensity(time));
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
