#include  "co2amp.h"

Pulse::Pulse(std::string id)
{
    this->id = id;
    yaml_path = id + ".yml";

    E.resize(x0*n0); // use 1D vector to store a 2D array [x0]X[n0]

    // cannot initialize pulse here: need process optics and layout (planes) first:
    // Dr of input plane (first optic in the layout) needed
    // Use Initialize() instead.
}


void Pulse::Initialize()
{
    std::string value="";

    Debug(2, "Creating pulse from file \'" + yaml_path + "\'");

    if(!YamlReadFile(yaml_path, &yaml_content))
    {
        configuration_error = true;
        return;
    }

    //---------------- Parameters that must be present in any case ---------------------
    if(!YamlGetValue(&value, &yaml_content, "t_in")){
        configuration_error = true;
        return;
    }
    time_in = std::stod(value);
    Debug(2, "t_in = " + toExpString(time_in) + " s");

    /*if(!YamlGetValue(&value, &yaml_content, "file"))
    {
        configuration_error = true;
        return;
    }*/

    //================================== From file ======================================
    if(YamlGetValue(&value, &yaml_content, "file", false))
    {
    //if(value!="null")
    //{
        if(!LoadPulse(value))
        {
            configuration_error = true;
            return;
        }
        // frequency shift between central frequency of the pulse (vc)
        // and central frequency of the calculation grid (v0)
        for(int x=0; x<x0; x++)
            for(int n=0; n<n0; n++)
                E[n0*x+n] *= exp(I*2.0*M_PI*(v0-vc)*Dt*(0.5+n));
        return;
    }

    //================================ Not from file ====================================

    //---------------------------- Basic pulse parameters -------------------------------
    if(!YamlGetValue(&value, &yaml_content, "E"))
    {
        configuration_error = true;
        return;
    }
    E0 = std::stod(value);
    Debug(2, "E = " + toExpString(E0) + " J");

    if(!YamlGetValue(&value, &yaml_content, "freq"))
    {
        configuration_error = true;
        return;
    }
    vc = std::stod(value);
    Debug(2, "freq = " + toExpString(vc) + " Hz");

    //----------------------------- Beam spatial profile --------------------------------
    if(!YamlGetValue(&value, &yaml_content, "beam"))
    {
        configuration_error = true;
        return;
    }
    std::string beam = value;
    Debug(2, "beam = " + beam);

    std::vector<double> BeamProfile(x0);
    double Dr = planes[0]->optic->Dr; // input plane (first optic in the layout)
    if(beam == "GAUSS" || beam == "SUPERGAUSS4" || beam == "SUPERGAUSS6" || beam == "SUPERGAUSS8" || beam == "SUPERGAUSS10" || beam == "FLATTOP")
    {
        if(!YamlGetValue(&value, &yaml_content, "w"))
        {
            configuration_error = true;
            return;
        }
        double w0 = std::stod(value);
        Debug(2, "w = " + toExpString(w0) + " m");
        if(beam == "GAUSS")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 2));
        if(beam == "SUPERGAUSS4")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 4));
        if(beam == "SUPERGAUSS6")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 6));
        if(beam == "SUPERGAUSS8")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 8));
        if(beam == "SUPERGAUSS10")
            for(int x=0; x<x0; x++)
                BeamProfile[x] = exp(-pow(Dr*(0.5+x)/w0, 10));
        if(beam == "FLATTOP")
            for(int x=0; x<x0; x++)
                Dr*(0.5+x)<=w0 ? BeamProfile[x]=1 : BeamProfile[x]=0;
    }
    else if(beam == "FREEFORM")
    {
        std::vector<double> r;
        std::vector<double> A;
        if(!YamlGetData(&r, &yaml_content, "beam_form", 0) || !YamlGetData(&A, &yaml_content, "beam_form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Beam profile loaded (use debug level 3 to display)");
        Debug(3, "Beam profile [r(m) amplitude(a.u)]");
        if(debug_level >= 3)
            for(size_t i=0; i<r.size(); i++)
                std::cout << toExpString(r[i]) <<  " " << toExpString(A[i]) << std::endl;
        for(int x=0; x<x0; x++)
            BeamProfile[x] = sqrt(Interpolate(&r, &A, Dr*(0.5+x)));
    }
    else
    {
        std::cout << "ERROR: wrong \'beam\' in config file \'" << yaml_path << "\'" << std::endl;
        configuration_error = true;
    }

    //---------------------------- Pulse temporal profile -------------------------------
    if(!YamlGetValue(&value, &yaml_content, "pulse"))
    {
        configuration_error = true;
        return;
    }
    std::string pulse = value;
    Debug(2, "pulse = " + pulse);

    std::vector<double> PulseProfile(n0);
    if(pulse == "GAUSS" || pulse == "FLATTOP")
    {
        if(!YamlGetValue(&value, &yaml_content, "fwhm"))
        {
            configuration_error = true;
            return;
        }
        double fwhm = std::stod(value);
        Debug(2, "tau = " + toExpString(fwhm) + " s");
        if(pulse == "GAUSS")
        {
            double tau = fwhm/sqrt(log(2.0)*2.0);	//(fwhm -> half-width @ 1/e^2)
            for(int n=0; n<n0; n++)
                PulseProfile[n] = exp(-pow((t_min+Dt*(0.5+n))/tau, 2));
        }
        if(pulse == "FLATTOP")
            for(int n=0; n<n0; n++)
                std::abs(t_min+Dt*(0.5+n))<=fwhm ? PulseProfile[n]=1 : PulseProfile[n]=0;
    }
    else if(pulse == "FREEFORM")
    {
        std::vector<double> t;
        std::vector<double> A;
        if(!YamlGetData(&t, &yaml_content, "pulse_form", 0) || !YamlGetData(&A, &yaml_content, "pulse_form", 1))
        {
            configuration_error = true;
            return;
        }
        Debug(2, "Pulse profile loaded (use debug level 3 to display)");
        Debug(3, "Pulse profile [t(s) amplitude(a.u)]");
        if(debug_level >= 3)
            for(size_t i=0; i<t.size(); i++)
                std::cout << toExpString(t[i]) <<  " " << toExpString(A[i]) << std::endl;
        for(int n=0; n<n0; n++)
            PulseProfile[n] = sqrt(Interpolate(&t, &A, t_min+Dt*(0.5+n)));
    }
    else
    {
        std::cout << "ERROR: wrong \'pulse\' in config file \'" << yaml_path << "\'" << std::endl;
        configuration_error = true;
    }

    //--------------------------- Initialize field array E ------------------------------

    Debug(2, "Initializing field array for pulse \'" + this->id + "\'");

    // Create 2D array
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[n0*x+n] = BeamProfile[x]*PulseProfile[n];

    // frequency shift between central frequency of the pulse (vc)
    // and central frequency of the calculation grid (v0)
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E[n0*x+n] *= exp(I*2.0*M_PI*(v0-vc)*Dt*(0.5+n));

    // Normalize intensity
    double Energy = 0;
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
            Energy += 2.0 * h * vc
                    * pow(abs(E[n0*x+n]),2)
                    * M_PI*pow(Dr,2)*(2*x+1) //ring area = Pi*(Dr*(x+1))^2 - Pi*(Dr*x)^2 = Pi*Dr^2*(2x+1)
                    * Dt; // J

    double af = sqrt(E0/Energy);
    for(int n=0; n<n0; n++)
        for(int x=0; x<x0; x++)
            E[n0*x+n] *= af;
}


void Pulse::Propagate(Plane *from, Plane *to, double time)
{
    double z   = from->space;
    double Dr1 = from->optic->r_max/x0;
    double Dr2 = to  ->optic->r_max/x0;
    int count=0;

    if(z==0 && Dr1==Dr2)  //nothing to be done
        return;

    StatusDisplay(this, from, time, "propagation...");

    // Create temporary field arrays
    std::vector<std::complex<double>> E1(x0*n0);

    Debug(2, "propagation: temporary field arrays created");

    if( z==0 || method==0 ) // no propagation - only change calculation grid step
    {
        for(int x=0; x<x0; x++)
        {
            for(int n=0; n<n0; n++)
            {
                E1[n0*x+n] = E[n0*x+n];
                E[n0*x+n] = 0;
            }
        }
        Debug(2, "propagation: arrays initialized");

        #pragma omp parallel for
        for(int x=0; x<x0; x++)
        {
            double r = Dr2*(0.5+x);

            if(r > Dr1*x0)
                continue;

            int x1 = (int)floor(r/Dr1 - 0.5);
            int x2 = x1+1;

            if(x1<0) x1=0;
            if(x2>=x0) x2=x0-1;

            double a = r/Dr1 - (x1+0.5);

            for(int n=0; n<n0; n++)
                E[n0*x+n] = E1[n0*x1+n]*(1-a) + E1[n0*x2+n]*a;
        }
    }

    else // diffraction propagation
    {
        for(int x=0; x<x0; x++)
        {
            FFT(&E[x*n0], &E1[x*n0]); // time -> frequency domain
            for(int n=0; n<n0; n++)
                E[n0*x+n] = 0;
        }

        #pragma omp parallel for
        for(int x2=0; x2<x0; x2++) // output plane
        {
            if(debug_level >= 0)
            {
                #pragma omp critical
                {
                    StatusDisplay(this, from, time,
                              "propagation: " + std::to_string(++count) + " of " + std::to_string(x0));
                }
            }

            double r1;
            double r2 = Dr2*(0.5+x2);

            // Fresnell diffraction with cylindrical symmetry
            if(method == 1)
            {
                double lambda, k_wave;
                for(int x1=0; x1<x0; x1++) // input plane
                {
                    r1 = Dr1*(0.5+x1);
                    for(int n=0; n<n0; n++)
                    {
                        lambda = c/(v_min+Dv*(0.5+n));
                        k_wave = 2.0*M_PI/lambda;
                        int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                        E[x2*n0+n1] += E1[n0*x1+n1]
                                * 2.0*M_PI*r1*Dr1
                                * exp(I*k_wave*(pow(r1,2)+pow(r2,2))/2.0/z)
                                / (I*lambda*z)
                                * j0(k_wave*r1*r2/z);
                    }
                }
            }

            // Rayleigh-Sommerfeld integral with cylindrical symmetry
            if(method == 2)
            {
                double lambda, k_wave, R, phi, Dphi;
                double R2, R2max; // R^2
                std::complex<double> tmp;
                for(int x1=0; x1<x0; x1++) // input plane
                {
                    r1 = Dr1*(0.5+x1);
                    //Dphi = M_PI/ceil(M_PI*(x1+0.5));
                    Dphi = 1/ceil(x1+0.5);
                    R2max = pow(r1,2) + pow(r2,2) + pow(z,2);
                    for(int n=0; n<n0; n++)
                    {
                        lambda = c/(v_min+Dv*(0.5+n));
                        k_wave = 2.0*M_PI/lambda;
                        tmp = 0;
                        for(phi=Dphi*0.5; phi<M_PI; phi+=Dphi){
                            R2 = R2max - 2*r1*r2*cos(phi);
                            R = sqrt(R2);
                            tmp += exp(I*k_wave*(R-z)) / R2;
                        }
                        tmp *= 2.0 * Dphi*r1*Dr1 / (I*lambda) * z;
                        int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                        E[n0*x2+n1] +=  E1[n0*x1+n1] * tmp;
                    }
                }
            }

            // Experimental propagation
            /*if(method == 3)
            {
                double lambda, k_wave, R, phi, Dphi;
                double R2, R2max; // R^2
                std::complex<double> tmp;
                for(int x1=0; x1<x0; x1++) // input plane
                {
                    r1 = Dr1*(0.5+x1);
                    //Dphi = 1/ceil(x1*x2/x0 * (from->optic->r_max + to->optic->r_max)/z +0.5);
                    Dphi = 1/ceil(x1 * (from->optic->r_max + to->optic->r_max)/z +0.5);
                    R2max = pow(r1,2) + pow(r2,2) + pow(z,2);
                    for(int n=0; n<n0; n++)
                    {
                        lambda = c/(v_min+Dv*(0.5+n));
                        k_wave = 2.0*M_PI/lambda;
                        tmp = 0;
                        for(phi=Dphi*0.5; phi<M_PI; phi+=Dphi){
                            R2 = R2max - 2*r1*r2*cos(phi);
                            R = sqrt(R2);
                            tmp += exp(I*k_wave*(R-z)) / R2;
                        }
                        tmp *= 2.0 * Dphi*r1*Dr1 / (I*lambda) * z;
                        int n1 = n<n0/2 ? n+n0/2 : n-n0/2;
                        E[n0*x2+n1] +=  E1[n0*x1+n1] * tmp;
                    }
                }
            }*/
        }

        for(int x=0; x<x0; x++)
        {
            IFFT(&E[n0*x], &E1[n0*x]);
            for(int n=0; n<n0; n++)
                E[n0*x+n] = E1[n0*x+n];
        }

        Debug(2, "propagation: diffraction integral calculations done");
    }

    Debug(2, "propagation: all done");

    Debug(2, "propagation: temporary field array deleted");
}


void Pulse::SavePulse()
{
    StatusDisplay(nullptr, nullptr, -1, "saving output pulse " + this->id + "...");

    std::vector<double> re(x0*n0);
    std::vector<double> im(x0*n0);

    std::vector<std::complex<double>> E1(x0*n0);

    // reverse frequency shift between central frequency of the pulse (vc)
    // and central frequency of the calculation grig (v0)
    for(int x=0; x<x0; x++)
        for(int n=0; n<n0; n++)
            E1[n0*x+n] = E[n0*x+n] * exp(-I*2.0*M_PI*(v0-vc)*Dt*(0.5+n));

    for(int i=0; i<x0*n0; i++)
    {
        re[i] = E1[i].real();
        im[i] = E1[i].imag();
    }

    hsize_t dims[] = {(hsize_t)x0,(hsize_t)n0};

    hid_t file = H5Fcreate((id+".pulse").c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    H5Gcreate(file, "pulse", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5LTmake_dataset (file, "pulse/re", 2, dims, H5T_NATIVE_DOUBLE, re.data());
    H5LTmake_dataset (file, "pulse/im", 2, dims, H5T_NATIVE_DOUBLE, im.data());

    H5LTset_attribute_double(file, "pulse", "r_max", &planes[planes.size()-1]->optic->r_max, 1);
    H5LTset_attribute_double(file, "pulse", "t_min", &t_min, 1);
    H5LTset_attribute_double(file, "pulse", "t_max", &t_max, 1);
    H5LTset_attribute_double(file, "pulse", "freq", &vc, 1);

    H5Fclose(file);
}


bool Pulse::LoadPulse(std::string filename)
{
    Debug(2, "Reading pulse data from file \'" + filename + "\'");

    hid_t file = H5Fopen(filename.c_str(),H5F_ACC_RDONLY, H5P_DEFAULT);
    if(file < 0) //H5Fopen reterns a negative value in case of error
    {
        file = H5Fopen((search_dir+"\\"+filename).c_str(),H5F_ACC_RDONLY, H5P_DEFAULT);
        if(file < 0)
        {
            std::cout << "ERROR: Cannot open HDF5 file \'" << filename << "\'\n";
            return false;
        }
    }

    // ------------------------------ READ ATTRIBUTES -----------------------------------
    double r_max1, t_min1, t_max1;
    int status=0;
    status += H5LTget_attribute_double(file, "pulse", "r_max", &r_max1);
    status += H5LTget_attribute_double(file, "pulse", "t_min", &t_min1);
    status += H5LTget_attribute_double(file, "pulse", "t_max", &t_max1);
    status += H5LTget_attribute_double(file, "pulse", "freq", &vc);
    if(status<0) // function returns -1 for failure, 0 for success
    {
        std::cout << "ERROR: Cannot read pulse attributes from file \'" << filename << "\'\n";
        return false;
    }
    Debug(2, "r_max = " + toExpString(r_max1) + " m");
    Debug(2, "t_min = " + toExpString(t_min1) + " s");
    Debug(2, "t_max = " + toExpString(t_max1) + " s");
    Debug(2, "freq = " + toExpString(vc) + " Hz");

    // --------------------------- GET READY TO READ DATA -------------------------------
    hid_t dataset_re = H5Dopen(file, "pulse/re", H5P_DEFAULT);
    hid_t dataset_im = H5Dopen(file, "pulse/im", H5P_DEFAULT);

    hsize_t dims[2];
    H5Sget_simple_extent_dims(H5Dget_space(dataset_re), dims, NULL);
    int x01 = dims[0];
    int n01 = dims[1];
    Debug(2, "Size of input arrays (time x coord): " + std::to_string(n01)
          + " x " + std::to_string(x01));

    // ------------------------------ PREPARE  ARRRAYS ----------------------------------
    std::vector<double> re(x01*n01);
    std::vector<double> im(x01*n01);
    std::vector<std::complex<double>> E1(x01*n01);

    // ------------------------------- READ PULSE DATA ----------------------------------
    status += H5Dread(dataset_re, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, re.data());
    status += H5Dread(dataset_im, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, im.data());

    if(status<0)
    {
        std::cout << "ERROR: Cannot read field data from file \'" << filename << "\'\n";
        H5Dclose(dataset_re);
        H5Dclose(dataset_im);
        H5Fclose(file);
        return false;
    }

    for(int i=0; i<x01*n01; i++)
    {
        E1[i] = re[i] + I*im[i];
    }

    // -------------------------------- CLOSE RESOURCES ---------------------------------
    H5Dclose(dataset_re);
    H5Dclose(dataset_im);
    H5Fclose(file);

    // ----------------------------- INTERPOLATE IF NEEDED ------------------------------
    //double Dr = planes[0]->optic->Dr;
    double r_max = planes[0]->optic->r_max;
    //double Dr1 = r_max1/x01;
    //double Dt1 = (t_max1-t_min1)/n01;

    if(n01!=n0 || x01!=x0)
    {
        std::cout << "Calculation grid mismatch between loaded pulse and current grid\n";
        return false;
    }
    if(abs(r_max1-r_max) > 1e-15)
    {
        std::cout << "Semi diameter mismatch between input beam and first optic in the system\n";
        return false;
    }
    if(abs(t_min1-t_min) > 1e-15 || abs(t_max1-t_max) > 1e-15)
    {
        std::cout << "Time range mismatch between loaded pulse and current grid\n";
        return false;
    }

    /*#pragma omp parallel for
    for(int x=0; x<x0; x++)
    {
        double r = Dr*(0.5+x);
        for(int n=0; n<n0; n++)
        {
            double t = t_min + Dt*(0.5+n);
            if(r>r_max1 || t<t_min1 || t>t_max1)
            {
                E[n0*x+n] = 0;
            }
            else
            {
                int x1 = (int)floor(r/Dr1 - 0.5);
                int x2 = x1+1;
                int n1 = (int)floor((t-t_min1)/Dt1 - 0.5);
                int n2 = n1+1;

                if(x1<0) x1=0;
                if(n1<0) n1=0;
                if(x1>=x01) x1=x01-1;
                if(n1>=n01) n1=n01-1;
                if(x2<0) x2=0;
                if(n2<0) n2=0;
                if(x2>=x01) x2=x01-1;
                if(n2>=n01) n2=n01-1;
                double a = r/Dr1 - (x1+0.5);
                double b = (t-t_min1)/Dt1 - (n1+0.5);
                E[n0*x+n] = E1[n01*x1+n1] * (1-a) * (1-b)
                        + E1[n01*x2+n1] * a     * (1-b)
                        + E1[n01*x1+n2] * (1-a) * b
                        + E1[n01*x2+n2] * a     * b;
            }
        }
    }*/

    // ------------------------------------ SUCCESS! ------------------------------------
    Debug(2, "Pulse read from file done!");
    return true;
}


void Pulse::SaveBeam()
{
    StatusDisplay(nullptr, nullptr, -1, "saving beam profile for pulse " + this->id + "...");

    // use field at pulse time moment closest to t=0
    /*
    int n_zerotime = 0;
    double zerotime = t_min+Dt*0.5;
    for(int n=0; n<n0; n++)
    {
        double t = t_min+Dt*(0.5+n);
        if(fabs(t)<fabs(zerotime))
        {
            zerotime = t;
            n_zerotime = n;
        }
    }
    Debug(2, "zerotime = " + toExpString(n_zerotime));
    Debug(2, "n = " + toString(n_zerotime));*/

    double Dr = planes[planes.size()-1]->optic->Dr; // last optic in the layout

    /*double *Fluence = new double[x0];
    for(int x=0; x<x0; x++)
        Fluence[x] = 0;*/
    std::vector<double> Fluence(x0);

    double Fmax = 0;
    for(int x=0; x<x0; x++)
    {
        for(int n=0; n<n0; n++)
            Fluence[x] += pow(abs(E[n0*x+n]),2); // a.u.
        if(Fluence[x] > Fmax)
            Fmax = Fluence[x];
    }
    for(int x=0; x<x0; x++)
        Fluence[x] /= Fmax; // a.u. normalized


    // ZBF: Using example C program from C:\Users\username\Documents\Zemax\POP\BEAMFILES

    /* set some parameters to use for the beam */
    int i, j, k, nx, ny, is_polarized, unittype;
    double dx, dy, waist, position, lambda, rayleigh, ficl;//, *cax;
    nx = x0*2;
    ny = x0*2;
    dx = Dr*1000; //  this is in mm if unittype is 0
    dy = Dr*1000;
    is_polarized = 0; // use 1 for a polarized beam
    unittype = 0; // millimeters

    waist = Dr*x0/2 * 1000; // assume w = semiDia/2, convert to mm
    position = 0.0;
    lambda = c/vc * 1000; //mm
    rayleigh = M_PI*waist*waist/lambda;
    ficl = 0.0; //  unused for this ZBF

    /* now make the beam */
    //double *cax = new double[2*nx*ny];
    std::vector<double> cax(2*nx*ny);

    k = 0;
    for (j = 0; j < ny; j++)
    {
        for (i = 0; i < nx; i++)
        {
            //std::complex<double> E1 = 0;
            double E1 = 0;
            double x_exact = sqrt(pow(i+0.5-x0,2)+pow(j+0.5-x0,2));
            int x_lo = (int)floor(x_exact);
            int x_hi = (int)ceil(x_exact);
            if( (x_lo < x0) && (x_hi < x0) )
            {
                if(x_lo == x_hi)
                    //E1 = E[x_lo][n_zerotime];
                    E1 = sqrt(Fluence[x_lo]);
                else
                    //E1 = E[x_lo][n_zerotime]*((double)x_hi-x_exact) + E[x_hi][n_zerotime]*(x_exact-(double)x_lo);
                    E1 = sqrt(Fluence[x_lo]*((double)x_hi-x_exact) + Fluence[x_hi]*(x_exact-(double)x_lo));
            }

            cax[k]   = E1;//.real();
            cax[k+1] = 0;//E1.imag();

            k += 2;
        }
    }


    // Write ZBF file
    FILE *out;
    out = fopen((id+".zbf").c_str(), "wb");

    /* write the format version number */
    i = 0;
    fwrite(&i, sizeof(int), 1, out);

    /* write the beam nx size */
    fwrite(&nx, sizeof(int), 1, out);

    /* write the beam ny size */
    fwrite(&ny, sizeof(int), 1, out);

    /* write the ispolarized flag */
    fwrite(&is_polarized, sizeof(int), 1, out);

    /* write out the current units */
    fwrite(&unittype, sizeof(int), 1, out);

    /* 4 unused integers */
    i = 0;
    fwrite(&i, sizeof(int), 1, out);
    fwrite(&i, sizeof(int), 1, out);
    fwrite(&i, sizeof(int), 1, out);
    fwrite(&i, sizeof(int), 1, out);

    /* dx */
    fwrite(&dx, sizeof(double), 1, out);

    /* dy */
    fwrite(&dy, sizeof(double), 1, out);

    /* beam parameters */
    fwrite(&position, sizeof(double), 1, out);
    fwrite(&rayleigh, sizeof(double), 1, out);
    fwrite(&lambda,   sizeof(double), 1, out);
    fwrite(&waist,    sizeof(double), 1, out);

    /* fiber coupling */
    fwrite(&ficl, sizeof(double), 1, out);

    /* 4 unused doubles */
    double x = 0.0;
    fwrite(&x, sizeof(double), 1, out);
    fwrite(&x, sizeof(double), 1, out);
    fwrite(&x, sizeof(double), 1, out);
    fwrite(&x, sizeof(double), 1, out);

    /* now the beam itself */
    //fwrite(cax, sizeof(double), nx*ny*2, out);
    fwrite(cax.data(), sizeof(double), nx*ny*2, out);

    fclose(out);

    // Write ASCII file
    out = fopen((id+".asc").c_str(), "w");
    k = 0;
    for (j = 0; j < ny; j++)
    {
        for (i = 0; i < nx; i++)
        {
            fprintf(out, "%d", (int)(cax[k]*cax[k]*4096));
            k += 2;
            if(i<nx-1)
                fprintf(out, " ");
        }
        fprintf(out, "\n");
    }
    fclose(out);

    //delete[] cax;
    //delete[] Fluence;
}
