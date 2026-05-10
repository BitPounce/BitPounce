#pragma once

#include <iostream>
#include <cstdint>

namespace BitPounce
{

	class MiniMT {
	private:
		static const int N = 624;
		static const int M = 397;
	
		static const uint32_t MATRIX_A = 0x9908b0df;
		static const uint32_t UPPER_MASK = 0x80000000;
		static const uint32_t LOWER_MASK = 0x7fffffff;
	
		uint32_t mt[N];
		int index;
	
	public:
		MiniMT(uint32_t seed) {
			index = N;
			mt[0] = seed;
		
			// Initialize state array
			for (int i = 1; i < N; i++) {
				mt[i] = 1812433253UL * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i;
			}
		}
	
		void twist() {
			int i;
            uint32_t x;

            for (i = 0; i < N - M; ++i) {
                x = (mt[i] & UPPER_MASK) | (mt[i + 1] & LOWER_MASK);
                mt[i] = mt[i + M] ^ (x >> 1) ^ ((x & 1) ? MATRIX_A : 0);
            }

            for (; i < N - 1; ++i) {
                x = (mt[i] & UPPER_MASK) | (mt[i + 1] & LOWER_MASK);
                mt[i] = mt[i + M - N] ^ (x >> 1) ^ ((x & 1) ? MATRIX_A : 0);
            }

            // Last element (wrap around to mt[0])
            x = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
            mt[N - 1] = mt[M - 1] ^ (x >> 1) ^ ((x & 1) ? MATRIX_A : 0);

            index = 0;
		}
	
		uint32_t extract() {
			if (index >= N) {
				twist();
			}
		
			uint32_t y = mt[index++];
		
			// Tempering (https://youtu.be/IRsPheErBj8)
			y ^= (y >> 11);
			y ^= (y << 7) & 0x9d2c5680;
			y ^= (y << 15) & 0xefc60000;
			y ^= (y >> 18);
		
			return y;
		}
	
		int range(int min, int max) {
			return min + (extract() % (max - min + 1));
		}
	};

	using Random = MiniMT;
}