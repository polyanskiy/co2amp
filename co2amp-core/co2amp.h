#ifndef CO2AMP_H
#define CO2AMP_H


//#include  <stdio.h>
//#include  <stdlib.h>
//#include  <math.h>
//#include <string>
#include <iostream>
#include <fstream>
#include<regex>
#include <vector>
//#include <ostream>
#include  <cmath>
#include  <complex>
//#include  <time.h>
//#include  <string.h>
//#include  <ctype.h>
#include  <omp.h>

#include <../libyaml/yaml.h>

#define bool int
#define true 1
#define false 0
#define I std::complex<double>(0,1)

//using namespace std; //this is considered a bad practice. Use std::... all the time instead

// ------- INITIAL PULSE -------
extern int from_file;
extern double E0, w0, tau0, vc;
extern double t_inj;
extern int n_pulses;
extern double Dt_train;
// ------- OPTICS, GEOMETRY -------
extern int n_AM, n_propagations;
//extern int n_components, n_propagations;
//extern int n_A, n_AF, n_AM, n_AT, n_BF, n_F, n_L, n_M, n_P, n_S, n_SF, n_W;
//extern std::string *component_id, *component_type, *component_yaml;
//extern double *component_Dr;
extern double *layout_distance, *layout_time;
extern int *layout_component;
extern bool noprop;
//extern double *alpha;  // temporary - for nonlinear absorption in Ge
// ------- PUMPING -------
extern std::string pumping; // pumping type ("discharge" or "optical")
extern int n_discharge_points; // number of pints in the discharge profile
extern double **discharge; // time current voltage
extern double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
extern double pump_wl, pump_sigma, pump_fluence; // optical pumping parameters
extern double q2, q3, q4, qT;
extern double q2_a, q3_a, q4_a, qT_a, t_a;
extern double q2_b, q3_b, q4_b, qT_b, t_b;
// ------- GAS MIXTURE -------
extern double p_CO2, p_N2, p_He;
extern double p_626, p_628, p_828, p_636, p_638, p_838;
extern double T0;
// ------- CALCULATION NET -------
extern double t_pulse_lim, t_pulse_shift;
extern double Dt_pump; // "main time" - for pumping/relaxation
extern int x0, n0, K0; // number of points in radial and time nets and number of pulses in the train
// ------- SPECTROSCOPY -------
extern double v_min, v_max;       // frequency limits, Hz
extern double nop[6][4][3][61];   // normalized populations
extern double v[6][4][4][61];     // transition frequencies, Hz
extern double sigma[6][4][4][61]; // transition cross-sections, m^2
// ------- OUTPUT ARRAYS -------
extern std::complex<double> ***E;
extern double **T, **e2, **e3, **e4;
extern double *gainSpectrum;
// ------- DEBUGGING -------
extern int debug_level; // debug output control 0 - nothing; 1 - some; 2 - everything
extern int bands;       // SUMM of 1 for regular + 2 for hot + 4 for sequence
extern bool flag_status_or_debug; // last message displayed: True if status False if debug
// ------- BOLTZMANN -------
extern int b0;
extern double E_over_N;
extern double Y1, Y2, Y3;
extern double Du;
extern double M1, M2, M3, C1, C2, B;
extern double *u;
extern double *Q;
extern double *Qm1, *Qm2, *Qm3;
extern double **Q1, **Q2;
extern double u1[11], u2[16];
extern double **M;
extern double *f;
// ------- MISC. CONSTANTS -------
extern double c, h; // spped of light [m/s]; Plank's [J s]
// ------- MISC. GLOBAL VARIABLES -------
extern double humidity; // air humidity [%]




// -------------------------- FUNCTIONS --------------------------

//////////////////////////// main.cpp ///////////////////////////
void Calculations(void);
//void StatusDisplay(int pulse, int k, double t, char *status);
//void StatusDisplay(int pulse, int k, double t, const char *status);
void StatusDisplay(int pulse, int k, double t, std::string status);
//void Debug(int level, char *str);
//void Debug(int level, const char *str);
void Debug(int level, std::string str);

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

/////////////////////////// input.cpp ////////////////////////////
void ReadCommandLine(int, char**);
void ConstantsInit(void);
void ArraysInit(void);
void IntensityNormalization(void);
void InitializeE(void);
std::complex<double> field(double, double);

/////////////////////////// memory.cpp ///////////////////////////
void AllocateMemory(void);
void FreeMemory(void);
void AllocateMemoryBoltzmann(void);
void FreeMemoryBoltzmann(void);

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

/////////////////////////// boltzmann.cpp ///////////////////////////
void Boltzmann(double);
void WriteEquations(void);
void SolveEquations(void);
void CalculateQ(void);
void InitInputArrays(void);
void InterpolateArray(double*, double*, int, double*);
void Save_f(void); //debug (test Boltzmann solver)

///////////////////////////// calc.cpp /////////////////////////////
void FFT(std::complex<double> *in, std::complex<double> *out);
void IFFT(std::complex<double> *in, std::complex<double> *out);
void FFTCore(std::complex<double> *in, std::complex<double> *out, bool Invert);
int BitReversal(int x);

//////////////////////////// yaml.cpp ///////////////////////////
//void YamlGetValue(char *value, char* path, char* key);
void YamlGetValue(std::string *value, std::string path, std::string key);


class Optic
{
public:
    std::string id;
    std::string type;
    std::string yaml;
    std::string test;
    double Dr; //m
    Optic(){}
};

extern std::vector<Optic> optics;


class A: public Optic
{
public:
    A(std::string id, std::string type, std::string yaml);
};


class AF: public Optic
{
public:
    AF(std::string id, std::string type, std::string yaml);
};


class AM: public Optic
{
public:
    AM(std::string id, std::string type, std::string yaml);
};


class AT: public Optic
{
public:
    AT(std::string id, std::string type, std::string yaml);
};


class BF: public Optic
{
public:
    BF(std::string id, std::string type, std::string yaml);
};


#endif // CO2AMP_H



