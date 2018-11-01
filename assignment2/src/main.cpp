#include <iostream>
#include "Golomb.hpp"
#include "Predictor.hpp"
#include <tuple>
#include <bitset>
#include <vector>


using namespace std;

int main()

{

    vector<short> left_channel = {1, 2, -5, -1, 3};
    vector<short> differences = {0, 1, -1, 1, 0};

    // Create Predictor
    Predictor pr(4, left_channel.size());

    // Generate The residuals
    pr.populate_v(left_channel);

    // Get the best ones
    vector<short> residuals = pr.get_residuals(pr.get_best_predictor());

    uint32_t m = 5;
    Golomb golombBits(m, "pila.bin");

    WRITEBits w = golombBits.open_to_write();

    for( uint32_t i = 0; i < residuals.size(); i++){
        // Write Residual of Left Channel
        golombBits.encode_and_write(residuals.at(i), w);
        // Write Difference from left channel t right
        golombBits.encode_and_write(differences.at(i), w);
    }


    golombBits.close(w);

    READBits r = golombBits.open_to_read();
    cout <<    golombBits.decode(r) << " ";
    cout <<    golombBits.decode(r) << endl;
    cout <<    golombBits.decode(r) << " ";
    cout <<    golombBits.decode(r) << endl;
    cout <<    golombBits.decode(r) << " ";
    cout <<    golombBits.decode(r) << endl;
    cout <<    golombBits.decode(r) << " ";
    cout <<    golombBits.decode(r) << endl;
    cout <<    golombBits.decode(r) << " ";
    cout <<    golombBits.decode(r) << endl;

    return 0;
}
