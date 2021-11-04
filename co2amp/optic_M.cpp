#include "co2amp.h"


M::M(std::string id)
{
    this->id = id;
    type = "M";
    yaml = id + ".yml";

    Debug(2, "Creating optic type \'" + type + "\' from file \'" + yaml + "\'");

    std::string value="";

    // r_max (semiDia)
    if(!YamlGetValue(&value, yaml, "semiDia"))
    {
        configuration_error = true;
        return;
    }
    r_max = std::stod(value);
    Debug(2, "semiDia = " + toExpString(r_max) + " m");

    // material
    if(!YamlGetValue(&value, yaml, "material"))
    {
        configuration_error = true;
        return;
    }
    material = value;
    Debug(2, "material = " + material);

    // thickness
    if(!YamlGetValue(&value, yaml, "thickness"))
    {
        configuration_error = true;
        return;
    }
    thickness = std::stod(value);
    Debug(2, "thickness = " + toExpString(thickness) + " m");

    // humidity (only relevant for air)
    humidity = 50;
    if(material == "air")
    {
        if(!YamlGetValue(&value, yaml, "humidity", false))
            std::cout << id << ": Using default humidity (50%)\n";
        else
            humidity = std::stod(value);
        Debug(2, "humidity = " + std::to_string(humidity) + " %");
    }

    // tilt (not relevant for air)
    tilt = 0;
    if(material != "air")
    {
        if(!YamlGetValue(&value, yaml, "tilt", false))
            std::cout << "Using default tilt (no tilt)\n";
        else
            tilt = std::stod(value);
        Debug(2, "tilt = " + std::to_string(tilt) + " degrees");
        tilt *= M_PI/180; // degrees to radians
    }

    // n2
    n2 = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "n2", false))
    {
        n2 = std::stod(value);
        std::cout << id << ": Custom n2 = " << value << " m^2/W\n";
    }

    /*n4 = 0;
    if(YamlGetValue(&value, yaml, "n4", false))
    {
        n4 = std::stod(value);
        std::cout << id << ": n4 = " << value << " m^4/W^2\n";
    }*/

    /*// Band gap
    Eg = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "Eg", false))
    {
        Eg = std::stod(value);
        std::cout << id << ": Custom Eg = " << value << " J\n";
    }*/

    // Linear absorption in valence band
    alpha0 = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "alpha0", false))
    {
        alpha0 = std::stod(value);
        std::cout << id << ": Custom linear absorption coefficient alpha0 = " << value << " 1/m\n";
    }

    // Multiphoton absorption order
    /*chi = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "chi", false))
    {
        chi = std::stod(value);
        std::cout << id << ": Custom multiphoton absorption order chi = " << value << "\n";
    }*/

    // Multiphoton absorption (jump over band gap)
    /*alpha1 = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "alpha1", false))
    {
        alpha1 = std::stod(value);
        std::cout << id << ": Custom multiphoton absorption coefficient alpha1 = " << value << " m^(2-1/chi)/W\n";
    }*/

    // Free-carrier absorption
    /*alpha2 = nan(""); //set to NAN
    if(YamlGetValue(&value, yaml, "alpha2", false))
    {
        alpha2 = std::stod(value);
        std::cout << id << ": Custom free-carrier absorption coefficient alpha2 = " << value << " m^2/J\n";
    }*/

    // number of slices
    slices = 1;
    if(!YamlGetValue(&value, yaml, "slices", false))
        std::cout << "Using default # of slices (1)\n";
    else
        slices = std::stod(value);
    Debug(2, "# of slices = " + std::to_string(slices));


    // ------- MISC INITIALISATIONS -------
    // allocate memory
    /*excited = new double* [slices];
    for(int i=0; i<slices; i++)
    {
        excited[i] = new double [x0];
        for(int x=0; x<x0; x++)
            excited[i][x] = 0;
    }*/


}


void M::InternalDynamics(double)
{
    // temporary - nonlinear absorption in Ge
    /*if(!strcmp(material,"Ge") || !strcmp(material,"Ge-Brewster"))
    {
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

    double Dv = 1.0/(t_max-t_min); // frequency step, Hz
    //double Dt = (t_max-t_min)/n0;  // pulse time step, s

    // account for tilt
    // tilt = angle of incidence  (radians) - "Theta1"
    // refr = angle of refraction (radians) - "Theta2"
    double refr = asin(sin(tilt)/(RefractiveIndex(pulse->vc)));
    double tilt_factor = cos(tilt)/cos(refr);   // intensity reduction due to tilt
    double th = thickness / cos(refr) / slices; // effective thickness of a slice
    double B_integral = 0;

    n2 = NonlinearIndex();
    /*alpha1 = MultiphotonAbsorptionCoefficient1();
    alpha2 = MultiphotonAbsorptionCoefficient2();
    chi = MultiphotonAbsorptionOrder();*/

    /*Eg = BandGap(material);
    double chi = ceil(Eg/(h*pulse->vc)) - 1;
    Debug(2, "chi = " + std::to_string(chi));*/

    if(tilt !=0 )
    {
        Debug(2, "tilt = " + std::to_string(tilt*180/M_PI) + " degrees (incidence angle)");
        Debug(2, "refr = " + std::to_string(refr*180/M_PI) + " degrees (refraction angle)");
        Debug(2, "tilt_factor = " + std::to_string(tilt_factor) + " (intensity reduction due to tilt)");
        Debug(2, "1/cos(refr) = " + std::to_string(1/cos(refr)) + " (effective thickness increase due to tilt)");
    }

    int count = 0;
    int n_peak = 0;
    #pragma omp parallel for
    for(int x=0; x<x0; x++)
    {
        if(debug_level >= 0)
        {
            #pragma omp critical
            {
                StatusDisplay(pulse, plane, time,
                          "material: " + std::to_string(++count) + " of " + std::to_string(x0));
            }
        }

        double intensity, chirpyness, shift, v;//, alphaNL;
        //double integral; // proportional to the number of created free carriers
        std::complex<double> *E1; //field in frequency domaine


        // find time position (n) of peak intensity
        if(x==0)
        {
            double peak_intensity = 0;
            for(int n=0; n<n0; n++)
            {
                intensity = pow(abs(pulse->E[x][n]), 2); // arb. units
                if(intensity > peak_intensity)
                {
                    peak_intensity = intensity;
                    n_peak = n;
                }
            }
        }

        // Pulse interaction with each slice
        for(int i=0; i<slices; i++)
        {
            // -------------- REFRACTION AND ABSORPTION --------------
            // - Use split-step method for each slice -

            // nonlinear index (n2) and multiphoton absorption Step 1 (half-thickness of the slice)
            //integral = 0;
            for(int n=0; n<n0; n++)
            {
                intensity = 2.0 * h * pulse->vc * pow(abs(pulse->E[x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows

                // nonlinear refraction
                shift = pulse->vc * th/2.0/c * (n2*intensity);// + n4*pow(intensity,2));
                pulse->E[x][n] *= exp(I*2.0*M_PI*shift);

                // nonlinear absorption
                /*alphaNL = pow(alpha1*intensity,chi) + alpha2*integral;
                pulse->E[x][n] *= sqrt(exp(-alphaNL*th/2.0));
                integral += alphaNL*intensity*Dt;*/

                // B-integral
                if(x==0 && n==n_peak)
                    B_integral += M_PI * pulse->vc / c * intensity * n2 * th; // 2 ommitted: half-thickness
            }

            // linear dispersion and absorption (full thickness of the slice)
            E1 = new std::complex<double>[n0];
            FFT(pulse->E[x], E1);
            shift = 0;
            for(int n=0; n<n0; n++)
            {
                v = v0 + Dv*(n-n0/2);
                //chirpyness = -c/th / (RefractiveIndex(v+Dv/2)-RefractiveIndex(v-Dv/2));// " * Dv " omitted
                chirpyness = c/th / (GroupIndex(v0+Dv/2)-GroupIndex(v0-Dv/2));// " * Dv " omitted
                shift += (v-pulse->vc) / chirpyness; // " * Dv " omitted
                int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                E1[n1] *= exp(I*2.0*M_PI*shift); // refraction
                E1[n1] *= sqrt(exp(-AbsorptionCoefficient(v)*th)); //linear absorption
            }
            IFFT(E1, pulse->E[x]);
            delete[] E1;

            // nonlinear index (n2) and multiphoton absorption Step 2 (half-thickness of the slice)
            //integral = 0;
            for(int n=0; n<n0; n++)
            {
                intensity = 2.0 * h * pulse->vc * pow(abs(pulse->E[x][n]), 2); // W/m2
                intensity *= tilt_factor; // reduced intensity in tilted windows

                // nonlinear refraction
                shift = pulse->vc * th/2.0/c * (n2*intensity);// + n4*pow(intensity,2));
                pulse->E[x][n] *= exp(I*2.0*M_PI*shift);

                // nonlinear absorption
                /*alphaNL = pow(alpha1*intensity,chi) + alpha2*integral;
                pulse->E[x][n] *= sqrt(exp(-alphaNL*th/2.0));
                integral += alphaNL*intensity*Dt;*/

                // B-integral
                if(x==0 && n==n_peak)
                    B_integral += M_PI * pulse->vc / c * intensity * n2 * th; // 2 ommitted: half-thickness
            }
        }
    }
    Debug(1, "B_integral(" + plane->optic->id + ") = " + std::to_string(B_integral) + " radians (beam center, peak power)");
}


double M::RefractiveIndex(double nu)
{
    // wavelength
    double x = c / nu * 1e6; // s^-1 -> um

    if(material == "AgBr") //Fit of Shrotter-1931 and McCarthy-1973
    {
        x= x<0.495 ? 0.495 : x;
        x= x>12.67 ? 12.67 : x;
        return sqrt(3.860 + 0.8677*pow(x,2) / (pow(x,2)-pow(0.3211,2)) + 21.61*pow(x,2) / (pow(x,2)-pow(254.2,2)));
    }
    if(material == "AgCl") //Tilton-1950
    {
        x= x<0.578 ? 0.578 : x;
        x= x>20.06 ? 20.06 : x;
        return sqrt(4.00804+0.079086/(pow(x,2)-0.04584)-0.00085111*pow(x,2)-0.00000019762*pow(x,4));
    }
    if(material == "AMTIR1") //SCHOTT IRG 22 Product flyer (Aptil 2017)
    {
        x= x<0.8 ? 0.8 : x;
        x= x>15.5 ? 15.5 : x;
        return sqrt(3.4834+2.8203/(1-0.1352/pow(x,2))+0.9773/(1-1420.7/pow(x,2)));
    }
    if(material == "BaF2") //Li-1980
    {
        x= x<0.15 ? 0.15 : x;
        x= x>15 ? 15 : x;
        return sqrt( 1.33973 + 0.81070/(1-pow(0.10065/x,2)) + 0.19652/(1-pow(29.87/x,2)) + 4.52469/(1-pow(53.82/x,2)));
    }
    if(material == "CdTe") //DeBell-1979
    {
        x= x<6 ? 6 : x;
        x= x>22 ? 22 :x;
        return sqrt( 1.0 + 6.1977889*pow(x,2)/(pow(x,2)-pow(0.317069,2)) + 3.2243821*pow(x,2)/(pow(x,2)-pow(72.0663,2)) );
    }
    if(material == "CsI") //Li-1976
    {
        x= x<0.25 ? 0.25 : x;
        x= x>67 ? 67 : x;
        return sqrt(1.27587+0.68689/(1-pow(0.130/x,2))+0.26090/(1-pow(0.147/x,2))+0.06256/(1-pow(0.163/x,2))+0.06527/(1-pow(0.177/x,2))+0.14991/(1-pow(0.185/x,2))+0.51818/(1-pow(0.206/x,2))+0.01918/(1-pow(0.218/x,2))+3.38229/(1-pow(161.29/x,2)));
    }
    if(material == "GaAs") //Skauli-2003
    {
        x= x<0.97 ? 0.97 : x;
        x= x>17 ? 17 : x;
        return sqrt(5.372514+5.466742/(1-pow(0.4431307/x,2))+0.02429960/(1-pow(0.8746453/x,2))+1.957522/(1-pow(36.9166/x,2)));
    }
    if(material == "Ge") //Burnett-2016
    {
        x= x<2 ? 2 : x;
        x= x>14 ? 14 : x;
        return sqrt(1+0.4886331/(1-1.393959/pow(x,2))+14.5142535/(1-0.1626427/pow(x,2))+0.0091224/(1-752.190/pow(x,2)));
    }
    if(material == "KBr") //Li-1976
    {
        x= x<0.2 ? 0.2 : x;
        x= x>42 ? 42 : x;
        return sqrt(3.39408+0.79221/(1-pow(0.146/x,2))+0.01981/(1-pow(0.173/x,2))+0.15587/(1-pow(0.187/x,2))+0.17673/(1-pow(60.61/x,2))+2.06217/(1-pow(87.72/x,2)));
    }
    if(material == "KCl") //Li-1976
    {
        x= x<0.18 ? 0.18 : x;
        x= x>35 ? 35 : x;
        return sqrt( 1.26486 + 0.30523*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.41620*pow(x,2)/(pow(x,2)-pow(0.131,2)) + 0.18870*pow(x,2)/(pow(x,2)-pow(0.162,2)) + 2.6200*pow(x,2)/(pow(x,2)-pow(70.42,2)) );
    }
    if(material == "NaCl") //Li-1976
    {
        x= x<0.2 ? 0.2 : x;
        x= x>30 ? 30 : x;
        return sqrt( 1.00055 + 0.19800*pow(x,2)/(pow(x,2)-pow(0.050,2)) + 0.48398*pow(x,2)/(pow(x,2)-pow(0.100,2)) + 0.38696*pow(x,2)/(pow(x,2)-pow(0.128,2)) + 0.25998*pow(x,2)/(pow(x,2)-pow(0.158,2)) + 0.08796*pow(x,2)/(pow(x,2)-pow(40.50,2)) + 3.17064*pow(x,2)/(pow(x,2)-pow(60.98,2)) + 0.30038*pow(x,2)/(pow(x,2)-pow(120.34,2)) );
    }
    if(material == "Si") //Edwards-1980
    {
        x= x<2.4373 ? 2.4373 : x;
        x= x>25 ? 25 :x;
        return 3.41983+0.159906/(pow(x,2)-0.028)-0.123109/pow((pow(x,2)-0.028),2)+1.26878E-6*pow(x,2)-1.95104E-9*pow(x,4);
    }
    if(material == "SiO2") //Malitson-1965
    {
        x= x<0.21 ? 0.21 : x;
        x= x>6.7 ? 6.7 :x;
        return sqrt(1+0.6961663/(1-pow(0.0684043/x,2))+0.4079426/(1-pow(0.1162414/x,2))+0.8974794/(1-pow(9.896161/x,2)));
    }
    if(material == "ZnS") //Klein-1986
    {
        x= x<0.405 ? 0.405 : x;
        x= x>13 ? 13 : x;
        return sqrt(8.393+0.14383/(pow(x,2)-pow(0.2421,2))+4430.99/(pow(x,2)-pow(36.71,2)));
    }
    if(material == "ZnSe") //Tatian-1984
    {
        x= x<0.54 ? 0.54 : x;
        x= x>18.2 ? 18.2 : x;
        return sqrt(1.0+4.45813734/(1-pow(0.200859853/x,2))+0.467216334/(1-pow(0.391371166/x,2))+2.89566290/(1-pow(47.1362108/x,2)));
    }
    if(material == "air") //Mathar-2007
    {
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
        for(j=0; j<6; j++)
        {
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


double M::GroupIndex(double nu)
{
    double Dv = 1.0/(t_max-t_min); // frequency step, Hz
    return RefractiveIndex(nu) + nu * (RefractiveIndex(nu+Dv/2)-RefractiveIndex(nu-Dv/2)) / Dv;
}


double M::NonlinearIndex()
{
    //***************//
    // n2 unit: m2/W //
    //***************//

    if(!std::isnan(n2)) // not NAN: custom nonlinear index from YAML configuration file
        return n2;

    if(material =="AgBr")
        return 1.0e-19;
    if(material =="AgCl")
        return 1.7e-19;
    if(material =="BaF2")
        //return 0.67e-13 * 4.19e-7 / 1.49; // esu -> m^2/W (5.62e-20) @ 1.06 um (Sheik-Bahae-1991)
        return 1.7e-20;                    // @ 9.2 um  Polyanskiy-2021 https://doi.org/10.1364/OE.434238
    if(material =="CdTe")
        return -2000e-13 * 4.19e-7 / 2.84; // esu -> m^2/W (-2.95e-17) @ 1.06 um (Sheik-Bahae-1991)
    if(material =="CsI")
        return 1.2e-19;
    if(material =="GaAs")
        return -2700e-13 * 4.19e-7 / 3.47; // esu -> m^2/W (-3.26e-17) @ 1.06 um (Sheik-Bahae-1991)
        //return 1.7e-17; // m^2/W - Kapetanakos et. al. IEEE J. Quant. Electron. 37(5) (2001)
    if(material =="Ge")
        return 2700e-13 * 4.19e-7 / 4.00;  // esu -> m^2/W (2.83e-17) @ 10.6 um (Sheik-Bahae-1991)
    if(material =="KCl")
        //return 2e-13 * 4.19e-7 / 1.49;    // esu -> m^2/W (5.62e-20) @ 1.06 um (Sheik-Bahae-1991)
        return 3.4e-20;                    // @ 9.2 um  Polyanskiy-2021 https://doi.org/10.1364/OE.434238
    if(material =="NaCl")
        //return 1.6e-13 * 4.19e-7 / 1.53;  // esu -> m^2/W (4.38e-20) @ 1.06 um (Sheik-Bahae-1991)
        return 3.5e-20;                    // @ 9.2 um  Polyanskiy-2021 https://doi.org/10.1364/OE.434238
    if(material =="Si")
        return 1e-17;                      // @ 2.2 um (Bristow-2007)
    if(material =="SiO2")
        return 1.1e-13 * 4.19e-7 / 1.40;   // esu -> m^2/W (3.29e-20) @ 1.06 um (Sheik-Bahae-1991)
    if(material =="ZnS")
        return 2.5e-19;
    if(material =="ZnSe")
        return 170e-13 * 4.19e-7 / 2.48;   // esu -> m^2/W (2.87e-18) @ 1.06 um (Sheik-Bahae-1991)
    if(material =="air")
        return 3e-23;                      // @ 9.2 um  Polyanskiy-2021 https://doi.org/10.1364/OL.423800
    return 0;
}


/*double M::BandGap(std::string material)
{
    if(!std::isnan(Eg)) // not NAN:  custom band gap from YAML configuration file
        return Eg;

    if(material =="KCl")
        return 8.7 * 1.60218e-19; // eV -> J
    if(material =="NaCl")
        return 9.0 * 1.60218e-19; // eV -> J
    if(material =="ZnSe")
        return 2.8 * 1.60218e-19; // eV -> J
    if(material =="Ge")
        return 0.67 * 1.60218e-19; // eV -> J
    if(material =="GaAs")
        return 1.43 * 1.60218e-19; // eV -> J
    if(material =="CdTe")
        return 1.5 * 1.60218e-19; // eV -> J
    if(material =="Si")
        return 1.14 * 1.60218e-19; // eV -> J
    if(material =="air")
        return 0;

    return 0;
}*/


double M::AbsorptionCoefficient(double nu)
{
    if(!std::isnan(alpha0)) // not NAN: custom absorption coefficient from YAML configuration file
        return alpha0;

    // wavelength
    double x = c / nu * 1e6; // s^-1 -> um

    if(material == "BaF2")
    {
        if(x<6)
            return 0;
        return 0.9 * (exp(1.17*(x-8.0)) - 1.0);  // 1/m
    }

    if(material == "CsI")
        return 0.2;

    if(material == "KBr")
        return 0.35;

    return 0;
}


/*double M::MultiphotonAbsorptionCoefficient1()
{
    if(!std::isnan(alpha1)) // not NAN: custom multiphoton absorption coefficient from YAML configuration file
        return alpha1;
    return 0;
}

double M::MultiphotonAbsorptionCoefficient2()
{
    if(!std::isnan(alpha2)) // not NAN: custom multiphoton absorption coefficient from YAML configuration file
        return alpha2;
    return 0;
}


double M::MultiphotonAbsorptionOrder()
{
    if(!std::isnan(chi)) // not NAN: custom multiphoton absorption order from YAML configuration file
        return chi;
    return 1; // not "0": to avoid 0^0 (= 1)
}*/

