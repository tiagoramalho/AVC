#include "../include/Decoder.hpp"

using namespace cv;


Decoder::Decoder(const string & in_file, const string & out_file):
    outfile(out_file.c_str()),r(in_file.c_str()){}

uint8_t Decoder::get_real_value_uniform(uint8_t prevision, int residual){
    uint8_t pixel_value = prevision + residual;
    return pixel_value;
}


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

void Decoder::decode_inter(Frame * current_frame, Frame * last_frame, 
        int k, Golomb & g, uint8_t type, vector<Point> & vectors){

    cv::Mat mat;
    int residual;
    int m = pow(2,k);
    g.set_m(m);

    if(type == 0){
        mat = current_frame->get_y();
    } else if (type == 1) {
        mat = current_frame->get_u();
    } else{
        mat = current_frame->get_v();
    }


    uint8_t * line = mat.ptr(0);
    this->outfile.write( (char*) line, mat.cols * mat.rows);

}

vector<Point> Decoder::get_vectors(int k, Golomb & g){
    vector<Point> vectors = {};
    int total = this->height * this->width / (this->block_size*this->block_size);
    printf("Total: %d\n", total);
    for (int i = 0; i < total; ++i)
    {
        int x = g.read_and_decode(this->r);
        int y = g.read_and_decode(this->r);
        vectors.push_back(Point(x,y));
        printf("Vectors(%d, %d)\n", x, y);
    }
    return vectors;
}


void Decoder::read_and_decode(){

    int frame_counter =0;
    int color_space;

    Golomb g;

    string line;

    map<char, string> header;

    line = this->r.readHeader();
    this->r.parse_header_pv(header, line);

    this->height = stoi(header['H']);

    this->width = stoi(header['W']);

    this->block_size = stoi(header['B']);

    Frame * current_frame;
    Frame * last_frame;


    switch(color_space){
        case 444:{
            current_frame = new Frame444 (this->height, this->width);
            last_frame = new Frame444 (this->height, this->width);
            break;
        }
        case 422:{
            current_frame = new Frame422 (this->height, this->width);
            last_frame = new Frame444 (this->height, this->width);
            break;
        }
        case 420:{
            current_frame = new Frame420 (this->height, this->width);
            last_frame = new Frame444 (this->height, this->width);
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
    int seed;
    while(1){

        type = this->r.read_type();
        if(this->r.gcount() == 0){
            break;
        }

        if(type == 0){
            printf("decode intra");

            /* Write Frame Header */
            this->write_header_frame();

            /* Decode Matrix Y */
            seed = this->r.read_seed();
            k = this->r.read_k();
            decode_intra(current_frame, seed, k, g, 0);

            /* Decode Matrix U */
            seed = this->r.read_seed();
            k = this->r.read_k();
            decode_intra(current_frame, seed, k, g, 1);

            /* Decode Matrix V */
            seed = this->r.read_seed();
            k = this->r.read_k();
            decode_intra(current_frame, seed, k, g, 2);

        }else if(type == 1){
            printf("decode inter");
        
            /* Write Frame Header */
            this->write_header_frame();

            /* Decode Matrix Y */
            k = this->r.read_k();

            vector<Point> vectors = get_vectors(k, g);


            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 0, vectors);

            /* Decode Matrix Y */
            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 0, vectors);

            k = this->r.read_k();
            decode_inter(current_frame, last_frame, k, g, 0, vectors);
        }


        last_frame = current_frame;
        printf("Done %d\n", frame_counter);
        frame_counter++;
    }

};
