#include "Encoder.hpp"


#include <cstdio>
#include "Frame.hpp"

Encoder::Encoder(const string & in_file, const string & out_file):
    infile(in_file.c_str()),w(out_file.c_str()){}

int Encoder::get_residual_uniform( uint8_t previous_pixel_value, uint8_t real_pixel_value ){

    int residual = real_pixel_value - previous_pixel_value;
    return residual;

}

int Encoder::get_residual_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, uint8_t real_pixel_value){

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

    return get_residual_uniform( pixel_prevision, real_pixel_value);
}


void Encoder::parse_header(  map<char,string> & header,
                    string header_line,
                    int delimiter(int)){

    string token;

    auto e=header_line.end();
    auto i=header_line.begin();

    while(i!=e){
        i=find_if_not(i,e, delimiter);
        if(i==e) break;
        auto j=find_if(i,e, delimiter);
        token = string(i,j);
        cout << token << endl;
        if(token.at(0) == 'W'){
            cout << "Width " << token.substr(1) << endl;
            header['W'] = token.substr(1);
        }
        else if(token.at(0) == 'H'){
            cout << "Height " << token.substr(1) <<endl;
            header['H'] = token.substr(1);
        }
        else if(token.at(0) == 'F'){
            cout << "Frame Rate " << token.substr(1) << endl;
            header['F'] = token.substr(1);
        }
        else if(token.at(0) == 'I'){
            cout << "Interlacing not parsed" << endl;
        }
        else if(token.at(0) == 'A'){
            cout << "Aspect Ratio not parsed" << endl;
        }
        else if(token.at(0) == 'C'){
            cout << "Colour Space " << token.substr(1) << endl;
            header['C'] = token.substr(1);
        }
        i=j;
    }
}

void Encoder::encode_and_write_frame(Frame * frame){

    printf("Pim");
    frame->print_type();
    printf("Pam");

    int mini_block_size = 8;
    int mini_x, mini_y;
    vector<int> residuals;

    /* encode the Luminance Matrix */
    uint8_t seed = frame->get_y().at<uint8_t>(0,0);
    uint8_t last_real = seed;
    uint8_t prediction;
    printf("Going to encode 1 mini_block");
    for( mini_y = 0; mini_y < mini_block_size; mini_y++){
        if( mini_y == 0 ){
            // First row
            for( mini_x = 1; mini_x < mini_block_size; mini_x++){
                residuals.push_back(get_residual_uniform(last_real, frame->get_y().at<uint8_t>(mini_x,mini_y)));
                last_real = frame->get_y().at<uint8_t>(mini_x,mini_y);
            }
        }else{
            // Other rows
            residuals.push_back(get_residual_uniform(seed, frame->get_y().at<uint8_t>(0,1)));
            for( mini_x = 1; mini_x < mini_block_size; mini_x++){
                prediction = get_residual_LOCO(
                        frame->get_y().at<uint8_t>(mini_x-1,mini_y),
                        frame->get_y().at<uint8_t>(mini_x,mini_y-1),
                        frame->get_y().at<uint8_t>(mini_x-1,mini_y-1),
                        frame->get_y().at<uint8_t>(mini_x,mini_y));

                residuals.push_back(prediction);
            }
        }
    }

    printf("Done");
};

void Encoder::encode_and_write(){
    string line;
    int cols, rows, frame_counter =0;
    unsigned char *imgData;
    Frame * f;

    getline(this->infile, line);

    map<char, string> header;

    parse_header(header, line);

    cols = stoi(header['W']);
    rows = stoi(header['H']);

    printf("Writing Header To Compressed File...");
    this->w.writeHeader(cols,rows,stoi(header['C']));
    printf("done %d\n",stoi(header['C']));

    switch(stoi(header['C'])){
        case 444:{
            printf("ALBERTINA %d\n",stoi(header['C']));
            Frame444 f44 (rows, cols);
            printf("ALBERTINA %d\n",stoi(header['C']));
            f = &f44;
            printf("ALBERTINA %d\n",stoi(header['C']));
            imgData = new unsigned char[cols * rows * 3];
            break;
        }
        default:
            exit(1);
    }


    printf("ALBERTINA %d\n",stoi(header['C']));
    f->print_type();
    printf("ALBERTINA %d\n",stoi(header['C']));

    while(1){
        getline (this->infile,line); // Skipping word FRAME
        this->infile.read((char *)imgData, cols * rows * 3);
        f->set_frame_data(imgData);
        if(this->infile.gcount() == 0){
          break;
        }
        encode_and_write_frame(f);
        //printf("Frames -> %d\n", frame_counter);
        frame_counter += 1;
    }
};


