#pragma once

class Demodulator
{

	public:
	static void demodulate(
	MutexFIFO<std::vector<std::complex<float>>>& match_output_fifo,
	MutexFIFO<size_t>& m_seq_fifo,
	SharedPrinter& printer,
	size_t Tupsample  //sampling rate/symbol rate here
	);
};
