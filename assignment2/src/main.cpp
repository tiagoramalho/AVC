#include <iostream>
#include "Golomb.hpp"

using namespace std;

int main()

{
  Golomb n(35);

  cout << n.encode( 1 ) << endl;
  cout << n.decode( 1 ) << endl;

  return 0;
}
