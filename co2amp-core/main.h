#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  <complex.h>
#include  <time.h>
#include  <string.h>
#include  <ctype.h>
#include  <omp.h>

#define bool int
#define true 1
#define false 0

// ------- INITIAL PULSE -------
int from_file;
double E0, r0, tau0, vc;
double t_inj;
int n_pulses;
double Dt_train;
// ------- OPTICS, GEOMETRY -------
int n_components, n_amsections, n_propagations;
char **component_id, **component_type, **component_param1, **component_param2;
double *component_Dr;
double *layout_distance, *layout_time;
int *layout_component;
bool noprop;
// ------- PUMPING -------
char pumping[16]; // pumping type ("discharge" or "optical")
int n_discharge_points; // number of pints in the discharge profile
double **discharge; // time current voltage
double Vd, D; // discharge pumping parameters (current and voltage profile is provided in the 'discharge.txt')
double pump_wl, pump_sigma, pump_fluence; // optical pumping parameters
//double C1i, C2i, C3i, C4i, C5i, C6i, C1u, C2u, C3u, C4u;
double q2, q3, q4, qT;
double q2_a, q3_a, q4_a, qT_a, t_a;
double q2_b, q3_b, q4_b, qT_b, t_b;
// ------- GAS MIXTURE -------
double p_CO2, p_N2, p_He;
double p_626, p_628, p_828, p_636, p_638, p_838;
double T0;
// ------- CALCULATION NET -------
double t_pulse_lim, t_pulse_shift;
double Dt_pump; // "main time" - for pumping/relaxation
int x0, n0, K0; // number of points in radial and time nets and number of pulses in the train
// ------- SPECTROSCOPY -------
double v_min, v_max;       // frequency limits, Hz
double nop[6][4][3][61];   // normalized populations
double v[6][4][4][61];     // transition frequencies, Hz
double sigma[6][4][4][61]; // transition cross-sections, m^2
// ------- MISC. CONSTANTS -------
double c, h; // spped of light [m/s]; Plank's [J s]
// ------- OUTPUT ARRAYS -------
double complex ***E;
double **T, **e2, **e3, **e4;
double *gainSpectrum;
// ------- DEBUGGING -------
int debug_level; // debug output control 0 - nothing; 1 - some; 2 - everything
int bands;       // SUMM of 1 for regular + 2 for hot + 4 for sequence
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
//double u1[11] = {0};
//double u2[16] = {0};
double **M;
double *f;


// -------------------------- FUNCTIONS --------------------------

//////////////////////////// co2amp.c ///////////////////////////
void Calculations(void);
void StatusDisplay(int pulse, int k, double t, char *status);
void Debug(int level, char *str);

//////////////////////////// band.c /////////////////////////////
void AmplificationBand(void);

/////////////////////////// dynamics.c //////////////////////////
void PumpingAndRelaxation(double t);
double Current(double);
double Voltage(double);
double e2e(double);
void InitializePopulations(void);
double VibrationalTemperatures(int am_section, int x, int mode);

///////////////////////// amplification.c /////////////////////////
void Amplification(int pulse, int k, double t, int am_section, double length);

/////////////////////////// input.c ////////////////////////////
void ReadCommandLine(int, char**);
void ConstantsInit(void);
void ArraysInit(void);
void IntensityNormalization(void);
void InitializeE(void);
double complex field(double, double);

/////////////////////////// memory.c ///////////////////////////
void AllocateMemory(void);
void FreeMemory(void);
void AllocateMemoryBoltzmann(void);
void FreeMemoryBoltzmann(void);

/////////////////////////// optics.c ///////////////////////////
void BeamPropagation(int pulse, int k, double t);
double RefractiveIndex(char* material, double frequency);
//double GroupIndex(char* material, double frequency);
double NonlinearIndex(char* material);
void Probe();
void Lens(int pulse, double Dr, double F);
void Mask(int pulse, double Dr, double radius);
void Absorber(int pulse, double transmission);
void Window(int pulse, char *material, double thickness);
void Stretcher(int pulse, double stretching);

/////////////////////////// output.c ///////////////////////////
void UpdateOutputFiles(int pulse, int component, double time);
void UpdateDynamicsFiles(double);
void SaveGainSpectrum(int pulse, int component);
void SaveOutputField(void);

/////////////////////////// boltzmann.c ///////////////////////////
void Boltzmann(double);
void WriteEquations(void);
void SolveEquations(void);
void CalculateQ(void);
void InitInputArrays(void);
void InterpolateArray(double*, double*, int, double*);

///////////////////////////// calc.c /////////////////////////////
void FFT(double complex *in, double complex *out);
void IFFT(double complex *in, double complex *out);
void FFTCore(double complex *in, double complex *out, bool Invert);
int BitReversal(int x);
