#include  "co2amp.h"


void A::PumpingAndRelaxation(double t)
{
/*    int x, k;
    double A, X, W;
    double K, K31, K32, K33, K21, K22, K23;
    double ra, rc, r2, r3;
    double f2, f3;
    double cv;
    double pump2, pump3, pump4;

    double N = 273.0*(p_CO2+p_N2+p_He)/T0;
    // Relative concentrations: y1: CO2, y2: N2, y3: He
    double y1 = p_CO2/(p_CO2+p_N2+p_He);
    double y2 = p_N2/(p_CO2+p_N2+p_He);
    double y3 = p_He/(p_CO2+p_N2+p_He);

    // re-solve Boltzmann equation from time to time; use linear interpolation otherwise
    if(pumping == "discharge"){ // Discharge pumping
        double step = 25.0e-9;
        if(t-Dt_pump/2<=t_b && t+Dt_pump/2>t_b){
            q2_a = q2_b;
            q3_a = q3_b;
            q4_a = q4_b;
            qT_a = qT_b;
            t_a = t_b;
            t_b = t+step;
            Boltzmann(t_b);
            q2_b = q2;
            q3_b = q3;
            q4_b = q4;
            qT_b = qT;
        }

        q2 = q2_a+(q2_b-q2_a)*(t-t_a)/(t_b-t_a);
        q3 = q3_a+(q3_b-q3_a)*(t-t_a)/(t_b-t_a);
        q4 = q4_a+(q4_b-q4_a)*(t-t_a)/(t_b-t_a);
        qT = qT_a+(qT_b-qT_a)*(t-t_a)/(t_b-t_a);

        W = Current(t)*Voltage(t) / Vd; // W/m^3
        pump4 = y2!=0.0 ? 0.8e-6*q4/N/y2*W : 0;   // 1/s
        pump3 = y1!=0.0 ? 0.8e-6*q3/N/y1*W : 0;   // 1/s
        pump2 = y1!=0.0 ? 2.8e-6*q2/N/y1*W : 0;   // 1/s
    }
    else{ // Optical pumping
        W = 0;
        pump4 = 0;
        pump3 = 0;
        pump2 = 0;
    }

    for(k=0; k<n_AM; k++){
        for(x=0; x<x0; x++){
            if( t>t_inj || (k==0 && x==0) ){ // calculate only once if t<t_inj
                A = T[k][x]/273.0 * pow(1.0+0.5*e2e(T[k][x]),-3);
                X = pow(T[k][x],-1.0/3);

                // Collisional relaxation rates, 1/s
                K = 240.0/pow(T[k][x],0.5) * 1e6;
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

                f2 = 2.0 * pow (1.0+e2[k][x],2) / (2.0+6.0*e2[k][x]+3.0*pow(e2[k][x],2));
                f3 = e3[k][x]*pow(1+e2[k][x]/2.0,3) - (1.0+e3[k][x])*pow(e2[k][x]/2.0,3)*exp(-500.0/T[k][x]);

                //collisional relaxation, pumping
                if(y2!=0.0)
                    e4[k][x] += ( pump4 - ra*(e4[k][x]-e3[k][x]) ) * Dt_pump;
                if(y1!=0.0)
                    e3[k][x] += ( pump3 + rc*(e4[k][x]-e3[k][x]) - r3*f3 ) * Dt_pump;
                if(y1!=0.0)
                    e2[k][x] += f2 *( pump2 + 3*r3*f3 - r2*(e2[k][x]-e2e(T[k][x])) ) * Dt_pump;

                cv = 2.5*(y1+y2) + 1.5*y3;
                T[k][x] += ( y1/cv * (500.0*r3*f3 + 960.0*r2*(e2[k][x]-e2e(T[k][x]))) + 2.7e-3*W*qT/N/cv ) * Dt_pump;
            }
            else{
                e4[k][x] = e4[0][0];
                e3[k][x] = e3[0][0];
                e2[k][x] = e2[0][0];
                T[k][x] = T[0][0];
            }
        }
    }
    UpdateDynamicsFiles(t);
*/
}


double A::Current(double t) //t - time in s
{
    int i;
    for(i=0; i<n_discharge_points-1; i++)
        if(discharge[0][i]<=t && discharge[0][i+1]>=t)
            return discharge[1][i] + (discharge[1][i+1]-discharge[1][i]) * (t-discharge[0][i])/(discharge[0][i+1]-discharge[0][i]);
    return 0;
}


double A::Voltage(double t) //t - time in s
{
    int i;
    for(i=0; i<n_discharge_points-1; i++)
        if(discharge[0][i]<=t && discharge[0][i+1]>=t)
            return discharge[2][i] + (discharge[2][i+1]-discharge[2][i]) * (t-discharge[0][i])/(discharge[0][i+1]-discharge[0][i]);
    return discharge[2][n_discharge_points-1]; // calculation of q will fail if U=0.
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
  int k, x;
  for(k=0; k<=n_AM-1; k++)
	for(x=0; x<=x0-1; x++){
          T[k][x] = T0;
          e4[k][x] = 1.0/(exp(3350.0/T0)-1.0);
          e2[k][x] = 2.0/(exp(960.0/T0)-1.0);
          e3[k][x] = 1.0/(exp(3380.0/T0)-1.0);
          if(pumping == "optical"){ // optical pumping
              int i;
              double Temp2, e1, fluence;
              fluence = pump_fluence / (h*c/pump_wl); // photons/m^2
              // number of quanta added to upper state:
              //double delta_e3 = 0.5*(1-exp(-fluence*pump_sigma)); // max 0.5 quanta per molecule (classic 2-level system)
              double delta_e3 = 1.0*(1-exp(-fluence*pump_sigma)); // max 1 quanta per molecule (arbitrary: attempt to allow multi-photon excitation)
              //double delta_e3 = fluence*pump_sigma; //no bleeching (no limit for number of quanta per molecule: user responsible for keeping pumping realistic)
              e3[k][x] += delta_e3; // fraction of molecules in upper state
              if(pump_wl>2.2e-6 && pump_wl<3.2e-6){ // excitation through combinational vibration (101,021)
                  double delta_e2 = 2.0*delta_e3; // "0" approximation: all lower level energy goes to nu2 mode (in reality e3 = e2/2 + e1)
                  for(i=0; i<10; i++){ // iterations: e2->Temp2->e1->e2->...
                      Temp2 = 960.0/log(2.0/(e2[k][x]+delta_e2)+1.0); // Temp2 after excitation
                      e1 = 1.0/(exp(1920.0/Temp2)-1); // number of quanta in nu1 at this temperature
                      delta_e2 = 2.0*delta_e3 * (e2[k][x]+delta_e2)/(2.0*e1+e2[k][x]+delta_e2); //corrected delta_e2
                  }
                  e2[k][x] += delta_e2;
              }
          }

	}
}

double A::e2e(double T)
{
  return 2.0/(exp(960.0/T)-1.0);
}


double A::VibrationalTemperatures(int am_section, int x, int mode){
    // See Nevdakh 2005 for details: dx.doi.org/10.1007/s10812-005-0034-4

    //int i;
    //double X1;

    // zero-approximation (neglect common ground level)
    double Temp2 = 960.0/log(2.0/e2[am_section][x]+1.0);  // equilibrium vibrational temperature of nu1 and nu2 modes
    double Temp3 = 3380.0/log(1.0/e3[am_section][x]+1.0); // equilibrium vibrational temterature of nu3 mode
    /*double X2 = exp(-960.0/Temp2);
    double X3 = exp(-3380.0/Temp3);

    // iterations: solve Nevdakhs's equations
    for(i=0; i<10; i++){
        X1 = exp(-1920.0/Temp2); // no need to solve 1st equation (e1=...): X1 is known if X2 is known (T1=T2)
        X3 = 1.0 - e2[am_section][x]/(1.0-X1)*(1.0-X2)/(2.0*X2); // 2nd equation (e2=...)
        X2 = 1.0 - sqrt( e3[am_section][x]/(1.0-X1)*(1.0-X3)/X3 ); // 3rd equation (e3=...)
        Temp2 = -920.0/log(X2);
    }
    Temp3 = -3380.0/log(X3);*/

    switch(mode){
        case 1:
            return Temp2;
        case 2:
            return Temp2;
        case 3:
            return Temp3;
    }

    return 0;
}
