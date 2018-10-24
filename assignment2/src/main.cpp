#include <iostream>
#include "Golomb.hpp"
#include <tuple>
#include <bitset>


using namespace std;

int main()

{
    uint32_t m = 5;
    Golomb n(m);

    uint32_t number_of_bits;
    uint32_t ret;

    int i = -8;
    while (i <= 7) {

		cout << "n -> "<< i  << endl;

		std::tie(number_of_bits,ret) = n.encode(i);

		cout << "hex return -> "<< std::bitset<8>(ret)  << endl;
		cout << "number of bits to write -> "<< number_of_bits << endl;

		cout << "Decoded value -> " <<  n.decode(ret, number_of_bits) << endl << endl << endl;
		i++;

    }
    return 0;
}
