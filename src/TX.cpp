#include "TX.h"



extern bool stop_signal_called;

std::thread TX::tx_thread
(	const std::string& cpu_format,
	const std::string& wire_format,
	const std::string& file,
	const std::string& tx_channels,
	const std::string& tx_args,
	std::string& ref,
	double& tx_rate,
	double& tx_freq,
	bool& set_gain,
	double& tx_gain,
	bool& bw_set,
	double& tx_bw,
	bool& ant_set,
	std::string& tx_ant,
	double& settling_time,
	size_t& sblocks,
	MutexFIFO<std::vector<std::complex<float>>>& data_fifo,
	SharedPrinter& printer
)
{

//Create the initial tx_usrp object from passed boost parameters
std::cout << boost::format("Creating the receive usrp device with: %s...") % tx_args
	<< std::endl;
uhd::usrp::multi_usrp::sptr tx_usrp = uhd::usrp::multi_usrp::make(tx_args);

/*
//Verifying passed RX channels are valid:
//rx_channel_nums holds all valid channel numbers at the end
std::vector<std::string> rx_channel_strings;
std::vector<size_t> rx_channel_nums;     
boost::split(rx_channel_strings, rx_channels, boost::is_any_of("\"',"));
for (size_t ch = 0; ch < rx_channel_strings.size(); ch++) 
{         
	size_t chan = std::stoi(rx_channel_strings[ch]);
        if (chan >= rx_usrp->get_rx_num_channels()) 
	{             
		throw std::runtime_error("Invalid RX channel(s) specified.");         
	}
	 else            
	{
		 rx_channel_nums.push_back(std::stoi(rx_channel_strings[ch]));
	}
}*/

// detect which tx channels to use
    std::vector<std::string> tx_channel_strings;
    std::vector<size_t> tx_channel_nums;
    boost::split(tx_channel_strings, tx_channels, boost::is_any_of("\"',"));
    for (size_t ch = 0; ch < tx_channel_strings.size(); ch++) {
        size_t chan = std::stoi(tx_channel_strings[ch]);
        if (chan >= tx_usrp->get_tx_num_channels()) {
            throw std::runtime_error("Invalid TX channel(s) specified.");
        } else
            tx_channel_nums.push_back(std::stoi(tx_channel_strings[ch]));
    }


//Locking mboard clock for tx
if(ref == "internal")
{
	std::cout << "setting mboard clock only for TX with parameter " << ref;
}
tx_usrp->set_clock_source(ref);    


//More verbose output:
std::cout << "Using TX Device: " << tx_usrp->get_pp_string();


//setting TX rate
std::cout << boost::format("Setting TX Rate: %f Msps...") % (tx_rate / 1e6)  << std::endl;
tx_usrp->set_tx_rate(tx_rate);
std::cout << boost::format("Actual TX Rate: %f Msps...")  % (tx_usrp->get_tx_rate() / 1e6)
<< std::endl               
<< std::endl;


//setting TX center frequency
for (size_t ch = 0; ch < tx_channel_nums.size(); ch++) 
{         
	//size_t channel = rx_channel_nums[ch];         
	//if (rx_channel_nums.size() > 1) 
	//{             
	std::cout << "Configuring TX Channel " << tx_channel_nums[0] << " ONLY" << std::endl;
	//}
}
size_t channel = tx_channel_nums[0];
std::cout << boost::format("Setting TX Freq: %f MHz...") % (tx_freq / 1e6)  << std::endl;
uhd::tune_request_t tx_tune_request(tx_freq);
//if (vm.count("rx-int-n"))             
//	rx_tune_request.args = uhd::device_addr_t("mode_n=integer");
tx_usrp->set_tx_freq(tx_tune_request, channel);
std::cout << boost::format("Actual TX Freq: %f MHz...")   % (tx_usrp->get_tx_freq(channel) / 1e6) 
 << std::endl
 << std::endl;

//setting TX gain
if(set_gain)
{
	std::cout << boost::format("Setting TX Gain: %f dB...") % tx_gain
        << std::endl;
        tx_usrp->set_tx_gain(tx_gain, channel);
        std::cout << boost::format("Actual TX Gain: %f dB...")  % tx_usrp->get_tx_gain(channel)
        << std::endl
        << std::endl;
}

// set the receive analog frontend filter bandwidth
if (bw_set) 
{             
	std::cout << boost::format("Setting TX Bandwidth: %f MHz...") % (tx_bw / 1e6)
        << std::endl;
        tx_usrp->set_tx_bandwidth(tx_bw, channel);
        std::cout << boost::format("Actual TX Bandwidth: %f MHz...")  % (tx_usrp->get_tx_bandwidth(channel) / 1e6)
        << std::endl
        << std::endl;         
}         
// set the receive antenna
if (ant_set)
    tx_usrp->set_tx_antenna(tx_ant, channel);

// enable RX DC offset correction
//std::cout << "Enabling TX DC offset correction..." << std::endl;
//tx_usrp->set_tx_dc_offset(true, channel);

//check LO locked
std::vector<std::string> tx_sensor_names;
tx_sensor_names = tx_usrp->get_tx_sensor_names(0);
if (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "lo_locked")  != tx_sensor_names.end()) 
{
         uhd::sensor_value_t lo_locked = tx_usrp->get_tx_sensor("lo_locked", 0);
         std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string()
                   << std::endl;         
	bool is_locked = false;
	for(int i=0; i<20;i++)
	{
		if(lo_locked.to_bool())
		{
			is_locked = true;
			break;
		}
		std::cout << "checking LO; iteration " << i  << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		lo_locked = tx_usrp->get_tx_sensor("lo_locked",0);
		//UHD_ASSERT_THROW(lo_locked.to_bool());
	}
	if(!is_locked)
	{
		std::cerr << "Warning: TX LO FAILED AFTER 20 RETRIES";
	}
	else
	{
		std::cout << "\nTX LO locked successfully" << std::endl;
	}
} 
std::cout << "TX worker thread expects std::complex<float> in, be aware of that" << std::endl;

std::thread worker(&TX::tx_worker,
                       tx_usrp,
                       cpu_format,
                       wire_format,
                       file,
		       sblocks,
		       REQ_SAMPLES,
                       settling_time,
                       tx_channel_nums,
                       std::ref(data_fifo),
		       std::ref(printer)
                       );

return worker;
}







void TX::tx_worker
(
uhd::usrp::multi_usrp::sptr tx_usrp,
const std::string& cpu_format,
const std::string& wire_format,
const std::string& file,
size_t samps_per_buff,
int num_requested_samples,
double settling_time,
std::vector<size_t> tx_channel_nums,
MutexFIFO<std::vector<std::complex<float>>>& data_fifo, //may need to be changed
SharedPrinter& printer
)
{

std::cout << "Worker thread actually instantiated";


bool overflow_message = true; //for error checking
// create a transmit streamer
uhd::stream_args_t stream_args(cpu_format, wire_format);
stream_args.channels             = tx_channel_nums;
uhd::tx_streamer::sptr tx_stream = tx_usrp->get_tx_stream(stream_args);

std::vector<std::complex<float>> fifo_output;

uhd::tx_metadata_t md;
md.start_of_burst = true;   // Beginning of packet
md.end_of_burst = true;     // End of packet
md.has_time_spec = false;

std::cout << "\nTransmitting blocks of :\t" << samps_per_buff << std::endl;

while(not stop_signal_called)
{
	while( not data_fifo.pop(fifo_output) ) //keep trying to pop until the FIFO has data
                {
                        if( stop_signal_called )
                        {
                        printer.print("Graceful exit of TX_THREAD");
                        return;
                        }
			std::cout << "ERROR WE ARE NOT PRODUCING ENOUGH" << std::endl;
                        //std::this_thread::sleep_for(std::chrono::milliseconds(1)); //could get rid of this to get closer to true one packet every second

                }
	std::cout << "We have a fifo of size:\t" << fifo_output.size() << std::endl;
	std::cout << "We ate transmitting a total of:\t" << samps_per_buff << std::endl;
	//In this case the FIFO had something added and we are ready to transmit
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	size_t num_tx_samps = tx_stream->send(fifo_output.data(), samps_per_buff, md);
	std::cout << "UHD REPORTS THAT WE HAVE SEND X SAMPLES: X=\t" << num_tx_samps;
	if (num_tx_samps < samps_per_buff) {
    		std::cerr << "TX timeout: only sent " << num_tx_samps 
              << " of " << samps_per_buff << " samples" << std::endl;
	}
	else
	{
		//for debugging
		//std::cout << "Successfull transmittion" << std::endl;
	}
}

/*bool overflow_message = true; //for error checking
// create a receive streamer
uhd::stream_args_t stream_args(cpu_format, wire_format);
stream_args.channels             = tx_channel_nums;
//stream_args.spp			 = samps_per_buff;
uhd::rx_streamer::sptr tx_stream = tx_usrp->get_tx_stream(stream_args);

uhd::rx_metadata_t md;
std::vector<std::complex<float>> buff (samps_per_buff); //samps per buff is our block size
uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
stream_cmd.stream_now = true; //this is fine for now but if we add more initialization may want to stall
tx_stream->issue_stream_cmd(stream_cmd);
double timeout = settling_time + 0.5f;
size_t num_block = 0;
while(not stop_signal_called)
{
	//buff[0] = std::complex<float>(num_block,0.0f);
	rx_stream->recv(&buff.front(),samps_per_buff, md, timeout);
	timeout		= 0.1f;
	if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            std::cout << "Timeout while streaming" << std::endl;
            break;
        }

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
            if (overflow_message) {
                overflow_message = false;
                std::cerr
                    << boost::format(
                           "Got an overflow indication. Please consider the following:\n"
                           "  Your write medium must sustain a rate of %fMB/s.\n"
                           "  Dropped samples will not be written to the file.\n"
                           "  Please modify this example for your purposes.\n"
                           "  This message will not appear again.\n")
                           % (rx_usrp->get_rx_rate() * sizeof(std::complex<float>) / 1e6);
            }
            continue;
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            throw std::runtime_error("Receiver error " + md.strerror());
        }
        data_fifo.push(buff); //This makes a copy which we want to re-use the buffer
	//num_block++;
        //then are happy to fill with new samples and repeat this process
}
//clean exit code
stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
rx_stream->issue_stream_cmd(stream_cmd);
*/
std::cout << "RX thread exited cleanly" << std::endl;

}
