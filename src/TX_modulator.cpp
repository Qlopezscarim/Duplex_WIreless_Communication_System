#include "TX_modulator.h"
extern bool stop_signal_called;


#ifdef BPSK
void TX_modulator::modulator(
    MutexFIFO<packet>& data_fifo,
    MutexFIFO<std::vector<std::complex<float>>>& modulated_fifo,
    MutexFIFO<std::vector<std::complex<float>>>& throttle_fifo,
    SharedPrinter& printer
)
{
    bool first_iter = true;
    while(not stop_signal_called)
        {
		if(modulated_fifo.size() > 10 || throttle_fifo.size() > 10) //only really need 10 packets queued up - anything more is just over the top
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
                else
                {
			/*
			Grabs blocks of integers to modulate
			*/
			packet Packet;
			while( not data_fifo.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                	{
                        	if( stop_signal_called )
                        	{
                        		std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                        		return;
                        	}
                        	std::this_thread::sleep_for(std::chrono::milliseconds(1));
                	}

			
			/*
			Modulates grabed integer blocks and pushes them to modulated_fifo
			*/
			std::vector<std::complex<float>> symbols;
			

			//Modulation done is MSB sequence!

			//Modulating the preamble
			std::bitset<13> prebits = Packet.preamble;
                        for (int i = static_cast<int>(prebits.size())-1; i >= 0; i--)
                        {
                                bool bit = prebits[i];

                                //debugging chunk
                                //if(first_iter == true)
                                //std::cout << i <<"\tMseq modulating a value of " << bit << std::endl;

                                // Map 0 -> +1, 1 -> -1
                                float val = 1.0f - 2.0f * static_cast<float>(bit);

                                //debugging chunk
                                //if(first_iter == true)
                                //std::cout << "Preseq pushed a value of " << val << std::endl;

                                symbols.emplace_back(val, 0.0f);
                        }

			//Modulating the m-sequence
			std::bitset<M_SEQ_SIZE> mbits = Packet.mseq;
			for (int i = static_cast<int>(mbits.size())-1; i >= 0; i--)
			{
				bool bit = mbits[i];
				
				//debugging chunk
				//if(first_iter == true)
                                //std::cout << i <<"\tMseq modulating a value of " << bit << std::endl;
				
				// Map 0 -> +1, 1 -> -1
                                float val = 1.0f - 2.0f * static_cast<float>(bit);
				
				//debugging chunk
				if(first_iter == true)
				std::cout << "Mseq pushed a value of " << val << std::endl;
                                
				symbols.emplace_back(val, 0.0f);
    			}

			//Modulating the packet number:
			uint16_t pack_bits = Packet.pack_num;
			for (int8_t i = 15; i >= 0; --i)
                        {
                                bool bit = 1 & ( pack_bits >> i);
                                // Map 0 -> +1, 1 -> -1
                                float val = (1.0f - 2.0f * static_cast<float>(bit));
				
				//debugging chunk
				//if(first_iter == true)
				//std::cout << "pack_num pushed a value of " << val << std::endl;
                                
				symbols.emplace_back(val, 0.0f);
                        }

			//Modulating the rand_payload
			std::vector<uint8_t> payload = Packet.payload;
			for(uint8_t byte : payload)
			{
				for(int8_t i=7; i>=0; --i)
				{
					bool bit = 1 & ( byte >> i);
                                	// Map 0 -> +1, 1 -> -1
                                	float val = (1.0f - 2.0f * static_cast<float>(bit));
					
					//debugging chunk
					if(first_iter == true)
					std::cout << bit << "," << std::endl;
                                	
					symbols.emplace_back(val, 0.0f);
				}
			}

			//Padding to account for filter implementation truncating
			for(int i = 0; i<Packet.padding; i++)
			{
				symbols.emplace_back(0.0f, 0.0f);
			}

			if(first_iter == true)
			{
				/*
				for(int i = 0; i < symbols.size(); i++)
				{
					std::cout << "mod input: " << symbols[i].real() << std::endl;
				}
				*/
				first_iter = false;
			}
			modulated_fifo.push(symbols);
			//std::cout << "Successful push to modulated_fifo " << std::endl;
                }
        }
	std::cout << "Graceful exit of TX_modulator thread" << std::endl;
        return;

}
#endif

#ifdef QPSK
void TX_modulator::modulator(
    MutexFIFO<packet>& data_fifo,
    MutexFIFO<std::vector<std::complex<float>>>& modulated_fifo,
    MutexFIFO<std::vector<std::complex<float>>>& throttle_fifo,
    SharedPrinter& printer
)
{
    bool first_iter = true;
    while(not stop_signal_called)
        {
                if(modulated_fifo.size() > 10 || throttle_fifo.size() > 10) //only really need 10 packets queued up - anything more is just over the top
                {
                        std::this_thread::sleep_for(std::chrono::milliseconds(3));
                }
                else
                {
                        /*
                        Grabs blocks of integers to modulate
                        */
                        packet Packet;
                        while( not data_fifo.pop(Packet) ) //keep trying to pop until the FIFO has data ; not needed but more explicit
                        {
                                if( stop_signal_called )
                                {
                                        std::cout << "Graceful exit of TX_modulator thread" << std::endl;
                                        return;
                                }
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }


                        /*
                        Modulates grabed integer blocks and pushes them to modulated_fifo
                        */
                        std::vector<std::complex<float>> symbols;


                        //Modulation done is MSB sequence!

                        //Modulating the preamble


			//We expect 7 symbols from the pre-amble (a zero is padded)
			//We expect 32 symbols from the m-sequ (a zero is padded)
			//We expect 8 symbols from the packet number
			//We expect 500 symbols from the payload
			//We expect a padding of 100
			//Total size is 647


			std::bitset<13> prebits = Packet.preamble; //this shouldn't be hardcoded
			size_t n = prebits.size();

    			for (int i = static_cast<int>(n) - 2; i >= 0; i -= 2)
    			{
        			bool bit0 = prebits[i];     // MSB
        			bool bit1 = prebits[i + 1]; // LSB

        			// Gray-coded QPSK mapping
        			float real = (bit0 == 0) ? 1.0f : -1.0f;
        			float imag = (bit1 == 0) ? 1.0f : -1.0f;

        			// Normalize to unit power
        			std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
        			symbols.emplace_back(sym);
    			}

    			//handling it being odd
    			if(n%2 != 0)
    			{
        			bool bit1 = prebits[0]; //MSB

		        	// Gray-coded QPSK mapping
        			float real = (0 == 0) ? 1.0f : -1.0f;
        			float imag = (bit1 == 0) ? 1.0f : -1.0f;

        			// Normalize to unit power
        			std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
        			symbols.emplace_back(sym);
    			}

                        //Modulating the m-sequence
                        std::bitset<M_SEQ_SIZE> mbits = Packet.mseq;
			n = M_SEQ_SIZE;

                        for (int i = static_cast<int>(n) - 2; i >= 0; i -= 2)
                        {
                                bool bit0 = mbits[i];     // MSB
                                bool bit1 = mbits[i + 1]; // LSB

                                // Gray-coded QPSK mapping
                                float real = (bit0 == 0) ? 1.0f : -1.0f;
                                float imag = (bit1 == 0) ? 1.0f : -1.0f;

                                // Normalize to unit power
                                std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
                                symbols.emplace_back(sym);
                        }

                        //handling it being odd
                        if(n%2 != 0)
                        {
                                bool bit1 = mbits[0]; //MSB

                                // Gray-coded QPSK mapping
                                float real = (0 == 0) ? 1.0f : -1.0f;
                                float imag = (bit1 == 0) ? 1.0f : -1.0f;

                                // Normalize to unit power
                                std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
                                symbols.emplace_back(sym);
                        }

                        //Modulating the packet number:
                        uint16_t pack_bits = Packet.pack_num;
			n = 16;

                        for (int i = static_cast<int>(n) - 2; i >= 0; i -= 2)
                        {
                                bool bit0 = 1 & ( pack_bits >> i);     // LSB in just the context of QPSK
                                bool bit1 = 1 & ( pack_bits >> i+1);   // MSB

                                // Gray-coded QPSK mapping
                                float real = (bit0 == 0) ? 1.0f : -1.0f;
                                float imag = (bit1 == 0) ? 1.0f : -1.0f;

                                // Normalize to unit power
                                std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
                                symbols.emplace_back(sym);
                        }

			//packet number shall never be odd

                        //Modulating the rand_payload
                        std::vector<uint8_t> payload = Packet.payload;
                        for(uint8_t byte : payload)
                        {
				for (int i = 8 - 2; i >= 0; i -= 2)
                        	{
                                	bool bit0 = 1 & ( byte >> i);     // MSB
                                	bool bit1 = 1 & ( byte >> i+1);; // LSB

                                	// Gray-coded QPSK mapping
                                	float real = (bit0 == 0) ? 1.0f : -1.0f;
                                	float imag = (bit1 == 0) ? 1.0f : -1.0f;

                                	// Normalize to unit power
                                	std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
                                	symbols.emplace_back(sym);
                        	}
                        }

                        //Padding to account for filter implementation truncating
                        for(int i = 0; i<Packet.padding; i++)
                        {
                                symbols.emplace_back(0.0f, 0.0f);
                        }

                        if(first_iter == true)
                        {
                                /*
                                for(int i = 0; i < symbols.size(); i++)
                                {
                                        std::cout << "mod input: " << symbols[i].real() << std::endl;
                                }
                                */
                                first_iter = false;
                        }
                        modulated_fifo.push(symbols);
                        //std::cout << "Successful push to modulated_fifo " << std::endl;
                }
        }
        std::cout << "Graceful exit of TX_modulator thread" << std::endl;
        return;

}

#endif
