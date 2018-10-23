#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>

class Golomb {

    private:
        int m;

    public:
        Golomb( int m );
        std::tuple<int,uint32_t> encode( short number);
        short decode( uint32_t number);
        std::tuple<uint32_t,uint32_t> truncatedBinary(uint32_t r);
};

#endif
