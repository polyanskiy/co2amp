#include "co2amp.h"


void A::Boltzmann(double time)
{
    int i;
    double u_lim;

    Y1 = p_CO2/(p_CO2+p_N2+p_He);
    Y2 = p_N2/(p_CO2+p_N2+p_He);
    Y3 = p_He/(p_CO2+p_N2+p_He);

    double E = Voltage(time)/(D*1e2); // Electric field stretch, V/cm
    double N = 2.7e19*(p_CO2+p_N2+p_He); // Total molecule number density, 1/cm^3
    E_over_N = E/N*1e16;

    M1 = 44.0;
    M2 = 28.0;
    M3 = 4.0;
    C1 = 8.2e-4;
    C2 = 5.06e-4;
    B = 2.5e-4;

    b0 = 1024;
    AllocateMemoryBoltzmann();

    ////////////////////////// Stage 1: coarse net //////////////////////
    u_lim = 1000;
    Du = u_lim/(b0-1); // Electron energy net step, eV
    InitInputArrays();
    WriteEquations(); // fill out the M matrix
    SolveEquations();

    ////////////////////////// Stage 2: fine net //////////////////////
    for(i=0; i<b0; i++)
    {
        if(f[1]/f[i] >= 1e6)
        {
            u_lim = Du*i;
            break;
        }
    }

    Du = u_lim/(b0-1); // Electron energy net step, eV
    InitInputArrays();
    WriteEquations(); // fill out the M matrix
    SolveEquations();
    CalculateQ();
    //Save_f(); //test Boltzmann solver
    FreeMemoryBoltzmann();
}



void A::WriteEquations(void)
{
    int i, j, k;

    for(i=0; i<b0; i++)
        u[i] = i*Du;

    for(i=0; i<b0; i++)
    {
        for(j=0; j<b0; j++)
            M[i][j] = 0;
        // Coeffitients of f_i
        M[i][i] += -u[i]*Y1 * ( Q1[1][i]+Q1[2][i]+Q1[3][i]+Q1[4][i]+Q1[5][i]+Q1[6][i]+Q1[7][i]+Q1[8][i]+Q1[9][i]+Q1[10][i] );
        M[i][i] += -u[i]*Y2 * (	Q2[1][i]+Q2[2][i]+Q2[3][i]+Q2[4][i]+Q2[5][i]+Q2[6][i]+Q2[7][i]+Q2[8][i]+Q2[9][i]+Q2[10][i]+Q2[11][i]+Q2[12][i]+Q2[13][i]+Q2[14][i]+Q2[15][i] );
        if(i < b0-1)
            M[i][i] += -1.0/3*pow(E_over_N,2)/4/pow(Du,2) * u[i+1]/(Y1*Qm1[i+1] + Y2*Qm2[i+1] + Y3*Qm3[i+1]);
        if(i > 0)
            M[i][i] += -1.0/3*pow(E_over_N,2)/4/pow(Du,2) * u[i-1]/(Y1*Qm1[i-1] + Y2*Qm2[i-1] + Y3*Qm3[i-1]);
        // Coefficients of f_[i+uxx/Du]
        for(k=1; k<11; k++)
        {
            j = (int)floor((u[i]+u1[k])/Du + 0.5);
            if(j <= b0-1)
                M[i][j] += Y1*u[j]*Q1[k][j];
        }
        for(k=1; k<16; k++)
        {
            j = (int)floor((u[i]+u2[k])/Du + 0.5);
            if(j <= b0-1)
                M[i][j] += Y2*u[j]*Q2[k][j];
        }
        // Coefficients of f_[i+1]
        if(i+1 < b0)
            M[i][i+1] += 1.09e-3/2/Du*pow(u[i+1],2)*(Y1/M1*Qm1[i+1] + Y2/M2*Qm2[i+1] + Y3/M3*Qm3[i+1]) + Y1*C1*u[i+1]/2/Du + Y2*C2*u[i+1]/2/Du + 6*B*Y2*u[i+1]/2/Du*Q[i+1];
        // Coefficients of f_[i-1]
        if(i-1 >= 0)
            M[i][i-1] += -1.09e-3/2/Du*pow(u[i-1],2)*(Y1/M1*Qm1[i-1] + Y2/M2*Qm2[i-1] + Y3/M3*Qm3[i-1]) - Y1*C1*u[i-1]/2/Du - Y2*C2*u[i-1]/2/Du - 6*B*Y2*u[i-1]/2/Du*Q[i-1];
        // Coefficients of f_[i+2]
        if(i+2 < b0)
            M[i][i+2] += 1.0/3*pow(E_over_N,2)/4/pow(Du,2) * u[i+1]/(Y1*Qm1[i+1] + Y2*Qm2[i+1] + Y3*Qm3[i+1]);
        // Coefficients of f_[i-2]
        if(i-2 >=0)
            M[i][i-2] += 1.0/3*pow(E_over_N,2)/4/pow(Du,2) * u[i-1]/(Y1*Qm1[i-1] + Y2*Qm2[i-1] + Y3*Qm3[i-1]);
    }
}


void A::SolveEquations(void)
{
    int i, j, jj;
    double a;

    for(i=0; i<b0; i++)
            f[i] = 0;

    for(j=1; j<b0; j++)
    {
        if(M[j-1][j-1]==0.0)
        {
            printf("!%d\n", j-1);
            jj = j;
            while(M[jj][jj]==0.0 && jj<b0-1)
                jj++;
            for(i=0; i<b0; i++) // swap j and jj lines;
            {
                a = M[j-1][i];
                M[j-1][i] = M[jj][i];
                M[jj][i] = a;
            }
        }
        if(M[j-1][j-1]==0.0)
        {
            printf("Error !!!!!!!!!!!!!!!!!!!%d\n", j-1);
            fflush(stdout);
        }
        for(jj=j; jj<b0; jj++)
        {
            a = M[jj][j-1] / M[j-1][j-1];
            //#pragma omp parallel for shared(M) private(i) // multithreaded
            for(i=0; i<b0; i++)
                M[jj][i] -= a*M[j-1][i];
        }
    }

    f[b0-1] = 1e-6;
    for(i=b0-2; i>=0; i--)
    {
        for(j=i+1; j<b0; j++)
            f[i] -= f[j]*M[i][j];
        f[i] /= M[i][i];
    }

    // Normalization
    a=0;
    for(i=0; i<b0; i++)
        a += f[i]*Du;
    for(i=0; i<b0; i++)
        f[i] /= a;
}


void A::CalculateQ(void)
{
    int i, k;
    double v = 0; // electron speed, cm/s
    // CO2 excitation
    double w1[11], z1[11];
    for(k=1; k<11; k++)
        w1[k] = 0;
    // N2 excitation
    double w2[16], z2[16];
    for(k=1; k<16; k++)
        w2[k] = 0;
    // translation
    double zt = 0;
    // rotations
    double zr = 0;

    for(i=0; i<b0-1; i++)
        v += -5.93e7 * 1.0/3.0*E_over_N * Du* Du*(i+0.5) * 1.0/(Y1*(Qm1[i]+Qm1[i+1])/2.0 + Y2*(Qm2[i]+Qm2[i+1])/2.0 + Y3*(Qm3[i]+Qm3[i+1])/2.0) * (f[i+1]-f[i])/Du;

    for(i=0; i<b0-1; i++)
    {
        for(k=1; k<11; k++)
            w1[k] += 5.93e-9 * Du * Du*(i+0.5) * (Q1[k][i]+Q1[k][i+1])/2.0 * (f[i]+f[i+1])/2.0;
        for(k=1; k<16; k++)
            w2[k] += 5.93e-9 * Du * Du*(i+0.5) * (Q2[k][i]+Q2[k][i+1])/2.0 * (f[i]+f[i+1])/2.0;

        zt += 5.93e7 * 1.09e-3 * Du * pow(Du*(i+0.5),2) *
                        (Y1/M1*(Qm1[i]+Qm1[i+1])/2.0 + Y2/M2*(Qm2[i]+Qm2[i+1])/2.0 + Y3/M3*(Qm3[i]+Qm3[i+1])/2.0) /
                        E_over_N / v * (f[i]+f[i+1])/2.0;

        zr += 5.93e7 * (Y1*C1+Y2*C2) * Du * Du*(i+0.5) * (f[i]+f[i+1])/2.0 / E_over_N / v;
        zr += 5.93e7 * 6.0*Y2*B * Du * Du*(i+0.5) * (Q[i]+Q[i+1])/2.0 * (f[i]+f[i+1])/2.0 / E_over_N / v;
    }

    for(k=1; k<11; k++)
        z1[k] = 1e16 * Y1 * u1[k] * w1[k] / E_over_N / v;

    for(k=1; k<16; k++)
        z2[k] = 1e16 * Y2 * u2[k] * w2[k] / E_over_N / v;

    // N2
    q4 = 0;
    for(k=1; k<9; k++)
        q4 += z2[k];

    // 000 -> 001
    q3 = z1[7];

    // 000 -> 010: z1[1]
    // 000 -> (100+020): z1[2]
    // 000 -> (0n0+n00): z1[3]...z1[6]
    q2 = 0;
    for(k=1; k<7; k++)
        q2 += z1[k];

    qT = zt + zr;

    // electron excitation and ionization
    double qEI = 0;
    for(k=9; k<16; k++)
        qEI += z2[k];
    for(k=8; k<11; k++)
        qEI += z1[k];

    // Fine-tune Q's (q >= 0)
    if(q2<0)
        q2 = 0;
    if(q3<0)
        q3 = 0;
    if(q4<0)
        q4 = 0;
    if(qT<0)
        qT = 0;

    double sum=q2+q3+q4+qT+qEI;
    if(sum>1) // Fine-tune Q's (Summ = 1)
    {
        q2 /= sum;
        q3 /= sum;
        q4 /= sum;
        qT /= sum;
        qEI /= sum;
    }
}



// Create fine array from coars input crossection data by linear interpolation
void A::InterpolateArray(double* input_x, double* input_y, int input_size, double* output_array)
{
    int i, j;
    double x, x1, x2, y1, y2;

    int output_size = b0;

    j=0;


    while((double)j*Du <= input_x[0])
    {
        output_array[j] = input_y[0];
        j++;
        if(j >= output_size)
            return;
    }

    for(i=1; i<=input_size-1; i++)
    {
        while((double)j*Du <= input_x[i])
        {
            x1 = input_x[i-1];
            x2 = input_x[i];
            y1 = input_y[i-1];
            y2 = input_y[i];
            x = (double)j*Du;

            output_array[j] = y1 + (y2-y1)/(x2-x1)*(x-x1);

            j++;
            if(j >= output_size)
                return;
        }
    }

    while(j < output_size)
    {
        output_array[j] = input_y[input_size-1];
        j++;
    }
}

void A::InitInputArrays(void)
{
    //--------------------- Q ---------------------
    double Q_x[] = {0.0015,	0.050,	0.250,	0.500,	0.800,	1.000,	1.500,	1.800,	1.900,	2.000,	2.150,	2.430,	2.600,	2.750,	2.900,	3.250,	3.600,	4.000,	4.500,	5.000,	5.500,	7.000,	9.000,	11.000,	15.000,	22.000,	25.000};
    double Q_y[] = {0.000,	0.100,	0.650,	1.150,	2.000,	2.650,	5.600,	7.500,	8.200,	8.600,	8.950,	9.000,	8.900,	8.400,	7.650,	6.200,	5.100,	4.500,	4.160,	3.970,	3.930,	4.170,	4.460,	4.420,	3.940,	3.150,	3.050};
    InterpolateArray(Q_x, Q_y, sizeof(Q_x)/sizeof(double), Q);

    //--------------------- Qm1 ---------------------
    double Qm1_x[] = {0.000,	0.040,	0.100,	0.300,	0.500,	0.600,	1.000,	1.700,	2.000,	2.500,	3.000,	4.100,	5.000,	7.400,	10.000,	20.000,	27.000,	50.000};
    double Qm1_y[] = {140.000,	84.000,	55.000,	21.000,	10.800,	9.400,	5.700,	5.000,	5.100,	6.000,	7.700,	9.400,	14.500,	10.000,	11.700,	16.000,	16.300,	13.000};
    InterpolateArray(Qm1_x, Qm1_y, sizeof(Qm1_x)/sizeof(double), Qm1);

    //--------------------- Qm2 ---------------------
    double Qm2_x[] = {0.000,	0.001,	0.002,	0.008,	0.010,	0.040,	0.080,	0.100,	0.200,	0.300,	0.400,	1.000,	1.200,	1.400,	1.800,	2.000,	2.500,	3.000,	4.000,	5.000,	7.000,	10.000,	14.000,	18.000,	20.000,	30.000,	100.000};
    double Qm2_y[] = {1.400,	1.400,	1.600,	2.000,	2.200,	4.000,	6.000,	6.500,	8.800,	9.800,	10.000,	10.000,	11.000,	12.500,	20.000,	25.000,	30.000,	26.000,	15.000,	12.000,	10.000,	10.000,	11.000,	12.200,	12.000,	10.000,	10.000};
    InterpolateArray(Qm2_x, Qm2_y, sizeof(Qm2_x)/sizeof(double), Qm2);

    //--------------------- Qm3 ---------------------
    double Qm3_x[] = {0.000,	0.010,	0.100,	0.200,	1.000,	2.000,	7.000,	10.000,	20.000};
    double Qm3_y[] = {5.000,	5.400,	5.800,	6.200,	6.500,	6.100,	5.000,	4.100,	3.000};
    InterpolateArray(Qm3_x, Qm3_y, sizeof(Qm3_x)/sizeof(double), Qm3);

    //--------------------- Q11 ---------------------
    u1[1] = 0.083;
    double Q11_x[] = {0.083,	0.085,	0.090,	0.100,	0.120,	0.140,	0.160,	0.200,	0.300,	0.400,	0.500,	0.600,	0.800,	1.000,	1.200,	1.600,	1.800,	2.000,	2.500,	3.000,	3.700,	4.000,	4.200,	4.500,	5.000,	6.000,	8.000,	9.000,	10.000,	10.100};
    double Q11_y[] = {0.000,	0.360,	1.040,	1.600,	1.840,	2.120,	2.160,	2.080,	1.760,	1.520,	1.280,	1.080,	0.800,	0.580,	0.480,	0.340,	0.350,	0.400,	0.640,	1.040,	1.400,	1.360,	1.200,	0.920,	0.530,	0.400,	0.360,	0.280,	0.160,	0.000};
    InterpolateArray(Q11_x, Q11_y, sizeof(Q11_x)/sizeof(double), Q1[1]);

    //--------------------- Q12 ---------------------
    u1[2] = 0.167;
    double Q12_x[] = {0.167,	0.200,	0.250,	0.300,	0.500,	0.700,	1.000,	1.400,	2.000,	3.000,	3.900,	4.500,	5.000,	6.000,	10.000,	20.000,	30.000};
    double Q12_y[] = {0.000,	0.540,	0.820,	0.820,	0.680,	0.560,	0.470,	0.450,	0.550,	1.150,	1.830,	1.400,	0.400,	0.280,	0.200,	0.100,	0.000};
    InterpolateArray(Q12_x, Q12_y, sizeof(Q12_x)/sizeof(double), Q1[2]);

    //--------------------- Q13 ---------------------
    u1[3] = 0.252;
    double Q13_x[] = {0.252,	2.700,	3.000,	3.300,	3.600,	4.500,	4.600,	5.000};
    double Q13_y[] = {0.000,	0.250,	0.400,	0.600,	0.650,	0.230,	0.100,	0.000};
    InterpolateArray(Q13_x, Q13_y, sizeof(Q13_x)/sizeof(double), Q1[3]);

    //--------------------- Q14 ---------------------
    u1[4] = 0.339;
    double Q14_x[] = {2.370,	3.000,	3.500,	4.000,	4.500,	4.600,	5.000};
    double Q14_y[] = {0.000,	0.260,	0.520,	0.500,	0.220,	0.100,	0.000};
    InterpolateArray(Q14_x, Q14_y, sizeof(Q14_x)/sizeof(double), Q1[4]);

    //--------------------- Q15 ---------------------
    u1[5] = 0.422;
    double Q15_x[] = {2.370,	3.000,	3.650,	3.800,	4.000,	4.300,	5.000};
    double Q15_y[] = {0.000,	0.170,	0.330,	0.310,	0.210,	0.100,	0.000};
    InterpolateArray(Q15_x, Q15_y, sizeof(Q15_x)/sizeof(double), Q1[5]);

    //--------------------- Q16 ---------------------
    u1[6] = 2.5;
    double Q16_x[] = {2.500,	3.000,	3.600,	4.000,	5.070};
    double Q16_y[] = {0.000,	0.190,	0.245,	0.210,	0.000};
    InterpolateArray(Q16_x, Q16_y, sizeof(Q16_x)/sizeof(double), Q1[6]);

    //--------------------- Q17 ---------------------
    u1[7] = 0.29;
    double Q17_x[] = {0.290,	0.300,	0.350,	0.400,	0.500,	0.800,	1.000,	2.000,	6.000,	10.000,	50.000};
    double Q17_y[] = {0.000,	0.440,	0.650,	0.730,	0.840,	1.000,	1.000,	0.780,	0.370,	0.250,	0.000};
    InterpolateArray(Q17_x, Q17_y, sizeof(Q17_x)/sizeof(double), Q1[7]);

    //--------------------- Q18 ---------------------
    u1[8] = 7.0;
    double Q18_x[] = {7.000,	8.000,	8.400,	9.000,	10.000,	10.500};
    double Q18_y[] = {0.000,	0.500,	0.600,	0.460,	0.175,	0.000};
    InterpolateArray(Q18_x, Q18_y, sizeof(Q18_x)/sizeof(double), Q1[8]);

    //--------------------- Q19 ---------------------
    u1[9] = 10.5;
    double Q19_x[] = {10.500,	11.500,	14.000,	20.000,	30.000,	50.000};
    double Q19_y[] = {0.000,	0.560,	0.800,	1.200,	2.000,	4.000};
    InterpolateArray(Q19_x, Q19_y, sizeof(Q19_x)/sizeof(double), Q1[9]);

    //--------------------- Q110 ---------------------
    u1[10] = 13.8;
    double Q110_x[] = {13.800,	15.000,	16.000,	17.000,	30.000,	40.000};
    double Q110_y[] = {0.000,	0.100,	0.130,	0.170,	1.550,	2.100};
    InterpolateArray(Q110_x, Q110_y, sizeof(Q110_x)/sizeof(double), Q1[10]);

    //--------------------- Q21 ---------------------
    u2[1] = 0.29;
    double Q21_x[] = {0.290,	0.500,	0.800,	1.000,	1.200,	1.300,	1.400,	1.600,	1.800,	1.900,	2.000,	2.050,	2.100,	2.150,	2.200,	2.300,	2.450,	2.530,	2.600,	2.620,	2.680,	2.730,	2.850,	2.920,	3.120,	3.300,	4.000};
    double Q21_y[] = {0.000,	0.0052,	0.0083,	0.0104,	0.0166,	0.0728,	0.135,	0.250,	0.520,	0.832,	3.020,	3.120,	2.080,	1.250,	0.832,	2.900,	1.040,	1.250,	1.750,	2.080,	1.730,	0.416,	0.320,	0.416,	0.728,	0.520,	0.000};
    InterpolateArray(Q21_x, Q21_y, sizeof(Q21_x)/sizeof(double), Q2[1]);

    //--------------------- Q22 ---------------------
    u2[2] = 0.58;
    double Q22_x[] = {1.830,	1.900,	2.000,	2.050,	2.100,	2.200,	2.350,	2.450,	2.500,	2.620,	2.750,	2.950,	3.050,	3.200,	3.400,	4.000};
    double Q22_y[] = {0.000,	0.208,	1.460,	2.290,	1.660,	0.790,	0.208,	1.980,	1.780,	0.208,	1.040,	1.660,	0.624,	0.208,	0.208,	0.000};
    InterpolateArray(Q22_x, Q22_y, sizeof(Q22_x)/sizeof(double), Q2[2]);

    //--------------------- Q23 ---------------------
    u2[3] = 0.87;
    double Q23_x[] = {1.900,	2.000,	2.100,	2.200,	2.300,	2.360,	2.420,	2.500,	2.610,	2.700,	2.750,	2.800,	2.920,	3.000,	3.250,	3.310};
    double Q23_y[] = {0.000,	0.416,	1.330,	1.870,	1.250,	0.208,	0.000,	0.499,	0.915,	0.624,	0.208,	0.000,	0.416,	0.208,	0.208,	0.000};
    InterpolateArray(Q23_x, Q23_y, sizeof(Q23_x)/sizeof(double), Q2[3]);

    //--------------------- Q24 ---------------------
    u2[4] = 1.16;
    double Q24_x[] = {2.050,	2.100,	2.200,	2.260,	2.550,	2.750,	2.770,	3.000,	3.050,	3.250};
    double Q24_y[] = {0.000,	0.416,	1.160,	1.580,	0.000,	0.832,	0.000,	0.208,	0.208,	0.000};
    InterpolateArray(Q24_x, Q24_y, sizeof(Q24_x)/sizeof(double), Q2[4]);

    //--------------------- Q25 ---------------------
    u2[5] = 1.45;
    double Q25_x[] = {2.100,	2.150,	2.200,	2.300,	2.460,	2.500,	2.600,	2.620,	2.680,	2.800,	2.900,	3.000,	3.200,	3.300,	3.350};
    double Q25_y[] = {0.000,	0.208,	0.541,	0.915,	1.120,	1.120,	0.208,	0.000,	0.000,	0.416,	0.750,	0.000,	0.250,	0.125,	0.000};
    InterpolateArray(Q25_x, Q25_y, sizeof(Q25_x)/sizeof(double), Q2[5]);

    //--------------------- Q26 ---------------------
    u2[6] = 1.74;
    double Q26_x[] = {2.300,	2.400,	2.500,	2.550,	2.600,	2.650,	2.700,	2.800,	2.900,	3.000,	3.100,	3.200};
    double Q26_y[] = {0.000,	0.750,	1.040,	1.120,	1.040,	0.624,	0.416,	0.208,	0.125,	2.500,	0.166,	0.000};
    InterpolateArray(Q26_x, Q26_y, sizeof(Q26_x)/sizeof(double), Q2[6]);

    //--------------------- Q27 ---------------------
    u2[7] = 2.03;
    double Q27_x[] = {2.400,	2.500,	2.750,	3.000,	3.200,	3.300,	3.400};
    double Q27_y[] = {0.000,	0.208,	0.750,	0.000,	0.166,	0.146,	0.000};
    InterpolateArray(Q27_x, Q27_y, sizeof(Q27_x)/sizeof(double), Q2[7]);

    //--------------------- Q28 ---------------------
    u2[8] = 2.32;
    double Q28_x[] = {2.600,	2.700,	2.900,	3.000,	3.100,	3.200,	3.300,	3.400};
    double Q28_y[] = {0.000,	0.208,	0.290,	0.208,	0.000,	0.000,	1.040,	0.000};
    InterpolateArray(Q28_x, Q28_y, sizeof(Q28_x)/sizeof(double), Q2[8]);

    //--------------------- Q29 ---------------------
    u2[9] = 5.0;
    double Q29_x[] = {5.000,	5.900,	6.100,	7.000,	9.000};
    double Q29_y[] = {0.000,	0.410,	0.410,	0.070,	0.000};
    InterpolateArray(Q29_x, Q29_y, sizeof(Q29_x)/sizeof(double), Q2[9]);

    //--------------------- Q210 ---------------------
    u2[10] = 6.8;
    double Q210_x[] = {6.800,	7.100,	8.100,	8.600,	9.500,	20.700};
    double Q210_y[] = {0.000,	0.570,	0.570,	0.250,	0.120,	0.000};
    InterpolateArray(Q210_x, Q210_y, sizeof(Q210_x)/sizeof(double), Q2[10]);

    //--------------------- Q211 ---------------------
    u2[11] = 8.4;
    double Q211_x[] = {8.400,	8.700,	9.100,	10.000,	20.700};
    double Q211_y[] = {0.000,	0.420,	0.420,	0.300,	0.000};
    InterpolateArray(Q211_x, Q211_y, sizeof(Q211_x)/sizeof(double), Q2[11]);

    //--------------------- Q212 ---------------------
    u2[12] = 11.25;
    double Q212_x[] = {11.250,	13.800,	14.000,	14.700,	15.000,	65.000};
    double Q212_y[] = {0.000,	0.410,	1.000,	1.000,	0.250,	0.000};
    InterpolateArray(Q212_x, Q212_y, sizeof(Q212_x)/sizeof(double), Q2[12]);

    //--------------------- Q213 ---------------------
    u2[13] = 12.5;
    double Q213_x[] = {12.500,	13.000,	13.600,	14.000,	20.700};
    double Q213_y[] = {0.000,	0.400,	0.400,	0.160,	0.000};
    InterpolateArray(Q213_x, Q213_y, sizeof(Q213_x)/sizeof(double), Q2[13]);

    //--------------------- Q214 ---------------------
    u2[14] = 14.0;
    double Q214_x[] = {14.000,	14.300,	14.800,	15.600,	20.600,	25.400,	100.000};
    double Q214_y[] = {0.000,	1.700,	1.700,	0.200,	0.200,	2.800,	2.800};
    InterpolateArray(Q214_x, Q214_y, sizeof(Q214_x)/sizeof(double), Q2[14]);

    //--------------------- Q215 ---------------------
    u2[15] = 15.6;
    double Q215_x[] = {15.600,	18.000,	20.000,	50.000,	100.000};
    double Q215_y[] = {0.000,	0.100,	0.210,	2.520,	2.520};
    InterpolateArray(Q215_x, Q215_y, sizeof(Q215_x)/sizeof(double), Q2[15]);
}


void A::Save_f()
{
    FILE *file;
    int i;

    file = fopen("f.dat", "w");

    fprintf(file, "eV\tf\n");

    for(i=0; i<b0; i++)
        fprintf(file, "%.4E\t%.4E\n", u[i], f[i]);

    fclose(file);
}


void A::AllocateMemoryBoltzmann(void)
{
    u = new double [b0];

    Q1 = new double* [11];
    for(int j=0; j<11; j++)
        Q1[j] = new double [b0];

    Q2 = new double* [16];
    for(int j=0; j<16; j++)
        Q2[j] = new double [b0];

    Qm1 = new double [b0];
    Qm2 = new double [b0];
    Qm3 = new double [b0];

    Q = new double [b0];

    M = new double* [b0];
    for(int j=0; j<b0; j++)
        M[j] =  new double [b0];

    f = new double [b0];
}


void A::FreeMemoryBoltzmann(void)
{
    delete[] u;

    for(int j=0; j<11; j++)
        delete[] Q1[j];
    delete[] Q1;

    for(int j=0; j<16; j++)
        delete[] Q2[j];
    delete[] Q2;

    delete[] Qm1;
    delete[] Qm2;
    delete[] Qm3;

    delete[] Q;

    for(int j=0; j<b0; j++)
        delete[] M[j];
    delete[] M;

    delete[] f;
}
