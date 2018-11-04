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

short predict1( short residual, vector<short> & frames , int idx)
{
    short prediction = residual + frames.at(idx-1);
    return prediction;
}

short predict2( short residual, vector<short> & frames , int idx)
{
    short prediction = residual + ( 2 * frames.at(idx-1) - frames.at(idx-2));
    return prediction;
}

short predict3( short residual, vector<short> & frames, int idx)
{
    //printf("Predict3\n");
    short prediction = residual + ( 3 * frames.at(idx-1) - 3 * frames.at(idx-2) + frames.at(idx-3));
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
    printf("========\n Decode \n========\n");

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
    
    vector<short> frames_left(block_size,0);
    vector<short> frames_right(block_size,0);

    // start reading frames
    //
    for( int i = 0; i < full_cavlac_frames; i++){
        // Decode Left Channel
        vector<uint32_t> header_frame = r.reade_header_frame();

        printf("FRAME LEFT%d:\n"
           "constant:   %d\n"
           "predictor:  %d\n"
           "k:          %d\n",i, header_frame.at(0), header_frame.at(1), header_frame.at(2));


        for (uint32_t j = 0; j < header_frame.at(1); j++)
        {
            frames_left[j] = r.readItem(16);
        }

        uint32_t m = pow(2,header_frame.at(2));
        n.set_m( m );

        switch( header_frame.at(1)){
            case 0:
                for(int j = 0; j < block_size; j++){
                    frames_left.at(j) = n.decode(r);
                }
                break;

            case 1:
                for(int j = 1; j < block_size; j++){
                    frames_left.at(j) = predict1(n.decode(r),frames_left, j);
                }
                break;

            case 2:
                for(int j = 2; j < block_size; j++){
                    frames_left.at(j) = predict2(n.decode(r),frames_left, j);
                }
                break;

            case 3:
                for(int j = 3; j < block_size; j++){
                    frames_left.at(j) = predict3(n.decode(r),frames_left, j);
                }
                break;
        }

        
        header_frame = r.reade_header_frame();

        printf("FRAME DIFFS%d:\n"
           "constant:   %d\n"
           "predictor:  %d\n"
           "k:          %d\n",i, header_frame.at(0), header_frame.at(1), header_frame.at(2));


        for (uint32_t j = 0; j < header_frame.at(1); j++)
        {
            frames_right[j] = r.readItem(16);
        }

        m = pow(2,header_frame.at(2));
        n.set_m( m );

        switch( header_frame.at(1)){
            case 0:
                for(int j = 0; j < block_size; j++){
                    frames_right.at(j) = n.decode(r);
                }
                break;

            case 1:
                for(int j = 1; j < block_size; j++){
                    frames_right.at(j) = predict1(n.decode(r),frames_right, j);
                }
                break;

            case 2:
                for(int j = 2; j < block_size; j++){
                    frames_right.at(j) = predict2(n.decode(r),frames_right, j);
                }
                break;

            case 3:
                for(int j = 3; j < block_size; j++){
                    frames_right.at(j) = predict3(n.decode(r),frames_right, j);
                }
                break;
        }
        
        for( int l = 0; l < block_size; l++){
            printf("%3d -> %8x | %8x\n",l,  frames_left.at(l), frames_right.at(l) + frames_left.at(l));
            short frame [2];
            frame[0] = frames_left.at(l);
            frame[1] = frames_right.at(l) + frames_left.at(l);
            sndFileOut.writef(frame, 1);
        }

        /*
        sndFileOut.writef(frames, block_size);
        //int count=0;
        //short frame [2];
        //while(count < block_size){
        //    frame[0] = frames[count*2];
        //    frame[1] = frames[count*2 + 1] + frames[count*2];
        //    sndFileOut.writef(frame, (sizeof(frame)*8)/16/channels);
        //    count++;
        //}
        */


    }

    // TODO handle the not complete blocks

    return 0;
}

int encodeMode(string file, int block_size, bool histogram)
{
    printf("========\n Encode \n========\n");
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
    w.preWrite(block_size, 16);

    printf("PROPERTIES of the file:\n"
           "number of frames: %d\n"
           "sample rate:      %d\n"
           "channels:         %d\n"
           "format:           %d\n"
           "block size:       %d\n", sndFileIn.frames(), sndFileIn.samplerate(), sndFileIn.channels(), sndFileIn.format(), block_size);

    // Codificar
    Predictor pr(4, block_size);
    int count = 0;
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
            if(index == 0)
                left_channel.at(n/2) = s;
            else 
                differences.at((n-1)/2) = s - left_channel.at((n-1)/2);
            n++;
        }

        // Generate The residuals
        pr.populate_v(left_channel);
        vector<short> predictor_settings = pr.get_best_predictor_settings();

        //IF U WANT TO PRINT
        //cout << "Constant Or Not: " << predictor_settings.at(2)<<endl;
        //cout << "best k: " << predictor_settings.at(1)<<endl;
        //cout << "Predictor Used: " << predictor_settings.at(0) <<endl;
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
        printf("FRAME LEFT%d:\n"
           "constant:   %d\n"
           "predictor:  %d\n"
           "k:          %d\n",count, constant, predictor_used, best_k);


        // printf("Header: %x\n", write_header);

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

        printf("FRAME DIFFS%d:\n"
           "constant:   %d\n"
           "predictor:  %d\n"
           "k:          %d\n",count, constant, predictor_used , best_k);


        // printf("Header Differences: %x\n", write_header);
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
            cout << "Foi constante no right" << endl;
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

        // Debug Prints
        for( int l = 0; l < nFrames; l++){
            printf("%3d -> %8x | %8x\n",l,  left_channel.at(l), left_channel.at(l) + differences.at(l));
        }

        count++;


    }
    w.flush();

    if(histogram){
        hist.full_dump();
        residuals_hist.full_dump();
    }

    return 0;
}
