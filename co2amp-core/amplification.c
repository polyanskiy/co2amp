#include  "co2amp.h"


void Amplification(int pulse, int k, double t, int am_section, double length)
{
    if(length == 0)
        return;

    int x;                                                           // coordinate step number
    int count = 0;
    char status[64];
    double Dt = t_pulse_lim/(n0-1);                                  // time step, s
    double N[6] = {2.7e25*p_626, 2.7e25*p_628, 2.7e25*p_828, 2.7e25*p_636, 2.7e25*p_638,2.7e25*p_838}; // CO2 number densities, 1/m^3
    double Nco2 = 2.7e25*p_CO2;
    double T2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.733*p_N2+0.64*p_He)); // transition dipole dephasing time, s
    double tauR = 1e-7 / (750*(p_CO2+1.2*p_N2+0.6*p_He));            // rotational termalisation time, s
    //double DELTA = 0; //temp

    // variables for gain spectrum calculation
    int n1;                           // frequency step number
    double Dv = (v_max-v_min)/(n0-1.0); // frequency time step, Hz
    double gamma = 1.0/T2;   // Lorentzian HWHM
    // initialize/clear gain spectrum array
    for(n1=0; n1<n0; n1++)
        gainSpectrum[n1] = 0;


    // ====================== AMPLIFICATIOIN ======================
    #pragma omp parallel for// multithreaded
    for(x=0; x<x0; x++){
        count++;
        sprintf(status, "amplification: %d of % d", count, x0);
        StatusDisplay(pulse, k, t, status);

        int n;  // time step number
        int i;  // isotopologue  number 0 - 626; 1 - 628; 2 - 828; 3 - 636; 4 - 638; 5 - 838
        int ba; // band                 0 - regular; 1 - hot-e; 2 - hot-f ; 3 - sequence
        int br; // branch               0 - 10P; 1 - 10R; 2 - 9P; 3 - 9R
        int vl; // vibrational level    0 - upper; 1 - lower of 10-um transition; 2 - lower of 9-um transition
        int j;  // rotational quantum number
        double Nvib[6][4][3], Nvib0[6][4][3];  // Population densities of vibrational lelvels (actual and equilibrium)
        double Nrot[6][4][3][61];              // Population densities of rotational lelvels (actual and equilibrium)
        double Dn[6][4][4][61];                // Population inversions (rotational transitions)
        double complex rho[6][4][4][61];       // polarizations
        double complex E_tmp1, E_tmp2, rho_tmp;// temporary variables for field and polarization
        double Temp2 = VibrationalTemperatures(am_section, x, 2); // equilibrium vibrational temperature of nu1 and nu2 modes
        double Temp3 = VibrationalTemperatures(am_section, x, 3); // equilibrium vibrational temterature of nu3 mode
        double delta;                          // change in population difference

        // Initial populations and polarizations
        for(i=0; i<6; i++){
            // Vibrational level population densities in thermal equilibrium - see Witteman p. 71 and Nevdakh 2007
            double Q = 1 / ( (1.0-exp(-1920.0/Temp2))*pow(1.0-exp(-960.0/Temp2),2.0)*(1.0-exp(-3380.0/Temp3)) ); // partition function
            // reg
            Nvib0[i][0][0] = N[i]*exp(-3380.0/Temp3)/Q;                      // upper
            Nvib0[i][0][1] = N[i]*exp(-1920.0/Temp2)/Q;                      // lower I (10 um)
            Nvib0[i][0][2] = N[i]*exp(-2.0*960.0/Temp2)/Q;                   // lower II (9 um)
            // hot-e
            Nvib0[i][1][0] = N[i]*exp(-3380.0/Temp3)*exp(-960.0/Temp2)/Q;    // upper
            Nvib0[i][1][1] = N[i]*exp(-(1920.0+960.0)/Temp2)/Q;              // lower I (10 um)
            Nvib0[i][1][2] = N[i]*exp(-3.0*960.0/Temp2)/Q;                   // lower II (9 um)
            // hot-f
            Nvib0[i][2][0] = N[i]*exp(-3380.0/Temp3)*exp(-960.0/Temp2)/Q;    // upper
            Nvib0[i][2][1] = N[i]*exp(-(1920.0+960.0)/Temp2)/Q;              // lower I (10 um)
            Nvib0[i][2][2] = N[i]*exp(-3.0*960.0/Temp2)/Q;                   // lower II (9 um)
            // seq
            Nvib0[i][3][0] = N[i]*exp(-2.0*3380.0/Temp3)/Q;                  // upper
            Nvib0[i][3][1] = N[i]*exp(-1920.0/Temp2)*exp(-3380.0/Temp3)/Q;   // lower I (10 um)
            Nvib0[i][3][2] = N[i]*exp(-2.0*960.0/Temp2)*exp(-3380.0/Temp3)/Q;// lower II (9 um)

            for(ba=0; ba<4; ba++){
                for(vl=0; vl<3; vl++){
                    Nvib[i][ba][vl] = Nvib0[i][ba][vl]; // initial population densities of vibrational levels
                    for(j=0; j<61; j++)
                        Nrot[i][ba][vl][j] = nop[i][ba][vl][j] * Nvib[i][ba][vl]; // initial population densities of rotational levels
                }
                for(br=0; br<4; br++){
                    for(j=0; j<61; j++){
                        rho[i][ba][br][j] = 0; // initial polarizations
                        Dn[i][ba][br][j] = 0;
                    }
                }
            }
        }

        // Amplification
        for(n=0; n<n0; n++){
            // population inversions
            for(i=0; i<6; i++){ // for each isotopologue
                if(N[i]==0) continue;
                for(j=0; j<61; j++){
                    // reg
                    Dn[i][0][0][j] = j>0  ? Nrot[i][0][0][j-1] - Nrot[i][0][1][j] : 0; // 10P
                    Dn[i][0][1][j] = j<60 ? Nrot[i][0][0][j+1] - Nrot[i][0][1][j] : 0; // 10R
                    Dn[i][0][2][j] = j>0  ? Nrot[i][0][0][j-1] - Nrot[i][0][2][j] : 0; // 9P
                    Dn[i][0][3][j] = j<60 ? Nrot[i][0][0][j+1] - Nrot[i][0][2][j] : 0; // 9R
                    // hot-e
                    Dn[i][1][0][j] = j>0  ? Nrot[i][1][0][j-1] - Nrot[i][1][1][j] : 0; // 10P
                    Dn[i][1][1][j] = j<60 ? Nrot[i][1][0][j+1] - Nrot[i][1][1][j] : 0; // 10R
                    Dn[i][1][2][j] = j>0  ? Nrot[i][1][0][j-1] - Nrot[i][1][2][j] : 0; // 9P
                    Dn[i][1][3][j] = j<60 ? Nrot[i][1][0][j+1] - Nrot[i][1][2][j] : 0; // 9R
                    // hot-f
                    Dn[i][2][0][j] = j>0  ? Nrot[i][2][0][j-1] - Nrot[i][2][1][j] : 0; // 10P
                    Dn[i][2][1][j] = j<60 ? Nrot[i][2][0][j+1] - Nrot[i][2][1][j] : 0; // 10R
                    Dn[i][2][2][j] = j>0  ? Nrot[i][2][0][j-1] - Nrot[i][2][2][j] : 0; // 9P
                    Dn[i][2][3][j] = j<60 ? Nrot[i][2][0][j+1] - Nrot[i][2][2][j] : 0; // 9R
                    //seq
                    Dn[i][3][0][j] = j>0  ? Nrot[i][3][0][j-1] - Nrot[i][3][1][j] : 0; // 10P
                    Dn[i][3][1][j] = j<60 ? Nrot[i][3][0][j+1] - Nrot[i][3][1][j] : 0; // 10R
                    Dn[i][3][2][j] = j>0  ? Nrot[i][3][0][j-1] - Nrot[i][3][2][j] : 0; // 9P
                    Dn[i][3][3][j] = j<60 ? Nrot[i][3][0][j+1] - Nrot[i][3][2][j] : 0; // 9R
                }
            }

            // gain spectrum
            if(x==0 && n==0){ // in the beam center(!) before amplification
                for(i=0; i<6; i++){
                    if(N[i]==0)
                        continue;
                    for(ba=0; ba<4; ba++){
                        if( (ba==0 && !(bands&1)) || (ba==1 && !(bands&2)) || (ba==2 && !(bands&2)) || (ba==3 && !(bands&4)) )
                            continue;
                        for(br=0; br<4; br++){
                            for(j=0; j<61; j++){
                                if(sigma[i][ba][br][j]==0 || v[i][ba][br][j]<v_min || v[i][ba][br][j]>v_max)
                                    continue;
                                for(n1=0; n1<n0; n1++)
                                    gainSpectrum[n1] += sigma[i][ba][br][j]*(M_PI*gamma) * Dn[i][ba][br][j] * gamma/M_PI/(pow(2.0*M_PI*(v_min+Dv*n1-v[i][ba][br][j]),2)+pow(gamma,2)); // Gain [m-1]
                            }
                        }
                    }
                }
            }

            // equation 2 (polarization) AND equation 1 (field)
            E_tmp1 = E[pulse][x][n]; // field before amplification
            for(i=0; i<6; i++){
                if(N[i]==0)
                    continue;
                for(ba=0; ba<4; ba++){
                    if( (ba==0 && !(bands&1)) || (ba==1 && !(bands&2)) || (ba==2 && !(bands&2)) || (ba==3 && !(bands&4)) )
                        continue;
                    for(br=0; br<4; br++){
                        for(j=0; j<61; j++){
                            if(sigma[i][ba][br][j]==0 || v[i][ba][br][j]<v_min || v[i][ba][br][j]>v_max)
                                continue;
                            // equation 2: terms defining polarization relaxation and phase
                            rho[i][ba][br][j] *= exp(-Dt/T2);                               // polarization relaxation
                            rho[i][ba][br][j] *= cexp(-I*2.0*M_PI*(vc-v[i][ba][br][j])*Dt); // polarization phase
                            // temporary variables
                            E_tmp2 = E[pulse][x][n];
                            rho_tmp = rho[i][ba][br][j];
                            // remaining of equation 2: 1st run (define polarization induced by input field)
                            rho_tmp -= sigma[i][ba][br][j]*Dn[i][ba][br][j]*E_tmp1/2.0 * (1-exp(-Dt/T2)) * cexp(-I*2.0*M_PI*(vc-v[i][ba][br][j])*Dt/2); // polarization excitation
                                // last exponent: phase in the middle of the time step (important for calculation stability!)
                            // equation 1: 1st run (0th approximation field in the middle of amplification section)
                            E_tmp2 -= rho[i][ba][br][j]*length/2.0;
                            // remaining of equation 2: 2nd run (define polarization induced by the field from previous step)
                            rho[i][ba][br][j] -= sigma[i][ba][br][j]*Dn[i][ba][br][j]*E_tmp2/2.0 * (1-exp(-Dt/T2)) * cexp(-I*2.0*M_PI*(vc-v[i][ba][br][j])*Dt/2); // polarization excitation
                            // equation 1: 2nd run (field after passing through entire amplification section)
                            E[pulse][x][n] -= rho[i][ba][br][j]*length; // field in the output
                        }
                    }
                }
            }

            // equation 3 (populations)
            for(i=0; i<6; i++){
                if(N[i]==0) continue;
                for(ba=0; ba<4; ba++){
                    if( (ba==0 && !(bands&1)) || (ba==1 && !(bands&2)) || (ba==2 && !(bands&2)) || (ba==3 && !(bands&4)) )
                        continue;
                    for(j=0; j<61; j++){
                        // ROTATIONAL REFILL
                        for(vl=0; vl<3; vl++)
                            Nrot[i][ba][vl][j] += (nop[i][ba][vl][j]*Nvib[i][ba][vl] - Nrot[i][ba][vl][j]) * (1-exp(-Dt/tauR));
                        // STIMULATED TRANSITIONS
                        for(br=0; br<4; br++){
                            if(sigma[i][ba][br][j] == 0)
                                continue;
                            delta = 2.0 * (rho[i][ba][br][j]*conj(E[pulse][x][n]) + conj(rho[i][ba][br][j]*conj(E[pulse][x][n]))) * Dt ;
                            /*if(x==0)
                                DELTA += delta;*/
                            // upper level
                            Nvib[i][ba][0] += delta;
                            if(br==0 || br==2) // P
                                Nrot[i][ba][0][j-1]+=delta;
                            else               // R
                                Nrot[i][ba][0][j+1]+=delta;
                            // lower level
                            if(br==0 || br==1){ // I (10um)
                                Nvib[i][ba][1]    -= delta;
                                Nrot[i][ba][1][j] -= delta;
                            }
                            else{               // II (9um)
                                Nvib[i][ba][2]    -= delta;
                                Nrot[i][ba][2][j] -= delta;
                            }
                        }
                    }
                }
            }
        }

        // vibrational temerature change
        double e1_tmp = 1.0/(exp(1920.0/Temp2)-1.0);
        double e2_tmp = 2.0/(exp(960.0/Temp2)-1.0);
        for(i=0; i<6; i++){
            if(N[i]==0) continue;
            for(ba=0; ba<4; ba++){
                double tmp = (Nvib0[i][ba][0] - Nvib[i][ba][0]) / Nco2; // multithread fails otherwise
                e3[am_section][x] -= tmp;
                e2[am_section][x] += tmp *  2.0*e2_tmp/(2*e1_tmp+e2_tmp); // 2.0*e2_tmp/(2*e1_tmp+e2_tmp): number of quanta added to nu2 per one laser transiton
            }
        }

    }

}
