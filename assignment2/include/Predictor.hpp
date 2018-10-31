#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <iostream>
#include <string.h>
#include <vector>
#include "fstreamBits.h"

using namespace std;

class Predictor {

    private:
		uint32_t max_order, sample_size;
		vector<vector<short>> v;
    	/*
        samples vector<short> samplesNew(FRAMES_BUFFER_SIZE * sndFileNew.channels());
        vector<short> order_one();
        vector<short> order_two();
        vector<short> order_three();
        vector<short> order_four();
        */

    public:

        Predictor(uint32_t max_order, uint32_t sample_size);
        short gen_residuals(vector<short> & samples, uint32_t index, uint32_t order);
        void populate_v(vector<short> & samples);

    	/*
        short r_order_zero(short xn);
        short r_order_one(short xn, short xn_minus_one);
        short r_order_two(short xn, short xn_minus_one);
        short r_order_three(short xn, short xn_minus_one);
        short r_order_four(short xn, short xn_minus_one);
        */
};

#endif
