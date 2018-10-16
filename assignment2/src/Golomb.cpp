#include "Golomb.hpp"
#include <math.h>
#include <tuple>

Golomb::Golomb( int m ): m(m) {}

std::tuple<int,uint32_t> Golomb::encode(short number){
    uint32_t ret = 0;
    uint32_t new_number = 0;

    uint32_t shift = log2(this->m);

    if(number >= 0)
        new_number = number * 2;
    else
        new_number = -2*number-1;

    uint32_t q = (uint32_t) ceil( (float) new_number/(float) this->m );
    uint32_t r = new_number - q*this->m;


    for( uint32_t i = 0; i < q; i++){
      ret = ret |  1U;
      ret = ret << 1;
    }

    ret = ret << shift;
    ret = ret | r;

    int number_of_bits = q + 1 + shift;

    return std::make_tuple(number_of_bits, ret);
}

short Golomb::decode(uint32_t number){

    return 0;
}
