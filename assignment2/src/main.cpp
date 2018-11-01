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

    // Codificar
    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        // Because of the last block
        // total size may not be multiple of block_size
        samples.resize(nFrames * 2);
        left_channel.resize(nFrames);
        differences.resize(nFrames);

        // Create Predictor
        Predictor pr(5, left_channel.size());

        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }

        // Generate The residuals
        pr.populate_v(left_channel);

        // Get the best ones
        vector<short> residuals = pr.get_residuals(pr.get_best_predictor());

        float pila = pr.calculate_entropy(left_channel);
        cout << "Original: " << pila << endl;

        short o;
        cin >> o;

    }
    return 0;
}
