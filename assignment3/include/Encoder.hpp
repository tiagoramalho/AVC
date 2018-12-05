#ifndef ENCODER_H
#define ENCODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

class Encoder {

    private:
        ifstream infile;
        Golomb g;

        void parse_header(  map<char,string> & header,  \
                            string header_line,         \
                            int delimiter(int) = ::isspace);

    public:

        Encoder(const string & in_file, const string & out_file);

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void encode_and_write_frame( Frame & frame );

        /* Function used to encode and write */
        void encode_and_write();
};

#endif
