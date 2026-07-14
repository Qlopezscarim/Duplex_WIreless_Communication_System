#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cmath>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>

#pragma once

#define H1_LENGTH	73
#define H2_LENGTH	200 //length of pulse shaping filter
//general syntax below
extern std::complex<float> h1[H1_LENGTH];

/* Generate a unit-energy truncated RRC impulse response for pulse shaping
    Assume D <= U <= 2*D
    This RRC pulse is sampled at U*symbol_rate.
    Excess bandwidth = U/D - 1
    sampling rate / symbol rate =  U/D
    Length of impulse response = 2*len+1
*/
void rrc_pulse(std::complex<float>* h, int len, int U, int D);
void rrc_pulse_b_25(std::complex<float>* h, int len, int U, int D);
