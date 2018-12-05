#ifndef ENCODER_H
#define ENCODER_H

#include "Golomb.hpp"

class Encoder {

    private:
        Golomb & g;

    public:

        Encoder(Golomb & golomb):g(golomb){};

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void encode_and_write_n( int n);

        /* Function used to encode and write */
        void encode_and_write();
};

#endif
