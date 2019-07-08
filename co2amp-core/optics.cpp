#include  "co2amp.h"


void BeamPropagation(int pulse, int k, double t)
{
    double z = layout_distance[k];
    double Dr1 = components[layout_component[k]].Dr;
    double Dr2 = components[layout_component[k+1]].Dr;

    if(z==0.0 && Dr1-Dr2==0.0)  //nothing to be done
        return;

    int x, n;
    std::complex<double> **E1;

    char str[256];

    // Allocate memory
    //E1 = malloc(sizeof(std::complex<double>*)*x0);
    E1 = new std::complex<double>*[x0];
    for(x=0; x<x0; x++)
        //E1[x] = malloc(sizeof(std::complex<double>)*n0);
        E1[x] = new std::complex<double>[n0];
    strcpy(str, "propagation: memory allocated");
    Debug(2, str);

    //#pragma omp parallel for shared(E, E1) private(x, n) // multithread
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++){
            E1[x][n] = E[pulse][x][n];
            E[pulse][x][n] = 0;
        }
    }
    //Debug(2, "propagation: temporary field array created");
    strcpy(str, "propagation: temporary field array created");
    Debug(2, str);

    if(z==0.0){ //only change calculation mesh step
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
        std::complex<double> tmp;

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
                //tmp /= 2*I*lambda*R;
                //tmp *= 2*cexp(-I*k_wave*R) * j0(k_wave*delta_R/2); // integral of the exponential part over the ring
                tmp /= I*lambda*R;
                tmp *= exp(-I*k_wave*R) * j0(k_wave*delta_R/2); // integral of the exponential part over the ring
                for(n=0; n<n0; n++)
                    E[pulse][x][n] += (E1[(int)(rho-0.5)][n]+E1[(int)(rho+0.5)][n]) / 2.0 * tmp;
            }
        }

    }
    Debug(2, "propagation: integration done");

    // Free memory
    for(x=0; x<x0; x++)
        //free(E1[x]);
        delete E1[x];
    //free(E1);
    delete E1;
    //Debug(2, "propagation: memory freed");
    strcpy(str, "propagation: memory freed");
    Debug(2, str);
}


void Probe()
{
    // do nothing
}


void Lens(int pulse, double Dr, double F)
{
    if(F==0.0)
        return;
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++)
            E[pulse][x][n] *= exp(I*2.0*M_PI*(vc/c)*pow(Dr*x,2)/(2.0*F));
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


void Attenuator(int pulse, double transmission)
{
    int x, n;
    for(x=0; x<x0; x++){
        for(n=0; n<n0; n++)
            E[pulse][x][n] *=  sqrt(transmission);
    }
}


void Window(int pulse, int k, double t, char *material, double thickness)
{
    int i, x, n;
    char status[64];
    int count=0;
    double Dv = (v_max-v_min) / (n0-1);
    //double Dt = t_pulse_lim / (n0-1);
    double intensity, delay, tilt_factor;
    std::complex<double> *spectrum;

    // account for tilt of Brewster windows
    tilt_factor = 1; // intensity reduction due to tilt
    if(!strcmp(material,"CdTe-Brewster") || !strcmp(material,"GaAs-Brewster") || !strcmp(material,"Ge-Brewster")
            || !strcmp(material,"KCl-Brewster") || !strcmp(material,"NaCl-Brewster") || !strcmp(material,"Si-Brewster")
            || !strcmp(material,"ZnSe-Brewster")){
        double theta1 = atan(RefractiveIndex(material, vc)); // Incidence angle (Brewster's)
        double theta2 = M_PI/2 - theta1; // Refraction angle
        thickness /= cos(theta2); // longer propagation in a tilt window
        tilt_factor = 1/tan(theta1);
    }

    int nslices=4; //slice window for accounting for mutual interaction of linear and nonlinear dispersion
    thickness /= nslices;

    #pragma omp parallel for shared(E) private(i, x, n, intensity, delay, spectrum) // mulithread
    for(x=0; x<x0; x++){
        count ++;
        sprintf(status, "material: %d of % d", count, x0);
        StatusDisplay(pulse, k, t, status);
        for(i=0; i<nslices; i++){ // slices

            // nonlinear index (n2) and nonlinear absorption Step 1 (half-thickness of the slice)
            for(n=0; n<n0; n++){
                intensity = 2.0 * h * vc * pow(abs(E[pulse][x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows
                delay = thickness/2.0/c * NonlinearIndex(material)*intensity; // phase delay (== group delay)
                E[pulse][x][n] *= exp(-I*2.0*M_PI*vc*delay); //effect of nonlinear index
            }

            // linear dispersion (full thickness of the slice)
            //spectrum = malloc(sizeof(std::complex<double>)*n0);
            spectrum = new std::complex<double>[n0];
            FFT(E[pulse][x], spectrum);
            for(n=0; n<n0; n++){
                // linear dispersion
                delay = thickness/c * (RefractiveIndex(material, v_min+Dv*n) - RefractiveIndex(material, vc)); // phase delay (!= group delay)
                spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*n)*(-delay)); // no "-" in the exponent in frequency domain E(omega)
                // eliminate time-frame shift introduced by the difference between phase and group velocity
                delay = -thickness/c * (c/vc) * (RefractiveIndex(material,vc+1e7)-RefractiveIndex(material,vc-1e7))/(c/(vc+1e7)-c/(vc-1e7)); // relative group delay
                spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*n)*delay);
            }
            IFFT(spectrum, E[pulse][x]);
            //free(spectrum);
            delete spectrum;

            // nonlinear index (n2) and nonlinear absorption Step 2 (half-thickness of the slice)
            for(n=0; n<n0; n++){
                intensity = 2.0 * h * vc * pow(abs(E[pulse][x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows
                delay = thickness/2.0/c * NonlinearIndex(material)*intensity; // phase delay (== group delay)
                E[pulse][x][n] *= exp(-I*2.0*M_PI*vc*delay); //effect of nonlinear index
            }

        }


        // temporary - nonlinear absorption in Ge
        /*if(!strcmp(material,"Ge") || !strcmp(material,"Ge-Brewster")){
            for(n=0; n<n0; n++){
                intensity = 2.0 * h * vc * pow(cabs(E[pulse][x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows
                alpha[x] += 4e-26*pow(intensity,3)*Dt;// BEST!!!
                E[pulse][x][n] *= sqrt( exp(-1.0*alpha[x]*thickness) );
            }
        }*/

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

    #pragma omp parallel for // mulithread
    for(x=0; x<x0; x++){
        int n;
        std::complex<double> *spectrum;
        double delay;
        //spectrum = malloc(sizeof(std::complex<double>)*n0);
        spectrum = new std::complex<double>[n0];
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++){
            delay = (v_min+Dv*n-vc) * stretching; // delay increases with frequency (red chirp) - group delay
            delay *= 0.5; // conversion to phase delay.
            spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*n-vc)*(-delay)); // no "-" in the exponent in frequency domain E(omega)
        }
        IFFT(spectrum, E[pulse][x]);
        //free(spectrum);
        delete spectrum;
    }
}


void Bandpass(int pulse, double bandcenter, double bandwidth) // bandcenter, bandwidth: Hz
{
    int x;
    double Dv = (v_max-v_min) / (n0-1);

    #pragma omp parallel for // multthread
    for(x=0; x<x0; x++){
        int n;
        std::complex<double> *spectrum;
        //spectrum = malloc(sizeof(std::complex<double>)*n0);
        spectrum = new std::complex<double>[n0];
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++){
            if(v_min+Dv*n < bandcenter-bandwidth/2 || v_min+Dv*n > bandcenter+bandwidth/2)
                spectrum[n] = 0;
        }
        IFFT(spectrum, E[pulse][x]);
        //free(spectrum);
        delete spectrum;
    }
}


void Filter(int pulse, std::string yaml_file_path)
{
    int x, n, n_raw_data_points, i;
    double Dv = (v_max-v_min) / (n0-1);
    double v;
    //char debug_out[65535], yaml_str[65535], yaml_str_copy[65535]; // // max array size in C (not a good practice to do it this way - change in future)
    std::string debug_out, yaml_str, yaml_str_copy;
    char* tmp_str;

    // read transmission profile data string from YAML file
    Debug(2, "a");
    YamlGetValue(&yaml_str, yaml_file_path, "transmittance");
    Debug(2, "b");
    debug_out = "Transmission data:\n" + yaml_str;
    Debug(2, debug_out);

    // count number of data points (= number of lines in the data string)
    n_raw_data_points = 0;
    yaml_str_copy = yaml_str; //strtok() modifies input string, so use a copy
    //tmp_str = strtok(yaml_str_copy,"\n");
    while(tmp_str != nullptr){
        n_raw_data_points ++;
        tmp_str = strtok(nullptr,"\n");
    }
    debug_out = "number of data points: =" + std::to_string(n_raw_data_points);
    Debug(2, debug_out);

    // create "raw" transmittance array
    //double **raw_data;
    double *raw_data[2];
    //raw_data = malloc(sizeof(double*)*2);
    //raw_data[0] = malloc(sizeof(double)*n_raw_data_points);
    //raw_data[1] = malloc(sizeof(double)*n_raw_data_points);
    raw_data[0] = new double[n_raw_data_points];
    raw_data[1] = new double[n_raw_data_points];

    yaml_str_copy = yaml_str; //strtok() modifies input string, so use a copy
    //tmp_str = strtok(yaml_str_copy," \t\n\r");
    //printf("%s ", tmp_str);
    i=0;
    //while(tmp_str != NULL){
    for(i=0; i<n_raw_data_points; i++){
        raw_data[0][i] = atof(tmp_str)*1e12; // Frequency, THz -> Hz
        tmp_str = strtok(nullptr," \t\n\r");
        //printf("%s ", tmp_str);
        raw_data[1][i] = atof(tmp_str);      // Transmittance
        tmp_str = strtok(nullptr," \t\n\r");
        //printf("%s ", tmp_str);
        //i++;
    }

    if(debug_level>=2){
        std::cout << "\nTRANSMITTANCE PROFILE:\n";
        std::cout << "Freq, THz\tTransmittance\n";
        for(i=0; i<n_raw_data_points; i++)
            std::cout << raw_data[0][i]*1e-12 << "\t" << raw_data[1][i] << "std::endl";
    }


    // create "full" transmittance array
    double *transmittance;
    //transmittance = malloc(sizeof(double)*n0);
    transmittance = new double[n0];

    for(n=0; n<n0; n++){
        v = v_min+Dv*n;
        if(v <= raw_data[0][0])
            transmittance[n] = raw_data[1][0];
        else{
            if(v >= raw_data[0][n_raw_data_points-1])
                transmittance[n] = raw_data[1][n_raw_data_points-1];
            else{
                for(i=0; i<n_raw_data_points-1; i++)
                    if(v>=raw_data[0][i] && v<=raw_data[0][i+1])
                        transmittance[n] = raw_data[1][i] + (raw_data[1][i+1]-raw_data[1][i]) * (v-raw_data[0][i])/(raw_data[0][i+1]-raw_data[0][i]);
            }
        }
    }


    //free(raw_data[0]);
    //free(raw_data[1]);
    //free(raw_data);
    delete raw_data[0];
    delete raw_data[1];

    if(debug_level>=2){
        printf("\nTRANSMITTANCE PROFILE:\n");
        printf("Freq, THz\tTransmittance\n");
        for(n=0; n<n0; n++)
            printf("%f\t%f\n", (v_min+Dv*n)*1e-12, transmittance[n]);
        fflush(stdout);
    }

    #pragma omp parallel for // multthread
    for(x=0; x<x0; x++){
        int n;
        std::complex<double> *spectrum;
        //spectrum = malloc(sizeof(std::complex<double>)*n0);
        spectrum = new std::complex<double>[n0];
        FFT(E[pulse][x], spectrum);
        for(n=0; n<n0; n++)
            spectrum[n] *= sqrt(transmittance[n]);
        IFFT(spectrum, E[pulse][x]);
        delete spectrum;
    }

    //free(transmittance);
    delete transmittance;
}


void Apodizer(int pulse, double alpha)
{
    if(alpha==0.0)
        return;
    int x, n;
    double transmission;
    for(x=(int)ceil((x0-1.0)*(1.0-alpha)); x<x0; x++){
        //transmission = 0.5 * (1.0 + cos(M_PI*( (x/(x0-1.0)-1.0)/alpha + 1.0)));
        transmission = pow(sin(M_PI*(x0-1.0-x)/(2.0*alpha*(x0-1.0))),2); // here (x0-1) is R from manual
        for(n=0; n<n0; n++){
            E[pulse][x][n] *=  sqrt(transmission);
        }
    }
}


void Air(int pulse, int k, double t, double H, double length)
{
    humidity = H;
    Window(pulse, k, t, "air", length);
}


double RefractiveIndex(char* material, double nu)
{
    // wavelength
    double x = c / nu * 1e6; // s^-1 -> um

    if(!strcmp(material,"KCl") || !strcmp(material,"KCl-Brewster")){ //Li-1976
        x= x<0.18 ? 0.18 : x;
        x= x>35 ? 35 : x;
        return sqrt( 1.26486 + 0.30523*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.41620*pow(x,2)/(pow(x,2)-pow(0.131,2)) + 0.18870*pow(x,2)/(pow(x,2)-pow(0.162,2)) + 2.6200*pow(x,2)/(pow(x,2)-pow(70.42,2)) );
    }
    if(!strcmp(material,"NaCl") || !strcmp(material,"NaCl-Brewster")) //Li-1976
    {
        x= x<0.2 ? 0.2 : x;
        x= x>30 ? 30 : x;
        return sqrt( 1.00055 + 0.19800*pow(x,2)/(pow(x,2)-pow(0.050,2)) + 0.48398*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.38696*pow(x,2)/(pow(x,2)-pow(0.128,2)) + 0.25998*pow(x,2)/(pow(x,2)-pow(0.158,2)) + 0.08796*pow(x,2)/(pow(x,2)-pow(40.50,2)) + 3.17064*pow(x,2)/(pow(x,2)-pow(60.98,2)) + 0.30038*pow(x,2)/(pow(x,2)-pow(120.34,2)) );
    }
    if(!strcmp(material,"ZnSe") || !strcmp(material,"ZnSe-Brewster")){ //Tatian-1984
        x= x<0.54 ? 0.54 : x;
        x= x>18.2 ? 18.2 : x;
        return sqrt(1.0+4.45813734/(1-pow(0.200859853/x,2))+0.467216334/(1-pow(0.391371166/x,2))+2.89566290/(1-pow(47.1362108/x,2)));
    }
    if(!strcmp(material,"Ge") || !strcmp(material,"Ge-Brewster")){ //Barnes-1979
        x= x<2.5 ? 2.5 : x;
        x= x>12 ? 12 : x;
        return sqrt( 9.28156 + 6.72880*pow(x,2)/(pow(x,2)-0.44105) + 0.21307*pow(x,2)/(pow(x,2)-3870.1) );
    }
    if(!strcmp(material,"GaAs") || !strcmp(material,"GaAs-Brewster")){ //Skauli-2003
        x= x<0.97 ? 0.97 : x;
        x= x>17 ? 17 : x;
        return sqrt(1+4.372514+5.466742/(1-pow(0.4431307/x,2))+0.02429960/(1-pow(0.8746453/x,2))+1.957522/(1-pow(36.9166/x,2)));
    }
    if(!strcmp(material,"CdTe") || !strcmp(material,"CdTe-Brewster")){ //DeBell-1979
        x= x<6 ? 6 : x;
        x= x>22 ? 22 :x;
        return sqrt( 1.0 + 6.1977889*pow(x,2)/(pow(x,2)-pow(0.317069,2)) + 3.2243821*pow(x,2)/(pow(x,2)-pow(72.0663,2)) );
    }
    if(!strcmp(material,"Si") || !strcmp(material,"Si-Brewster")){ //Edwards-1980
        x= x<2.4373 ? 2.4373 : x;
        x= x>25 ? 25 :x;
        return 3.41983+0.159906/(pow(x,2)-0.028)-0.123109/pow((pow(x,2)-0.028),2)+1.26878E-6*pow(x,2)-1.95104E-9*pow(x,4);
    }
    if(!strcmp(material,"air")){ //Mathar-2007
        x= x<7.5 ? 7.5 : x;
        x= x>14.1 ? 14.1 :x;

        int j;
        double sigma = 1.0e4/x; // um->cm^-1

        double T = 273.15+22;   // Temperature: K
        double p = 101325;      // Pressure: Pa
        double H = humidity;    // Humidity: 0-100% (humidity = global variable set by air() function
        H= H<0 ? 0 : H;
        H= x>100 ? 100 : H;

        // model parameters
        double cref[6] = { 0.199885e-3,  0.344739e-9,  -0.273714e-12,  0.393383e-15, -0.569488e-17,  0.164556e-19}; // cm^j
        double cT[6]   = { 0.593900e-1, -0.172226e-5,   0.237654e-8,  -0.381812e-11,  0.305050e-14, -0.157464e-16}; // cm^j · K
        double cTT[6]  = {-6.50355,      0.103830e-1,  -0.139464e-4,   0.220077e-7,  -0.272412e-10,  0.126364e-12}; // cm^j · K^2
        double cH[6]   = {-0.221938e-7,  0.347377e-10, -0.465991e-13,  0.735848e-16, -0.897119e-19,  0.380817e-21}; // cm^j · %^-1
        double cHH[6]  = { 0.393524e-12, 0.464083e-15, -0.621764e-18,  0.981126e-21, -0.121384e-23,  0.515111e-26}; // cm^j · %^-2
        double cp[6]   = { 0.266809e-8,  0.695247e-15,  0.159070e-17, -0.303451e-20, -0.661489e-22,  0.178226e-24}; // cm^j · Pa^-1
        double cpp[6]  = { 0.610508e-17, 0.227694e-22,  0.786323e-25, -0.174448e-27, -0.359791e-29,  0.978307e-32}; // cm^j · Pa^-2
        double cTH[6]  = { 0.106776e-3, -0.168516e-6,   0.226201e-9,  -0.356457e-12,  0.437980e-15, -0.194545e-17}; // cm^j · K · %^-1
        double cTp[6]  = { 0.77368e-6,   0.216404e-12,  0.581805e-15, -0.189618e-17, -0.198869e-19,  0.589381e-22}; // cm^j · K · Pa^-1
        double cHp[6]  = {-0.206365e-15, 0.300234e-19, -0.426519e-22,  0.684306e-25, -0.467320e-29,  0.126117e-30}; // cm^j · %^-1 · Pa^-1

        double sigmaref = 1e4/10.1;  // cm^-1
        double Tref = 273.15+17.5;   // K
        double pref = 75000;         // Pa
        double Href = 10;            // %

        double n = 1;
        for(j=0; j<6; j++){
            n += ( cref[j] + cT[j]*(1.0/T-1.0/Tref) + cTT[j]*pow(1.0/T-1.0/Tref,2.0)
                   + cH[j]*(H-Href) + cHH[j]*pow(H-Href,2.0)
                   + cp[j]*(p-pref) + cpp[j]*pow(p-pref,2.0)
                   + cTH[j]*(1.0/T-1.0/Tref)*(H-Href)
                   + cTp[j]*(1.0/T-1.0/Tref)*(p-pref)
                   + cHp[j]*(H-Href)*(p-pref) ) * pow(sigma-sigmaref,(double)j);
        }
        return n;

    }
    return 1.0;
}


double NonlinearIndex(char* material)
{
    if(!strcmp(material,"KCl") || !strcmp(material,"KCl-Brewster")) //Sheik-Bahae-1991
        return 2e-13 * 4.19e-7 / 1.49; // esu -> m^2/W (5.62e-20) @ 1.06 um
    if(!strcmp(material,"NaCl") || !strcmp(material,"NaCl-Brewster")) //Sheik-Bahae-1991
        return 1.6e-13 * 4.19e-7 / 1.53; // esu -> m^2/W (4.38e-20) @ 1.06 um
    if(!strcmp(material,"ZnSe") || !strcmp(material,"ZnSe-Brewster")) //Sheik-Bahae-1991
        return 170e-13 * 4.19e-7 / 2.48; // esu -> m^2/W (2.87e-18) @ 1.06 um
    if(!strcmp(material,"Ge") || !strcmp(material, "Ge-Brewster")) //Sheik-Bahae-1991
        return 2700e-13 * 4.19e-7 / 4.00; // esu -> m^2/W (2.83e-17) @ 10.6 um
    if(!strcmp(material,"GaAs") || !strcmp(material,"GaAs-Brewster")) //Sheik-Bahae-1991
        return -2700e-13 * 4.19e-7 / 3.47; // esu -> m^2/W (-3.26e-17) @ 1.06 um
        //return 1.7e-17; // m^2/W - Kapetanakos et. al. IEEE J. Quant. Electron. 37(5) (2001)
    if(!strcmp(material,"CdTe") || !strcmp(material,"CdTe-Brewster")) //Sheik-Bahae-1991
        return -2000e-13 * 4.19e-7 / 2.84; // esu -> m^2/W (-2.95e-17) @ 1.06 um
    if(!strcmp(material,"Si") || !strcmp(material,"Si-Brewster")) //Bristow-2007
        return 1e-17; // m^2/W @ 2.2 um
    if(!strcmp(material,"air")) //Geints-2014
        return 3e-23; // m^2/W
    return 0;
}

