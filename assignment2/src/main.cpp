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

int decodeMode(string file);
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
                exit(decodeMode(file));
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


int decodeMode(string file){
    cout << "decode" << endl;

    // TODO: verify if the file is a Cavlac one
    Golomb n();

    READBits w (file);

    // read header of file
    vector<uint32_t> properties = w.read_header_cavlac();

    for( int a: properties)
        cout << a << ";";

    // open an sndfile for writing
    // after having parameters from header of cavlac file
    SndfileHandle sndFileOut { file, SFM_WRITE, (int) properties.at(3),
        (int) properties.at(2), (int) properties.at(1) };

    // start reading frames



    return 0;
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

    Golomb golo;

    WRITEBits w (file+".cavlac");
    // frames -> 32 bits -> 4 bytes
    // samplerate -> 32 bits -> 4 bytes
    // channels- > 16 bits -> 2 bytes
    // format -> 16 bits -> 2 bytes

    //TODO ver frames
    w.preWrite(sndFileIn.frames(), 32);
    w.preWrite(sndFileIn.samplerate(), 32);
    w.preWrite(sndFileIn.channels(), 16);
    w.preWrite(sndFileIn.format(), 32);
    printf("%04x\n", sndFileIn.format());
    w.preWrite(block_size, 16);
    cout << "Properties: " << sndFileIn.frames() << "," << sndFileIn.samplerate()
        << "," << sndFileIn.channels() << "," << sndFileIn.format() << "," << block_size << endl;

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
        pr.clean_averages();
        pr.populate_v(left_channel);
        /*
        char op;
        cin >> op;
        */
        vector<short> predictor_settings = pr.get_best_predictor_settings();

        //IF U WANT TO PRINT
        //cout << "Constant Or Not: " << predictor_settings.at(2)<<endl;
        //cout << "best k: " << predictor_settings.at(1)<<endl;
        //cout << "Predictor Used: " << predictor_settings.at(0) <<endl;
        uint8_t constant = predictor_settings.at(2);
        uint8_t best_k = predictor_settings.at(1);
        uint8_t predictor_used = predictor_settings.at(0);

        uint32_t write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        //cout << "Header: " << hex << write_header << endl;

        uint32_t m = pow(2,best_k);

        //cout << "m: " << m << endl;
        golo.set_m( m );

        w.preWrite(write_header, 8);

        vector<short> residuals = pr.get_residuals(predictor_used);

        for(short const& value: residuals) {
            golo.encode_and_write(value, w);
        }

        // Write Frame Header
        pr.clean_averages();
        pr.populate_v(differences);


        constant = predictor_settings.at(2);
        best_k = predictor_settings.at(1);
        predictor_used = predictor_settings.at(0);

        write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        //cout << "Header: " << hex << write_header << endl;

        m = pow(2,best_k);

        //cout << "m: " << m << endl;
        golo.set_m( m );

        w.preWrite(write_header, 8);

        residuals = pr.get_residuals(predictor_used);

        for(short const& value: residuals) {
          golo.encode_and_write(value, w);
        }


    }

    w.flush();
    return 0;
}
