#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    yaml = id + ".yml";

    Debug(2, "Creating pulse from file \'" + yaml + "\'");

    std::string value="";

    if(!YamlGetValue(&value, yaml, "E0")){
        configuration_error = true;
        return;
    }
    E0 = std::stod(value);
    Debug(2, "E0 = " + toExpString(E0) + " J");

    if(!YamlGetValue(&value, yaml, "w0")){
        configuration_error = true;
        return;
    }
    w0 = std::stod(value);
    Debug(2, "w0 = " + toExpString(w0) + " m");

    if(!YamlGetValue(&value, yaml, "tau0")){
        configuration_error = true;
        return;
    }
    tau0 = std::stod(value);
    Debug(2, "tau0 = " + toExpString(tau0) + " s");

    if(!YamlGetValue(&value, yaml, "nu0")){
        configuration_error = true;
        return;
    }
    nu0 = std::stod(value);
    Debug(2, "nu0 = " + toExpString(nu0) + " Hz");

    time_inj = 0;
    if(!YamlGetValue(&value, yaml, "time_inj"))
        std::cout << "Using default injection time (0 s)\n";
    else
        time_inj = std::stod(value);
    Debug(2, "time_inj = " + toExpString(time_inj) + " s");

    // allocate memory
    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];
}


void Pulse::InitializeE()
{
    Debug(2, "Initializing field array for pulse \'" + this->id + "\'");

    int x, n;
    double Energy, af;
    //FILE *file;

    double Dr = layout[0]->optic->Dr; // use first optic in the layout
    double Dt = (t_max-t_min)/(n0-1);

    // Create 2D array
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] = field(Dr*x, t_min + Dt*n);

    // frequency shift between central frequency of the pulse (nu0) and central frequency of the calculation grig (vc)
    for(x=0; x<x0; x++)
        for(n=0; n<n0; n++)
            E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*(Dt*n));

    // Normalize intensity
    Energy = 0;
    for(n=0; n<n0-1; n++)
        for(x=0; x<x0-1; x++)
            Energy += 2.0 * h * nu0
                    * (pow(abs(E[x][n]),2) + pow(abs(E[x][n+1]),2)
                    + pow(abs(E[x+1][n]),2) + pow(abs(E[x+1][n+1]),2))/4
                    * 2*M_PI*(Dr*x+Dr/2)*Dr * Dt; // J

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


void Pulse::Propagate(Plane *from, Plane *to, double time)
{
    double z   = from->space;
    double Dr1 = from->optic->Dr;
    double Dr2 = to->optic->Dr;

    if(z==0 && Dr1==Dr2)  //nothing to be done
        return;

    // Create temporary field array
    std::complex<double> **E1;
    E1 = new std::complex<double>*[x0];
    for(int x=0; x<x0; x++)
        E1[x] = new std::complex<double>[n0];
    Debug(2, "propagation: temporary field array created");

    // Copy field to temporary array, zero main field array
    for(int x=0; x<x0; x++){
        for(int n=0; n<n0; n++){
            E1[x][n] = E[x][n];
            E[x][n] = 0;
        }
    }
    Debug(2, "propagation: temporary field array populated");

    if(z==0){ //only change calculation grid step
        #pragma omp parallel for
        for(int x=0; x<x0; x++){
            double x_exact = Dr2 / Dr1 * (double)x;
            int x_lo = (int)floor(x_exact);
            int x_hi = (int)ceil(x_exact);
            if( (x_lo < x0) && (x_hi < x0) ){
                if(x_lo == x_hi){
                    for(int n=0; n<n0; n++)
                        E[x][n] = E1[x_lo][n];
                }
                else{
                    for(int n=0; n<n0; n++)
                        E[x][n] = E1[x_lo][n]*((double)x_hi-x_exact) + E1[x_hi][n]*(x_exact-(double)x_lo);
                }
            }
        }
    }

    else{ //Huygens-Fresnell diffraction
        double lambda = c/vc; // wavelength, m
        double k_wave = 2.0*M_PI/lambda; // wave-number
        int count=0;

        #pragma omp parallel for
        for(int x=0; x<x0; x++){ // output plane radial coordinate
            #pragma omp critical
            {
                StatusDisplay(this, from, time,
                          "propagation: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
            double rho, R_min, R_max, R, delta_R;
            std::complex<double> tmp;
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
                for(int n=0; n<n0; n++)
                    E[x][n] += (E1[(int)(rho-0.5)][n]+E1[(int)(rho+0.5)][n]) / 2.0 * tmp;
            }
        }
    }
    Debug(2, "Propagation: integration done");

    // delete temporary array
    for(int x=0; x<x0; x++)
        delete[] E1[x];
    delete[] E1;
    Debug(2, "Propagation: temporary field array deleted");
}
