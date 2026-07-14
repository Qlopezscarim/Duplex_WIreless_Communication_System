#include "AVG_POWER_THREAD.h"

extern bool stop_signal_called;
extern bool first_packet;

void AVG_POWER::power_thread
(MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
MutexFIFO<std::vector<std::complex<float>>>& output_fifo,
size_t& sblocks,
SharedPrinter& printer,
std::string file_name)
{
	//initializing packet counter
	int num_packets_per_dec = 0;

	//initializing timer:
	auto start = std::chrono::steady_clock::now();
	auto end = std::chrono::steady_clock::now();
    	auto elapsed = end - start;

	//initializing handler for FIFO output
	std::vector<std::complex<float>> fifo_output;

	while(not stop_signal_called)
	{
		while( not data_fifo.pop(fifo_output) ) //keep trying to pop until the FIFO has data
		{
			if( stop_signal_called )
			{
			std::cout << "Graceful exit of AVG_POWER_THREAD" << std::endl;
			return;
			}
			

			end = std::chrono::steady_clock::now();
                	elapsed = end - start;
                	/*
                	In this case we are not detecting any packets and time is still passing
                	*/
                	if(elapsed > std::chrono::seconds(10))
                	{
                        	printer.print("A total of:\t" + std::to_string(num_packets_per_dec) + "\t packets were counted in ten seconds");
                        	num_packets_per_dec = 0;
                        	start = std::chrono::steady_clock::now();
                	}
			

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}
		//handling timer initialization
		if (first_packet && file_name == "no")
		{
			start = std::chrono::steady_clock::now();
			first_packet = false;
		}

		//Code to store to file (no is the defualt file name)
		else if( first_packet && file_name != "no")
		{
			first_packet = false;
#ifdef DEBUG
			std::ofstream out(file_name);
			if (!out.is_open())
			{
				std::cerr << "Error: could not open file" << file_name << std::endl;
				return;
			}
			
			for (size_t i = 0; i<sblocks ; i++) //hadnle packet number offset weirdness 
			{
				out << fifo_output[i+1].real() << " " << fifo_output[i+1].imag() << "\n";
        			//out << c.real() << " " << c.imag() << "\n";
    			}

			out.close();
			printer.print("Data written to ");
			printer.print(file_name);
#endif
			start = std::chrono::steady_clock::now(); //only want to start timer now
		}

		float total_power = 0;
		//non-optimized no SIMD code
		for(size_t i = 1; i<fifo_output.size() ; i++)
		{
			//pass for now
			total_power = total_power + std::norm(fifo_output[i]);
		}
		total_power = total_power/sblocks;

		//normalized signal passed to match filter:
		output_fifo.push(fifo_output);

		//thread safe implementation
		printer.print("packet number: \t" + std::to_string(fifo_output[0].real()) + " Total power: \t" + std::to_string(total_power) + std::to_string(sblocks) + std::to_string(fifo_output.size()) );
		//timer handling ------------------------------------------
		num_packets_per_dec++;
		end = std::chrono::steady_clock::now();
    		elapsed = end - start;
		/*
		In this case we are catching a packet after the first 10 seconds: print the number of packets
		counted - 1 since those are the ones that heppened in the 10 second time period and initialize
		counted packets to 1, then reset start timer.
		*/
		if(elapsed > std::chrono::seconds(10))
		{
			printer.print("A total of:\t" + std::to_string(num_packets_per_dec) + "\t packets were counted in 10 seconds");
			num_packets_per_dec = 1;
			start = std::chrono::steady_clock::now();
		}
	}

	std::cout << "Graceful exit of AVG_POWER_THREAD" << std::endl;

}
