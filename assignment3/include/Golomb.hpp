#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>
#include <string.h>
#include "fstreamBits.h"

using namespace std;

class Golomb {

    private:
        uint32_t m, b, t;

    public:
        Golomb();

        void set_m( uint32_t m);

        void encode_and_write( int number, WRITEBits & w);

        int read_and_decode(READBits & r);
};

#endif
