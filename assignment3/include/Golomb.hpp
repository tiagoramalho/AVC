#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>
#include <string.h>
#include "fstreamBits.h"

using namespace std;

class Golomb {

    private:
        uint32_t m, b, t;
        Stream & stream;
        tuple<uint32_t,uint32_t> truncatedBinary(uint32_t r);

    public:
        Golomb(WRITEBits & w);

        Golomb(READBits & r);

        void set_m( uint32_t m);

        void encode_and_write( int number);

        int decode();
        void close();
};

#endif
