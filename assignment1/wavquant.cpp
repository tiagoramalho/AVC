#include "fstreamBits.h"
#include <iostream>
#include <sndfile.hh>
#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;
constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {

    if(argc < 5) {
        cerr << "Usage: wavquant <input file> <output file> <operation (e/d)> <number of bits>" << endl;
        return 1;
    }

    uint32_t bits = atoi(argv[4]);
    uint32_t shamnt = 16 - bits;
    auto op { argv[3] };

    /*
     * Encode Operation
    */

    if (strcmp(op,"e") == 0){
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

        cout << "Input file has:" << endl;
        cout << '\t' << sndFileIn.frames() << " frames" << endl;
        cout << '\t' << sndFileIn.samplerate() << " samples per second" << endl;
        cout << '\t' << sndFileIn.channels() << " channels" << endl;
        cout << '\t' << sndFileIn.format() << " format" << endl;

        WRITEBits w (argv[2]);
        w.writeHeader(sndFileIn.frames(), sndFileIn.samplerate(), sndFileIn.channels(), sndFileIn.format());

        size_t nFrames;
        vector<short> samples(FRAMES_BUFFER_SIZE * sndFileIn.channels());
        while((nFrames = sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE))){
            samples.resize(nFrames * sndFileIn.channels());
            for(uint32_t i = 0; i< samples.size(); i++){
                short s = samples[i]>>shamnt;
                w.preWrite(s, bits);
            } 
        }
        w.flush();
    /*
     * Decode Operation
    */
    }else if(strcmp(op,"d") == 0){

        READBits r (argv[1]);
        string headerLine = r.readHeader();
        string delimiter = ";";
        size_t pos;
        size_t i = 0;
        string header [4];
        while ((pos = headerLine.find(delimiter)) != std::string::npos) {
            header[i] = headerLine.substr(0, pos);
            headerLine.erase(0, pos + delimiter.length());
            i++;
        }

        int count = 0;
        int frames = stoi(header[0]);
        int samplerate = stoi(header[1]);
        int channels = stoi(header[2]);
        int format = stoi(header[3]);
        short frame [2];

        SndfileHandle sndFileOut;
        sndFileOut = SndfileHandle(argv[2], SFM_WRITE, format, channels, samplerate);

        while(count < frames){

            frame[0] = r.readItem(bits);
            frame[1] = r.readItem(bits);
            sndFileOut.writef(frame, (sizeof(frame)*8)/16/channels);
            count++;
        }
        // sndFileOut.writeSync();
        // sndFileOut = SndfileHandle(argv[2], SFM_WRITE, format, channels, samplerate);
        cout << "output file has:" << endl;
        cout << '\t' << sndFileOut.frames() << " frames" << endl;
        cout << '\t' << sndFileOut.samplerate() << " samples per second" << endl;
        cout << '\t' << sndFileOut.channels() << " channels" << endl;
        cout << '\t' << sndFileOut.format() << " format" << endl;

    }else{
        cerr << "Error: operation need to be an 'e'<encode> or an 'd'<decode> " << endl;
        return 1;
    }
    return 0;
}

