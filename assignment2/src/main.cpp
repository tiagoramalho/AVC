#include <iostream>
#include "Golomb.hpp"
#include "Predictor.hpp"
#include <tuple>
#include <bitset>
#include <vector>


using namespace std;

int main()

{

    vector<short> v = {1, 2, 3, 4, 5};

    Predictor pr(4, 5);

    cout << v.at(1) << endl;

    pr.populate_v(v);

    vector<float> entropies = pr.calculate_entropies(false);

    for( int i = 0; i < entropies.size(); i++){
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
