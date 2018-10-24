#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>

class Golomb {

    private:
        /*
         * t = 2^b − m
         */
        uint32_t m, b, t;

    public:
        Golomb( uint32_t m );
        std::tuple<uint32_t,uint32_t> encode( short number);
        std::tuple<uint32_t,uint32_t> truncatedBinary(uint32_t r);
        short decode(uint32_t number , uint32_t bits_to_read);
};

#endif
