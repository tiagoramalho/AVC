#ifndef DECODER_H
#define DECODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

class Decoder {

    private:
        int height, width;
        ofstream outfile;
        READBits r;

        // void parse_header_pv(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);
        
        uint8_t get_real_value_uniform(uint8_t prevision, int residual);
        uint8_t get_real_value_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, int residual);


        void write_header_y4m(uint32_t width, uint32_t height, string fps, uint32_t colorspace){
            outfile << "YUV4MPEG2" << " W" << width << " H" << height << " F" << fps << " Ip A1:1 C"<< colorspace << endl;
        }

        void write_header_frame(){
            outfile << "FRAME" << endl;
        }

    public:

        Decoder(const string & in_file, const string & out_file);

        /*  Function used to encode and write N Frames ( used for easy debug ) */
        void read_and_decode_and_write_n(Frame * frame, uint8_t seed,int k, Golomb & g);

        /* Function used to encode and write */
        void read_and_decode();
};

#endif
