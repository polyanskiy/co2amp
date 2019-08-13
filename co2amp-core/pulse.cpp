#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    this->yaml = id + ".yml";

    this->E0 = -1;
    this->w0 = -1;
    this->tau0 = -1;
    this->nu0 = -1;
    this->t0 = -1;

    Debug(2, "Creating pulse from file \'" + this->yaml + "\' ...");

    std::string value="";    
    if(YamlGetValue(&value, yaml, "E0"))
        E0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "w0"))
        w0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "tau0"))
        tau0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "nu0"))
        nu0 = std::stod(value);
    if(YamlGetValue(&value, yaml, "t0"))
        t0 = std::stod(value);

    Debug(2, "E0 = " + toExpString(E0) + " J");
    Debug(2, "w0 = " + toExpString(w0) + " m");
    Debug(2, "tau0 = " + toExpString(tau0) + " s");
    Debug(2, "nu0 = " + toExpString(nu0) + " Hz");
    Debug(2, "t0 = " + toExpString(t0) + " s");

    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];
}


void Pulse::InitializeE()
{
    Debug(2, "Initializing field array for pulse \'" + this->id + "\' ...");

    int x, n;
    double Energy, af;
    //FILE *file;

    double Dr = layout[0]->optic->Dr; // use first optic in the layout
    double Dt = (t_max-t_min)/(n0-1);

    // Create 2D array
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] = field(Dr*x, t_min + Dt*n);

    // frequency shift between central frequency of the pulse (v0) and central frequency of the calculation grig (vc)
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*(Dt*n));

    // Normalize intensity
    Energy = 0;
    for(n=0; n<n0-1; n++)
        for(x=0; x<x0-1; x++)
            Energy += 2.0 * h * nu0 *
                    (pow(abs(E[x][n]),2) + pow(abs(E[x][n+1]),2) +
                    pow(abs(E[x+1][n]),2) + pow(abs(E[x+1][n+1]),2))/4 *
                    2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J

    af = sqrt(E0/Energy);
    for(n=0; n<n0; n++)
        for(x=0; x<x0; x++)
            E[x][n] *= af;


    /*else{
        file = fopen("field_in.bin", "rb");
        for(pulse=0; pulse<n_pulses; pulse++){
            for(x=0; x<x0; x++)
                fread(E[pulse][x], sizeof(std::complex<double>)*n0, 1, file);
        }
        fclose(file);
    }*/
}


std::complex<double> Pulse::field(double r, double t)
{
    double xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
    std::complex<double> pulse = exp(-pow(t/xx, 2));
    std::complex<double> beam = exp(-pow(r/w0, 2));
    return pulse*beam;
}


void Pulse::Propagate(int from, int to, double clock_time)
{
    double z = layout[from]->space;
    double Dr1 = layout[from]->optic->Dr;
    double Dr2 = layout[to]->optic->Dr;

    if(z==0 && Dr1==Dr2)  //nothing to be done
        return;

    int x, n;

    // Create temporary field array
    std::complex<double> **E1;
    E1 = new std::complex<double>*[x0];
    for(x=0; x<x0; x++)
        E1[x] = new std::complex<double>[n0];
    Debug(2, "propagation: temporary field array created");

    // Copy field to temporary array, zero main field array
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++){
            E1[x][n] = E[x][n];
            E[x][n] = 0;
        }
    }
    Debug(2, "propagation: temporary field array populated");

    if(z==0){ //only change calculation grid step
        double x_exact;
        int x_lo, x_hi;
        #pragma omp parallel for shared(E, E1) private(x, n, x_lo, x_hi, x_exact) // multithread
        for(x=0; x<x0; x++){
            x_exact = Dr2 / Dr1 * (double)x;
            x_lo = (int)floor(x_exact);
            x_hi = (int)ceil(x_exact);
            if( (x_lo < x0) && (x_hi < x0) ){
                if(x_lo == x_hi){
                    for(n=0; n<n0; n++)
                        E[x][n] = E1[x_lo][n];
                }
                else{
                    for(n=0; n<n0; n++)
                        E[x][n] = E1[x_lo][n]*((double)x_hi-x_exact) + E1[x_hi][n]*(x_exact-(double)x_lo);
                }
            }
        }
    }

    else{ //Huygens-Fresnell diffraction
        double lambda = c/vc; // wavelength, m
        double k_wave = 2.0*M_PI/lambda; // wave-number
        int count=0;
        double rho, R_min, R_max, R, delta_R;
        std::complex<double> tmp;

        #pragma omp parallel for shared(E, E1, count) private(x, n, rho, R_min, R_max, R, delta_R, tmp) // multithread
        for(x=0; x<x0; x++){ // output plane radial coordinate
            #pragma omp critical
            {
                StatusDisplay(this->pulse_n, from, clock_time,
                          "propagation: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
            for(rho=0.5; rho<x0-0.5; rho++){ // x0-1 rings in the input plane
                R_min = sqrt(pow(rho*Dr1-x*Dr2,2)+pow(z,2)); // minimum distance from the ring to the current poin in the output plane (x)
                R_max = sqrt(pow(rho*Dr1+x*Dr2,2)+pow(z,2)); // maximum --''--
                R = (R_min+R_max)/2; // average --''--
                delta_R = (R_max-R_min);
                // Huygens-Fresnell integral (summation over concentric rings in the input plane)
                tmp = M_PI*(pow((rho+0.5)*Dr1,2)-pow((rho-0.5)*Dr1,2)); // ring area ( = dS )
                //tmp /= 2*I*lambda*R;
                //tmp *= 2*cexp(-I*k_wave*R) * j0(k_wave*delta_R/2); // integral of the exponential part over the ring
                tmp /= I*lambda*R;
                tmp *= exp(-I*k_wave*R) * j0(k_wave*delta_R/2); // integral of the exponential part over the ring
                for(n=0; n<n0; n++)
                    E[x][n] += (E1[(int)(rho-0.5)][n]+E1[(int)(rho+0.5)][n]) / 2.0 * tmp;
            }
        }
    }
    Debug(2, "propagation: integration done");

    // delete temporary array
    for(x=0; x<x0; x++)
        delete E1[x];
    delete E1;
    Debug(2, "propagation: temporary field array deleted");
}
