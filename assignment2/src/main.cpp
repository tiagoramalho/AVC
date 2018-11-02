#include "Golomb.hpp"
#include "Predictor.hpp"

#include <iostream>
#include <tuple>
#include <math.h>
#include <bitset>
#include <vector>
#include <sndfile.hh>


using namespace std;

int main(int argc, char *argv[]) {

    if(argc < 3) {
        cerr << "Usage: wavcp <wav input file> <number of samples per block>" << endl;
        return 1;
    }

    uint32_t block_size = atoi(argv[2]);
    if(block_size == 0) {
        cerr << "Error: Invalid number of samples per block" << endl;
        return 1;
    }

    SndfileHandle sndFileIn { argv[1] };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
        return 1;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
        return 1;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
        return 1;
    }


    vector<short> left_channel(block_size);
    vector<short> differences(block_size);

    size_t nFrames;
    vector<short> samples(block_size * 2);

    uint32_t m = 5;
    Golomb golombBits(m, "pila.bin");

    WRITEBits w = golombBits.open_to_write();
    // frames -> 32 bits -> 4 bytes
    // samplerate -> 32 bits -> 4 bytes
    // channels- > 16 bits -> 2 bytes
    // format -> 16 bits -> 2 bytes

    //TODO ver frames
    w.preWrite(sndFileIn.frames(), 32);
    w.preWrite(sndFileIn.samplerate(), 32);
    w.preWrite(sndFileIn.channels(), 16);
    w.preWrite(sndFileIn.format(), 16);
    w.preWrite(block_size, 16);

    // Codificar
    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        // Because of the last block
        // total size may not be multiple of block_size
        samples.resize(nFrames * 2);
        left_channel.resize(nFrames);
        differences.resize(nFrames);

        // Create Predictor
        // TODO: Maybe put out of the loop
        // and do a resize of the pr->block_size
        Predictor pr(4, left_channel.size());

        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }

        // Generate The residuals
        pr.clean_averages();
        pr.populate_v(left_channel);
        /*
        char op;
        cin >> op;
        */
        vector<short> predictor_settings = pr.get_best_predictor_settings(0);
        
        //IF U WANT TO PRINT
        //cout << "Constant Or Not: " << predictor_settings.at(2)<<endl;
        //cout << "best k: " << predictor_settings.at(1)<<endl;
        //cout << "Predictor Used: " << predictor_settings.at(0) <<endl;
        uint8_t constant = predictor_settings.at(2);
        uint8_t best_k = predictor_settings.at(1);
        uint8_t predictor_used = predictor_settings.at(0);

        uint32_t write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        //cout << "Header: " << hex << write_header << endl;

        uint32_t m = pow(2,best_k);

        //cout << "m: " << m << endl;
        golombBits.set_m( m );

        golombBits.write_frame_header( write_header, 8 , w);

        vector<short> residuals = pr.get_residuals(predictor_used);

        for(short const& value: residuals) {
          golombBits.encode_and_write(value, w);
        }

        // Write Frame Header
        pr.clean_averages();
        pr.populate_v(differences);


        constant = predictor_settings.at(2);
        best_k = predictor_settings.at(1);
        predictor_used = predictor_settings.at(0);

        write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        //cout << "Header: " << hex << write_header << endl;

        m = pow(2,best_k);

        //cout << "m: " << m << endl;
        golombBits.set_m( m );

        golombBits.write_frame_header( write_header, 8 , w);

        residuals = pr.get_residuals(predictor_used);

        for(short const& value: residuals) {
          golombBits.encode_and_write(value, w);
        }

        


    }
    golombBits.close(w);
    return 0;
}
