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
#include <cstdint>
#include <bitset>
#include "utility/packet.h"
class TX_producer
{
public:
static void producer
(
MutexFIFO<packet>& data_fifo,
size_t& sblocks, //output size
SharedPrinter& printer
);



};
