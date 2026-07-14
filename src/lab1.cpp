/*
//	Code written from reference 2.4.8 txrx_loopback_to_file.cpp (RX)
*/

#include <uhd/utils/safe_main.hpp>
#include <csignal>
#include "lab1.h"
#include "RX.h"
#include "TX.h"
#include "POWER_THREAD.h"
#include "AVG_POWER_THREAD.h"
#include "Shared_Printer.h"
#include "multi_filter.h"
#include "utility/impulse.h"
#include "TX_producer.h"
#include "TX_modulator.h"
#include "file_gen.h"
#include <cstdint>
#include <bitset>
#include "Correlator.h"
#include "Demodulator.h"
#include "Carrier_sync.h"

namespace po = boost::program_options;

//Tracks if first packet - lets us know if we want to store it to the output file
bool first_packet = true;

// Interrupt signal handler
bool stop_signal_called = false;
void sig_int_handler(int) {
    stop_signal_called = true;
}

int UHD_SAFE_MAIN(int argc, char *argv[]) {
    //INPUT HANDLING WITH BOOST
    //TX paramters to be set
	std::string tx_args, wave_type, tx_ant, tx_subdev, ref, otw, tx_channels;
	double tx_rate, tx_freq, tx_gain, wave_freq, tx_bw;
	float ampl;

    //RX parameters to be set
	std::string rx_args, file, type, rx_ant, rx_subdev, rx_channels;
	size_t total_num_samps, spb, sblocks;
	double rx_rate, rx_freq, rx_gain, rx_bw;
	double settling;


    // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("tx-args", po::value<std::string>(&tx_args)->default_value(""), "uhd transmit device address args")
        ("rx-args", po::value<std::string>(&rx_args)->default_value(""), "uhd receive device address args")
        ("file", po::value<std::string>(&file)->default_value("no"), "name of the file to write binary samples to (Do not name it no)")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type in file: double, float, or short")
        ("sblocks", po::value<size_t>(&sblocks)->default_value(10000), "total size of blocks for recieve")
        ("settling", po::value<double>(&settling)->default_value(double(0.2)), "settling time (seconds) before receiving")
        ("spb", po::value<size_t>(&spb)->default_value(0), "samples per buffer, 0 for default")
        ("tx-rate", po::value<double>(&tx_rate), "rate of transmit outgoing samples")
        ("rx-rate", po::value<double>(&rx_rate), "rate of receive incoming samples")
        ("tx-freq", po::value<double>(&tx_freq), "transmit RF center frequency in Hz")
        ("rx-freq", po::value<double>(&rx_freq), "receive RF center frequency in Hz")
        ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of the waveform [0 to 0.7]")
        ("tx-gain", po::value<double>(&tx_gain), "gain for the transmit RF chain")
        ("rx-gain", po::value<double>(&rx_gain), "gain for the receive RF chain")
        ("tx-ant", po::value<std::string>(&tx_ant), "transmit antenna selection")
        ("rx-ant", po::value<std::string>(&rx_ant), "receive antenna selection")
        ("tx-subdev", po::value<std::string>(&tx_subdev), "transmit subdevice specification")
        ("rx-subdev", po::value<std::string>(&rx_subdev), "receive subdevice specification")
        ("tx-bw", po::value<double>(&tx_bw), "analog transmit filter bandwidth in Hz")
        ("rx-bw", po::value<double>(&rx_bw), "analog receive filter bandwidth in Hz")
        ("wave-type", po::value<std::string>(&wave_type)->default_value("CONST"), "waveform type (CONST, SQUARE, RAMP, SINE)")
        ("wave-freq", po::value<double>(&wave_freq)->default_value(0), "waveform frequency in Hz")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "clock reference (internal, external, mimo)")
        ("otw", po::value<std::string>(&otw)->default_value("sc16"), "specify the over-the-wire sample mode")
        ("tx-channels", po::value<std::string>(&tx_channels)->default_value("0"), "which TX channel(s) to use (specify \"0\", \"1\", \"0,1\", etc)")
        ("rx-channels", po::value<std::string>(&rx_channels)->default_value("0"), "which RX channel(s) to use (specify \"0\", \"1\", \"0,1\", etc)")
        ("tx-int-n", "tune USRP TX with integer-N tuning")
        ("rx-int-n", "tune USRP RX with integer-N tuning")
    ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    //END OF INPUT HANDLING WITH BOOST

    //Print out help message if neccessary/asked for 
    if (vm.count("help")) {
        std::cout << "Lab1 " << desc << std::endl;
        return ~0;
    }

    // Set the signal handler
    std::signal(SIGINT, &sig_int_handler);
    // Keep running until CTRL-C issued

    // Parameter checking
    bool gain_set	= false;
    bool bw_set 	= false;
    bool ant_set	= false;
    // RX_RATE check
    if (not vm.count("rx-rate") && not vm.count("tx-rate")) 
    {         
	std::cerr << "Please specify the sample rate with --rx-rate" << std::endl;
        std::cerr << "or please specify the sample rate with --tx-rate" << std::endl;
	return ~0;     
    }
   if (not vm.count("rx-freq") && not vm.count("tx-freq"))
   {
	std::cerr << "Please specify the center frequency with --rx-freq" <<
	"or please specify the center frequency with --tx-freq"
	<< std::endl;
	return ~0;
   }
   std::cout << "NOTE NO IMPLEMENTATION FOR rx-int-n CURRENTLY" << std::endl;
   if(vm.count("rx-gain") || vm.count("tx-gain"))
   {
   gain_set = true;
   }
   if(vm.count("rx-bw") || vm.count("tx-bw"))
   {
   bw_set = true;
   }
   if(vm.count("rx-ant") || vm.count("tx-ant"))
   {
   ant_set = true;
   }


//From here we dictate if tx or rx arguments are passed - TX takes priority though if passed
if(vm.count("tx-freq"))
{
	std::cout << "\n Please note that the program is running in Tx mode" << std::endl;

//instantiating FIFO from filt_thread to file_gen
MutexFIFO<std::vector<std::complex<float>>> data_fifo;

//instantiating FIFO from modulator to filt_thread
MutexFIFO<std::vector<std::complex<float>>> data_fifo2;

//instantiating FIFO from tx_producer to modulator
MutexFIFO<packet> data_fifo3;

//instantiating FIFO from file_gen to tx_thread
MutexFIFO<std::vector<std::complex<float>>> data_fifo4;

//instantiating shared printer
SharedPrinter printer;

//instantiating the data producer thread

size_t size_of_producer_blocks = 125;

std::thread producer_thread(TX_producer::producer,
std::ref(data_fifo3),
std::ref(size_of_producer_blocks),
std::ref(printer)
);


//instantiating modulator thread
std::thread modulator_thread(TX_modulator::modulator,
std::ref(data_fifo3),
std::ref(data_fifo2),
std::ref(data_fifo), //To keep the filter thread modular for RX we don't want to throttle our FIFO queues downstream:
		     // we would rather do it here for TX only by just forcefull producing less: without this we grow exponentially
std::ref(printer)
);


//instantiating filter thread

int upsample_rate = 5;
int downsample_rate = 4;
size_t length_of_filter = (H2_LENGTH * 2) +1;
size_t size_of_packet = PACKET_SIZE; //huh?
std::complex<float> h2[H2_LENGTH * 2 + 1]; //actual length of impulse response
rrc_pulse(h2, H2_LENGTH, upsample_rate, downsample_rate);

/*for(int i =0; i< H2_LENGTH*2+1;i++)
{
        std::cout << h2[i].real() << "\t" << h2[i].imag() << std::endl;
}*/


std::thread filt_thread(Multi_Filter::filter_thread,
std::ref(data_fifo2), //go to fifo2 later
std::ref(data_fifo),
std::ref(size_of_packet),
std::ref(printer),
std::ref(upsample_rate),
std::ref(downsample_rate),
std::ref(length_of_filter),
std::ref(h2),
false
);



#ifdef DEBUGTX //If we are debugging output the file otherwise don't
std::cout << "We are debugging the TX chain and not actually outputting anything - please change the compiler options to stop this" << std::endl;
//garbage fifo instantiation (doesn't go anywhere!)
MutexFIFO<std::vector<std::complex<float>>> data_fifo_ignore;
//instantiating modulator thread
std::thread file_gen_thread(FILE_GEN::file_thread,
std::ref(data_fifo),
std::ref(data_fifo_ignore),
std::ref(printer),
std::ref(file)
);

#else
size_t size_block_transmit = (size_of_packet * upsample_rate / downsample_rate);
//instantiating TX thread:
std::thread worker = TX::tx_thread(
        "fc32",
        otw,
        "",
        tx_channels,
        tx_args,
        ref,
        tx_rate,
        tx_freq,
        gain_set,
        tx_gain,
        bw_set,
        tx_bw,
        ant_set,
        tx_ant,
        settling, //time
        std::ref(size_block_transmit), //size of blocks
        std::ref(data_fifo),
	std::ref(printer));
#endif


size_t count = 0;
while(not stop_signal_called)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep fo>        count++;
        count++;
	if(count % 1000 == 0)
        {
                std::cout << "FIFO from filter_thread to TX_thread:\t\t\t\t" << data_fifo.size() << std::endl;
                std::cout << "FIFO from modulator_thread to filter_thread:\t\t\t" << data_fifo2.size()<< std::endl;
                std::cout << "FIFO from producer_thread to modulator_thread:\t\t\t" << data_fifo3.size()<< std::endl;
#ifdef DEBUGTX
		std::cout << "Debugging thread \t" << data_fifo_ignore.size();
#endif
        }
    }
    //want other threads to exit cleanly before exiting program:
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // sleep for>
#ifdef DEBUGTX
    file_gen_thread.join();
#else
    worker.join();
#endif
    producer_thread.join();
    filt_thread.join();
    modulator_thread.join();
    std::cout << "Clean program exit; all threads joined back forcefully" << std::endl;

return EXIT_SUCCESS;
}
else /***********************START OF RX CODE*************************************************/
{
	std::cout << "\n Please note that the program is running in Rx mode" << std::endl;

//instantiating FIFO from filter to POWER_THREAD
MutexFIFO<std::vector<std::complex<float>>> data_fifo;

//instantiating FIFO for POWER_THREAD to avg_power_thread
MutexFIFO<std::vector<std::complex<float>>> data_fifo2;

//instantiating FIFO for RX to filter
MutexFIFO<std::vector<std::complex<float>>> data_fifo3;

//instantiating FIFO from AVG_POWER to match filter
MutexFIFO<std::vector<std::complex<float>>> data_to_match_filter;

//instantiating FIFO from Correlator to Carrier
MutexFIFO<std::vector<std::complex<float>>> Correlator_to_Carrier;

//instantiating FIFO from Carrier to demodulator
MutexFIFO<std::vector<std::complex<float>>> Correlator_to_Demodulator;

//instantiating FIFO from AVG_POWER to match filter
MutexFIFO<size_t> Correlator_peak;




//instantiating shared printer
SharedPrinter printer;

//instantiating packet size variable for ref passing - relic of old power thread
size_t pack_size = SIZE_BLOCKS_TO_STORE;


int upsample_rate = 8;
int downsample_rate = 1;
size_t length_of_filter = (H2_LENGTH*2)+1;
size_t size_of_packet = SIZE_BLOCKS_TO_STORE;
std::complex<float> h3[H2_LENGTH * 2 + 1]; //actual length of impulse response
rrc_pulse_b_25(h3, H2_LENGTH, upsample_rate, downsample_rate);

/*for (int i = 0; i<H2_LENGTH * 2 + 1; i++)
{
	std::cout << h3[i].real() << " " << h3[i].imag() << std::endl;
}*/

#ifdef DEBUG2
//nothing
#else
//Applying this filter after packet detection and AGC
std::thread filt_thread(Multi_Filter::filter_thread,
std::ref(data_to_match_filter),
std::ref(data_fifo3),
std::ref(size_of_packet),
std::ref(printer),
std::ref(upsample_rate),
std::ref(downsample_rate),
std::ref(length_of_filter),
std::ref(h3),
true //skip the packet number
);
#endif

//size_t fil_output_size = (sblocks * Multi_Filter::U)/(Multi_Filter::D);
//size_t fil_output_size = sblocks;

std::thread power_proc1(POWER::power_thread,
std::ref(data_fifo),
std::ref(data_fifo2),
std::ref(printer)
);


std::thread power_proc2(AVG_POWER::power_thread,
std::ref(data_fifo2),
std::ref(data_to_match_filter),
std::ref(pack_size),
std::ref(printer),
file//name of file is a std::string
);


//instantiating RX thread:
std::thread worker = RX::rx_thread(
        "fc32",
        otw,
        "",
        rx_channels,
        rx_args,
        ref,
        rx_rate,
        rx_freq,
        gain_set,
        rx_gain,
        bw_set,
        rx_bw,
        ant_set,
        rx_ant,
        settling, //time
        std::ref(sblocks), //size of blocks
        std::ref(data_fifo));

#ifdef DEBUG2 //If we are debugging output the file otherwise don't
//garbage fifo instantiation (doesn't go anywhere!)
MutexFIFO<std::vector<std::complex<float>>> data_fifo_ignore;
//instantiating modulator thread
std::thread file_gen_thread(FILE_GEN::file_thread,
std::ref(data_to_match_filter),
std::ref(data_fifo_ignore),
std::ref(printer),
std::ref(file)
);
#endif


#ifdef DEBUG1 //If we are debugging output the file otherwise don't
//garbage fifo instantiation (doesn't go anywhere!)
MutexFIFO<std::vector<std::complex<float>>> data_fifo_ignore;
//instantiating modulator thread
std::thread file_gen_thread(FILE_GEN::file_thread,
std::ref(data_fifo3),
std::ref(data_fifo_ignore),
std::ref(printer),
std::ref(file)
);

size_t net_upsample = 10;
std::thread correlator_thread(Correlator::correlator,
        std::ref(data_fifo_ignore),
	std::ref(Correlator_to_Carrier),
	std::ref(Correlator_peak),
        std::ref(printer),
	net_upsample
    );
#else

size_t net_upsample = 10;
std::thread correlator_thread(Correlator::correlator,
        std::ref(data_fifo3),
	std::ref(Correlator_to_Carrier),
	std::ref(Correlator_peak),
        std::ref(printer),
	net_upsample
    );

#endif

//RELIC OF DBPSK
/*std::thread demodulator_thread(Demodulator::demodulate,
        std::ref(Correlator_to_Demodulator),
        std::ref(Correlator_peak),
        std::ref(printer),
        net_upsample
    );
*/

//Handles carrier offset

std::thread carrier_sync_thread(Carrier_sync::carrier_sync,
	std::ref(Correlator_to_Carrier),
	std::ref(Correlator_to_Demodulator),
	std::ref(printer)
	);


/*MutexFIFO<std::vector<std::complex<float>>> data_fifo_ignore;
//instantiating modulator thread
std::thread file_gen_thread(FILE_GEN::file_thread,
std::ref(Correlator_to_Demodulator),
std::ref(data_fifo_ignore),
std::ref(printer),
std::ref(file)
);*/

//Typical demodulator
std::thread demodulator_thread(Coherent_demodulator::coherent_demodulate,
        std::ref(Correlator_to_Demodulator),
        std::ref(printer)
    );

//garbage to figure out m_seq modulation
std::vector<std::complex<float>> mod_m = init_mod_mseq();
for (auto& i:mod_m)
{
	std::cout << i.real() <<"," << i.imag() << "j," << std::endl;
}


    size_t count = 0;
    while(not stop_signal_called)
    {
	std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep for 1ms to make sure can catch CTRL-C
	count++;
	if(count % 3000 == 0)
	{
		std::cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t|FIFO 1:\t" << data_fifo.size() << std::endl;
		std::cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t|FIFO 2:\t" << data_fifo2.size()<< std::endl;
		std::cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t|FIFO 3:\t" << data_fifo3.size()<< std::endl;
		std::cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t|FIFO Correlator to Carrier " <<  Correlator_to_Carrier.size() << std::endl;
		std::cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t|FIFO Carrier to Demodulator " <<  Correlator_to_Demodulator.size() << std::endl;
	}
    }
    //want other threads to exit cleanly before exiting program:
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // sleep for 1ms to make
    
    worker.join();
    filt_thread.join();
    power_proc1.join();
    power_proc2.join();
    correlator_thread.join();
    carrier_sync_thread.join();
    demodulator_thread.join();

#if defined(DEBUG1) || defined(DEBUG2)
    file_gen_thread.join();
    #ifndef DEBUG2
        filter_thread.join();
    #endif
#endif

    std::cout << "Clean program exit; all threads joined back forcefully" << std::endl;
    return EXIT_SUCCESS;
}

}

