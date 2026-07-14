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
#include "Shared_Printer.h"

//#define BLOCK_SIZE 1000
#define REQ_SAMPLES 0 //does nothing is a relic from RX
#define SAMP_TYPE 0
#define TIMEOUT 0

class TX
{
public:
//spawns main RX thread according to passes parameters
static std::thread tx_thread(const std::string& cpu_format,
	const std::string& wire_format,
        const std::string& file,
	const std::string&  tx_channels,
	const std::string& tx_args,
	std::string& ref,
	double& tx_rate,
	double& tx_freq,
	bool& set_gain,
	double& tx_gain,
	bool& bw_set,
	double& tx_bw,
	bool& ant_set,
	std::string& tx_ant,
	double& settling_time,
	size_t& sblocks,
	MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
	SharedPrinter& printer);


static void tx_worker
(
uhd::usrp::multi_usrp::sptr tx_usrp,
const std::string& cpu_format,
const std::string& wire_format,
const std::string& file,
size_t samps_per_buff,
int num_requested_samples,
double settling_time,
std::vector<size_t> tx_channel_nums,
MutexFIFO<std::vector<std::complex<float>>>& data_fifo, //may need to be changed
SharedPrinter& printer

);

};
