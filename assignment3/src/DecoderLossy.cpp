#include "../include/Decoder.hpp"

using namespace cv;

Decoder::Decoder(const string & in_file, const string & out_file):
    outfile(out_file.c_str()),r(in_file.c_str()){}

void Decoder::read_and_decode(){

    int frame_counter =0;

    Golomb g, g_zeros;

    string line;

    map<char, string> header;
}