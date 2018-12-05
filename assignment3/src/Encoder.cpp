#include "Encoder.hpp"

#include <cstdio>
#include "Frame.hpp"

Encoder::Encoder(const string & in_file, const string & out_file):
    infile(in_file.c_str()), g(out_file.c_str(), 0){}

void Encoder::encode_and_write_n(int n){
    printf("Going To Encode %d frames", n);
};

void Encoder::encode_and_write(){
    printf("Going To Encode all frames");
};
