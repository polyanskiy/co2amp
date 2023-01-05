#ifndef CO2AMP_H
#define CO2AMP_H

#include <ctime>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <cmath>
#include <complex>
#include <omp.h>
#include "hdf5_hl.h"

#define I std::complex<double>(0,1)


class Plane; // forward-declaration


class Pulse
{
public:
    Pulse(std::string yaml);
    void Initialize(void);
    void Propagate(Plane *from, Plane *to, double time);
    void SavePulse(void); // HDF5 .pulse file
    void SaveBeam(void); // Zemax beam file (.zbf) and ASCII (.asc) beam profile
    std::string id;
    std::string yaml;
    int number;
    double vc;
    double time_in;
    std::complex<double> **E; // field array
private:
    double E0;
    bool LoadPulse(std::string);
};


class Optic
{
public:
    Optic(){}
    virtual void InternalDynamics(double){}
    virtual void PulseInteraction(Pulse*, Plane*, double){}

    std::string type;
    std::string id;
    std::string yaml;         // path to configuration file
    int number;               // number of the optic in the optics list
    double r_max;             // m
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
    A(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
private:
    double length;
    // ------- BANDS -------
    bool band_reg;
    bool band_seq;
    bool band_hot;
    // ------- PUMPING -------
    std::string pumping; // pumping type ("discharge" or "optical")
    std::vector<double> discharge_time;
    std::vector<double> discharge_voltage;
    std::vector<double> discharge_current;
    std::vector<double> pumping_pulse_time;
    std::vector<double> pumping_pulse_intensity;
    double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
    double pump_wl, pump_sigma;//, pump_fluence; // optical pumping parameters
    double q2, q3, q4, qT;
    double q2_a, q3_a, q4_a, qT_a, time_a;
    double q2_b, q3_b, q4_b, qT_b, time_b;
    // ------- GAS MIXTURE -------
    double p_CO2, p_N2, p_He;
    double p_626, p_628, p_828, p_636, p_638, p_838;
    double T0;
    // ------- SPECTROSCOPY -------
    double nop[6][4][3][61];   // normalized populations
    double v[6][4][4][61];     // transition frequencies, Hz
    double sigma[6][4][4][61]; // transition cross-sections, m^2
    // ------- BOLTZMANN -------
    int b0;
    double E_over_N;
    double Y1, Y2, Y3;
    double Du;
    double M1, M2, M3, C1, C2, B;
    double *u;
    double *Q;
    double *Qm1, *Qm2, *Qm3;
    double **Q1, **Q2;
    double u1[11], u2[16];
    double **M;
    double *f;
    // ------- OUTPUT ARRAYS -------
    double *T, *e2, *e3, *e4;
    double *gainSpectrum;

    //////////////////////////// optic_A_band.cpp /////////////////////////////
    void AmplificationBand(void);

    /////////////////////////// optic_A_dynamics.cpp //////////////////////////
    double Current(double);
    double Voltage(double);
    double PumpingPulseIntensity(double);
    double e2e(double);
    void InitializePopulations(void);
    double VibrationalTemperatures(int x, int mode);
    void UpdateDynamicsFiles(double time);

    ///////////////////////// optic_A_amplification.cpp /////////////////////////
    void Amplification(int pulse, int k, double t, int am_section, double length);
    void SaveGainSpectrum(Pulse *pulse, Plane *plane);

    /////////////////////////// optic_A_boltzmann.cpp ///////////////////////////
    void Boltzmann(double);
    void AllocateMemoryBoltzmann(void);
    void FreeMemoryBoltzmann(void);
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
    C(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
private:
    double *Chirpyness; // Chirpiness array (Hz/s)
    void WriteChirpynessFile();
};


class L: public Optic // Lens
{
public:
    L(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
    double F; // focal length, m
};


class M: public Optic // Matter (window, air)
{
public:
    M(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
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
    F(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
private:
    double *Transmittance; // transmittance array
    void WriteTransmittanceFile();
};


class P: public Optic // Probe
{
public:
    P(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
};


class S: public Optic // Spectral filter
{
public:
    S(std::string yaml);
    virtual void InternalDynamics(double time);
    virtual void PulseInteraction(Pulse *pulse, Plane *plane=nullptr, double time=0);
private:
    double *Transmittance; // transmittance array
    void WriteTransmittanceFile();
};



//////////////////////////////////////// GLOGAL STUFF ////////////////////////////////////////

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
bool YamlGetValue(std::string *value, std::string path, std::string key, bool required=true);
bool YamlGetData(std::vector<double> *data, std::string path, std::string key, int column_n);
double Interpolate(std::vector<double> *X, std::vector<double> *Y, double x);
std::string toExpString(double num);
std::string toString(int num);
std::string toString(double num);

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
