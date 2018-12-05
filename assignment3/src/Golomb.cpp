#include "Golomb.hpp"
#include <math.h>
#include <tuple>
#include <bitset>
#include <string.h>

using namespace std;

Golomb::Golomb(WRITEBits & w): m(1), stream(w){
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

Golomb::Golomb(READBits & r): m(1), stream(r){
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

void Golomb::set_m(uint32_t m){
    this->m = m;
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

void Golomb::encode_and_write(int number){
    WRITEBits * wb = static_cast<WRITEBits*>(& this->stream);

    uint32_t new_number = 0;
    uint32_t shift = log2(this->m);

    if(number >= 0)
        new_number = number * 2;
    else
        new_number = -2*number-1;

    uint32_t q = (uint32_t) floor( (float) new_number/(float) this->m );

    for( uint32_t i = 0; i < q; i++){
        wb->writeBits(1);
    }
    wb->writeBits(0);

    uint32_t r = new_number - q*this->m;

    wb->preWrite(r, shift);
}

std::tuple<uint32_t,uint32_t> Golomb::truncatedBinary(uint32_t r){

    uint32_t bin;
    uint32_t shift;

    if( r < t){
        shift = b-1;
        bin = r;
    }else{
        shift = b;
        bin = r + t;
    }

    return std::make_tuple(bin, shift);
}

int Golomb::decode(){

    READBits* rb = static_cast<READBits*>( & this->stream);

    uint32_t q = 0;
    int result  = 0;
    uint32_t bit = rb->readBits();
    while(bit ==  1){
        bit = rb->readBits();
        q++;
    }

    /*
     * Calculate r
     */

    uint32_t resto = 0;
    for (uint32_t i = 0; i < b; ++i)
    {
        resto = resto << 1 | rb->readBits();
    }

    result = q * m + resto;

    if(result % 2 == 0)
    {
        return  result / 2;
    }
    else{
        return  (result + 1) / (-2);
    }
}
