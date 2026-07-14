#include "file_gen.h"

extern bool stop_signal_called;

void FILE_GEN::file_thread
(MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
MutexFIFO<std::vector<std::complex<float>>>& data_fifo2,
SharedPrinter& printer,
std::string file_name)
{


	//initializing handler for FIFO output
        std::vector<std::complex<float>> fifo_output;

	//initializer for is first packet or not
	bool first_packet = true;
        while(not stop_signal_called)
        {
                while( not data_fifo.pop(fifo_output) ) //keep trying to pop until the FIFO has data
                {
                        if( stop_signal_called )
                        {
                        std::cout << "Graceful exit of FILE_GEN_THREAD" << std::endl;
                        return;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(1));

                }


		if( first_packet && file_name != "no")
        	{
			std::cout << "WRITING OUT FILE -----" << std::endl;
                        first_packet = false;
                        std::ofstream out(file_name);
                        if (!out.is_open())
                        {
                                std::cerr << "Error: could not open file" << file_name << std::endl;
                                return;
                        }

                        for (size_t i = 0; i<fifo_output.size() ; i++)
                        {
                                out << fifo_output[i].real() << " " << fifo_output[i].imag() << "\n";
                        }

                        out.close();
                        printer.print("Data written to ");
                        printer.print(file_name);
                }
	data_fifo2.push(fifo_output);
	}
	std::cout << "Graceful exit of FILE_GEN_THREAD" << std::endl;
        return;
}
