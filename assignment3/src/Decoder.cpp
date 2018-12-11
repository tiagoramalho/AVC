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

void Decoder::read_and_decode_and_write_n(Frame * frame, uint8_t seed,int k, Golomb & g){
    cv::Mat mat;
    int residual;
    g.set_m(k);
    mat = frame->get_y();

    mat.at<uint8_t>(0,0) = seed;

    /* Iteratiting over firstline's cols */
    cout << "FRAME DECODE \n";
    printf("(%d, %d) -> %02x\n", 0, 0, seed);

    for (int x = 1; x < mat.cols; ++x)
    {   
        residual = g.read_and_decode(this->r);
        mat.at<uint8_t>(x,0) = get_real_value_uniform(mat.at<uint8_t>(x-1,0), residual);
        printf("(%d, %d) -> %02x; residual -> %02x\n", x, 0, mat.at<uint8_t>(x,0), residual);
    }

    uint8_t * line = mat.ptr(0);
    this->outfile.write( (char*) line, mat.cols);

    /* Iterate over the lines. Start at j=1 */
    for (int y = 1; y < mat.rows; ++y)
    {

        /* Specific for the first col */
        residual = g.read_and_decode(this->r);
        mat.at<uint8_t>(0,y) = get_real_value_uniform(mat.at<uint8_t>(0,y-1), residual);

        /* Iterate over the collumns. Start at j=1 */
        for (int x = 1; x < mat.cols; ++x)
        {
            residual = g.read_and_decode(this->r);
            mat.at<uint8_t>(x,y) = get_real_value_LOCO(
                mat.at<uint8_t>(x-1,y),
                mat.at<uint8_t>(x,y-1),
                mat.at<uint8_t>(x-1,y-1),
                residual);
        }
    }
}



void Decoder::read_and_decode(){
    printf("Entrada\n");

    int frame_counter =0, color_space;

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
    printf("color_space: %d\n", color_space);


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

    printf("Entrada3\n");

    /* Write File Header */
    /* TODO corrigir o 50 hardcoded abaixo. Mudar para FPS */
    this->write_header_y4m(stoi(header['W']), stoi(header['H']), "50:1", stoi(header['C']));

    while(1){

        /* Write Frame Header */
        this->write_header_frame();

        /*
         * Decode Matrix Y
         */
        line = this->r.readHeader();
        //cout << "line" << line << endl;
        if(line.size() == 0){
            printf("break\n");
            break;
        }
        printf("Entrad5\n");

        /* Decode Matrix Y */
        this->r.parse_header_pv(header, line);
        read_and_decode_and_write_n(f, stoi(header['S']), stoi(header['K']), g);



        /* Decode Matrix U */
        // this->r.parse_header_pv(header, line);
        // read_and_decode_and_write_n(f, stoi(header['S']), stoi(header['K']), g);

        /* Decode Matrix V */
        // this->r.parse_header_pv(header, line);
        // read_and_decode_and_write_n(f, stoi(header['S']), stoi(header['K']), g);

        break;

        frame_counter++;
    }

};


/*
TODO
Estás a fazer merda no width e height tamanho das matrizes
Quando for 422/420
*/