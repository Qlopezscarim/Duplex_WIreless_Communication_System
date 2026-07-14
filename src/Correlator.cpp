#include "Correlator.h"
extern bool stop_signal_called;

void Correlator::correlator(
MutexFIFO<std::vector<std::complex<float>>>& match_output_fifo,
MutexFIFO<std::vector<std::complex<float>>>& correlator_forward,
MutexFIFO<size_t>& correlator_output,
SharedPrinter& printer,
size_t Tupsample  //sampling rate/symbol rate here
)

{

	
	
	size_t columns = m_seq.size();
	size_t rows = Tupsample;
	std::vector<std::vector<std::complex<float>>> sliding_window = 
	std::vector<std::vector<std::complex<float>>>(
    	rows,                                   // number of rows
    	std::vector<std::complex<float>>(columns)  // each row has num_cols complex<float>
	);

#ifdef BPSK
	std::vector<std::complex<float>> downsampled_packet (PACKET_SIZE_NO_PADDING - PREAMBLE_SIZE); //we get rid of the pre-amble
#endif

#ifdef QPSK
	//For QPSK we have to account for the preamble being modulated in two bits per symbol
	std::vector<std::complex<float>> downsampled_packet (PACKET_SIZE_NO_PADDING - ceil(PREAMBLE_SIZE/2.0f));
#endif

	//We want to look the length of the total block - size of useful information
	size_t end_tau = SIZE_BLOCKS_TO_STORE    -     ((PACKET_SIZE - H2_PADDING - m_seq.size())); 
	//Could need to add one to handle fractional bits later

	std::vector<std::complex<float>> orig_m_seq = init_mod_mseq();

    while(not stop_signal_called)
        {
		std::vector<std::complex<float>> Packet;
                while( not match_output_fifo.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                {
                	if( stop_signal_called )
                        {
                        	std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                                return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

		Peak m_iter = Peak(); //We are looking for a new peak now so construct the object
		size_t current_index = 0;

		for (size_t tau = 0; tau < end_tau; tau++)
		{
			for (size_t i = 0; i<rows; i++)
			{
				for (size_t j = 0; j<columns ; j++)
				{
					size_t pack_index = i + j*10 + ( rows * tau);
					if(pack_index >= Packet.size())
					{
						std::cout << "OUT OF INDEX RANGE" << std::endl;
					}
					else
					{
						sliding_window[i][j] = Packet[pack_index];
					}
				}
			}
			
			for(size_t l = 0; l<rows; l++)
			{
				double current_sum = std::abs(computeSum(sliding_window[l],orig_m_seq));

				if(current_sum > m_iter.value)
				{

				m_iter.value = current_sum;
				m_iter.peak_index = current_index;

				}
				current_index++;
			}
			
		}

#ifdef BPSK
                for (int i = 0; i < PACKET_SIZE_NO_PADDING - PREAMBLE_SIZE; ++i)
                {
                        downsampled_packet[i] = Packet[m_iter.peak_index + (i) * 10]; // first data symbol
                }
#endif

#ifdef QPSK
		//Once again in QPSK we need to deal with the pre-amble being modulated in 2 bits per symbol
		//with padding for the last bit if odd (hence the ceil)
                for (int i = 0; i < PACKET_SIZE_NO_PADDING - ( ceil(PREAMBLE_SIZE/2.0f) ); ++i)
                {
                        downsampled_packet[i] = Packet[m_iter.peak_index + (i) * 10]; // first data symbol
                }
#endif

		std::cout << "The found m-sequence peak is: \t" << m_iter.peak_index << std::endl;
                std::cout << "the found m-sequence value is \t" << m_iter.value << std::endl;


#ifdef BPSK
		//may later want to bother thresholding this
		if( m_iter.value > 22 && m_iter.value < 25)
		{
			correlator_output.push(m_iter.peak_index);
			correlator_forward.push(downsampled_packet);
		}
#endif

#ifdef QPSK
                if( m_iter.value > 12 && m_iter.value < 15)
                {
                        correlator_output.push(m_iter.peak_index);
                        correlator_forward.push(downsampled_packet);
                }
#endif


        }

        std::cout << "Graceful exit of Correlator thread" << std::endl;
        return;

}
