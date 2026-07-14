#include <complex>

// Filter: U=4, D=5, cutoff=0.05, taps=73
std::complex<float> h1[] = {
    std::complex<float>(-0.0002440039f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(0.0005343322f, 0.0f),
    std::complex<float>(0.0011826438f, 0.0f),
    std::complex<float>(0.0015658916f, 0.0f),
    std::complex<float>(0.0012498028f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(-0.0019706240f, 0.0f),
    std::complex<float>(-0.0039172374f, 0.0f),
    std::complex<float>(-0.0047566031f, 0.0f),
    std::complex<float>(-0.0035338433f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(0.0049790430f, 0.0f),
    std::complex<float>(0.0094616358f, 0.0f),
    std::complex<float>(0.0110460673f, 0.0f),
    std::complex<float>(0.0079284231f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(-0.0105547282f, 0.0f),
    std::complex<float>(-0.0196007278f, 0.0f),
    std::complex<float>(-0.0224356718f, 0.0f),
    std::complex<float>(-0.0158388754f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(0.0205964320f, 0.0f),
    std::complex<float>(0.0379979322f, 0.0f),
    std::complex<float>(0.0433759698f, 0.0f),
    std::complex<float>(0.0306738276f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(-0.0406937339f, 0.0f),
    std::complex<float>(-0.0766733428f, 0.0f),
    std::complex<float>(-0.0903181889f, 0.0f),
    std::complex<float>(-0.0668443664f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(0.1043486145f, 0.0f),
    std::complex<float>(0.2282707763f, 0.0f),
    std::complex<float>(0.3458159049f, 0.0f),
    std::complex<float>(0.4299973683f, 0.0f),
    std::complex<float>(0.4605596464f, 0.0f),
    std::complex<float>(0.4299973683f, 0.0f),
    std::complex<float>(0.3458159049f, 0.0f),
    std::complex<float>(0.2282707763f, 0.0f),
    std::complex<float>(0.1043486145f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(-0.0668443664f, 0.0f),
    std::complex<float>(-0.0903181889f, 0.0f),
    std::complex<float>(-0.0766733428f, 0.0f),
    std::complex<float>(-0.0406937339f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(0.0306738276f, 0.0f),
    std::complex<float>(0.0433759698f, 0.0f),
    std::complex<float>(0.0379979322f, 0.0f),
    std::complex<float>(0.0205964320f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(-0.0158388754f, 0.0f),
    std::complex<float>(-0.0224356718f, 0.0f),
    std::complex<float>(-0.0196007278f, 0.0f),
    std::complex<float>(-0.0105547282f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(0.0079284231f, 0.0f),
    std::complex<float>(0.0110460673f, 0.0f),
    std::complex<float>(0.0094616358f, 0.0f),
    std::complex<float>(0.0049790430f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(-0.0035338433f, 0.0f),
    std::complex<float>(-0.0047566031f, 0.0f),
    std::complex<float>(-0.0039172374f, 0.0f),
    std::complex<float>(-0.0019706240f, 0.0f),
    std::complex<float>(-0.0000000000f, 0.0f),
    std::complex<float>(0.0012498028f, 0.0f),
    std::complex<float>(0.0015658916f, 0.0f),
    std::complex<float>(0.0011826438f, 0.0f),
    std::complex<float>(0.0005343322f, 0.0f),
    std::complex<float>(0.0000000000f, 0.0f),
    std::complex<float>(-0.0002440039f, 0.0f)
};

/* Generate a unit-energy truncated RRC impulse response for pulse shaping
    Assume D <= U <= 2*D
    This RRC pulse is sampled at U*symbol_rate.
    Excess bandwidth = U/D - 1
    sampling rate / symbol rate =  U/D
    Length of impulse response = 2*len+1
*/
void rrc_pulse(std::complex<float>* h, int len, int U, int D)
{
   float beta = float(U - D)/D; //roffoff factor
    h[len] = 1.0-beta+4.0*beta/M_PI;
    float scale = std::norm(h[len]);
    for (int n=1; n<=len; n++) {
        if (n == U/beta/4.0) {
            h[len+n] = beta/sqrt(2.0)*((1.0+2.0/M_PI)*sin(M_PI/4.0/beta)+(1.0-2.0/M_PI)*cos(M_PI/4.0/beta));
        } else {
            h[len+n] = (sin(n*M_PI*(1.0-beta)/U) + 4.0*n*beta/U*cos(n*M_PI*(1.0+beta)/U))*U/n/M_PI/(1.0-16.0*n*n*beta*beta/U/U);
        }
        h[len-n] = h[len+n];
        scale += 2.0*std::norm(h[len+n]);
    }
    scale = sqrt(scale);
    for (int n=0; n<2*len+1; n++) {
        h[n] /=  scale;
    }
}


void rrc_pulse_b_25(std::complex<float>* h, int len, int U, int D)
{
   float beta = float(5 - 4)/4; //roffoff factor
    h[len] = 1.0-beta+4.0*beta/M_PI;
    float scale = std::norm(h[len]);
    for (int n=1; n<=len; n++) {
        if (n == U/beta/4.0) {
            h[len+n] = beta/sqrt(2.0)*((1.0+2.0/M_PI)*sin(M_PI/4.0/beta)+(1.0-2.0/M_PI)*cos(M_PI/4.0/beta));
        } else {
	    h[len+n] = (sin(n*M_PI*(1.0-beta)/U) + 4.0*n*beta/U*cos(n*M_PI*(1.0+beta)/U))*U/n/M_PI/(1.0-16.0*n*n*beta*beta/U/U);
        }
        h[len-n] = h[len+n];
        scale += 2.0*std::norm(h[len+n]);
    }
    scale = sqrt(scale);
    for (int n=0; n<2*len+1; n++) {
        h[n] /=  scale;
    }
}
