#include "Correlator.h"
#include "Demodulator.h"
#include "utility/payload.h"
extern bool stop_signal_called;

extern std::vector<bool> PAYLOAD;

void Demodulator::demodulate(
MutexFIFO<std::vector<std::complex<float>>>& match_output_fifo,
MutexFIFO<size_t>& m_seq_fifo,
SharedPrinter& printer,
size_t Tupsample  //sampling rate/symbol rate here
)

{

    size_t num_error_packets = 0;
    size_t num_packets_total = 0;
    std::vector<bool> demod (1016);

    while(not stop_signal_called)
        {
                std::vector<std::complex<float>> Packet;
		size_t m_peak;

                while( not match_output_fifo.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                {
                        if( stop_signal_called )
                        {
                                std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                                return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
		while( not m_seq_fifo.pop(m_peak) ) //keep trying to pop until the FIFO has data ; not needed bu>
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

		size_t index_of_max = m_peak;
		
		// Last symbol of M-sequence as reference
    		//std::complex<float> last_symbol = Packet[index_of_max + 30 * 10];M_SEQ_SIZE
		std::complex<float> last_symbol = Packet[index_of_max + (M_SEQ_SIZE-1) * 10];
		
    		//last_symbol = std::conj(last_symbol);
		size_t error_bits = 0;
		
		for (int i = 0; i < total_bits; ++i) 
		{
        		std::complex<float> symbol = Packet[index_of_max + (i + M_SEQ_SIZE) * 10]; // first data symbol at position 31
        		
			
			std::complex<float> diff = symbol * std::conj(last_symbol);
        
        		// Map phase difference to bit based on real part sign
        		if (diff.real() < 0)
        		{
                		demod[i] = 1;
        		}
        		else
        		{
                		demod[i] = 0;
        		}

        		last_symbol = symbol;
			
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
