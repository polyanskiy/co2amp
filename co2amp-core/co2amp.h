#ifndef CO2AMP_H
#define CO2AMP_H

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <cmath>
#include <complex>
#include <omp.h>

//#include <../libyaml/yaml.h>

#define I std::complex<double>(0,1)


class Pulse
{
public:
    Pulse(std::string yaml);
    std::string id;
    std::string yaml;
    int from_file;
    double E0, w0, tau0, vc, t_inj;
    // ------- OUTPUT ARRAY -------
    std::complex<double> **E;
private:
    void IntensityNormalization(void);
    void InitializeE(void);
    std::complex<double> field(double, double);
};


class Optic
{
public:
    Optic(){}
    std::string id;
    std::string type;
    std::string yaml;
    std::string test;
    double CA; // clear aperture, m
    double Dr; // m
    void InternalDynamics(double){}
    void PulseInteraction(int){}
};


class LayoutComponent
{
public:
    LayoutComponent(Optic *optic)
    {
        this->optic = optic;
        this->distance = 0;
        this->time = 0;
    }
    Optic *optic;
    double distance;
    double time;
};



class A: public Optic // Amplifier section
{
public:
    A(std::string yaml);
    void InternalDynamics(double);
    void PulseInteraction(int);
private:
    // ------- BANDS -------
    bool band_reg;
    bool band_seq;
    bool band_hot;
    // ------- PUMPING -------
    std::string pumping; // pumping type ("discharge" or "optical")
    int n_discharge_points; // number of pints in the discharge profile
    double **discharge; // time current voltage
    double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
    double pump_wl, pump_sigma, pump_fluence; // optical pumping parameters
    double q2, q3, q4, qT;
    double q2_a, q3_a, q4_a, qT_a, t_a;
    double q2_b, q3_b, q4_b, qT_b, t_b;
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
    double **T, **e2, **e3, **e4;
    double *gainSpectrum;

    // ------- OTHER VARIABLES -------
    //double humidity; // air humidity [%]




    //////////////////////////// band.cpp /////////////////////////////
    void AmplificationBand(void);

    /////////////////////////// dynamics.cpp //////////////////////////
    void PumpingAndRelaxation(double t);
    double Current(double);
    double Voltage(double);
    double e2e(double);
    void InitializePopulations(void);
    double VibrationalTemperatures(int am_section, int x, int mode);

    ///////////////////////// amplification.cpp /////////////////////////
    void Amplification(int pulse, int k, double t, int am_section, double length);

    /////////////////////////// boltzmann.cpp ///////////////////////////
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
};


class L: public Optic // Lens
{
public:
    L(std::string yaml);
};


class M: public Optic // Matter (window, air)
{
public:
    M(std::string yaml);
};


class F: public Optic // Spatial (ND) filter
{
public:
    F(std::string yaml);
};


class P: public Optic // Probe
{
public:
    P(std::string yaml);
};


class S: public Optic // Spectral filter
{
public:
    S(std::string yaml);
};



//////////////////////////////////////// GLOGAL STUFF ////////////////////////////////////////

// --------------------------------------- VARIABLES -----------------------------------------

// -- PULSES, OPTICS, GEOMETRY ---
extern std::vector<Pulse> pulses;
extern std::vector<Optic> optics;
extern std::vector<LayoutComponent> layout;
extern int n_AM, n_propagations;
extern bool noprop;
// ------- CALCULATION NET -------
extern double vc;                 // center frequency
extern double t_pulse_lim, t_pulse_shift;
extern double Dt_pump;            // "main time" - for pumping/relaxation
extern int x0, n0;                // number of points in radial and time nets
//extern double v_min, v_max;     // frequency limits, Hz
// ------- DEBUGGING -------
extern int debug_level;           // debug output control 0 - nothing; 1 - some; 2 - everything
extern bool flag_status_or_debug; // last message displayed: True if status False if debug
// ------- MISC. CONSTANTS -------
extern double c, h;               // spped of light [m/s]; Plank's [J s]


// --------------------------------------- FUNCTIONS -------------------------------------------

//////////////////////////// main.cpp ///////////////////////////
void Calculations(void);
//void Abort(std::string){}
void StatusDisplay(int pulse, int k, double t, std::string status);
void Debug(int level, std::string str);

/////////////////////////// misc.cpp /////////////////////////////
Optic* FindOpticByID(std::string str);
bool is_number(std::string);
bool YamlGetValue(std::string *value, std::string path, std::string key);

/////////////////////////// input.cpp ////////////////////////////
bool ReadCommandLine(int, char**);
bool ReadConfigFiles(std::string);
bool ReadLayoutConfigFile(std::string);
void ArraysInit(void);

/////////////////////////// memory.cpp ///////////////////////////
void AllocateMemory(void);
void FreeMemory(void);

/////////////////////////// optics.cpp ///////////////////////////
void BeamPropagation(int pulse, int k, double t);
double RefractiveIndex(char* material, double frequency);
double NonlinearIndex(char* material);
void Probe(void);
void Lens(int pulse, double Dr, double F);
void Mask(int pulse, double Dr, double radius);
void Attenuator(int pulse, double transmission);
void Window(int pulse, int k, double t, char *material, double thickness);
void Stretcher(int pulse, double stretching);
void Bandpass(int pulse, double bandcenter, double bandwidth);
void Filter(int pulse, std::string yamlfile);
void Apodizer(int pulse, double alpha);
void Air(int pulse, int k, double t, double humidity, double length);

/////////////////////////// output.cpp ///////////////////////////
void UpdateOutputFiles(int pulse, int component, double time);
void UpdateDynamicsFiles(double);
void SaveGainSpectrum(int pulse, int component);
void SaveOutputField(void);

///////////////////////////// calc.cpp /////////////////////////////
void FFT(std::complex<double> *in, std::complex<double> *out);
void IFFT(std::complex<double> *in, std::complex<double> *out);
void FFTCore(std::complex<double> *in, std::complex<double> *out, bool Invert);
int BitReversal(int x);


#endif // CO2AMP_H
