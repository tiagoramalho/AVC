#include "Decoder.hpp"

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

void Decoder::read_and_decode_and_write_n(Frame * frame, uint8_t seed,int k, Golomb & g, uint8_t type){
    cv::Mat mat;
    int residual;
    g.set_m(k);

    if(type == 0){
        mat = frame->get_y();
    } else if (type == 1) {
        mat = frame->get_u();
    } else{
        mat = frame->get_v();
    }

    mat.at<uint8_t>(0,0) = seed;
    printf("%02x\n", seed);
    printf("%d\n", k);

    /* Iteratiting over firstline's cols */

    for (int x = 1; x < mat.cols; x++)
    {   
        residual = g.read_and_decode(this->r);
        mat.at<uint8_t>(0,x) = get_real_value_uniform(mat.at<uint8_t>(0,x-1), residual);
    }

    //uint8_t * line = mat.ptr(0);
    //this->outfile.write( (char*) line, mat.cols);

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
            //printf("%d", residual);
            mat.at<uint8_t>(y,x) = get_real_value_LOCO(
                mat.at<uint8_t>(y,x-1),
                mat.at<uint8_t>(y-1,x),
                mat.at<uint8_t>(y-1,x-1),
                residual);
        }

        //uint8_t * line = mat.ptr(y);
        //this->outfile.write( (char*) line, mat.cols);

    }
    uint8_t * line = mat.ptr(0);
    this->outfile.write( (char*) line, mat.cols * mat.rows);

}



void Decoder::read_and_decode(){
    //printf("Entrada\n");

    int frame_counter =0;
    int color_space;

    Golomb g;

    string line;

    map<char, string> header;

    line = this->r.readHeader();
    this->r.parse_header_pv(header, line);
    // cout << "line" << line << endl;

    /* TODO meter width e height */
    this->height = stoi(header['H']);
    this->width = stoi(header['W']);
    color_space = stoi(header['C']);

    Frame * f;
    //printf("color_space: %d\n", color_space);


    switch(color_space){
        case 444:{
            f = new Frame444 (this->height, this->width);
            break;
        }
        case 422:{
            f = new Frame422 (this->height, this->width);
            break;
        }
        case 420:{
            f = new Frame420 (this->height, this->width);
            break;
        }
        default:
            exit(1);
    }

    //printf("Entrada3\n");

    /* Write File Header */
    /* TODO corrigir o 50 hardcoded abaixo. Mudar para FPS */
    this->write_header_y4m(stoi(header['W']), stoi(header['H']), "50:1", stoi(header['C']));

    vector<int> h (2,0);
    while(1){

        /* Write Frame Header */
        this->write_header_frame();

        /*
         * Decode Matrix Y
         */
        h = this->r.readHeaderNoLine();
        //cout << "line" << line << endl;
        if(line.size() == 0){
            printf("break\n");
            break;
        }
        // printf("Entrad5\n");

        /* Decode Matrix Y */
        this->r.parse_header_pv(header, line);
        read_and_decode_and_write_n(f, h.at(0), h.at(1), g, 0);

        /* Decode Matrix U */
        h = this->r.readHeaderNoLine();
        this->r.parse_header_pv(header, line);

        read_and_decode_and_write_n(f, h.at(0), h.at(1), g, 1);

        /* Decode Matrix V */
        h = this->r.readHeaderNoLine();

        this->r.parse_header_pv(header, line);
        
        read_and_decode_and_write_n(f, h.at(0), h.at(1), g, 2);

        exit(1);
        // break;
        printf("Done %d\n", frame_counter);
        frame_counter++;
    }

};


/*
TODO
Estás a fazer merda no width e height tamanho das matrizes
Quando for 422/420
*/
