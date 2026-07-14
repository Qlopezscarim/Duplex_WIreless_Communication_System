#include <bitset>
#pragma once
//old 31 bit length sequence
//inline std::bitset<M_SEQ_SIZE> m_seq = 0b1111100011011101010000100101100;

inline std::bitset<M_SEQ_SIZE> m_seq = 0b111111010101100110111011010010011100010111100101000110000100000;

inline std::bitset<13> global_preamble = 0b0000100101100;

#ifdef BPSK
//static  mod_m_seq()
//Modulating the m-sequence
inline std::vector<std::complex<float>> init_mod_mseq()
{
	std::vector<std::complex<float>> symbols;
	for (int i = static_cast<int>(m_seq.size())-1; i >= 0; i--)
	{
		bool bit = m_seq[i];


		// Map 0 -> +1, 1 -> -1
		float val = 1.0f - 2.0f * static_cast<float>(bit);

		symbols.emplace_back(val, 0.0f);
	}
	return symbols;
}
#endif


#ifdef QPSK
inline std::vector<std::complex<float>> init_mod_mseq()
{
    std::vector<std::complex<float>> symbols;

    // Make sure we have an even number of bits
    size_t n = m_seq.size();

    for (int i = static_cast<int>(n) - 2; i >= 0; i -= 2)
    {
        bool bit0 = m_seq[i];     // MSB
        bool bit1 = m_seq[i + 1]; // LSB

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
	bool bit1 = m_seq[0]; //MSB

	// Gray-coded QPSK mapping
        float real = (0 == 0) ? 1.0f : -1.0f;
        float imag = (bit1 == 0) ? 1.0f : -1.0f;

        // Normalize to unit power
        std::complex<float> sym(real / std::sqrt(2.0f), imag / std::sqrt(2.0f));
        symbols.emplace_back(sym);
    }

    return symbols;
}
#endif
