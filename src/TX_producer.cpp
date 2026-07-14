#include <random>
#include <iostream>
#include "TX_producer.h"
#include "utility/m_sequence.h"
#include "utility/packet.h"

extern bool stop_signal_called;
extern std::bitset<M_SEQ_SIZE> m_seq;

void TX_producer::producer
(
MutexFIFO<packet>& data_fifo,
size_t& sblocks, //output size
SharedPrinter& printer
)
{
	size_t iteration = 0;
	uint16_t packet_num = 1;
	std::vector<uint8_t> rand_payload (sblocks);
	while(not stop_signal_called)
	{
		if(iteration < sblocks)
		{
			std::mt19937 rng(iteration);
			std::uniform_int_distribution<std::mt19937::result_type> dist6(0,255);
			rand_payload[iteration] = dist6(rng);
			iteration++;
		}
		else if (data_fifo.size() < 15)
		{
			packet packet_to_fifo = packet(global_preamble,m_seq, packet_num, rand_payload);
			/*
			//It should be noted this operation is as follows:
			//we construct the packet with references
			//we then pass a copy of the packet to the fifo
			//so we cannot manipulate the above m_seq,packet_num,rand_payload until
			//the push!
			*/
			data_fifo.push(packet_to_fifo);
			packet_num++;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
	}
	printer.print("Graceful exit of TX producer thread");
}
