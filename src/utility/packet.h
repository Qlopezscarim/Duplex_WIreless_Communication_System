#ifndef PACKET_H
#define PACKET_H

#include <bitset>
#include <vector>
#include <cstdint>
#include "impulse.h"
#include <math.h>

#ifdef BPSK
	#define H2_PADDING (ceil(H2_LENGTH/2))					//this padding only accounts for delay from filters
	#define PACKET_SIZE_NO_PADDING (1092) 					//13 bit pre-amble, 63 bits acq, 16 bit packet header, 1000 bit payload
	#define PACKET_SIZE_BASE ( 1092 + ceil(H2_LENGTH / 2) )  		//base total packet/tx size
	#define PACKET_SIZE PACKET_SIZE_BASE					//packet size adjusted for continous filtering
#endif

#ifdef QPSK
	#define H2_PADDING (ceil(H2_LENGTH/2) + 1)                                    //this padding only accounts for delay from filters
        #define PACKET_SIZE_NO_PADDING (1094/2) //Two bits are padding in QPSK:547                                   //13 bit pre-amble, 63 bits acq, 16 bit packet header, 1000 bit payload
        #define PACKET_SIZE_BASE (PACKET_SIZE_NO_PADDING + ceil(H2_LENGTH / 2) + 1 )                  //base total packet/tx size
        #define PACKET_SIZE PACKET_SIZE_BASE                                    //packet size adjusted for continous filtering
#endif

//these are agnostic - definitions not in symbols:
#define M_SEQ_SIZE 63
#define PREAMBLE_SIZE 13

struct packet
{
	std::bitset<M_SEQ_SIZE> mseq;
	std::bitset<13> preamble;
	std::vector<uint8_t> payload;
	uint16_t pack_num;
	uint32_t padding = PACKET_SIZE - PACKET_SIZE_NO_PADDING;


	//defualt constructor
	packet()
	{
		//Will never dry allocate a packet unless it's going to be overwritten by a fifo
		//don't want to waste time with defualt values for it to be overwritten
		//hence we do nothing here
	}

	packet(std::bitset<13> in_preamble,std::bitset<M_SEQ_SIZE>& in_mseq, uint16_t in_pack_num,std::vector<uint8_t>& in_payload)
	{
		preamble = in_preamble;
		mseq = in_mseq;
		payload = in_payload;
		pack_num = in_pack_num;
	}
};


#endif //PACKET_H
