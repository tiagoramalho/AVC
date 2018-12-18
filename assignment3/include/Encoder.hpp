#ifndef ENCODER_H
#define ENCODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

class Encoder {

    private:
        ifstream infile;
        WRITEBits w;
        int profile, periodicity, block_size, search_area;

        int get_best_k( int size, int tck );

        void parse_header(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);

        void get_best_fit( cv::Mat macroblock, cv::Mat searchingArea, vector<cv::Point> & to_encode);

        int get_residuals_from_matrix(cv::Mat * matrix, vector<int> * residuals);
        int get_residual_uniform( uint8_t pixel_value, uint8_t real_pixel_value);
        int get_residual_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C,uint8_t real_pixel_value);

        void intra_encode_write_4(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous);
        void intra_encode_write_2(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous);
        void intra_encode_write_0(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous);

    public:

        Encoder(const string & in_file, const string & out_file, int profile, int periodicity, int block_size, int search_area);

        void encode_and_write_frame_intra( Frame * frame , int f_counter , Golomb * g);

        void encode_and_write_frame_inter( Frame * frame , Frame * previous_frame, int f_counter , Golomb * g);

        /* Function used to encode and write */
        void encode_and_write();
};

#endif
