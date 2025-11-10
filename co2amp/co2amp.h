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
    Optic(){}
    virtual void InternalDynamics(double){}
    virtual void PulseInteraction(Pulse*, Plane*, double, int, int){}

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
    A(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
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
    std::string pumping; // pumping type ("discharge" or "optical")
    // for discharge
    std::vector<double> discharge_time;
    std::vector<double> discharge_voltage;
    std::vector<double> discharge_current;
    double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
    double q2, q3, q4, qT;
    double q2_a, q3_a, q4_a, qT_a, time_a;
    double q2_b, q3_b, q4_b, qT_b, time_b;
    // for optical
    std::vector<double> pump_pulse_time;
    std::vector<double> pump_pulse_intensity;
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
    static constexpr int b0 = 1024; // Number of points in calculations
    //int b0;
    double E_over_N;
    double Y1, Y2, Y3;
    double Du;
    double M1, M2, M3, C1, C2, B;
    /*double *u;
    double *Q;
    double *Qm1, *Qm2, *Qm3;
    double **Q1, **Q2;
    double u1[11], u2[16];
    double **M;
    double *f;*/
    double u[b0];
    double Q[b0];
    double Qm1[b0], Qm2[b0], Qm3[b0];
    double Q1[11][b0], Q2[16][b0];
    double u1[11], u2[16];
    double M[b0][b0];
    double f[b0];

    //////////////////////////// optic_A_band.cpp /////////////////////////////
    void AmplificationBand(void);

    /////////////////////////// optic_A_dynamics.cpp //////////////////////////
    double Current(double);
    double Voltage(double);
    double PumpPulseIntensity(double);
    double e2e(double);
    void InitializePopulations(void);
    double VibrationalTemperatures(int x, int mode);
    void UpdateDynamicsFiles(double time);

    ///////////////////////// optic_A_amplification.cpp /////////////////////////
    void Amplification(int pulse, int k, double t, int am_section, double length);
    void SaveGainSpectrum(Pulse *pulse, Plane *plane);

    /////////////////////////// optic_A_boltzmann.cpp ///////////////////////////
    void Boltzmann(double);
    //void AllocateMemoryBoltzmann(void);
    //void FreeMemoryBoltzmann(void);
    void WriteEquations(void);
    void SolveEquations(void);
    void CalculateQ(void);
    void InitInputArrays(void);
    void InterpolateArray(double*, double*, int, double*);
    void Save_f(void); //debug (test Boltzmann solver)
};


class C: public Optic // Chirp (Stretcher/Compressor)
{
public:
    C(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
private:
    std::vector<double> Chirp; // Chirp array (Hz/s) in frequency domain
    void WriteChirpFile();
};


class L: public Optic // Lens
{
public:
    L(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
    double F; // focal length, m
};


class M: public Optic // Matter (window, air)
{
public:
    M(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
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
    F(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
private:
    double *Transmittance; // transmittance array
    void WriteTransmittanceFile();
};


class P: public Optic // Probe
{
public:
    P(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
};


class S: public Optic // Spectral filter
{
public:
    S(std::string yaml_path);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0, int n_min=0, int n_max=0);
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
extern int save_interval;         // # of ticks between data entries in dynamics files
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
void StatusDisplay(Pulse *pulse, Plane *plane, double time, std::string status);
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
void UnwrapPhase(const std::complex<double>* Field, double* Phase);

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
//void FFTCore(std::complex<double> *out, bool Invert);
int BitReversal(int x);
//void Rearrange(std::complex<double> *in, std::complex<double> *out);


#endif // CO2AMP_H
