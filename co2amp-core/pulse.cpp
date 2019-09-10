#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    yaml = id + ".yml";
    // cannot initialize pulse here: need process optics and layout (planes) first:
    // Dr of input plane (first optic in the layout) needed
    // Use Initialize() instead.

    E = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E[x] = new std::complex<double>[n0];
}


void Pulse::Initialize()
{
    Debug(2, "Creating pulse from file \'" + yaml + "\'");

    double Dt = (t_max-t_min)/n0;
    std::string value="";

    //---------------- Paarameters that must be present in any case ---------------------
    if(!YamlGetValue(&value, yaml, "time_inj")){
        configuration_error = true;
        return;
    }
    time_inj = std::stod(value);
    Debug(2, "time_inj = " + toExpString(time_inj) + " s");

    if(!YamlGetValue(&value, yaml, "from_file")){
        configuration_error = true;
        return;
    }

    //================================== From file ======================================
    if(value!="true" && value!="false"){
        configuration_error = true;
        std::cout << "ERROR: Wrong \'from_file\' parameter (must be \"true\" or \"false\")\n";
        return;
    }
    if(value=="true"){
        if(!YamlGetValue(&value, yaml, "file")){
            configuration_error = true;
            return;
        }
        if(!LoadPulse(value)){
            configuration_error = true;
            return;
        }
        // frequency shift between central frequency of the pulse (nu0)
        // and central frequency of the calculation grig (vc)
        for(int x=0; x<x0; x++)
            for(int n=0; n<n0; n++)
                E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*Dt*(0.5+n));
        return;
    }

    //================================ Not from file ====================================

    //---------------------------- Basic pulse parameters -------------------------------
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

    //----------------------------- Beam spatial profile --------------------------------
    if(!YamlGetValue(&value, yaml, "beam")){
        configuration_error = true;
        return;
    }
    std::string beam = value;
    Debug(2, "beam = " + beam);

    double *BeamProfile = new double[x0];
    double Dr = planes[0]->optic->r_max/x0; // input plane (first optic in the layout)
    if(beam == "GAUSS" || beam == "FLATTOP"){
        if(!YamlGetValue(&value, yaml, "w0")){
            configuration_error = true;
            return;
        }
        double w0 = std::stod(value);
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

    //---------------------------- Pulse temporal profile -------------------------------
    if(!YamlGetValue(&value, yaml, "pulse")){
        configuration_error = true;
        return;
    }
    std::string pulse = value;
    Debug(2, "pulse = " + pulse);

    double *PulseProfile = new double[n0];
    if(pulse == "GAUSS" || pulse == "FLATTOP"){
        if(!YamlGetValue(&value, yaml, "tau0")){
            configuration_error = true;
            return;
        }
        double tau0 = std::stod(value);
        Debug(2, "tau0 = " + toExpString(tau0) + " s");
        if(pulse == "GAUSS"){
            double xx = tau0/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
            for(int n=0; n<n0; n++)
                PulseProfile[n] = exp(-pow((t_min+Dt*(0.5+n))/xx, 2));
        }
        if(pulse == "FLATTOP")
            for(int n=0; n<n0; n++)
                std::abs(t_min+Dt*(0.5+n))<=tau0 ? PulseProfile[n]=1 : PulseProfile[n]=0;
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

    //--------------------------- Initialize field array E ------------------------------

    Debug(2, "Initializing field array for pulse \'" + this->id + "\'");

    // Create 2D array
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[x][n] = BeamProfile[x]*PulseProfile[n];

    // frequency shift between central frequency of the pulse (nu0)
    // and central frequency of the calculation grig (vc)
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[x][n] *= exp(I*2.0*M_PI*(nu0-vc)*Dt*(0.5+n));

    // Normalize intensity
    double Energy = 0;
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
            Energy += 2.0 * h * nu0
                    * pow(abs(E[x][n]),2)
                    * M_PI*pow(Dr,2)*(2*x+1) //ring area = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
                    * Dt; // J

    double af = sqrt(E0/Energy);
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
            E[x][n] *= af;
}


void Pulse::Propagate(Plane *from, Plane *to, double time)
{ 
    double z   = from->space;
    double Dr1 = from->optic->r_max/x0;
    double Dr2 = to  ->optic->r_max/x0;

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
    StatusDisplay(nullptr, nullptr, -1, "saving output pulse " + this->id + "...");

    double Dt = (t_max-t_min)/n0;

    // see dynamic array example
    // https://support.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
    double **re = new double* [x0];
    double **im = new double* [x0];

    re[0] = new double[x0*n0];
    im[0] = new double[x0*n0];

    for (int x=1; x<x0; x++){
        re[x] = re[0]+x*n0;
        im[x] = im[0]+x*n0;
    }

    std::complex<double> **E1;
    E1 = new std::complex<double>* [x0];
    for(int x=0; x<x0; x++)
        E1[x] = new std::complex<double>[n0];
    // reverse frequency shift between central frequency of the pulse (nu0)
    // and central frequency of the calculation grig (vc)
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E1[x][n] = E[x][n] * exp(-I*2.0*M_PI*(nu0-vc)*Dt*(0.5+n));


    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++){
            re[x][n] = real(E1[x][n]);
            im[x][n] = imag(E1[x][n]);
        }

    hid_t file = H5Fcreate((id+".pulse").c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hsize_t dims[2];
    dims[0] = x0;
    dims[1] = n0;

    H5Gcreate(file, "pulse", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5LTmake_dataset (file, "pulse/re", 2, dims, H5T_NATIVE_DOUBLE, &re[0][0]);
    H5LTmake_dataset (file, "pulse/im", 2, dims, H5T_NATIVE_DOUBLE, &im[0][0]);

    double dbl[1];
    dbl[0] = planes[planes.size()-1]->optic->r_max; // last optic in the layout
    H5LTset_attribute_double(file, "pulse", "r_max", dbl, 1);
    dbl[0] = t_min;
    H5LTset_attribute_double(file, "pulse", "t_min", dbl, 1);
    dbl[0] = t_max;
    H5LTset_attribute_double(file, "pulse", "t_max", dbl, 1);
    dbl[0] = nu0;
    H5LTset_attribute_double(file, "pulse", "nu0", dbl, 1);

    H5Fclose(file);

    // ----------------------------- REMOVE TEMPORARY ARRAYS ----------------------------
    delete[] re[0];
    delete[] im[0];
    delete[] re;
    delete[] im;

    for(int x=0; x<x0; x++)
        delete[] E1[x];
    delete[] E1;
}


bool Pulse::LoadPulse(std::string filename)
{
    Debug(2, "Reading pulse data from file \'" + filename + "\'");

    hid_t file = H5Fopen(filename.c_str(),H5F_ACC_RDONLY, H5P_DEFAULT);
    if(file < 0){ //H5Fopen reterns a negative value in case of error
        file = H5Fopen((search_dir+"\\"+filename).c_str(),H5F_ACC_RDONLY, H5P_DEFAULT);
        if(file < 0){
            std::cout << "ERROR: Cannot open HDF5 file \'" << filename << "\'\n";
            return false;
        }
    }

    // ------------------------------ READ ATTRIBUTES -----------------------------------
    double dbl[1];
    hid_t status1 = H5LTget_attribute_double(file, "pulse", "r_max", dbl);
    double r_max1 = dbl[0];
    hid_t status2 = H5LTget_attribute_double(file, "pulse", "t_min", dbl);
    double t_min1 = dbl[0];
    hid_t status3 = H5LTget_attribute_double(file, "pulse", "t_max", dbl);
    double t_max1 = dbl[0];
    hid_t status4 = H5LTget_attribute_double(file, "pulse", "nu0", dbl);
    nu0 = dbl[0];
    if(status1<0 || status2<0 || status3<0 || status4<0){
        std::cout << "ERROR: Cannot read pulse attributes from file \'" << filename << "\'\n";
        return false;
    }
    Debug(2, "r_max = " + toExpString(r_max1) + " m");
    Debug(2, "t_min = " + toExpString(t_min1) + " s");
    Debug(2, "t_max = " + toExpString(t_max1) + " s");
    Debug(2, "nu0 = " + toExpString(nu0) + " Hz");

    // --------------------------- GET READY TO READ DATA -------------------------------
    hid_t dataset_re = H5Dopen(file, "pulse/re", H5P_DEFAULT);
    hid_t dataset_im = H5Dopen(file, "pulse/im", H5P_DEFAULT);

    hsize_t dims[2];
    H5Sget_simple_extent_dims(H5Dget_space(dataset_re), dims, NULL);
    int x01 = dims[0];
    int n01 = dims[1];
    Debug(2, "Size of input arrays (time x coord): " + std::to_string(x01)
          + " x " + std::to_string(n01));

    // --------------------------- PREPARE OUTPUT ARRRAYS -------------------------------
    // see dynamic array example
    // https://support.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_readdyn.c
    double **re = new double* [x01];
    double **im = new double* [x01];

    re[0] = new double[x01*n01];
    im[0] = new double[x01*n01];

    for (int x=1; x<x01; x++){
        re[x] = re[0]+x*n01;
        im[x] = im[0]+x*n01;
    }

    // ------------------------------- READ PULSE DATA ----------------------------------
    status1 = H5Dread(dataset_re, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &re[0][0]);
    status2 = H5Dread(dataset_im, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &im[0][0]);

    if(status1<0 || status2<0){
        std::cout << "ERROR: Cannot read field data from file \'" << filename << "\'\n";
        return false;
    }

    std::complex<double> **E1;
    E1 = new std::complex<double>* [x01];
    for(int x=0; x<x01; x++)
        E1[x] = new std::complex<double>[n01];

    for(int x=0; x<x01; x++)
        for(int n=0; n<n01; n++)
            E1[x][n] = re[x][n] + I*im[x][n];

    // --------------------- CLOSE RESOURCES, REMOVE OUTPUT ARRAYS ----------------------
    H5Dclose(dataset_re);
    H5Dclose(dataset_im);
    H5Fclose(file);

    delete[] re[0];
    delete[] im[0];
    delete[] re;
    delete[] im;

    // ----------------------------- INTERPOLATE IF NEEDED ------------------------------
    double Dr = optics[0]->r_max/x0;
    double Dt = (t_max-t_min)/n0;
    double Dr1 = r_max1/x01;
    double Dt1 = (t_max1-t_min1)/n01;

    #pragma omp parallel for
    for(int x=0; x<x0; x++){
        double r = Dr*(0.5+x);
        for(int n=0; n<n0; n++){
            double t = t_min + Dt*(0.5+n);
            if(r>r_max1 || t<t_min1 || t>t_max1){
                E[x][n] = 0;
                break;
            }
            int x1 = (int)floor(r/Dr1 - 0.5);
            int x2 = x1+1;
            int n1 = (int)floor((t-t_min1)/Dt1 - 0.5);
            int n2 = n1+1;

            if(x1<0) x1=0;
            if(n1<0) n1=0;
            if(x2>=x01) x2=x01-1;
            if(n2>=n01) n2=n01-1;
            double a = r/Dr1 - (x1+0.5);
            double b = (t-t_min1)/Dt1 - (n1+0.5);
            E[x][n] = E1[x1][n1] * (1-a) * (1-b)
                    + E1[x2][n1] * a     * (1-b)
                    + E1[x1][n2] * (1-a) * b
                    + E1[x2][n2] * a     * b;
        }
    }

    // ----------------------------- REMOVE TEMPORARY ARRAY -----------------------------
    for(int x=0; x<x01; x++)
        delete[] E1[x];
    delete[] E1;

    // ------------------------------------ SUCCESS! ------------------------------------
    Debug(2, "Pulse read from file done!");
    return true;
}
