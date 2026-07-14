#include "multi_filter.h"
#include "utility/impulse.h"
#include "filters.h"
extern bool stop_signal_called;

size_t Multi_Filter::Nthreads = 1;
 
void Multi_Filter::filter_thread
(
MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
MutexFIFO<std::vector<std::complex<float>>>& data_fifo2,
size_t& sblocks, //input X size
SharedPrinter& printer,
int U,
int D,
size_t h_l, //impulse filter length
std::complex<float>* h,
bool skip_pack_num
)
{

/*for(int i =0; i< H2_LENGTH*2+1;i++)
{
        std::cout << h[i].real() << "\t" << h[i].imag() << std::endl;
}*/


        std::vector<std::complex<float>> fifo_output;

	std::cout << "Length of impulse response filter = " << h_l << "\n";

	std::cout << "U: "<< 			U << "\n";
	std::cout << "D: "<<			D << "\n";
	std::cout << "size of input: " <<	sblocks	     	<< "\n";
	std::cout << "Number of threads" <<	Nthreads        << "\n";

	FilterOverlapSave filter(U, D, sblocks, h_l, h, Nthreads);
        filter.set_head(true); //initialization

	std::vector<std::complex<float>> buff (filter.out_len());

	std::cout << "Output length of filter according to initialization: " << filter.out_len() << std::endl;

        while(not stop_signal_called)
        {
                while( not data_fifo.pop(fifo_output) ) //keep trying to pop until the FIFO has data
                {
                        if( stop_signal_called )
                        {
			printer.print("Graceful exit of FILTER_THREAD");
                        return;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));

                }
		
		if(skip_pack_num)
		{
			size_t output_length = filter.filter(fifo_output.data() + 1,buff.data());
		}
		else
		{
			size_t output_length = filter.filter(fifo_output.data(),buff.data());
		}

//		std::cout << "OUTPUT LENGTH :\t" << output_length << std::endl;
		data_fifo2.push(buff);

		filter.set_head(true); //noncontinous filtering

        }
	printer.print("Graceful exit of FILTER_THREAD");
        return;
}
