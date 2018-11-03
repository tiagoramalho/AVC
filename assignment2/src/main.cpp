#include "Golomb.hpp"
#include "Predictor.hpp"
#include "cxxopts.hpp"
#include "wavhist.h"

#include <iostream>
#include <tuple>
#include <math.h>
#include <bitset>
#include <vector>
#include <queue>
#include <sndfile.hh>



using namespace std;

int decodeMode(string file);
int encodeMode(string file, int block_size, bool histogram);

short predict0( short residual)
{
    return residual;
}

short predict1( short residual, vector<short> & last )
{
    short prediction = residual + last[0];
    return prediction;
}

short predict2( short residual, vector<short> & last )
{
    short prediction = residual + ( 2 * last[1] - last[0]);
    last[0] = last[1];
    last[1] = prediction;
    return prediction;
}

short predict3( short residual, vector<short> & last)
{
    printf("Predict3\n");
    short prediction = residual + ( 3 * last[2] - 3 * last[1] + last[0]);
    last[0] = last[1];
    last[1] = last[2];
    last[2] = prediction;
    printf("Predict3\n");
    return prediction;
}

int main(int argc, char *argv[])
{
    try {
        cxxopts::Options options("CAVLAC", "Lossless Audio Codec made for CAV");

        // Mode of operation of the codec
        // 0 - encode
        // 1 - decode
        int mode_operation = 0;
        int block_size = 0;
        string file;

        bool histogram;


        options.add_options()
            ("h,help", "Print help")
            ("f,file", "File (obrigatory)", cxxopts::value<std::string>())
            ("m,modeopps", "Mode of operation (obrigatory)", cxxopts::value(mode_operation))
            ("b,blocksize", "Block Size (needed when encoding)", cxxopts::value(block_size))
            ("H,histogram", "If present when executed the program will write the histograms of the residuals", cxxopts::value(histogram));

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
                histogram = result["H"].as<bool>();

                exit(encodeMode(file, block_size, histogram));
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


int decodeMode(string file)
{
    printf("\n========\n Decode \n========\n");
    vector<short> last(3,0);

    // TODO: verify if the file is a Cavlac one
    Golomb n;

    READBits r (file);

    // read header of file
    vector<uint32_t> properties = r.read_header_cavlac();

    int number_of_frames = (int) properties.at(0);
    int sample_rate = (int) properties.at(1);
    int channels = (int) properties.at(2);
    int format = (int) properties.at(3);
    int block_size = (int) properties.at(4);

    printf("PROPERTIES of the file:\n"
           "number of frames: %d\n"
           "sample rate:      %d\n"
           "channels:         %d\n"
           "format:           %d\n"
           "block size:       %d\n", number_of_frames, sample_rate, channels, format, block_size);


    int full_cavlac_frames = number_of_frames/block_size;

    // Open an sndfile for writing
    // after having parameters from header of cavlac file
    string new_file = file.substr(0, file.size()-11);
    SndfileHandle sndFileOut { new_file+"_new.wav", SFM_WRITE,format,channels,sample_rate };

    short frames[block_size * channels];
    // start reading frames
    //
    for( int i = 0; i < full_cavlac_frames; i++){
        printf("pup\n");

        // Decode Left Channel
        vector<uint32_t> header_frame = r.reade_header_frame();
        printf("pup\n");

        for (uint32_t j = 0; j < header_frame.at(1); j++)
        {
            printf("pup\n");
            last[i] = r.readItem(16);
        }
        printf("pup\n");

        uint32_t m = pow(2,header_frame.at(2));
        n.set_m( m );

        printf("pup\n");
        switch( header_frame.at(1)){
            case 0:
                printf("Best predictor 0");
                for( int i=0; i < block_size; i = i + 2){
                    printf("pup\n");
                    frames[i] = n.decode(r);
                }
                break;

            case 1:
                printf("Best predictor 1");
                for( int i=0; i < block_size; i = i + 2){
                    printf("pup\n");
                    frames[i] = predict1(n.decode(r),last);
                }
                break;

            case 2:
                printf("Best predictor 2");
                for( int i=0; i < block_size; i = i + 2){
                    printf("pup\n");
                    frames[i] = predict2(n.decode(r),last);
                }
                break;

            case 3:
                printf("Best predictor 3");
                for( int i=0; i < block_size; i = i + 2){
                    printf("pup\n");
                    frames[i] = predict3(n.decode(r),last);
                }
                break;
        }

        // Decode Differences Channel
        // TODO
        break;
    }

    // TODO handle the not complete blocks

    return 0;
}

int encodeMode(string file, int block_size, bool histogram)
{
    printf("\n========\n Encode \n========\n");
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

    WAVHist residuals_hist { sndFileIn, file, 4 };
    WAVHist hist { sndFileIn, file };

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
    //printf("%04x\n", sndFileIn.frames());
    w.preWrite(block_size, 16);
    cout << "Properties: " << sndFileIn.frames() << "," << sndFileIn.samplerate()
        << "," << sndFileIn.channels() << "," << sndFileIn.format() << "," << block_size << endl;

    // Codificar
    Predictor pr(4, block_size);
    while((nFrames = sndFileIn.readf(samples.data(), block_size))) {
        /*
         * Resize vector, because last block couldn't be multiple of block size
         */
        samples.resize(nFrames * 2);
        left_channel.resize(nFrames);
        differences.resize(nFrames);

        /*
         * Resize block size and vector of residuals on Predictor
         */
        pr.set_block_size_and_clean(left_channel.size());

        uint32_t index = 0, n = 0;
        for(auto s : samples) {
            index = n % sndFileIn.channels();
            if(index == 0) left_channel.at(n/2) = s;
            else differences.at((n-1)/2) = left_channel.at((n-1)/2) - s;
            n++;
        }

        // Generate The residuals
        pr.populate_v(left_channel);
        vector<short> predictor_settings = pr.get_best_predictor_settings();

        //IF U WANT TO PRINT
        cout << "Constant Or Not: " << predictor_settings.at(2)<<endl;
        cout << "best k: " << predictor_settings.at(1)<<endl;
        cout << "Predictor Used: " << predictor_settings.at(0) <<endl;
        uint8_t constant = predictor_settings.at(2);
        uint8_t best_k = predictor_settings.at(1);
        uint8_t predictor_used = predictor_settings.at(0);

        /*
        * Write Frame Header of left channel
        */
        uint32_t write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        printf("Header: %x\n", write_header);

        uint32_t m = pow(2,best_k);

        //cout << "m: " << m << endl;

        golo.set_m( m );
        w.preWrite(write_header, 8);

        vector<short> residuals;


        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
            cout << "Foi constante no left" << endl;
            w.preWrite(left_channel.at(0), 16);
        } else {
            residuals = pr.get_residuals(predictor_used);
            uint32_t i = 0;
            for (i = 0; i < predictor_used; ++i)
            {
                w.preWrite(left_channel.at(i), 16);
            }
            for (i = predictor_used; i < residuals.size(); ++i)
            {
                golo.encode_and_write(residuals.at(i), w);
            }
        }
        if(histogram){
            hist.simple_update_index(0, pr.get_residuals(predictor_used));
            residuals_hist.simple_update_index(0, pr.get_residuals(0));
            residuals_hist.simple_update_index(2, pr.get_residuals(1));
            residuals_hist.simple_update_index(4, pr.get_residuals(2));
            residuals_hist.simple_update_index(6, pr.get_residuals(3));
        }


        /*
         * Clean averages vector, residuals vector
         */
        pr.set_block_size_and_clean(differences.size());
        pr.populate_v(differences);


        /*
        * Write Frame Header of differences
        */
        constant = predictor_settings.at(2);
        best_k = predictor_settings.at(1);
        predictor_used = predictor_settings.at(0);

        write_header = constant;
        write_header = write_header << 2;
        write_header = write_header | predictor_used;
        write_header = write_header << 4;
        write_header = write_header | best_k;

        printf("Header Differences: %x\n", write_header);
        //cout << "Header: " << hex << write_header << endl;

        m = pow(2,best_k);

        //cout << "m: " << m << endl;
        golo.set_m( m );

        w.preWrite(write_header, 8);

        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
            cout << "Foi constante nas samples" << endl;
            w.preWrite(differences.at(0), 16);
        } else {
            residuals = pr.get_residuals(predictor_used);
            uint32_t i = 0;
            for (i = 0; i < predictor_used; ++i)
            {
                w.preWrite(differences.at(i), 16);
            }
            for (i = predictor_used; i < residuals.size(); ++i)
            {
                golo.encode_and_write(residuals.at(i), w);
            }
        }
        if(histogram){
            hist.simple_update_index(1, pr.get_residuals(predictor_used));
            residuals_hist.simple_update_index(1, pr.get_residuals(0));
            residuals_hist.simple_update_index(3, pr.get_residuals(1));
            residuals_hist.simple_update_index(5, pr.get_residuals(2));
            residuals_hist.simple_update_index(7, pr.get_residuals(3));
        }

        break;

    }
    w.flush();

    if(histogram){
        hist.full_dump();
        residuals_hist.full_dump();
    }

    return 0;
}
