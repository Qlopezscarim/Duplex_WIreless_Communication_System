#pragma once

#include "Correlator.h"
#include "Demodulator.h"
#include "utility/payload.h"
#include <vector>
#include <complex>
#include <fftw3.h>
#ifdef USE_VOLK
#include <volk/volk.h>
#endif
struct carrier_offset {
    int peak_index;
    double value;
    double phase;
    double omega;

    // Constructor initializes peak_value to the smallest possible double
    carrier_offset()
        : peak_index(-1),
          value(0),
	  phase(0),
          omega(0)
    {}
};

class Carrier_sync
{

	public:
	static void carrier_sync(
	MutexFIFO<std::vector<std::complex<float>>>& timing_aligned,
	MutexFIFO<std::vector<std::complex<float>>>& to_demodulator,
	SharedPrinter& printer
	);
};
