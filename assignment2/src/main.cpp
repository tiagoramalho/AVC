#include "Golomb.hpp"
#include "Predictor.hpp"
#include "cxxopts.hpp"

#include <iostream>
#include <tuple>
#include <math.h>
#include <bitset>
#include <vector>
#include <sndfile.hh>


using namespace std;

void decodeMode();
int encodeMode(string file, int block_size);

int main(int argc, char *argv[]) {
    try {
        cxxopts::Options options("CAVLAC", "Lossless Audio Codec made for CAV");

        // Mode of operation of the codec
        // 0 - encode
        // 1 - decode
        int mode_operation = 0;

        int block_size = 0;

        string file;

        options.add_options()
            ("h,help", "Print help")
            ("f,file", "File (obrigatory)", cxxopts::value<std::string>())
            ("m,modeopps", "Mode of operation (obrigatory)", cxxopts::value(mode_operation))
            ("b,blocksize", "Block Size (needed when encoding)", cxxopts::value(block_size));

        auto result = options.parse(argc, argv);

        if (result.count("help")){
            cout << options.help() << endl;
            exit(0);
        }

        // Verify if a file was passed because its needed
        // in both modes of operation
        if (result.count("f") != 1){
            cout << endl << "You always need to specify a file" << endl << endl;
            cout << options.help() << endl;
            exit(1);
        }else{
            file = result["f"].as<string>();
        }

        if(result.count("m")){
            mode_operation = result["m"].as<int>();
            if (  !mode_operation ){

                // Encoding Mode
                if(result.count("b") != 1){
                    cout << endl << "In Encoding Mode you need to specify a block size" << endl << endl;
                    cout << options.help() << endl;
                    exit(1);
                }

                block_size = result["b"].as<int>();

                exit(encodeMode(file, block_size));
            }else{
                // Decoding Mode
                decodeMode();
                exit(0);
            }
        }else{
            cout << options.help() << endl;
            exit(1);
        }

        cout << options.help() << endl;
        exit(0);
    }catch(const cxxopts::OptionException& e){
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
    return 0;
}


void decodeMode(){
    cout << "decode" << endl;

}

int encodeMode(string file, int block_size){
    cout << "encode" << endl;

    SndfileHandle sndFileIn { file };
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

    vector<short> left_channel(block_size);
    vector<short> differences(block_size);

    size_t nFrames;
    vector<short> samples(block_size * 2);

    uint32_t m = 5;
    Golomb golombBits(m, "pila.bin");

    WRITEBits w = golombBits.open_to_write();

    // Codificar
    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        // Because of the last block
        // total size may not be multiple of block_size
        samples.resize(nFrames * 2);
        left_channel.resize(nFrames);
        differences.resize(nFrames);

        // Create Predictor
        // TODO: Maybe put out of the loop
        // and do a resize of the pr->block_size
        Predictor pr(4, left_channel.size());

        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }

        // Generate The residuals
        pr.populate_v(left_channel);
        // Get best predictor settings
        //
        // TODO: Make this all variables
        vector<short> predictor_settings = pr.get_best_predictor_settings(0);
        cout << "Constant Or Not: " << predictor_settings.at(2)<<endl;
        cout << "best k: " << predictor_settings.at(1)<<endl;
        cout << "Predictor Used: " << predictor_settings.at(0) <<endl;

        uint32_t write_header = predictor_settings.at(2);
        write_header = write_header << 2;
        write_header = write_header | predictor_settings.at(0);
        write_header = write_header << 4;
        write_header = write_header | predictor_settings.at(1);

        cout << "Header: " << hex << write_header << endl;

        uint32_t m = pow(2,predictor_settings.at(1));

        cout << "m: " << m << endl;
        golombBits.set_m( m );

        golombBits.write_frame_header( write_header, 8 , w);

        vector<short> residuals = pr.get_residuals(predictor_settings.at(0));

        for(short const& value: residuals) {
            golombBits.encode_and_write(value, w);
        }

        // Write Frame Header

        // Write Residuals

        // Generate The residuals
        // pr.populate_v(differences);

        // // Get best predictor settings
        // vector<short> predictor_settings_b = pr.get_best_predictor_settings(0);

        // cout << "Id Best Predictor Differences: " << predictor_settings_b[0] <<endl;

        // // Write Frame Header
        // // Write Residuals
        // golombBits.set_m(predictor_settings.at(1));


        // // Get the best ones
        // vector<short> residuals = pr.get_residuals(predictor_settings.at(0));

        // double sum = 0.0;
        // for(uint32_t i = 0; i < differences.size(); i++){
        //     sum = sum + (abs(differences.at(i))/differences.size());
        // }

        // cout << "Median of differences " << sum;

        // Write only one block  | Debug purposes
        break;
    }
    return 0;
}
