#include "../include/Decoder.hpp"

using namespace cv;


Decoder::Decoder(const string & in_file, const string & out_file):
    outfile(out_file.c_str()),r(in_file.c_str()){}

uint8_t Decoder::get_real_value_uniform(uint8_t prevision, int residual){
    uint8_t pixel_value = prevision + residual;
    return pixel_value; } 

uint8_t Decoder::get_real_value_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, int residual){

    uint8_t pixel_prevision, maxAB, minAB;

    maxAB = std::max(pixel_A, pixel_B);
    minAB = std::min(pixel_A, pixel_B);

    if( pixel_C >= maxAB){
        pixel_prevision = minAB;
    }else if ( pixel_C <= minAB ){
        pixel_prevision = maxAB;
    }else{
        pixel_prevision = pixel_A + pixel_B - pixel_C;
    }

    return get_real_value_uniform( pixel_prevision, residual);
}

void Decoder::decode_intra(Frame * frame, uint8_t seed,int k, Golomb & g, uint8_t type){
    cv::Mat mat;
    int residual;
    int m = pow(2,k);
    g.set_m(m);

    if(type == 0){
        mat = frame->get_y();
    } else if (type == 1) {
        mat = frame->get_u();
    } else{
        mat = frame->get_v();
    }

    mat.at<uint8_t>(0,0) = seed;

    /* Iteratiting over firstline's cols */

    for (int x = 1; x < mat.cols; x++)
    {   
        residual = g.read_and_decode(this->r);
        mat.at<uint8_t>(0,x) = get_real_value_uniform(mat.at<uint8_t>(0,x-1), residual);
        
    }

    /* Iterate over the lines. Start at j=1 */
    for (int y = 1; y < mat.rows; y++)
    {
        /* Specific for the first col */
        residual = g.read_and_decode(this->r);
        mat.at<uint8_t>(y,0) = get_real_value_uniform(mat.at<uint8_t>(y-1, 0), residual);

        /* Iterate over the collumns. Start at j=1 */
        for (int x = 1; x < mat.cols; x++)
        {
            residual = g.read_and_decode(this->r);
            mat.at<uint8_t>(y,x) = get_real_value_LOCO(
                mat.at<uint8_t>(y,x-1),
                mat.at<uint8_t>(y-1,x),
                mat.at<uint8_t>(y-1,x-1),
                residual);
        }

    }
    uint8_t * line = mat.ptr(0);
    this->outfile.write( (char*) line, mat.cols * mat.rows);

}

void Decoder::decode_inter(Frame * current_frame, Frame * last_frame, int k, Golomb & g, uint8_t type, vector<Point> & vectors, int shamnt){

    int adjusted_size_x = this->block_size;
    int adjusted_size_y = this->block_size;
    cv::Mat mat, previous_mat;
    int m = pow(2,k);
    g.set_m(m);
    int count_read = 0;


    if(type == 0){

        adjusted_size_x = this->block_size;
        adjusted_size_y = this->block_size;

        mat = current_frame->get_y();
        previous_mat = last_frame->get_y();

    } else if (type == 1) {

        if (color_space == 422) {
            adjusted_size_x = this->block_size / 2;
            adjusted_size_y = this->block_size;
        } else if (color_space == 420) {
            adjusted_size_x = this->block_size / 2;
            adjusted_size_y = this->block_size / 2;
        }

        mat = current_frame->get_u();
        previous_mat = last_frame->get_u();

    } else{

        if (color_space == 422) {
            adjusted_size_x = this->block_size / 2;
            adjusted_size_y = this->block_size;
        } else if (color_space == 420) {
            adjusted_size_x = this->block_size / 2;
            adjusted_size_y = this->block_size / 2;
        }
        mat = current_frame->get_v();
        previous_mat = last_frame->get_v();

    }
    int ratiox = this->block_size / adjusted_size_x;
    int ratioy = this->block_size / adjusted_size_y;


    cv::Mat macroblock;
    cv::Mat my_macroblock = Mat(adjusted_size_y, adjusted_size_x, CV_32S);
    cv::Mat match_area;
    cv::Mat my_match_area;
    int index = 0;
    for( int y_curr_frame = 0; y_curr_frame < mat.rows; y_curr_frame += adjusted_size_y ){

        for( int x_curr_frame = 0; x_curr_frame < mat.cols; x_curr_frame += adjusted_size_x ){

            for (int x = 0; x < adjusted_size_x; x++)
            {
                for (int y = 0; y < adjusted_size_y; y++)
                {
                    if(shamnt > 0){
                        my_macroblock.at<int32_t>(y,x) = ((g.read_and_decode(this->r)<< 1) | 1U ) << (shamnt-1);
                    }else{
                        my_macroblock.at<int32_t>(y,x) = g.read_and_decode(this->r);
                    }
                    count_read++;
                }
            }


            Point tmp = vectors.at(index);

            my_match_area = previous_mat(cv::Rect(x_curr_frame + tmp.x/ratiox, y_curr_frame + tmp.y/ratioy, adjusted_size_x, adjusted_size_y));
            my_match_area.convertTo(match_area, CV_32S);

            my_macroblock =  match_area - my_macroblock;


            my_macroblock.convertTo(macroblock, CV_8U);

            for (int x = x_curr_frame; x < x_curr_frame+adjusted_size_x; ++x)
            {
                for (int y = y_curr_frame; y < y_curr_frame+adjusted_size_y; ++y)
                {
                    mat.at<uint8_t>(y,x) = macroblock.at<uint8_t>(y-y_curr_frame,x-x_curr_frame); 
                }
            }

            index+=1;
        }
    }

    uint8_t * line = mat.ptr(0);
    this->outfile.write( (char*) line, mat.cols * mat.rows);


}

vector<Point> Decoder::get_vectors(int k, Golomb & g){
    vector<Point> vectors = {};
    int m = pow(2,k);
    g.set_m(m);
    int total = this->height * this->width / (this->block_size*this->block_size);
    for (int i = 0; i < total; ++i)
    {
        int x = g.read_and_decode(this->r);
        int y = g.read_and_decode(this->r);
        vectors.push_back(Point(x,y));
    }
    return vectors;
}


void Decoder::read_and_decode(){

    int frame_counter =0;

    Golomb g;

    string line;

    map<char, string> header;

    line = this->r.readHeader();
    this->r.parse_header_pv(header, line);

    this->height = stoi(header['H']);
    this->width = stoi(header['W']);
    this->block_size = stoi(header['B']);
    this->color_space = stoi(header['C']);


    Frame * current_frame;
    Frame * last_frame;


    switch(this->color_space){
        case 444:{
            current_frame = new Frame444 (this->height, this->width);
            last_frame = new Frame444 (this->height, this->width);
            break;
        }
        case 422:{
            current_frame = new Frame422 (this->height, this->width);
            last_frame = new Frame422 (this->height, this->width);
            break;
        }
        case 420:{
            current_frame = new Frame420 (this->height, this->width);
            last_frame = new Frame420 (this->height, this->width);
            break;
        }
        default:
            exit(1);
    }


    /* Write File Header */
    /* TODO corrigir o 50 hardcoded abaixo. Mudar para FPS */

    this->write_header_y4m(stoi(header['W']), stoi(header['H']), "50:1", stoi(header['C']));

    int type;
    int k;
    int shamnt;
    int seed;
    while(1){

        type = this->r.read_type();
        if(type == 7){
            break;
        }

        if(type == 0){
            printf("decode intra\n");

            /* Write Frame Header */
            this->write_header_frame();

            /* Decode Matrix Y */
            k = this->r.read_k();
            seed = this->r.read_seed();
            decode_intra(current_frame, seed, k, g, 0);

            /* Decode Matrix U */
            k = this->r.read_k();
            seed = this->r.read_seed();
            decode_intra(current_frame, seed, k, g, 1);

            /* Decode Matrix V */
            k = this->r.read_k();
            seed = this->r.read_seed();
            decode_intra(current_frame, seed, k, g, 2);

        }else if(type == 1){
            printf("decode inter\n");
        
            /* Write Frame Header */
            this->write_header_frame();

            k = this->r.read_k();
            vector<Point> vectors = get_vectors(k, g);


            /* Decode Matrix Y */
            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 0, vectors, 0);

            /* Decode Matrix U */
            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 1, vectors, 0);

            /* Decode Matrix V */
            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 2, vectors, 0);
        }else if(type == 2){
            printf("decode inter quant\n");
        
            /* Write Frame Header */
            this->write_header_frame();

            k = this->r.read_k();
            vector<Point> vectors = get_vectors(k, g);


            /* Decode Matrix Y */
            k = this->r.read_k();
            shamnt = this->r.read_shamnt();
            decode_inter(current_frame, last_frame, k, g, 0, vectors, shamnt);

            /* Decode Matrix U */
            k = this->r.read_k();
            shamnt = this->r.read_shamnt();
            decode_inter(current_frame, last_frame, k, g, 1, vectors, shamnt);

            /* Decode Matrix V */
            k = this->r.read_k();
            shamnt = this->r.read_shamnt();
            decode_inter(current_frame, last_frame, k, g, 2, vectors, shamnt);
        }



        std::swap(last_frame, current_frame);
        printf("Done %d\n", frame_counter);
        frame_counter++;
    }

};
