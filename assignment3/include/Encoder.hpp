#ifndef ENCODER_H
#define ENCODER_H

#include "Golomb.hpp"

class Encoder {

    private:
        ifstream infile;
        Golomb g;

    public:

        Encoder(const string & in_file, const string & out_file);

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void encode_and_write_n( int n);

        /* Function used to encode and write */
        void encode_and_write();
};

#endif
