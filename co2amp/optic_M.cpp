#include "co2amp.h"


M::M(std::string id)
{
    this->id = id;
    type = "M";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max (R)
    if(!YamlGetValue(&value, yaml, "R")){
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "R = " + toExpString(r_max) + " m");

    // material
    if(!YamlGetValue(&value, yaml, "material")){
        configuration_error = true;
        return;
    }
    material = value;
    Debug(2, "material = " + material);

    // thickness
    if(!YamlGetValue(&value, yaml, "thickness")){
        configuration_error = true;
        return;
    }
    thickness = std::stod(value);
    Debug(2, "thickness = " + toExpString(thickness) + " m");

    // humidity (only relevant for air)
    humidity = 50;
    if(material == "air"){
        if(!YamlGetValue(&value, yaml, "humidity"))
            std::cout << "Using default humidity (50%)\n";
        else
            humidity = std::stod(value);
        Debug(2, "humidity = " + std::to_string(humidity) + " %");
    }

    // tilt (not relevant for air)
    tilt = 0;
    if(material != "air"){
        if(!YamlGetValue(&value, yaml, "tilt"))
            std::cout << "Using default tilt (no tilt)\n";
        else
            tilt = std::stod(value);
        Debug(2, "tilt = " + std::to_string(tilt) + " degrees");
        tilt *= M_PI/180; // degrees to radians
    }

    // number of slices
    slices = 1;
    if(!YamlGetValue(&value, yaml, "slices"))
        std::cout << "Using default # of slices\n";
    else
        slices = std::stod(value);
    Debug(2, "# of slices = " + std::to_string(slices));
}


void M::InternalDynamics(double)
{
    // temporary - nonlinear absorption in Ge
    /*if(!strcmp(material,"Ge") || !strcmp(material,"Ge-Brewster")){
        for(n=0; n<n0; n++){
            intensity = 2.0 * h * vc * pow(cabs(E[pulse][x][n]), 2); // W/m2
            intensity *= tilt_factor; // reduced intensity in tilted windows
            alpha[x] += 4e-26*pow(intensity,3)*Dt;// BEST!!!
            E[pulse][x][n] *= sqrt( exp(-1.0*alpha[x]*thickness) );
        }
    }*/
}


void M::PulseInteraction(Pulse *pulse, Plane* plane, double time)
{
    Debug(2, "Interaction with a material");
    StatusDisplay(pulse, plane, time, "material...");

    double Dv = 1.0/(t_max-t_min);       // frequency step, Hz
    double v_min = vc - Dv*n0/2;

    // account for tilt (longer path and lower intensity)
    // tilt = angle of incidence  (radians) - "Theta1"
    // refr = angle of refraction (radians) - "Theta2"
    double refr = asin(sin(tilt)/(RefractiveIndex(material, pulse->v0)));
    double tilt_factor = cos(tilt)/cos(refr);   // intensity reduction due to tilt
    double th = thickness / cos(refr) / slices; // effective thickness of a slice

    if(tilt !=0 ){
        Debug(2, "tilt = " + std::to_string(tilt*180/M_PI) + " degrees (incidence angle)");
        Debug(2, "refr = " + std::to_string(refr*180/M_PI) + " degrees (refraction angle)");
        Debug(2, "tilt_factor = " + std::to_string(tilt_factor) + " (intensity reduction due to tilt)");
        Debug(2, "1/cos(refr) = " + std::to_string(1/cos(refr)) + " (effective thickness increase due to tilt)");
    }

    int count = 0;
    #pragma omp parallel for
    for(int x=0; x<x0; x++){
        if(debug_level >= 0){
            #pragma omp critical
            {
                StatusDisplay(pulse, plane, time,
                          "material: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
        }

        double intensity, delay;
        std::complex<double> *spectrum;

        for(int i=0; i<slices; i++){
            // nonlinear index (n2) and nonlinear absorption Step 1 (half-thickness of the slice)
            for(int n=0; n<n0; n++){
                intensity = 2.0 * h * vc * pow(abs(pulse->E[x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows
                delay = th/2.0/c * NonlinearIndex(material)*intensity; // phase delay (== group delay)
                pulse->E[x][n] *= exp(-I*2.0*M_PI*vc*delay); //effect of nonlinear index
            }

            // linear dispersion (full thickness of the slice)
            spectrum = new std::complex<double>[n0];
            FFT(pulse->E[x], spectrum);
            for(int n=0; n<n0; n++){
                // linear dispersion
                delay = th/c * (RefractiveIndex(material, v_min+Dv*(0.5+n)) - RefractiveIndex(material, vc)); // phase delay (!= group delay)
                spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*(0.5+n))*(-delay)); // no "-" in the exponent in frequency domain E(omega)
                // eliminate time-frame shift introduced by the difference between phase and group velocity
                delay = -th/c * (c/vc) * (RefractiveIndex(material,vc+1e7)-RefractiveIndex(material,vc-1e7))/(c/(vc+1e7)-c/(vc-1e7)); // relative group delay
                spectrum[n] *= exp(I*2.0*M_PI*(v_min+Dv*(0.5+n))*delay);
            }
            IFFT(spectrum, pulse->E[x]);
            delete[] spectrum;

            // nonlinear index (n2) and nonlinear absorption Step 2 (half-thickness of the slice)
            for(int n=0; n<n0; n++){
                intensity = 2.0 * h * vc * pow(abs(pulse->E[x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows
                delay = th/2.0/c * NonlinearIndex(material)*intensity; // phase delay (== group delay)
                pulse->E[x][n] *= exp(-I*2.0*M_PI*vc*delay); //effect of nonlinear index
            }
        }
    }
}


double M::RefractiveIndex(std::string material, double nu, double humidity)
{
    // wavelength
    double x = c / nu * 1e6; // s^-1 -> um

    if(material == "KCl"){ //Li-1976
        x= x<0.18 ? 0.18 : x;
        x= x>35 ? 35 : x;
        return sqrt( 1.26486 + 0.30523*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.41620*pow(x,2)/(pow(x,2)-pow(0.131,2)) + 0.18870*pow(x,2)/(pow(x,2)-pow(0.162,2)) + 2.6200*pow(x,2)/(pow(x,2)-pow(70.42,2)) );
    }
    if(material == "NaCl"){ //Li-1976
        x= x<0.2 ? 0.2 : x;
        x= x>30 ? 30 : x;
        return sqrt( 1.00055 + 0.19800*pow(x,2)/(pow(x,2)-pow(0.050,2)) + 0.48398*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.38696*pow(x,2)/(pow(x,2)-pow(0.128,2)) + 0.25998*pow(x,2)/(pow(x,2)-pow(0.158,2)) + 0.08796*pow(x,2)/(pow(x,2)-pow(40.50,2)) + 3.17064*pow(x,2)/(pow(x,2)-pow(60.98,2)) + 0.30038*pow(x,2)/(pow(x,2)-pow(120.34,2)) );
    }
    if(material == "ZnSe"){ //Tatian-1984
        x= x<0.54 ? 0.54 : x;
        x= x>18.2 ? 18.2 : x;
        return sqrt(1.0+4.45813734/(1-pow(0.200859853/x,2))+0.467216334/(1-pow(0.391371166/x,2))+2.89566290/(1-pow(47.1362108/x,2)));
    }
    if(material == "Ge"){ //Barnes-1979
        x= x<2.5 ? 2.5 : x;
        x= x>12 ? 12 : x;
        return sqrt( 9.28156 + 6.72880*pow(x,2)/(pow(x,2)-0.44105) + 0.21307*pow(x,2)/(pow(x,2)-3870.1) );
    }
    if(material == "GaAs"){ //Skauli-2003
        x= x<0.97 ? 0.97 : x;
        x= x>17 ? 17 : x;
        return sqrt(1+4.372514+5.466742/(1-pow(0.4431307/x,2))+0.02429960/(1-pow(0.8746453/x,2))+1.957522/(1-pow(36.9166/x,2)));
    }
    if(material == "CdTe"){ //DeBell-1979
        x= x<6 ? 6 : x;
        x= x>22 ? 22 :x;
        return sqrt( 1.0 + 6.1977889*pow(x,2)/(pow(x,2)-pow(0.317069,2)) + 3.2243821*pow(x,2)/(pow(x,2)-pow(72.0663,2)) );
    }
    if(material == "Si"){ //Edwards-1980
        x= x<2.4373 ? 2.4373 : x;
        x= x>25 ? 25 :x;
        return 3.41983+0.159906/(pow(x,2)-0.028)-0.123109/pow((pow(x,2)-0.028),2)+1.26878E-6*pow(x,2)-1.95104E-9*pow(x,4);
    }
    if(material == "air"){ //Mathar-2007
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


double M::NonlinearIndex(std::string material)
{
    if(material =="KCl")  //Sheik-Bahae-1991
        return 2e-13 * 4.19e-7 / 1.49;    // esu -> m^2/W (5.62e-20) @ 1.06 um
    if(material =="NaCl") //Sheik-Bahae-1991
        return 1.6e-13 * 4.19e-7 / 1.53;  // esu -> m^2/W (4.38e-20) @ 1.06 um
    if(material =="ZnSe") //Sheik-Bahae-1991
        return 170e-13 * 4.19e-7 / 2.48;  // esu -> m^2/W (2.87e-18) @ 1.06 um
    if(material =="Ge")   //Sheik-Bahae-1991
        return 2700e-13 * 4.19e-7 / 4.00;  // esu -> m^2/W (2.83e-17) @ 10.6 um
    if(material =="GaAs") //Sheik-Bahae-1991
        return -2700e-13 * 4.19e-7 / 3.47; // esu -> m^2/W (-3.26e-17) @ 1.06 um
        //return 1.7e-17; // m^2/W - Kapetanakos et. al. IEEE J. Quant. Electron. 37(5) (2001)
    if(material =="CdTe") //Sheik-Bahae-1991
        return -2000e-13 * 4.19e-7 / 2.84; // esu -> m^2/W (-2.95e-17) @ 1.06 um
    if(material =="Si")   //Bristow-2007
        return 1e-17;                      // m^2/W @ 2.2 um
    if(material =="air")  //Geints-2014
        return 3.5e-23;                      // m^2/W
    return 0;
}

