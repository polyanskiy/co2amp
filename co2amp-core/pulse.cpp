#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    yaml = id + ".yml";
}


void Pulse::Initialize()
{
    Debug(2, "Creating pulse from file \'" + yaml + "\'");

    std::string value="";

    if(!YamlGetValue(&value, yaml, "E0")){
        configuration_error = true;
        return;
    }
    E0 = std::stod(value);
    Debug(2, "E0 = " + toExpString(E0) + " J");

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


    // Beam spatial profile
    if(!YamlGetValue(&value, yaml, "beam")){
        configuration_error = true;
        return;
    }
    std::string beam = value;
    Debug(2, "beam = " + beam);

    BeamProfile = new double[x0];
    double Dr = planes[0]->optic->Dr; // use first optic in the layout
    if(beam == "GAUSS" || beam == "FLATTOP"){
        if(!YamlGetValue(&value, yaml, "w0")){
            configuration_error = true;
            return;
        }
        w0 = std::stod(value);
        Debug(2, "w0 = " + toExpString(w0) + " m");
        if(beam == "GAUSS")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 2));
        if(beam == "FLATTOP")
            for(int x=0; x<x0; x++)
                Dr*(0.5+x)<=w0 ? BeamProfile[x]=1 : BeamProfile[x]=0;
    }
    else if(beam == "FREEFORM"){
        std::vector<double> r;
        std::vector<double> A;
        if(!YamlGetData(&r, yaml, "beam_form", 0) || !YamlGetData(&A, yaml, "beam_form", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Beam profile [r(m) amplitude(a.u)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<r.size(); i++)
                std::cout << toExpString(r[i]) <<  " " << toExpString(A[i]) << std::endl;

        for(int x=0; x<x0; x++)
            BeamProfile[x] = sqrt(Interpolate(&r, &A, Dr*(0.5+x)));
    }
    else{
        std::cout << "ERROR: wrong \'beam\' in config file \'" << yaml << "\'" << std::endl;
        configuration_error = true;
    }


    // Pulse temporal profile
    if(!YamlGetValue(&value, yaml, "pulse")){
        configuration_error = true;
        return;
    }
    std::string pulse = value;
    Debug(2, "pulse = " + pulse);

    PulseProfile = new double[n0];
    double Dt = (t_max-t_min)/n0;
    if(pulse == "GAUSS" || pulse == "FLATTOP"){
        if(!YamlGetValue(&value, yaml, "tau0")){
            configuration_error = true;
            return;
        }
        tau0 = std::stod(value);
        Debug(2, "tau0 = " + toExpString(tau0) + " s");
        if(pulse == "GAUSS"){
            double xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
            for(int n=0; n<n0; n++)
                PulseProfile[n] = exp(-pow((t_min+Dt*(0.5+n))/xx, 2));
        }
        if(pulse == "FLATTOP")
            for(int n=0; n<n0; n++){
                std::abs(t_min+Dt*(0.5+n))<=tau0 ? PulseProfile[n]=1 : PulseProfile[n]=0;
            }
    }
    else if(pulse == "FREEFORM"){
        std::vector<double> t;
        std::vector<double> A;
        if(!YamlGetData(&t, yaml, "pulse_form", 0) || !YamlGetData(&A, yaml, "pulse_form", 1)){
            configuration_error = true;
            return;
        }
        Debug(2, "Pulse profile [t(s) amplitude(a.u)] (only displayed if debug level >= 3)");
        if(debug_level >= 3)
            for(int i=0; i<t.size(); i++)
                std::cout << toExpString(t[i]) <<  " " << toExpString(A[i]) << std::endl;

        for(int n=0; n<n0; n++)
            PulseProfile[n] = sqrt(Interpolate(&t, &A, t_min+Dt*(0.5+n)));
    }
    else{
        std::cout << "ERROR: wrong \'pulse\' in config file \'" << yaml << "\'" << std::endl;
        configuration_error = true;
    }

    // allocate memory
    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];













    Debug(2, "Initializing field array for pulse \'" + this->id + "\'");

    //int x, n;
    double Energy, af;
    //FILE *file;

    //double Dr = planes[0]->optic->Dr; // use first optic in the layout
    //double Dt = (t_max-t_min)/n0;

    // Create 2D array
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[x][n] = BeamProfile[x]*PulseProfile[n];

    // frequency shift between central frequency of the pulse (nu0) and central frequency of the calculation grig (vc)
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*(Dt*(0.5+n)));

    // Normalize intensity
    Energy = 0;
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
            Energy += 2.0 * h * nu0
                    * pow(abs(E[x][n]),2)
                    * M_PI*pow(Dr,2)*(2*x+1) //ring area = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
                    * Dt; // J

    af = sqrt(E0/Energy);
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
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


/*std::complex<double> Pulse::field(double r, double t)
{
    double xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
    std::complex<double> pulse = exp(-pow(t/xx, 2));
    std::complex<double> beam = exp(-pow(r/w0, 2));
    return pulse*beam;
}*/


void Pulse::Propagate(Plane *from, Plane *to, double time)
{ 
    double z   = from->space;
    double Dr1 = from->optic->Dr;
    double Dr2 = to  ->optic->Dr;

    if(z==0 && Dr1==Dr2)  //nothing to be done
        return;

    StatusDisplay(this, from, time, "propagation...");

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

    if( z==0 || noprop==true ){ //only change calculation grid step
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
        double lambda = c/nu0; // wavelength, m
        double k_wave = 2.0*M_PI/lambda; // wave-number
        int count=0;

        #pragma omp parallel for
        for(int x2=0; x2<x0; x2++){ // output plane
            if(debug_level >= 0){
                #pragma omp critical
                {
                    StatusDisplay(this, from, time,
                              "propagation: " + std::to_string(++count) + " of " + std::to_string(x0));
                }
            }
            double R_min, R_max, R, delta_R;
            std::complex<double> tmp;
            for(int x1=0; x1<x0; x1++){ // input plane
                R_min = sqrt(pow(Dr1*(0.5+x1)-Dr2*(0.5+x2),2)+pow(z,2)); // minimum distance from the ring to the current poin in the output plane (x)
                R_max = sqrt(pow(Dr1*(0.5+x1)+Dr2*(0.5+x2),2)+pow(z,2)); // maximum --''--
                R = (R_min+R_max)/2;                                     // average --''--
                delta_R = (R_max-R_min);
                // Huygens-Fresnell integral (summation over concentric rings in the input plane)
                // (see manual for details)
                tmp = M_PI*pow(Dr1,2)*(2*x1+1) //ring area dS = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
                        * exp(-I*k_wave*R) / (I*lambda*R) * j0(k_wave*delta_R/2);
                for(int n=0; n<n0; n++)
                    E[x2][n] += E1[x1][n] * tmp;
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


void Pulse::SavePulse()
{
    double Dr = planes[planes.size()-1]->optic->Dr; // last optic in the layout
    double Dt = (t_max-t_min)/n0;

    FILE *file_re;
    FILE *file_im;

    StatusDisplay(nullptr, nullptr, -1, "saving output pulse " + this->id + "...");

    file_re = fopen("re.dat", "wb");
    file_im = fopen("im.dat", "wb");

    // "0.00000E+000" for data alignment
    fprintf(file_re, "%e", 0.0);
    fprintf(file_im, "%e", 0.0);

    // first string: time (s)
    for(int n=0; n<n0; n++){
        fprintf(file_re, " %+e", t_min+(0.5+n)*Dt);
        fprintf(file_im, " %+e", t_min+(0.5+n)*Dt);
    }
    fprintf(file_re, "\n");
    fprintf(file_im, "\n");

    // each string represents the field E(t) at radial position defined by the first number
    for(int x=0; x<x0; x++){
        // radial coordinate
        fprintf(file_re, "%e", (0.5+x)*Dr);
        fprintf(file_im, "%e", (0.5+x)*Dr);
        // pulse (field) data
        for(int n=0; n<n0; n++){
            fprintf(file_re, " %+e", real(E[x][n]));
            fprintf(file_im, " %+e", imag(E[x][n]));
        }
        fprintf(file_re, "\n");
        fprintf(file_im, "\n");
    }

    fclose(file_re);
    fclose(file_im);

}
