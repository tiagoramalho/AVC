#include "Golomb.hpp"
#include <math.h>
#include <tuple>
#include <bitset>
#include <string.h>

using namespace std;

Golomb::Golomb(): m(1){
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

void Golomb::set_m(uint32_t m){
    this->m = m;
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

void Golomb::encode_and_write(int number, WRITEBits & w){
    uint32_t new_number = 0;
    uint32_t shift = log2(this->m);
    if(number >= 0)
        new_number = number * 2;
    else
        new_number = -2*number-1;
    uint32_t q = (uint32_t) floor( (float) new_number/(float) this->m );
    for( uint32_t i = 0; i < q; i++){
        w.writeBits(1);
    }
    w.writeBits(0);
    uint32_t r = new_number - q*this->m;
    w.preWrite(r, shift);
}

int Golomb::read_and_decode(READBits & r){
    uint32_t q = 0;
    int result  = 0;
    uint32_t bit = r.readBits();
    while(bit ==  1){
        bit = r.readBits();
        q++;
    }
    /*
     * Calculate r
     */
    uint32_t resto = 0;
    for (uint32_t i = 0; i < b; ++i)
    {
        resto = resto << 1 | r.readBits();
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
