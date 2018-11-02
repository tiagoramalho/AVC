#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>
#include <string.h>
#include "fstreamBits.h"

using namespace std;

class Golomb {

    private:
        uint32_t m, b, t;
        tuple<uint32_t,uint32_t> truncatedBinary(uint32_t r);

    public:
        Golomb();
        //READBits open_to_read();
        //WRITEBits open_to_write();

        void set_m( uint32_t m);

        void encode_and_write( short number, WRITEBits & w);

        short decode(READBits & r);
        void close(WRITEBits & w);
};

#endif
