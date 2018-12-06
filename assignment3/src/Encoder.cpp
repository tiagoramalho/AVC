#include "Encoder.hpp"

#include <cstdio>

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

void Encoder::encode_and_write_frame(Frame & frame){
    printf("Going To Encode Frame");


};

void Encoder::encode_and_write(){
    printf("Going To Encode");

    string line;

    getline(this->infile, line);

    map<char, string> header;

    parse_header(header, line);

    this->w.writeHeader(12,12,12);
    printf("Pum\n");
};


