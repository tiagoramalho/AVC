#include <iostream>
#include "Golomb.hpp"
#include "Predictor.hpp"
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

    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        samples.resize(nFrames * 2);
        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }
    }


    Predictor pr(4, left_channel.size());

    cout << left_channel.at(1) << endl;

    pr.populate_v(left_channel);

    vector<float> entropies = pr.calculate_entropies(false);

    for( uint32_t i = 0; i < entropies.size(); i++){
      cout << entropies.at(i) << endl;
    }

    /*
    uint32_t m = 5;
    Golomb n(m, "pila.bin");

    int i = -8;
    //n.encode_and_write(i);
    //uint32_t number_of_bits;
    //uint32_t ret;

    WRITEBits w = n.open_to_write();

    while (i <= 7) {

        cout << "n -> "<< i  << endl;

        n.encode_and_write(i, w);

        //cout << "hex return -> "<< std::bitset<8>(ret)  << endl;
        //cout << "number of bits to write -> "<< number_of_bits << endl;

        //cout << "Decoded value -> " <<  n.decode() << endl << endl << endl;
        i++;

    }
    n.close(w);

    READBits r = n.open_to_read();
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;
    cout <<    n.decode(r) << endl;

    */
    return 0;
}
