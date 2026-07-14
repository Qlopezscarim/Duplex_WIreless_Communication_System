#include "Correlator.h"
#include "Coherent_demodulator.h"
#include "utility/payload.h"
extern bool stop_signal_called;

extern std::vector<bool> PAYLOAD;

#ifdef BPSK

void Coherent_demodulator::coherent_demodulate(
MutexFIFO<std::vector<std::complex<float>>>& Carrier_sync_output,
SharedPrinter& printer
)

{

    size_t num_error_packets = 0;
    size_t num_packets_total = 0;
    std::vector<bool> demod (1016);

    while(not stop_signal_called)
        {
                std::vector<std::complex<float>> Packet;

                while( not Carrier_sync_output.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                {
                        if( stop_signal_called )
                        {
                                std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                                return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
		
		const int payload_bits_to_check = 1000;
    		const int total_bits = payload_bits_to_check + 16; // including packet number

		
		size_t error_bits = 0;
		
		for (int i = 0; i < total_bits; ++i) 
		{
        		std::complex<float> symbol = Packet[(i + M_SEQ_SIZE)]; // first data symbol at position 31
        		
			
			std::complex<float> diff = symbol;
        
        		// Map phase difference to bit based on real part sign
        		if (diff.real() < 0)
        		{
                		demod[i] = 1;
        		}
        		else
        		{
                		demod[i] = 0;
        		}

			//check against payload
			if(i >= 16)
			{
				if(demod[i] != PAYLOAD[i-16])
				{
					error_bits = error_bits + 1;
					std::cout <<"Index of errors relative to payload" << i-16 << std::endl;
				}
			}
    		}
		num_packets_total = num_packets_total + 1;
		if(error_bits > 0)
		{
			num_error_packets++;
		}
		uint16_t demodulated_packet_num = 0;

		for (size_t i = 0; i < 16; ++i) {
        	demodulated_packet_num <<= 1;
        	demodulated_packet_num |= demod[i] ? 1 : 0;
    		}


		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << "Demodulator info for detected packet:" << num_packets_total << std::endl;
		std::cout << "Long time error rate:\t\t\t\t" << (double(num_error_packets)/num_packets_total)*100 << "%" << std::endl;
		std::cout << "Demodulated " << demodulated_packet_num << " error rate of:\t\t\t" << (double(error_bits) / total_bits)*100 << "%" << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;


        }
        std::cout << "Graceful exit of Demodulator thread" << std::endl;
        return;

}
#endif

#ifdef QPSK
void Coherent_demodulator::coherent_demodulate(
MutexFIFO<std::vector<std::complex<float>>>& Carrier_sync_output,
SharedPrinter& printer
)

{

    size_t num_error_packets = 0;
    size_t num_packets_total = 0;
    std::vector<bool> demod (1016);

    while(not stop_signal_called)
        {
                std::vector<std::complex<float>> Packet;

                while( not Carrier_sync_output.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                {
                        if( stop_signal_called )
                        {
                                std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                                return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                const int payload_bits_to_check = 1000;
                const int total_bits = payload_bits_to_check + 16; // including packet number


                size_t error_bits = 0;

	for (int i = 0; i < total_bits; i += 2)
	{
    		// QPSK: 2 bits per symbol
    		int sym_idx = (i/2) + ceil(M_SEQ_SIZE/2.0f); //start of our packet number!
    		std::complex<float> symbol = Packet[sym_idx];

    		float I = symbol.real();
    		float Q = symbol.imag();

    		// Inverse of your mapping:
    		// real = +1 → bit0 = 0,   real = -1 → bit0 = 1
    		// imag = +1 → bit1 = 0,   imag = -1 → bit1 = 1
    		demod[i+1]   = (I < 0) ? 1 : 0;  
    		demod[i]     = (Q < 0) ? 1 : 0; //imag stores the more signifigant bit

		if(i >= 16)
                {
                       		if(demod[i] != PAYLOAD[i-16])
                       		{
                                        error_bits = error_bits + 1;
                                        std::cout <<"Index of errors relative to payload" << i-16 << std::endl;
                                }
				if(demod[i+1] != PAYLOAD[i+1-16])
				{
					error_bits = error_bits + 1;
                                        std::cout <<"Index of errors relative to payload" << i+1-16 << std::endl;
				}
                }
	}


                num_packets_total = num_packets_total + 1;
                if(error_bits > 0)
                {
                        num_error_packets++;
                }
                uint16_t demodulated_packet_num = 0;

                for (size_t i = 0; i < 16; ++i) {
                demodulated_packet_num <<= 1;
                demodulated_packet_num |= demod[i] ? 1 : 0;
                }


                std::cout << "--------------------------------------------------------------------------------" << std::endl;
                std::cout << "Demodulator info for detected packet:" << num_packets_total << std::endl;
                std::cout << "Long time error rate:\t\t\t\t" << (double(num_error_packets)/num_packets_total)*100 << "%" << std::endl;
                std::cout << "Demodulated " << demodulated_packet_num << " error rate of:\t\t\t" << (double(error_bits) / total_bits)*100 << "%" << std::endl;
                std::cout << "--------------------------------------------------------------------------------" << std::endl;


        }
        std::cout << "Graceful exit of Demodulator thread" << std::endl;
        return;

}
#endif
