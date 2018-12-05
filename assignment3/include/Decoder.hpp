#ifndef DECODER_H
#define DECODER_H

#include "Golomb.hpp"

class Decoder {

    private:
        Golomb & g;

    public:

        Decoder(Golomb & golomb):g(golomb){};

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void read_and_decode_n( int n);

        /* Function used to encode and write */
        void read_and_decode();
};

#endif
