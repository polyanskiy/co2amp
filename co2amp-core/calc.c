#include  "co2amp.h"

void FFT(double complex *in, double complex *out) // in: field, out: spectrum
{
    int i;

    FFTCore(in, out, false);

    double complex tmp;

    for(i=0; i<n0/4; i++){
        tmp = out[i];
        out[i] = out[n0/2-i-1];
        out[n0/2-i-1] = tmp;
        tmp = out[n0/2+i];
        out[n0/2+i] = out[n0-i-1];
        out[n0-i-1] = tmp;
    }

    double Dt = t_pulse_lim/(n0-1);
    for(i=0; i<n0; i++)
        out[i] *= Dt;

}


void IFFT(double complex *in, double complex *out) // in: spectrum, out: field
{
    int i;
    double complex tmp, qq[n0];

    for(i=0; i<n0; i++)
        qq[i] = in[i];

    for(i=0; i<n0/4; i++){
        tmp = qq[i];
        qq[i] = qq[n0/2-i-1];
        qq[n0/2-i-1] = tmp;
        tmp = qq[n0/2+i];
        qq[n0/2+i] = qq[n0-i-1];
        qq[n0-i-1] = tmp;
    }

    FFTCore(qq, out, true);

    double Dt = t_pulse_lim/(n0-1);
    for(i=0; i<n0; i++)
        out[i] /= (Dt*n0);
}


void FFTCore(double complex *in, double complex *out, bool Invert){

    int i;

    for(i=0; i<n0; i++)
        out[i] = in[BitReversal(i)];

    int Step, Group, Pair;
    int Jump, Match;
    double delta, Sine;
    double complex Multiplier, Factor, Product;

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
