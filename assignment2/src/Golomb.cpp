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

// READBits Golomb::open_to_read(){
//   return READBits(path);
// }
//
// WRITEBits Golomb::open_to_write(){
//   return WRITEBits(path);
// }

void Golomb::set_m(uint32_t m){
    this->m = m;
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

void Golomb::encode_and_write(short number, WRITEBits & w){
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

    // TODO
    //if( (this->m & -(this->m)) == this->m)
    //  std::tie(r,shift) = truncatedBinary(r);

    ret = ret << shift;
    ret = ret | r;

    uint32_t number_of_bits = q + 1 + shift;

    w.preWrite(ret, number_of_bits);

    //return std::make_tuple(number_of_bits, ret);
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

short Golomb::decode(READBits & r){
    /*
     * Ver:
     * https://w3.ual.es/~vruiz/Docencia/Apuntes/Coding/Text/03-symbol_encoding/09-Golomb_coding/index.html
     */
    //printf("Entrou no Golomb\n");

    uint32_t q = 0;
    uint32_t result  = 0;
    uint32_t bit = r.readBits();
    while(bit ==  1){
        bit = r.readBits();
        q++;
    }

    //printf("MEIO 1\n");

    /*
     * Calculate r
     */

    uint32_t resto = 0;
    for (uint32_t i = 0; i < b; ++i)
    {
        resto = resto << 1 | r.readBits();
    }

    /*
    uint32_t resto = 0;
    for(uint32_t i = 0; i<b -1; i++){
        resto = resto << 1 | r.readBits();
        printf("Preso for %d\n", i);

    }
    printf("MEIO 2\n");

    if(resto < t){
        result = q * m + resto;
        printf("MEIO 3\n");
    }
    else {
        resto = resto << 1 | r.readBits();
        result = q * m + resto - t;
    }
    */
    result = q * m + resto;
    

    //printf("Saiu no Golomb\n");
    if(result % 2 == 0)
        return (short) result / 2;
    else
        return (short) (result + 1) / (-2);
}
