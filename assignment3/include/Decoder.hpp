#ifndef DECODER_H
#define DECODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

using namespace cv;

class Decoder {

    private:
        int height, width, color_space;
        ofstream outfile;
        READBits r;
        int profile, periodicity, block_size, search_area;


        // void parse_header_pv(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);
        
        uint8_t get_real_value_uniform(uint8_t prevision, int residual);
        uint8_t get_real_value_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, int residual);


        void write_header_y4m(uint32_t width, uint32_t height, string fps, uint32_t colorspace){
            if(colorspace == 420){
                outfile << "YUV4MPEG2" << " W" << width << " H" << height << " F" << fps << " Ip A1:1"<< endl;
            }else{
                outfile << "YUV4MPEG2" << " W" << width << " H" << height << " F" << fps << " Ip A1:1 C"<< colorspace << endl;
            }
        }


        vector<Point> get_vectors(int k, Golomb & g);


        void write_header_frame(){
            outfile << "FRAME" << endl;
        }

    public:

        Decoder(const string & in_file, const string & out_file);


        /*  Function used to encode and write N Frames intra-mode  */
        void decode_intra(Frame * frame, uint8_t seed,int k, Golomb & g, uint8_t type);

        /*  Function used to encode and write N Frames  inter-mode  */
        void decode_inter(Frame * current_frame , Frame * last_frame,
            int k, Golomb & g, uint8_t type, vector<Point> & vectors, int shamnt);

        /* Function used to encode and write */
        void read_and_decode();
};

#endif
