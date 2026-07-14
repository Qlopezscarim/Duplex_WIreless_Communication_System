#pragma once

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
#include "queue.h"
#include "RX.h"
#include "Shared_Printer.h"
#include <bitset>
#include "utility/packet.h"
#include "utility/m_sequence.h"
#include "POWER_THREAD.h"

struct Peak {
    int peak_index;
    double value;

    // Constructor initializes peak_value to the smallest possible double
    Peak() 
        : peak_index(-1), 
          value(0) 
    {}
};

template <typename T>
std::complex<T> computeSum(
    const std::vector<std::complex<T>>& row,
    const std::vector<std::complex<T>>& original_m_seq)
{
    std::complex<T> sum = 0;
    for (size_t i = 0; i < row.size(); ++i) {
        sum += row[i] * std::conj(original_m_seq[i]);
    }
    return sum;
}



class Correlator
{
public:
    static void correlator(
        MutexFIFO<std::vector<std::complex<float>>>& match_output_fifo,
	MutexFIFO<std::vector<std::complex<float>>>& correlator_forward,
	MutexFIFO<size_t>& correlator_output,
        SharedPrinter& printer,
	size_t Tupsample
    );

};
