#ifndef ENCODER_H
#define ENCODER_H

#include "Golomb.hpp"
#include "Frame.hpp"

using namespace cv;

class Encoder {

    private:
        /* Temporário */

        double dataLuminance[8][8] = {
            {16, 11, 10, 16, 24, 40, 51, 61},
            {12, 12, 14, 19, 26, 58, 60, 55},
            {14, 13, 16, 24, 40, 57, 69, 56},
            {14, 17, 22, 29, 51, 87, 80, 62},
            {18, 22, 37, 56, 68, 109, 103, 77},
            {24, 35, 55, 64, 81, 104, 113, 92},
            {49, 64, 78, 87, 103, 121, 120, 101},
            {72, 92, 95, 98, 112, 100, 103, 99}
        };

        Mat mat_luminance = Mat(8, 8, CV_64FC1, &dataLuminance);

        double dataChrominance[8][8] = {
            {17, 18, 24, 27, 99, 99, 99, 99},
            {18, 21, 26, 66, 99, 99, 99, 99},
            {24, 26, 56, 99, 99, 99, 99, 99},
            {47, 66, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99}
        };

        Mat mat_chrominance = Mat(8, 8, CV_64FC1, &dataChrominance);


        ifstream infile;
        WRITEBits w;

        int cols, rows;
        int profile, periodicity, block_size, search_area, color_space, shamnt_y, shamnt_u, shamnt_v;

        int get_best_k( int size, int tck );

        void parse_header(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);

        void get_best_fit( cv::Mat macroblock, cv::Mat searchingArea, vector<cv::Point> & to_encode);

        int get_residuals_from_matrix(cv::Mat * matrix, vector<int> * residuals);
        int get_residual_uniform( uint8_t pixel_value, uint8_t real_pixel_value);
        int get_residual_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C,uint8_t real_pixel_value);

        void inter_encode_write_4(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous, int shamnt, int shamnt_sum);

        void inter_encode_write_2(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous, int shamnt, int shamnt_sum);

        void inter_encode_write_0(cv::Mat y_frame, Golomb * g, vector<cv::Point> to_encode_vector, cv::Mat y_previous, int shamnt, int shamnt_sum);

        void write_residuals(vector<int> to_encode_residuals, Golomb * g, int shamnt, int shamnt_sum );
    public:

        Encoder(const string & in_file, const string & out_file, int profile, int periodicity, int block_size, int search_area, int shamnt_y, int shamnt_u, int shamnt_v);
        Encoder(const string & in_file, const string & out_file, int profile, int periodicity, int block_size, int search_area);


        void encode_and_write_frame_intra( Frame * frame , int f_counter , Golomb * g);
        void encode_and_write_frame_intra_lossy( Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix );
        void write_frame_component_lossy(Golomb & g, Golomb & g_zeros, vector<tuple<int, uint8_t>> & write_vector);


        void encode_and_write_frame_inter( Frame * frame , Frame * previous_frame, int f_counter , Golomb * g, int shamnt_y, int shamnt_u, int shamnt_v);

        /* Function used to encode and write */
        void encode_and_write();
        void encode_and_write_lossy();

};

#endif
