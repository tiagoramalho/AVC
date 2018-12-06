#include "Encoder.hpp"


#include <cstdio>
#include "Frame.hpp"

Encoder::Encoder(const string & in_file, const string & out_file):
    infile(in_file.c_str()),w(out_file.c_str()){}

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

    /* encode the Luminance Matrix */

        /* Use the (0,0) as seed and predict base on a linear predictor the first line */
        /* Predict (0,1) based on (0,0) */

        /* From here is possible to use "full" jpeg predictor */

        /* Encode the residuals with Golomb */

    /* encode the Crominance U Mantrix */
        /* same */

    /* encode the Crominance V Mantrix */
        /* same */

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
            Frame444 f44 (rows, cols);
            f = &f44;
            imgData = new unsigned char[cols * rows * 3];
            break;
        }
        default:
            exit(1);
    }


    f->print_type();

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


