#include "fstreamBits.h"
#include <map>
#include <sndfile.hh>
#include <vector>
#include <math.h>

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

int main(int argc, char *argv[]) {

	if(argc < 1) {
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
    std::map<uint64_t, uint64_t> code;
    
    uint64_t m = 4;

    vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());

    while((nFrames = sndFile.readf(samples.data(), 65536))) {
        for(auto n : samples) {
            std::map<uint64_t, uint64_t>::iterator it = code.find(n);

            if (it == code.end()){
                uint64_t valor = (uint64_t) n;
                int q = (int) Math.Ceiling(((float) n)/((float) m ));
                int r = n - q*m;
                 
                code.insert((uint64_t) n, )
                

            }

        }
    }
    

    return 0;
}

