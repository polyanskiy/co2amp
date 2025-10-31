#include  "co2amp.h"

void FFT(std::complex<double> *in, std::complex<double> *out) // in: field, out: spectrum
{
    // Do FFT
    FFTCore(in, out, false);

    // Normalize output
    for(int n=0; n<n0; n++)
        out[n] *= Dt;
}


void IFFT(std::complex<double> *in, std::complex<double> *out) // in: spectrum, out: field
{
    // Do IFFT (true=inverse)
    FFTCore(in, out, true);

    // Normalize output
    for(int n=0; n<n0; n++)
        out[n] *= Dv;
}


void FFTCore(std::complex<double> *in, std::complex<double> *out, bool Invert)
{
    for(int n=0; n<n0; n++)
        out[n] = in[BitReversal(n)];

    int Step, Group, Pair;
    int Jump, Match;
    double delta, Sine;
    std::complex<double> Multiplier, Factor, Product;

    //   Iteration through dyads, quadruples, octads and so on...
    for (Step=1; Step<n0; Step*=2)
    {
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
        for (Group = 0; Group < Step; ++Group)
        {
            //   Iteration within group
            for (Pair = Group; Pair < n0; Pair += Jump)
            {
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
    int out = 0;
    int m=1;

    for(int i=n0/2; i>0; i/=2)
    {
        if(in >= i)
        {
            out += m;
            in -= i;
        }
        m *= 2;
    }

    return out;
}
