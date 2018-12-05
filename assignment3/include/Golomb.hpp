#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>
#include <string.h>
#include "fstreamBits.h"

using namespace std;

class Golomb {

    private:
        uint32_t m, b, t;
        Stream stream;
        tuple<uint32_t,uint32_t> truncatedBinary(uint32_t r);

    public:
        // Mode 0 - Read
        // Mode 1 - Write
        Golomb(const string & file, int mode);

        void set_m( uint32_t m);

        void encode_and_write( int number);

        int  read_and_decode();
        void close();
};

#endif
