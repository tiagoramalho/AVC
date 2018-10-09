#include <iostream>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <sndfile.hh>
#include "wavhist.h"
#include <sstream>
#include <string.h>

#include <iterator>
#include <numeric>
#include <algorithm>

using namespace std;

double euclid_distance_squared(vector<short> &v1, vector<short> &v2){
    double dist = 0.0;
    double d;
    for( unsigned f = 0; f < v1.size(); f++){
        d = v1[f] - v2[f];
        dist += d*d;
    }
    return dist;
}
uint32_t getBestMatchingUnit(vector<vector<short>> &cb, vector<short> &row){
    int idx = 0;
    int dst {};
    int smallDst = euclid_distance_squared(cb.at(0), row);
    for(uint32_t i = 1; i< cb.size(); i++){
        dst = euclid_distance_squared(cb[i], row);
        if( dst < smallDst){
            smallDst = dst;
            idx =i;
        }
    }
    return idx;
}

int main(int argc, char *argv[]) {

    if(argc != 7) {
        cerr << "Usage: wavvq <input file> <output file> <codebook> <operation (e/d)> <block_size> <channels>" << endl;
        cerr << "\t \t Ex: wavvq samples/sample02.wav index.txt output/cb.txt e 3 2" << endl;
        cerr << "\t \t Ex: wavvq index.txt newfile.wav output/cb.txt d 3 2" << endl;
    }

    string in_file = argv[1], out_file = argv[2], codebook_file = argv[3];
    auto op { argv[4] };
    int block_size = stoi(argv[5]), channels = stoi(argv[6]);

    ifstream cbFile;
    cout << "open codebook" << endl;
    vector<vector<short>> codebook;
    cbFile.open(codebook_file);


    if (!cbFile) {
        cerr << "Unable to open codebook";
        exit(1);   // call system to stop
    }

    string tmp;
    while (getline(cbFile, tmp)) {
        istringstream iss(tmp);
        codebook.push_back(
                vector<short>(
                    istream_iterator<short>(iss),
                    istream_iterator<short>() ));
    }

    for ( uint32_t i = 0; i < codebook.size(); i++){

        for ( uint32_t j = 0; j < codebook.at(i).size(); j++){
            cout << codebook.at(i).at(j) << " ";
        }

        cout << endl;
    }

    if (strcmp(op,"e") == 0){


        SndfileHandle sndFile { in_file };
        if(sndFile.error()) {
            cerr << "Error: invalid input file" << endl;
            return 1;
        }

        if((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
            cerr << "Error: file is not in WAV format" << endl;
            return 1;
        }

        if((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
            cerr << "Error: file is not in PCM_16 format" << endl;
            return 1;
        }

        int frames = sndFile.frames();
        int channels = sndFile.channels();
        
        vector<short> samples( frames * channels);
        sndFile.readf(samples.data(), frames * channels);
        vector<short> block (block_size);

        ofstream outFileIndexes(out_file);

        int idx;

        for( int c = 0; c < channels; c++){
            for( uint32_t i = c; i < samples.size(); i += ( block_size )* channels){
                try{
                    for ( int b = 0; b < block_size; b++){
                        block.at(b) = samples.at((b*channels) + i);
                    }
                    idx = getBestMatchingUnit(codebook, block);
                    outFileIndexes << idx << " " << endl;
                }catch(exception e){}
            }
        }

        return 0;


    }else if(strcmp(op,"d") == 0){


        ifstream inFile;
        vector<int> indexes;
        inFile.open(in_file);

        if (!inFile) {
            cerr << "Unable to open file";
            exit(1);   // call system to stop
        }

        int x;
        while (inFile >> x) {
            indexes.push_back(x);
        }

        inFile.close();



        SndfileHandle sndFileOut;
        sndFileOut = SndfileHandle(out_file,SFM_WRITE,65538, channels, 44100);

        short frame [channels];
        cout << sizeof(frame) <<endl;

        for( uint32_t i = 0; i < indexes.size() / 2; i ++ ){
            for( int j = 0; j < block_size; j++){
                for( int k = 0; k < channels; k++){
                    frame[k] = codebook[indexes.at(i + (k * indexes.size()/2) )][j];
                }
                sndFileOut.writef(frame,1);
            }
        }
        sndFileOut.writeSync();
        return 0;
    }else{
        cerr << "Error: operation need to be an 'e'<encode> or an 'd'<decode> " << endl;
        return 1;
    }

}
