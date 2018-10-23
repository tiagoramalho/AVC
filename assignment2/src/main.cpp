#include <iostream>
#include "Golomb.hpp"
#include <tuple>

using namespace std;

int main()

{
  Golomb n(5);

  int a;
  uint32_t b;

  std::tie(a,b) = n.encode(-7);

  cout << "number of bits to write -> "<< a << endl;
  cout << "hex return -> "<< hex << b << endl;
  cout << n.decode( 1 ) << endl;

  return 0;
}
