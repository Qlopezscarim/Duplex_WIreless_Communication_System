#include "Carrier_sync.h"

extern bool stop_signal_called;

extern std::vector<bool> PAYLOAD;

void Carrier_sync::carrier_sync(
MutexFIFO<std::vector<std::complex<float>>>& timing_aligned,
MutexFIFO<std::vector<std::complex<float>>>& to_demodulator,
SharedPrinter& printer
)

{
   //every packet if I have a known pilot sequence of 63 
   //I just multiply the downsampled and match filtered RX and
   //multiply it by the conjugate of my pilot sequence and then the DTFT

    std::vector<std::complex<float>> orig_m_seq = init_mod_mseq(); //initialized to use for known pilot symbols
    size_t columns = m_seq.size();				   //used to know length of pilot symbols

    //Setting size of FFT/resolution. May need to change for different rates - not implemented yet!
    size_t N = 32768*2*2*2;

    //Allocating space for FFT work:
    fftwf_complex* in = (fftwf_complex*) fftwf_malloc(sizeof(fftw_complex) * N);
    fftwf_complex* out = (fftwf_complex*) fftwf_malloc(sizeof(fftw_complex) * N);

    std::complex<float>* in_use = reinterpret_cast<std::complex<float>*>(in);
    std::complex<float>* out_use = reinterpret_cast<std::complex<float>*>(out);

    fftwf_plan p = fftwf_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

#ifdef BPSK
	size_t omega_basis = 71;
#endif

#ifdef QPSK
	size_t omega_basis = 35;
#endif


    while(not stop_signal_called)
        {
                std::vector<std::complex<float>> Packet;
		carrier_offset container;		//check intializer for default values here

                while( not timing_aligned.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                {
                        if( stop_signal_called )
                        {
				//We are exiting this thread free fftw memory:
				fftwf_destroy_plan(p);
				fftwf_free(in); 
				fftwf_free(out);

                                std::cout << "Graceful exit of Carrier_sync thread" << std::endl;
                                return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }


		//Need to copy packet to SIMD memory aligned map and pad zeros:
		for(size_t i = 0; i<N; i++)
		{
#ifdef BPSK
			if( i < m_seq.size() ) //Our pilot sequence!
			{
				in_use[i] = Packet[i] * std::conj(orig_m_seq[i]);
			}
#endif

#ifdef QPSK
			if( i < (m_seq.size()/2) - 1 ) //Our pilot sequence minus the end odd one
                        {
                                in_use[i] = Packet[i] * std::conj(orig_m_seq[i]);
                        }
#endif
			else //zero padding for FFT resolution
			{
				in_use[i] = std::complex<float>(0.0f,0.0f);
			}
		}

		//execute FFT
		fftwf_execute(p);


		// ----------------------- Start of actual one shot estimation after fft ------------------
		for (size_t i = 0; i<N; i++)
		{
			if( std::abs(out_use[i]) > container.value)
			{
				container.peak_index = i;
				container.value = std::abs(out_use[i]);
			}
		}

		if (container.peak_index <= N/2)
		{
    			// Positive or zero frequency
    			container.omega = 2.0f * M_PI * (float)container.peak_index / (float)N;
		}
		else
		{
    			// Negative frequency
    			int k_neg = container.peak_index - N;   // makes it negative
    			container.omega = 2.0f * M_PI * (float)k_neg / (float)N;
		}


		// Phase offset
		container.phase = std::arg(out_use[container.peak_index]);

		for (size_t n = 0; n < Packet.size(); n++)
                {
                        //The 0.8 factor accounts for the difference in symbol/sampling rate somehow?
                        Packet[n] *= std::exp(std::complex<float>(0.0f, -(container.phase + container.omega * n)));
                }

		double accumulated_drift = 0.0f;

#ifdef BPSK
		//sucky unoptimized PLL
		for (size_t n = 0; n < Packet.size()-1; n++)
                {
			if(Packet[n].real() > 0 )
              		{
                      		accumulated_drift += (std::arg(Packet[n]));
              		}
			else
	              	{
                      		accumulated_drift += (std::arg(Packet[n]) - M_PI);
              		}

                        Packet[n+1] *= std::exp(std::complex<float>(0.0f, -(accumulated_drift)));
                }
#endif

#ifdef QPSK
		for (size_t n = 0; n < Packet.size()-1; n++)
                {
                        if(Packet[n].real() >= 0 && Packet[n].imag() >= 0 ) //Quadrant I
                        {
                                accumulated_drift += (std::arg(Packet[n]) - M_PI/4.0);
                        }
			else if (Packet[n].real() >= 0 && Packet[n].imag() <= 0) //Quadrant IV
			{
				accumulated_drift += (std::arg(Packet[n]) - (7*M_PI)/4.0);
			}
			else if(Packet[n].real() <= 0 && Packet[n].imag() >= 0) //Quadrant II
			{
				accumulated_drift += (std::arg(Packet[n]) - (3*M_PI)/4.0);
			}
                        else //Quadrant III
                        {
                                accumulated_drift += (std::arg(Packet[n]) - (5*M_PI)/4.0);
                        }

                        Packet[n+1] *= std::exp(std::complex<float>(0.0f, -(accumulated_drift)));
                }
#endif

//		if(Packet[omega_basis].real() > 0 )
//		{
//			container.omega = ((std::arg(Packet[omega_basis]))  / omega_basis); // 0.90f  +  ((std::arg(Packet[omega_basis/2])) / (omega_basis/2)) * 0.10f;
//			std::cout << "POSITIVE" << std::endl;
//		}
//		else
//		{
//			std::cout << "NEGATIVE" << std::endl;
//			container.omega = ((std::arg(Packet[omega_basis]) - M_PI) / omega_basis) * 0.90f + 0.10f * ((std::arg(Packet[omega_basis/2]) - M_PI) / (omega_basis/2));
//		}


		// ------------------ Removing phase offset --------------------------------------------------
//		for (size_t n = 0; n < Packet.size(); n++) 
//		{
			//The 0.8 factor accounts for the difference in symbol/sampling rate somehow?
//    			Packet[n] *= std::exp(std::complex<float>(0.0f, -(container.omega * n)));
//		}

		std::cout << "Peak index: " << container.peak_index 
          	<< " Omega: " << container.omega 
          	<< " Phase: " << container.phase << std::endl;


		to_demodulator.push(Packet);

        }
	//We are exiting this thread free fftw memory:
        fftwf_destroy_plan(p);
        fftwf_free(in);
        fftwf_free(out);

        std::cout << "Graceful exit of Carrier_sync thread" << std::endl;
        return;

}
