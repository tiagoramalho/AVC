#ifndef GOLOMB_H
#define GOLOMB_H

#include <iostream>

class Golomb {

    private:
        int m;

    public:
        Golomb( int m );
        uint32_t encode( int number);
        int decode( uint32_t number);

};

#endif
