#include  "co2amp.h"


void BeamPropagation(int pulse, int k, double t)
{
    double z = layout_distance[k];
    double Dr1 = component_Dr[layout_component[k]];
    double Dr2 = component_Dr[layout_component[k+1]];

    if(z==0 && Dr1==Dr2)  //nothing to be done
        return;

    int x, n;
    double complex **E1;

    // Allocate memory
    E1 = malloc(sizeof(double complex*)*x0);
    for(x=0; x<x0; x++)
        E1[x] = malloc(sizeof(double complex)*n0);
    Debug(2, "propagation: memory allocated");

    //#pragma omp parallel for shared(E, E1) private(x, n) // multithread
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++){
            E1[x][n] = E[pulse][x][n];
            E[pulse][x][n] = 0;
        }
    }
    Debug(2, "propagation: temporary field array created");

    if(z==0){ //only change calculation mesh step
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
                        E[pulse][x][n] = E1[x_lo][n];
                }
                else{
                    for(n=0; n<n0; n++)
                        E[pulse][x][n] = E1[x_lo][n]*((double)x_hi-x_exact) + E1[x_hi][n]*(x_exact-(double)x_lo);
                }
            }
        }
    }

    else{ //Huygens-Fresnell diffraction
        double lambda = c/vc; // wavelength, m
        double k_wave = 2.0*M_PI/lambda; // wave-number
        char status[64];
        int count=0;
        double rho, R_min, R_max, R, delta_R;
        double complex tmp;

        #pragma omp parallel for shared(E, E1) private(x, n, rho, R_min, R_max, R, delta_R, tmp) // multithread
        for(x=0; x<x0; x++){ // output plane radial coordinate
            count ++;
            sprintf(status, "propagation: %d of % d", count, x0);
            StatusDisplay(pulse, k, t, status);
            for(rho=0.5; rho<x0-0.5; rho++){ // x0-1 rings in the input plane
                R_min = sqrt(pow(rho*Dr1-x*Dr2,2)+pow(z,2)); // minimum distance from the ring to the current poin in the output plane (x)
                R_max = sqrt(pow(rho*Dr1+x*Dr2,2)+pow(z,2)); // maximum --''--
                R = (R_min+R_max)/2; // average --''--
                delta_R = (R_max-R_min);
                // Huygens-Fresnell integral (summation over concentric rings in the input plane)
                tmp = M_PI*(pow((rho+0.5)*Dr1,2)-pow((rho-0.5)*Dr1,2)); // ring area ( = dS )
                tmp /= 2*I*lambda*R;
                tmp *= 2*cexp(-I*k_wave*R) * j0(k_wave*delta_R/2); // integral of the exponential part over the ring
                for(n=0; n<n0; n++)
                    E[pulse][x][n] += (E1[(int)(rho-0.5)][n]+E1[(int)(rho+0.5)][n]) / 2 * tmp;
            }
        }

    }
    Debug(2, "propagation: integration done");

    // Free memory
    for(x=0; x<x0; x++)
        free(E1[x]);
    free(E1);
    Debug(2, "propagation: memory freed");
}


void Probe()
{
    // do nothing
}

void Lens(int pulse, double Dr, double F)
{
    if(F==0)
        return;
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++)
            E[pulse][x][n] *= cexp(I*2.0*M_PI*(vc/c)*pow(Dr*x,2)/(2.0*F));
    }
}


void Mask(int pulse, double Dr, double radius)
{
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++){
            if(Dr*x < radius)
                E[pulse][x][n] = 0;
        }
    }
}


void Absorber(int pulse, double transmission)
{
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++)
            E[pulse][x][n] *=  sqrt(transmission);
    }
}


void Window(int pulse, char *material, double thickness)
{
    int x, n;
    double Dv = (v_max-v_min) / (n0-1);
    double intensity, delay;
    double complex *spectrum;

    #pragma omp parallel for shared(E) private(x, n, intensity, delay, spectrum) // mulithread
    for(x=0; x<x0; x++){

        // linear dispersion
        spectrum = malloc(sizeof(double complex)*n0);
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++){
            // linear dispersion
            delay = thickness / c * (RefractiveIndex(material, v_min+Dv*n) - RefractiveIndex(material, vc)); // phase delay (!= group delay)
            spectrum[n] *= cexp(I*2.0*M_PI*(v_min+Dv*n)*(-delay)); // no "-" in the exponent in frequency domain E(omega)
            // eliminate time-frame shift introduced by the difference between phase and group velocity
            delay = -thickness/c * (c/vc) * (RefractiveIndex(material,vc+1000)-RefractiveIndex(material,vc-1000))/(c/(vc+1000)-c/(vc-1000)); // relative group delay
            spectrum[n] *= cexp(I*2.0*M_PI*(v_min+Dv*n)*delay);
        }
        IFFT(spectrum, E[pulse][x]);
        free(spectrum);

        // nonlinear index (n2)
        for(n=0; n<n0; n++){
            intensity = 2.0 * h * vc * pow(cabs(E[pulse][x][n]), 2); // W/m2
            delay = thickness / c * NonlinearIndex(material)*intensity; // phase delay (== group delay)
            E[pulse][x][n] *= cexp(-I*2.0*M_PI*vc*delay);
        }


        // tests
        /*spectrum = malloc(sizeof(double complex)*n0);
        FFT(E[pulse][x], spectrum);
        for(n=n0/2; n<n0; n++)
            spectrum[n]=0;
        IFFT(spectrum, E[pulse][x]);
        free(spectrum);*/

        //for(n=0; n<n0/2; n++)
        //    E[pulse][x][n]=1e16;

        //for(n=n0/2; n<n0; n++)
        //    E[pulse][x][n]=0;
    }
}


void Stretcher(int pulse, double stretching) // stretching: s/Hz
{
    int x;
    double Dv = (v_max-v_min) / (n0-1);

    #pragma omp parallel for // multthread
    for(x=0; x<x0; x++){
        int n;
        double complex *spectrum;
        double delay;
        spectrum = malloc(sizeof(double complex)*n0);
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++){
            delay = (v_min+Dv*n-vc) * stretching; // delay increases with frequency (red chirp) - group delay
            delay *= 0.5; // conversion to phase delay.
            spectrum[n] *= cexp(I*2.0*M_PI*(v_min+Dv*n-vc)*(-delay)); // no "-" in the exponent in frequency domain E(omega)
        }
        IFFT(spectrum, E[pulse][x]);
        free(spectrum);
    }
}


void Bandpass(int pulse, double bandcenter, double bandwidth) // bandcenter, bandwidth: Hz
{
    int x;
    double Dv = (v_max-v_min) / (n0-1);

    #pragma omp parallel for // multthread
    for(x=0; x<x0; x++){
        int n;
        double complex *spectrum;
        spectrum = malloc(sizeof(double complex)*n0);
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++){
            if(v_min+Dv*n < bandcenter-bandwidth/2 || v_min+Dv*n > bandcenter+bandwidth/2)
                spectrum[n] = 0;
        }
        IFFT(spectrum, E[pulse][x]);
        free(spectrum);
    }
}


double RefractiveIndex(char* material, double nu)
{
    int i;
    for (i = 0; material[i] != '\0'; i++)
       material[i] = (char)tolower(material[i]); // material name to lowcase

    // wavelength
    double x = c / nu * 1e6; // s^-1 -> um

    if(!strcmp(material, "kcl")){ //Li-1976
        x= x<0.18 ? 0.18 : x;
        x= x>35 ? 35 : x;
        return sqrt( 1.26486 + 0.30523*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.41620*pow(x,2)/(pow(x,2)-pow(0.131,2)) + 0.18870*pow(x,2)/(pow(x,2)-pow(0.162,2)) + 2.6200*pow(x,2)/(pow(x,2)-pow(70.42,2)) );
    }
    if(!strcmp(material, "nacl")) //Li-1976
    {
	    x= x<0.2 ? 0.2 : x;
        x= x>30 ? 30 : x;
        return sqrt( 1.00055 + 0.19800*pow(x,2)/(pow(x,2)-pow(0.050,2)) + 0.48398*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.38696*pow(x,2)/(pow(x,2)-pow(0.128,2)) + 0.25998*pow(x,2)/(pow(x,2)-pow(0.158,2)) + 0.08796*pow(x,2)/(pow(x,2)-pow(40.50,2)) + 3.17064*pow(x,2)/(pow(x,2)-pow(60.98,2)) + 0.30038*pow(x,2)/(pow(x,2)-pow(120.34,2)) );
    }
    if(!strcmp(material, "znse")){ //Tatian-1984
        x= x<0.54 ? 0.54 : x;
        x= x>18.2 ? 18.2 : x;
        //return sqrt( 1.0 + 4.2980149*pow(x,2)/(pow(x,2)-pow(0.1920630,2)) + 0.62776557*pow(x,2)/(pow(x,2)-pow(0.37878260,2)) + 2.8955633*pow(x,2)/(pow(x,2)-pow(46.994595,2)) );
        return sqrt(1.0+4.45813734/(1-pow(0.200859853/x,2))+0.467216334/(1-pow(0.391371166/x,2))+2.89566290/(1-pow(47.1362108/x,2)));
    }
    if(!strcmp(material, "ge")){ //Barnes-1979
        x= x<2.5 ? 2.5 : x;
        x= x>12 ? 12 : x;
        return sqrt( 9.28156 + 6.72880*pow(x,2)/(pow(x,2)-0.44105) + 0.21307*pow(x,2)/(pow(x,2)-3870.1) );
    }
    if(!strcmp(material, "gaas")){ //Skauli-2003
        x= x<0.97 ? 0.97 : x;
        x= x>17 ? 17 : x;
        //return sqrt( 3.5 + 7.4969*pow(x,2)/(pow(x,2)-pow(0.4082,2)) + 1.9347*pow(x,2)/(pow(x,2)-pow(37.17,2)) );
        return sqrt(1+4.372514+5.466742/(1-pow(0.4431307/x,2))+0.02429960/(1-pow(0.8746453/x,2))+1.957522/(1-pow(36.9166/x,2)));
    }
    if(!strcmp(material, "cdte")){ //DeBell-1979
        x= x<6 ? 6 : x;
        x= x>22 ? 22 :x;
        return sqrt( 1.0 + 6.1977889*pow(x,2)/(pow(x,2)-pow(0.317069,2)) + 3.2243821*pow(x,2)/(pow(x,2)-pow(72.0663,2)) );
    }
    if(!strcmp(material, "si")){ //Edwards-1980
        x= x<2.4373 ? 2.4373 : x;
        x= x>25 ? 25 :x;
        return 3.41983+0.159906/(pow(x,2)-0.028)-0.123109/pow((pow(x,2)-0.028),2)+1.26878E-6*pow(x,2)-1.95104E-9*pow(x,4);
    }
    return 1.0;
}



double NonlinearIndex(char* material)
{
    int i;
    for (i = 0; material[i] != '\0'; i++)
       material[i] = (char)tolower(material[i]); // material name to lowcase

    if(!strcmp(material, "kcl")) //Sheik-Bahae-1991
        return 2e-13 * 4.19e-7 / 1.49; // esu -> m^2/W (5.62e-20) @ 1.06 um
    if(!strcmp(material, "nacl")) //Sheik-Bahae-1991
        return 1.6e-13 * 4.19e-7 / 1.53; // esu -> m^2/W (4.38e-20) @ 1.06 um
    if(!strcmp(material, "znse")) //Sheik-Bahae-1991
        return 170e-13 * 4.19e-7 / 2.48; // esu -> m^2/W (2.87e-18) @ 1.06 um
    if(!strcmp(material, "ge")) //Sheik-Bahae-1991
        return 2700e-13 * 4.19e-7 / 4.00; // esu -> m^2/W (2.83e-17) @ 10.6 um
    if(!strcmp(material, "gaas")) //Sheik-Bahae-1991
        return -2700e-13 * 4.19e-7 / 3.47; // esu -> m^2/W (-3.26e-17) @ 1.06 um
        //return 1.7e-17; // m^2/W - Kapetanakos et. al. IEEE J. Quant. Electron. 37(5) (2001)
    if(!strcmp(material, "cdte")) //Sheik-Bahae-1991
        return -2000e-13 * 4.19e-7 / 2.84; // esu -> m^2/W (-2.95e-17) @ 1.06 um
    if(!strcmp(material, "si")) //Bristow-2007
        return 1e-17; // m^2/W @ 2.2 um

    return 0;
}
