#include "Golomb.hpp"
#include <math.h>
#include <tuple>
#include <bitset>
#include <string.h>

using namespace std;

Golomb::Golomb( uint32_t m , string path): m(m), path(path) {
    b = ceil(log2(m));
    t = std::pow(2,b) - m;
}

READBits Golomb::open_to_read(){
  return READBits(path);
}

WRITEBits Golomb::open_to_write(){
  return WRITEBits(path);
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

    std::tie(r,shift) = truncatedBinary(r);

    ret = ret << shift;
    ret = ret | r;

    uint32_t number_of_bits = q + 1 + shift;


    w.preWrite(ret, number_of_bits);

    //return std::make_tuple(number_of_bits, ret);
}

void Golomb::close(WRITEBits & w){
    w.flush();
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

short Golomb::decode(){
    /*
     * Ver:
     * https://w3.ual.es/~vruiz/Docencia/Apuntes/Coding/Text/03-symbol_encoding/09-Golomb_coding/index.html
     */
    while(1){

    }

    //uint32_t q = 0;
    //uint32_t bits_to_read_cp = bits_to_read - 1;


    ///*
    // * Calqulate q
    // */
    //while((number >> bits_to_read_cp & 1) == 1){
    //    q = q + 1;
    //    bits_to_read_cp = bits_to_read_cp - 1;
    //}

    ///*
    // * Calculate r
    // */



    //uint32_t mask = ((1 << bits_to_read_cp) - 1);
    //uint32_t r = (number & mask) >> 1;
    //// Temporary!!!!!! EXPLAIN!!!
    //// Having to do with "x ← the next k − 1 bits in the input"
    //if(bits_to_read_cp < b)
    //    r = (number & mask);

    //uint32_t result;
    //if(r < t){
    //    result = q * m + r;
    //}
    //else {
    //    r = r * 2 + (number & 1);
    //    result = q * m + r - t;
    //}

    ///*
    // * To short
    // */
    //if(result % 2 == 0)
    //    return (short) result / 2;
    //else
    //    return (short) (result + 1) / (-2);
    return (short) 1;
}
