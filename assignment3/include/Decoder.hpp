#ifndef DECODER_H
#define DECODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

class Decoder {

    private:
        ofstream outfile;
        READBits r;

        void parse_header_pv(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);


    public:

        Decoder(const string & in_file, const string & out_file);

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void read_and_decode_n( int n);

        /* Function used to encode and write */
        void read_and_decode();
};

#endif
