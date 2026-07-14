#pragma once
#include "queue.h"
#include "Shared_Printer.h"
#include "file_gen.h"

class Coherent_demodulator
{

	public:
	static void coherent_demodulate(
	MutexFIFO<std::vector<std::complex<float>>>& Carrier_sync_output,
	SharedPrinter& printer
	);
};
