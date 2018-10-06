#include <iostream>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <sndfile.hh>
#include "wavhist.h"
#include <sstream>

#include <iterator>
#include <numeric>
#include <algorithm>

using namespace std;

double euclid_distance_squared(vector<short> &v1, vector<short> &v2){
  double dist = 0.0;
  double d;
  for( unsigned f = 0; f < v1.size(); f++){
    d = v1[f] - v2[f];
    dist += d*d;
  }
  return dist;
}
uint32_t getBestMatchingUnit(vector<vector<short>> &cb, vector<short> &row){
    int idx = 0;
    int dst {};
    int smallDst = euclid_distance_squared(cb.at(0), row);
    for(uint32_t i = 1; i< cb.size(); i++){
        dst = euclid_distance_squared(cb[i], row);
        if( dst < smallDst){
            smallDst = dst;
            idx =i;
        }
    }
    return idx;
}

int main(int argc, char *argv[]) {

  if(argc < 3) {
    cerr << "Usage: wavhist <output file> <input file> <codebook> " << endl;
    return 1;
  }

  // open file with codebook indexes
  cout << "open file" << endl;
  ifstream inFile;
  vector<int> indexes;
  inFile.open(argv[2]);

  if (!inFile) {
    cerr << "Unable to open file";
    exit(1);   // call system to stop
  }

  int x;
  while (inFile >> x) {
    indexes.push_back(x);
  }

  inFile.close();

  // open codebook
  cout << "open codebook" << endl;
  vector<vector<short>> codebook;
  inFile.open(argv[3]);
  if (!inFile) {
    cerr << "Unable to open codebook";
    exit(1);   // call system to stop
  }

  string tmp;
  while (getline(inFile, tmp)) {
    istringstream iss(tmp);
    codebook.push_back(
        vector<short>(
        istream_iterator<short>(iss),
        istream_iterator<short>() ));
  }

  for ( uint32_t i = 0; i < codebook.size(); i++){

    for ( uint32_t j = 0; j < codebook.at(i).size(); j++){
      cout << codebook.at(i).at(j) << " ";
    }

    cout << endl;
  }


  SndfileHandle sndFileOut;
  sndFileOut = SndfileHandle(argv[1],SFM_WRITE,65538, 2, 44100);

  short frame [2];
  cout << sizeof(frame) <<endl;

  for( uint32_t i = 0; i < indexes.size(); i = i+2){
    frame[0] = codebook[indexes.at(i)][0];
    frame[1] = codebook[indexes.at(i+1)][0];
    sndFileOut.writef(frame, 1);
    frame[0] = codebook[indexes.at(i)][1];
    frame[1] = codebook[indexes.at(i+1)][1];
    sndFileOut.writef(frame, 1);
    frame[0] = codebook[indexes.at(i)][2];
    frame[1] = codebook[indexes.at(i+1)][2];
    sndFileOut.writef(frame, 1);
    frame[0] = codebook[indexes.at(i)][3];
    frame[1] = codebook[indexes.at(i+1)][3];
    sndFileOut.writef(frame, 1);
  }
  sndFileOut.writeSync();
  cout << sndFileOut.frames();
  return 0;
}
