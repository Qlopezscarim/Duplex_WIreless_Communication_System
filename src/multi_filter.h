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

class Multi_Filter
{
public:
static size_t    Nthreads;


static void filter_thread
	(
        MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
        MutexFIFO<std::vector<std::complex<float>>>& data_fifo2,
        size_t& sblocks,
        SharedPrinter& printer,
	int U,
	int D,
	size_t h_l,
	std::complex<float>* h,
	bool skip_pack_num
        );
};
