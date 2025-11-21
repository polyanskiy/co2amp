#ifndef CO2AMP_H
#define CO2AMP_H

#include <ctime>
#include <iostream>
#include <fstream>
#include <regex>
#include <cstring>
#include <array>
#include <vector>
#include <set>
#include <cmath>
#include <complex>
#include <omp.h>
#include "hdf5_hl.h"
#include <filesystem>
#include <unordered_map>
#include <string>


#define I std::complex<double>(0,1)


class Plane; // forward-declaration


class Pulse
{
public:
    Pulse(std::string yaml_path);
    void Initialize(void);
    void Propagate(Plane *from, Plane *to, double time);
    void SavePulse(void); // HDF5 .pulse file
    void SaveBeam(void); // Zemax beam file (.zbf) and ASCII (.asc) beam profile
    std::string id;
    std::string yaml_path;    // path to configuration file
    std::string yaml_content; // content of configuration file
    int number;
    double vc;
    double time_in;
    std::vector<std::complex<double>> E; // field array
private:
    double E0;
    bool LoadPulse(std::string);
};


class Optic
{
public:
    Optic(std::string id, std::string type);
    virtual void Initialize(void){}
    virtual void InternalDynamics(int){}
    virtual void PulseInteraction(Pulse*, Plane*, int, int, int){}

    std::string type;
    std::string id;
    std::string yaml_path;    // path to configuration file
    std::string yaml_content; // content of configuration file
    int number;               // number of the optic in the optics list
    double r_max;             // semi-diameter
    double Dr;                // raddial coordinate step
};


class Plane // Layout component
{
public:
    Plane(Optic *optic)
    {
        this->optic = optic;
        this->space = 0;
    }
    Optic *optic;
    double space;
    double time_from_first_plane;
    int number;
};


class A: public Optic // Amplifier section
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
private:
    // ---------- FLAGS ----------
    bool flag_interaction; // true if a pulse is interacting with the amplifier section
    // -------- CONSTANTS --------
    static constexpr int NumIso = 12; // Number of isotopologues
    static constexpr int NumGrp = 10; // Groups of vibrational levels (for Boltzman distribution calculations)
    static constexpr int NumVib = 18; // Vibrational levels
    static constexpr int NumRot = 80; // Rotational sub-levels with J = 0...79
    // -------- GEOMETRY ---------
    double length;
    // ------- GAS MIXTURE -------
    double p_CO2, p_N2, p_He;
    double p_iso[NumIso];
    double N_CO2, N_iso[NumIso]; // Number densies of CO2 and its isotopologues, 1/m^3
    double T0;
    // ---------- BANDS ----------
    bool band_reg;
    bool band_seq;
    bool band_hot;
    bool band_4um;
    // --------- PUMPING ---------
    std::string pumping;   // pumping type ("discharge" or "optical")
    double save_interval;  // time interval between data points in e & T files (default 1e-9 s)
    double solve_interval; // time interval between re-solving Boltzman equation (default 25e-9 s)
    // for discharge
    std::vector<double> voltage;
    std::vector<double> current;
    double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
    std::vector<double> q2, q3, q4, qT;
    // for optical
    std::vector<double> normalized_intensity;
    std::vector<double> fluence;
    std::string pump_level; // energy level for optical pumping
                            // "001": direct pumping @ ~4.3 um
                            // "021": combination (101+021) vibration @ ~2.8 um
                            // "002": overtone pumping @ ~2.2 um (only possible for non-symmetric molecules like 628
                            // "041": combination (201+121+041) vibration @ ~ 2.0 um
                            // "003" overtone pumping @ ~1.4 um
    double pump_wl, pump_sigma; // optical pumping parameters

    // ------- SPECTROSCOPY, TEMPERATURES, POPULATIONS ------

    std::vector<double> T, e2, e3, e4;

    // Population number densities (for each coordinate)
    std::vector<double> N_grp[NumIso][NumGrp];         // Groups of vibrational levels
    std::vector<double> N_vib[NumIso][NumVib];         // Vibrational levels
    std::vector<double> N_rot[NumIso][NumVib][NumRot]; // Rotational levels

    // Fractional populations of rotational sublevels
    // (normalized Boltzmann distribution)
    double f_rot[NumIso][NumVib][NumRot];

    std::vector<double> v[NumIso];     // transition frequencies, Hz
    std::vector<double> sigma[NumIso]; // transition cross-sections, m^2
    std::vector<int> vl_up[NumIso];    // upper vibrational level of the transition (see initialization for numbering)
    std::vector<int> vl_lo[NumIso];    // lower vibrational level of the transition
    std::vector<int> j_up[NumIso];     // rotational quantum number of the upper level of the transition
    std::vector<int> j_lo[NumIso];     // rotational quantum number of the lower level of the transition
    std::vector<double> gainSpectrum;
    std::vector<std::complex<double>> rho[NumIso];

    // -------- BOLTZMANN --------
    static constexpr int b0 = 1024;  // Number of points in calculations

    /////////////////////////////// optic_A.cpp ///////////////////////////////
    void InitializePumpPulse(void);
    void WritePumpingFiles(void);

    //////////////////////////// optic_A_band.cpp /////////////////////////////
    void AmplificationBand(void);

    /////////////////////////// optic_A_dynamics.cpp //////////////////////////
    double e2e(double);
    void InitializePopulations(void);
    double VibrationalTemperatures(int x, int mode);
    void Update_eT_Files(int m);

    ///////////////////////// optic_A_amplification.cpp /////////////////////////
    void SaveGainSpectrum(Pulse *pulse, Plane *plane);

    /////////////////////////// optic_A_boltzmann.cpp ///////////////////////////
    void Boltzmann(int, double[5]);
    void WriteAndSolveEquations(int, double, bool, double*, double*);
    void InterpolateArray(double*, double*, int, double, double*);
    //void Save_f(double, double*); //debug (test Boltzmann solver)
};


class C: public Optic // Chirp (Stretcher/Compressor)
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
private:
    std::vector<double> Chirp; // Chirp array (Hz/s) in frequency domain
    void WriteChirpFile();
};


class L: public Optic // Lens
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
    double F; // focal length, m
};


class M: public Optic // Matter (window, air)
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
private:
    // ------- GENERAL -------
    std::string material;
    double thickness; // m
    double tilt;      // radians, default=0
    int slices;       // number of slices (more slices - better accuracy)
    // ------- REFRACTION -------
    double humidity;  // %, only for air, default=50
    double n2;        // optional - nonlinear index m^2/W - use NonlinearIndex() function if not set
    //double n4;        // optional - next-order nonlinearity index m^4/W^2 (0 if not set)
    // ------- ABSORPTION -------
    double alpha0;    // optional - linear absorption coefficient 1/m
    /*
    double chi;       // optional - nonlinear absorption order
    double alpha1;    // optional - multiphoton absorption coefficient m^2/W / m^(1/chi)
    double alpha2;    // optional - linear absorption in conduction band
    */

    //double **excited; // a number proportional to density of conduction electrons

    double RefractiveIndex(double nu);
    double GroupIndex(double nu);
    double NonlinearIndex();
    double AbsorptionCoefficient(double nu);
    /*
    double MultiphotonAbsorptionCoefficient1();
    double MultiphotonAbsorptionCoefficient2();
    double MultiphotonAbsorptionOrder();
    */
};


class F: public Optic // Spatial (ND) filter
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
private:
    double *Transmittance; // transmittance array
    void WriteTransmittanceFile();
};


class P: public Optic // Probe
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
};


class S: public Optic // Spectral filter
{
public:
    using Optic::Optic;
    virtual void Initialize(void);
    virtual void InternalDynamics(int m);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, int m=0, int n_min=0, int n_max=0);
private:
    double *Transmittance; // transmittance array
    void WriteTransmittanceFile();
};



//////////////////////////////////////// GLOBAL STUFF ////////////////////////////////////////

// --------------------------------------- VARIABLES -----------------------------------------

// --- PULSES, OPTICS, GEOMETRY ----
extern std::vector<Pulse*> pulses;
extern std::vector<Optic*> optics;
extern std::vector<Plane*> planes;
// ------- CALCULATION GRID --------
extern double v0;                 // central frequency
extern double t_min, t_max;       // pulse (fast) time limits
extern double time_tick;          // lab (slow) time step
extern int x0, n0;                // number of points in radial and time grids
extern int m0;                    // number of points in lab (slow) time grid
// --- CALCULATED GRID PARAMETERS --
// *** Be very careful not to confuse limits and values in the outer bins of the grid ***
// t[0] = t_min+0.5*Dt;   t[n] = t_min+(n+0.5)*Dt;   t[n0-1] = t_min+(n0-0.5)*Dt = t_max-0.5*Dt
// v[0] = v_min+0.5*Dv;   v[n] = v_min+(n+0.5)*Dv;   v[n0-1] = v_min+(n0-0.5)*Dv = v_max-0.5*Dv
extern double v_min;// v_max;     // frequency domain limits
extern double Dt, Dv;             // time and frequency steps
// ----------- DEBUGGING -----------
extern int debug_level;           // debug output control 0: nothing; 1: some; 2: a lot; 3: everything
extern bool flag_status_or_debug; // last message displayed: True if status False if debug
// --- MISC CONSTANTS AND FLAGS ----
extern double c, h;               // spped of light [m/s]; Plank's [J s]
extern bool configuration_error;  // true if error in a configuration file is detected
extern std::string search_dir;    // additional directory for HDF5 pulse files
extern int method;                // propagation method
                                  // 0: no beam variation due to proparation (old "-noprop" parameter)
                                  // 1: Fresnel (default)
                                  // 2: Rayleigh-Sommerfeld


// --------------------------------------- FUNCTIONS -------------------------------------------

//////////////////////////// main.cpp ///////////////////////////
void Calculations(void);
void StatusDisplay(Pulse *pulse, Plane *plane, int m, std::string status);
void Debug(int level, std::string str);

/////////////////////////// misc.cpp /////////////////////////////
Optic* FindOpticByID(std::string str);
bool is_number(std::string);
bool YamlReadFile(std::string path, std::string *yaml_file_content);
bool YamlGetValue(std::string *value, std::string *yaml_file_content, std::string key, bool required=true);
bool YamlGetData(std::vector<double> *data, std::string *yaml_file_content, std::string key, int column_n);
double Interpolate(std::vector<double> *X, std::vector<double> *Y, double x);
std::string toExpString(double num);
std::string toString(int num);
std::string toString(double num);
void UnwrapPhase(Pulse* pulse, int x, double* phase);

/////////////////////////// input.cpp ////////////////////////////
std::string ReadCommandLine(int, char**);
bool ReadConfigFiles(std::string);
bool ReadLayoutConfigFile(std::string);

/////////////////////////// output.cpp ///////////////////////////
void UpdateOutputFiles(Pulse *pulse, Plane *plane, double time);
void SaveOutputField(void);

///////////////////////////// calc.cpp /////////////////////////////
void FFT(std::complex<double> *in, std::complex<double> *out);
void IFFT(std::complex<double> *in, std::complex<double> *out);
void FFTCore(std::complex<double> *in, std::complex<double> *out, bool Invert);
int BitReversal(int x);

#endif // CO2AMP_H
