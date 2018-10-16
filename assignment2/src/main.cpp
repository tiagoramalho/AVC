#include <iostream>
#include "Golomb.hpp"
#include <tuple>

using namespace std;

int main()

{
  Golomb n(35);

  int a;
  uint32_t b;

  std::tie(a,b) = n.encode(1);

  cout << a << endl;
  cout << n.decode( 1 ) << endl;

  return 0;
}
