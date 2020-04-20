#include  "co2amp.h"

void FFT(std::complex<double> *in, std::complex<double> *out) // in: field, out: spectrum
{
    // Do FFT
    FFTCore(in, out, false);

    // Re-arrange data points in output array
    std::complex<double> *tmp;
    tmp = new std::complex<double>[n0];
    for(int n=0; n<n0; n++)
        tmp[n] = out[n];
    for(int n=0; n<n0/4; n++){
        out[n] = tmp[n0/2-n-1];
        out[n0/2-n-1] = tmp[n];
        out[n0/2+n] = tmp[n0-n-1];
        out[n0-n-1] = tmp[n0/2+n];
    }
    delete tmp;

        // Normalize output
    double Dt = (t_max-t_min)/n0;
    for(int n=0; n<n0; n++)
        out[n] *= Dt;

    // test - regular Fourier transform
    /*double Dt = (t_max-t_min)/n0;
    double Dv = 1.0/(t_max-t_min);
    double v_min = -Dv*(n0/2);
    for(int n=0; n<n0; n++)
        out[n] = 0;
    for(int n=0; n<n0; n++)
    {
        for(int n1=0; n1<n0; n1++)
            //out[n] += in[n1]*exp(-2.0*M_PI*I * (double)n1 * (double)n / (double)n0);
            out[n] += in[n1]*exp(-2.0*M_PI*I
                                 * (t_min+Dt*(0.5+n1))
                                 * (v_min+Dv*(0.5+n))) * Dt;
    }*/
}


void IFFT(std::complex<double> *in, std::complex<double> *out) // in: spectrum, out: field
{
    // Re-arrange data points in input array
    std::complex<double> *tmp, *in1;
    tmp = new std::complex<double>[n0];
    in1 = new std::complex<double>[n0];
    for(int n=0; n<n0; n++)
        tmp[n] = in[n];
    for(int n=0; n<n0/4; n++){
        in1[n] = tmp[n0/2-n-1];
        in1[n0/2-n-1] = tmp[n];
        in1[n0/2+n] = tmp[n0-n-1];
        in1[n0-n-1] = tmp[n0/2+n];
    }
    delete tmp;

    // Do FFT
    FFTCore(in1, out, true);
    delete in1;

    // Normalize output
    double Dv = 1.0/(t_max-t_min);
    for(int n=0; n<n0; n++)
        out[n] *= Dv;

    // test - regular Fourier transform
    /*double Dt = (t_max-t_min)/n0;
    double Dv = 1.0/(t_max-t_min);
    double v_min = -Dv*n0/2;
    for(int n=0; n<n0; n++)
        out[n] = 0;
    for(int n=0; n<n0; n++)
    {
        for(int n1=0; n1<n0; n1++)
            //out[n] += in[n1]*exp(-2.0*M_PI*I * (double)n1 * (double)n / (double)n0);
            out[n] += in[n1]*exp(2.0*M_PI*I
                                 * (t_min+Dt*(0.5+n))
                                 * (v_min+Dv*(0.5+n1))) * Dv;
    }*/
}


void FFTCore(std::complex<double> *in, std::complex<double> *out, bool Invert){

    int i;

    for(i=0; i<n0; i++)
        out[i] = in[BitReversal(i)];

    int Step, Group, Pair;
    int Jump, Match;
    double delta, Sine;
    std::complex<double> Multiplier, Factor, Product;

    //   Iteration through dyads, quadruples, octads and so on...
    for (Step=1; Step<n0; Step*=2){
        //   Jump to the next entry of the same transform factor
        Jump = Step*2;
        //   Angle increment
        delta = Invert ? -M_PI/Step : M_PI/Step;
        //   Auxiliary sin(delta / 2)
        Sine = sin(delta * .5);
        //   Multiplier for trigonometric recurrence
        Multiplier = -2. * Sine * Sine + I*sin(delta);
        //   Start value for transform factor, fi = 0
        Factor = 1.;
        //   Iteration through groups of different transform factor
        for (Group = 0; Group < Step; ++Group){
            //   Iteration within group
            for (Pair = Group; Pair < n0; Pair += Jump){
                //   Match position
                Match = Pair + Step;
                //   Second term of two-point transform
                Product = Factor * out[Match];
                //   Transform for fi + pi
                out[Match] = out[Pair] - Product;
                //   Transform for fi
                out[Pair] += Product;
            }
            //   Successive transform factor via trigonometric recurrence
            Factor = Multiplier * Factor + Factor;
        }
    }
}


int BitReversal(int in)
{
    int i;
    int out = 0;
    int m=1;

    for(i=n0/2; i>0; i/=2){
        if(in >= i){
            out += m;
            in -= i;
        }
        m *= 2;
    }

    return out;
}
