#include <iostream>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <sndfile.hh>
#include "wavhist.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

double euclideanDistance(vector<short>&v1, vector<short>&v2){

    double distance = 0;
    for(uint32_t i = 0; i<v1.size();i++){

        distance += pow((v1[i] - v2[i]),2);

    }
    return sqrt(distance);
}
int getBestMatchingUnit(vector<vector<short>> cb, vector<short> row){

    int idx = 0;
    double dst {};
    double smallDst = euclideanDistance(cb[0], row);
    for(uint32_t i = 1; i< cb.size(); i++){
        dst = euclideanDistance(cb[i], row);
        if( dst < smallDst){
            smallDst = dst;
            idx =i;
        }
    }
    return idx;
}
vector<short> getCentroid(vector<vector<short>> v){
    vector<short> vR;
    //cout << v[0][0] << endl;
    vR.resize( v[0].size()) ;
    for(uint32_t i = 0; i<v.size(); i++){
        //cout << i << endl;
        for(uint32_t j = 0; j<v[i].size(); j++){
            //cout << j << endl;
            vR[j] += v[i][j]/v.size();
        }
    }
    return vR;

}

int main(int argc, char *argv[]) {

	if(argc < 5) {
		cerr << "Usage: wavhist <input file> <block size> <overlap size> <codebook size> " << endl;
		return 1;
	}

	SndfileHandle sndFile { argv[1] };
	if(sndFile.error()) {
		cerr << "Error: invalid input file" << endl;
		return 1;
    }

	if((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format" << endl;
		return 1;
	}

	if((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format" << endl;
		return 1;
	}


    size_t nFrames;
    vector<vector<short>> dataSet;
    vector<vector<short>> codebook;

    vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());
    WAVHist hist { sndFile };
    while((nFrames = sndFile.readf(samples.data(), 4))) {
        samples.resize(8);
        vector<short> A = {samples[0], samples[2], samples[4], samples[6]};
        vector<short> B = {samples[1], samples[3], samples[5], samples[7]};

        dataSet.push_back(A);
        dataSet.push_back(B);
    }

    cout << "create cb" << endl;

    //cout << sndFile.frames();
    for(int i = 0; i < stoi(argv[4]); i++){
        codebook.push_back(dataSet[rand() % dataSet.size()]);
    }

    cout << "get v" << endl;
    
    bool change;
    do{
        change = false;
        map<uint32_t, vector<vector<short>>> mapDst {};
        for(uint32_t i = 0; i<dataSet.size(); i++){
            int idx = getBestMatchingUnit(codebook, dataSet[i]);
            mapDst[idx].push_back(dataSet[i]);
        
        }

        cout << "change cb" << endl;
        for(auto [value, vv] : mapDst){
            vector<short> pilinhadobranco = getCentroid(vv);
            if (codebook[value] != pilinhadobranco){
                codebook[value] = pilinhadobranco;
                change = true;
            }
        }
    }while(change);




    cout << dataSet.size() << endl;
    cout << sndFile.frames();

    return 0;
}


