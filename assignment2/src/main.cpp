#include "Golomb.hpp"
#include "Predictor.hpp"

#include <iostream>
#include <tuple>
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
    // Codificar
    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        // Because of the last block
        // total size may not be multiple of block_size
        samples.resize(nFrames * 2);
        left_channel.resize(nFrames);
        differences.resize(nFrames);

        // Create Predictor
        Predictor pr(4, left_channel.size());

        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }

        // Generate The residuals
        pr.populate_v(left_channel);

        // Get best predictor settings
        vector<short> predictor_settings = pr.get_best_predictor_settings(0);

        golombBits.set_m(predictor_settings.at(1));


        // Get the best ones
        vector<short> residuals = pr.get_residuals(predictor_settings.at(0));

        double sum = 0.0;
        for(uint32_t i = 0; i < differences.size(); i++){
            sum = sum + (abs(differences.at(i))/differences.size());
        }

        cout << "Median of differences " << sum;

        char o;
        cin >> o;


        // Write only one block  | Debug purposes
        // break;

    }
    return 0;
}
