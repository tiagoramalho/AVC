#include <iostream>
#include <math.h>
#include <sndfile.hh>
#include <vector>

using namespace std;
constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {

    if(argc < 3) {
        cerr << "Usage: wavcmp <audio file> <original file>" << endl;
        return 1;
    }

    SndfileHandle sndFileNew { argv[1] };

    if(sndFileNew.error()) {
        cerr << "Error: invalid new audio file" << endl;
        cerr << sndFileNew.error() << endl;
        return 1;
    }

    if((sndFileNew.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: new audio file is not in WAV format" << endl;
        return 1;
    }

    if((sndFileNew.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: new audio file is not in PCM_16 format" << endl;
        return 1;
    }

    SndfileHandle sndFileOriginal { argv[2] };
    if(sndFileOriginal.error()) {
        cerr << "Error: invalid original audio file" << endl;
        return 1;
    }

    if((sndFileOriginal.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: original audio file is not in WAV format" << endl;
        return 1;
    }

    if((sndFileOriginal.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: original audio file is not in PCM_16 format" << endl;
        return 1;
    }

    if(sndFileNew.frames()!=sndFileOriginal.frames() || 
       sndFileNew.samplerate()!=sndFileOriginal.samplerate() || 
       sndFileNew.channels()!=sndFileOriginal.channels() || 
       sndFileNew.format()!=sndFileOriginal.format()){
    
        cerr << "Error: the two files don't have the same header characteristics" << endl;
        return 1;
    
    
    }

    int frames;
    int channels = sndFileOriginal.channels();

    vector<short> samplesNew(FRAMES_BUFFER_SIZE * sndFileNew.channels());
    vector<short> samplesOriginal(FRAMES_BUFFER_SIZE * sndFileOriginal.channels());
    long originalChannels[channels] = {};
    long newChannels[channels] = {};

    while((frames = sndFileNew.readf(samplesNew.data(), FRAMES_BUFFER_SIZE))){
        samplesNew.resize(frames*channels);
        
        frames = sndFileOriginal.readf(samplesOriginal.data(), FRAMES_BUFFER_SIZE);
        samplesOriginal.resize(frames*channels);
        for(int i = 0; i < frames*channels; i++){
            originalChannels[i%channels]+=(long)pow(samplesOriginal[i], 2);
            newChannels[i%channels]+= (long)pow(samplesOriginal[i]-samplesNew[i], 2);
        } 
    }
    long double SNR;
    for(int i = 0; i < channels; i++){
        SNR = (long double)originalChannels[i]/(long double)  newChannels[i];
        cout << "SNR of channel: " << i << " is: " <<10*log10(SNR) << " db" << endl;
    }

    return 0;

}

