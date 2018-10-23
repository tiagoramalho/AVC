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

    uint32_t q = (uint32_t) floor( (float) new_number/(float) this->m );

    for( uint32_t i = 0; i < q; i++){
      ret = ret |  1U;
      ret = ret << 1;
    }

    uint32_t r = new_number - q*this->m;

    std::tie(r,shift) = truncatedBinary(r);

    ret = ret << shift;
    ret = ret | r;

    int number_of_bits = q + 1 + shift;

    return std::make_tuple(number_of_bits, ret);
}

std::tuple<uint32_t,uint32_t> Golomb::truncatedBinary(uint32_t r){

    uint32_t b = ceil(log2(this->m));
    uint32_t bin;
    uint32_t shift;
    uint32_t tpb = std::pow(2,b);

    if( r < tpb - this->m){
        shift = b-1;
        bin = r;
    }else{
        shift = b;
        bin = r+ tpb - this->m;
    }

    return std::make_tuple(bin, shift);
}

short Golomb::decode(uint32_t number , int bits_to_read){

    return 0;
}
