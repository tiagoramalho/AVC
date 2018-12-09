#include "Decoder.hpp"

Decoder::Decoder(const string & in_file, const string & out_file):
    outfile(out_file.c_str()),r(in_file.c_str()){}

void Decoder::parse_header_pv(  map<char,string> & header,
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
        else if(token.at(0) == 'K'){
            cout << "K " << token.substr(1) <<endl;
            header['K'] = token.substr(1);
        }
        else if(token.at(0) == 'S'){
            cout << "Seed " << token.substr(1) <<endl;
            header['S'] = token.substr(1);
        }
        else if(token.at(0) == 'C'){
            cout << "Colour Space " << token.substr(1) << endl;
            header['C'] = token.substr(1);
        }
        i=j;
    }
    if (header.find('C') == header.end()){
        cout << "Colour Space " << token.substr(1) << endl;
        header['C'] = "420";
    }
}

void Decoder::read_and_decode(){
    int cols, rows, frame_counter =0;

    Golomb g;

    string line;

    map<char, string> header;

    parse_header_pv(header, line);


};
