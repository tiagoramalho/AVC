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

int encodeLossyMode(string file, int block_size, bool histogram, int shamt);
int decodeLossyMode(string file, int shamt);


int predict1( int residual, vector<int> & frames , int idx)
{
    int prediction = residual + frames.at(idx-1);
    return prediction;
}

int predict2( int residual, vector<int> & frames , int idx)
{
    int prediction = residual + ( 2 * frames.at(idx-1) - frames.at(idx-2));
    return prediction;
}

int predict3( int residual, vector<int> & frames, int idx)
{
    int prediction = residual + ( 3 * frames.at(idx-1) - 3 * frames.at(idx-2) + frames.at(idx-3) );
    return prediction;
}

vector<int> constant_frame(int size, READBits & r){
    int j = 0;
    vector<int> frames(size,0);
    short item = r.readItem(16);
    while(j < size){
        frames[j] = item;
        j++;
    }
    return frames;

}
vector <int> frames_decode(uint32_t predictor, READBits & r, Golomb & n, int k, int size){

    vector<int> frames(size,0);
    for (uint32_t j = 0; j < predictor; j++)
    {
        frames[j] = r.readItem(16);
    }

    uint32_t m = pow(2,k);
    n.set_m( m );

    switch( predictor ){
        case 0:
            for(int j = 0; j < size; j++){
                frames.at(j) = n.decode(r);
            }
            break;

        case 1:
            for(int j = 1; j < size; j++){
                frames.at(j) = predict1(n.decode(r),frames, j);
            }
            break;

        case 2:
            for(int j = 2; j < size; j++){
                frames.at(j) = predict2(n.decode(r),frames, j);
            }
            break;

        case 3:
            for(int j = 3; j < size; j++){
                frames.at(j) = predict3(n.decode(r),frames, j);
            }
            break;
    }
    return frames;
}

vector <int> frames_lossy_decode(uint32_t predictor, READBits & r, Golomb & n, int k, int size, int shamt){

    vector<int> frames(size,0);
    for (uint32_t j = 0; j < predictor; j++)
    {
        frames[j] = r.readItem(16 - shamt) ;
    }

    uint32_t m = pow(2,k);
    n.set_m( m );

    switch( predictor ){
        case 0:
            for(int j = 0; j < size; j++){
                frames.at(j) = ((n.decode(r) << 1) | 1U ) << (shamt-1);
            }
            break;

        case 1:
            for(int j = 1; j < size; j++){
                frames.at(j) = predict1(((n.decode(r) << 1) | 1U ) << (shamt-1), frames, j);
            }
            break;

        case 2:
            for(int j = 2; j < size; j++){
                frames.at(j) = predict2(((n.decode(r) << 1) | 1U ) << (shamt-1), frames, j);
            }
            break;

        case 3:
            for(int j = 3; j < size; j++){
                frames.at(j) = predict3(((n.decode(r) << 1) | 1U ) << (shamt-1), frames, j);
            }
            break;
    }
    return frames;
}

void write_samples_block(int size, vector<int> left, vector<int> right, SndfileHandle & sndFileOut){

    for( int l = 0; l < size; l++){
        short frame [2];

        frame[0] = left.at(l);
        frame[1] = right.at(l);
        sndFileOut.writef(frame, 1);
    }

}

int main(int argc, char *argv[])
{
    try {
        cxxopts::Options options("CAVLAC", "Lossless Audio Codec made for CAV");

        /* Mode of operation of the codec
         * 0 - encode
         * 1 - decode
         */
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

        /*
         * Verify if a file was passed because its needed in both modes of operation
         */
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

                /*
                 * Encoding Mode
                 */
                if(result.count("b") != 1){
                    cout << endl << "In Encoding Mode you need to specify a block size" << endl << endl;
                    cout << options.help() << endl;
                    exit(1);
                }

                block_size = result["b"].as<int>();
                histogram = result["H"].as<bool>();

                exit(encodeLossyMode(file, block_size, histogram, 4));
            }else{
                /*
                 * Decoding Mode
                 */
                exit(decodeLossyMode(file,4));
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

    // TODO: verify if the file is a Cavlac one
    Golomb n;

    READBits r (file);

    /*
     * Read header of file
     */
    vector<uint32_t> properties = r.read_header_cavlac();

    int number_of_frames = (int) properties.at(0);
    int sample_rate = (int) properties.at(1);
    int channels = (int) properties.at(2);
    int format = (int) properties.at(3);
    int block_size = (int) properties.at(4);
    int full_cavlac_frames = number_of_frames/block_size;
    int lastBlock = number_of_frames - (full_cavlac_frames*block_size);


    /*
     * Open an sndfile for writing, after having parameters from header of cavlac file
     */
    string new_file = file.substr(0, file.size()-11);
    SndfileHandle sndFileOut { new_file+"_new.wav", SFM_WRITE,format,channels,sample_rate };

    vector<int> frames_left(block_size,0);
    vector<int> frames_right(block_size,0);

    /*
     * Start reading frames
     */
    vector<uint32_t> header_frame;
    for( int i = 0; i < full_cavlac_frames; i++){
        /*
         * Decode Left Channel
         */
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_left.resize(block_size);
            frames_left = constant_frame(block_size, r);
        }
        else{


            frames_left.resize(block_size);
            frames_left = frames_decode(header_frame.at(1), r, n, header_frame.at(2), block_size);
        }

        /*
         * Decode Right Channel
         */
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_right.resize(block_size);
            frames_right = constant_frame(block_size, r);
        }else{

            frames_right.resize(block_size);
            frames_right = frames_decode(header_frame.at(1), r, n, header_frame.at(2), block_size);

        }
        write_samples_block(block_size, frames_left, frames_right, sndFileOut);
    }
    /*
     * Decode last block
     */
    if(lastBlock > 0){
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_left.resize(lastBlock);
            frames_left = constant_frame(lastBlock, r);
        }else{
            frames_left.resize(lastBlock);
            frames_left = frames_decode(header_frame.at(1), r, n, header_frame.at(2), lastBlock);

        }

        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_right.resize(lastBlock);
            frames_right = constant_frame(lastBlock, r);
        }else{
            frames_right.resize(lastBlock);
            frames_right = frames_decode(header_frame.at(1), r, n, header_frame.at(2), lastBlock);
        }
        write_samples_block(lastBlock, frames_left, frames_right, sndFileOut);
    }

    return 0;
}

int encodeMode(string file, int block_size, bool histogram)
{
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

    vector<int> left_channel(block_size);
    vector<int> differences(block_size);

    size_t nFrames;
    vector<short> samples(block_size * 2);

    Golomb golo;

    WRITEBits w (file+".cavlac");
    /* 
     * Header Format
     *
     * frames -> 32 bits -> 4 bytes
     * samplerate -> 32 bits -> 4 bytes
     * channels- > 16 bits -> 2 bytes
     * format -> 16 bits -> 2 bytes
     */

    w.preWrite(sndFileIn.frames(), 32);
    w.preWrite(sndFileIn.samplerate(), 32);
    w.preWrite(sndFileIn.channels(), 16);
    w.preWrite(sndFileIn.format(), 32);
    w.preWrite(block_size, 16);


    /*
     * Coding
     */
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
                differences.at((n-1)/2) = s;
            n++;
        }

        /*
         * Generate The residuals
         */
        pr.populate_v(left_channel);

        /*
         * Get predictor settings
         */
        vector<short> predictor_settings = pr.get_best_predictor_settings();
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

        uint32_t m = pow(2,best_k);


        golo.set_m( m );
        w.preWrite(write_header, 8);

        vector<int> residuals;


        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
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
        predictor_settings = pr.get_best_predictor_settings();


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

        m = pow(2,best_k);

        golo.set_m( m );

        w.preWrite(write_header, 8);

        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
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

        /* Debug Prints
         * for( uint32_t l = 0; l < nFrames; l++){
         *  printf("%3d -> %8x | %8x\n",l,  left_channel.at(l), left_channel.at(l) + differences.at(l));
         * }
         */
        count++;
    }
    w.flush();

    if(histogram){
        hist.full_dump();
        residuals_hist.full_dump();
    }
    return 0;
}

int encodeLossyMode(string file, int block_size, bool histogram, int shamt)
{
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

    vector<int> left_channel(block_size);
    vector<int> differences(block_size);

    size_t nFrames;
    vector<short> samples(block_size * 2);

    Golomb golo;

    WRITEBits w (file+".cavlac");
    /* 
     * Header Format
     *
     * frames -> 32 bits -> 4 bytes
     * samplerate -> 32 bits -> 4 bytes
     * channels- > 16 bits -> 2 bytes
     * format -> 16 bits -> 2 bytes
     */

    w.preWrite(sndFileIn.frames(), 32);
    w.preWrite(sndFileIn.samplerate(), 32);
    w.preWrite(sndFileIn.channels(), 16);
    w.preWrite(sndFileIn.format(), 32);
    w.preWrite(block_size, 16);


    /*
     * Coding
     */
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
                differences.at((n-1)/2) = s;
            n++;
        }

        /*
         * Generate The residuals
         */
        pr.gen_lossy_residuals(left_channel, shamt);
        //pr.populate_v(left_channel);

        /*
         * Get predictor settings
         */
        vector<short> predictor_settings = pr.get_best_predictor_settings();
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

        uint32_t m = pow(2,best_k);


        golo.set_m( m );
        w.preWrite(write_header, 8);

        vector<int> residuals;


        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
            w.preWrite(left_channel.at(0), 16);
        } else {
            residuals = pr.get_residuals(predictor_used);
            uint32_t i = 0;
            for (i = 0; i < predictor_used; ++i)
            {
                w.preWrite(left_channel.at(i) >> shamt, 16 - shamt);
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
        pr.gen_lossy_residuals(differences, shamt);
        //pr.populate_v(differences);
        predictor_settings = pr.get_best_predictor_settings();


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

        m = pow(2,best_k);

        golo.set_m( m );

        w.preWrite(write_header, 8);

        /*
         * If constant samples only need write one frame
         */
        if (constant == 1)
        {
            w.preWrite(differences.at(0), 16);
        } else {
            residuals = pr.get_residuals(predictor_used);
            uint32_t i = 0;
            for (i = 0; i < predictor_used; ++i)
            {
                w.preWrite(differences.at(i) >> shamt, 16 - shamt);
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

        /* Debug Prints
         * for( uint32_t l = 0; l < nFrames; l++){
         *  printf("%3d -> %8x | %8x\n",l,  left_channel.at(l), left_channel.at(l) + differences.at(l));
         * }
         */
        count++;
    }
    w.flush();

    if(histogram){
        hist.full_dump();
        residuals_hist.full_dump();
    }
    return 0;
}

int decodeLossyMode(string file, int shamt)
{

    // TODO: verify if the file is a Cavlac one
    Golomb n;

    READBits r (file);

    /*
     * Read header of file
     */
    vector<uint32_t> properties = r.read_header_cavlac();

    int number_of_frames = (int) properties.at(0);
    int sample_rate = (int) properties.at(1);
    int channels = (int) properties.at(2);
    int format = (int) properties.at(3);
    int block_size = (int) properties.at(4);
    int full_cavlac_frames = number_of_frames/block_size;
    int lastBlock = number_of_frames - (full_cavlac_frames*block_size);


    /*
     * Open an sndfile for writing, after having parameters from header of cavlac file
     */
    string new_file = file.substr(0, file.size()-11);
    SndfileHandle sndFileOut { new_file+"_new.wav", SFM_WRITE,format,channels,sample_rate };

    vector<int> frames_left(block_size,0);
    vector<int> frames_right(block_size,0);

    /*
     * Start reading frames
     */
    vector<uint32_t> header_frame;
    for( int i = 0; i < full_cavlac_frames; i++){
        /*
         * Decode Left Channel
         */
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_left.resize(block_size);
            frames_left = constant_frame(block_size, r);
        }
        else{


            frames_left.resize(block_size);
            frames_left = frames_lossy_decode(header_frame.at(1), r, n, header_frame.at(2), block_size, shamt);
        }

        /*
         * Decode Right Channel
         */
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_right.resize(block_size);
            frames_right = constant_frame(block_size, r);
        }else{

            frames_right.resize(block_size);
            frames_right = frames_lossy_decode(header_frame.at(1), r, n, header_frame.at(2), block_size, shamt);

        }
        write_samples_block(block_size, frames_left, frames_right, sndFileOut);
    }
    /*
     * Decode last block
     */
    if(lastBlock > 0){
        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_left.resize(lastBlock);
            frames_left = constant_frame(lastBlock, r);
        }else{
            frames_left.resize(lastBlock);
            frames_left = frames_lossy_decode(header_frame.at(1), r, n, header_frame.at(2), lastBlock, shamt);

        }

        header_frame = r.reade_header_frame();
        if(header_frame.at(0) == 1){
            frames_right.resize(lastBlock);
            frames_right = constant_frame(lastBlock, r);
        }else{
            frames_right.resize(lastBlock);
            frames_right = frames_lossy_decode(header_frame.at(1), r, n, header_frame.at(2), lastBlock, shamt);
        }
        write_samples_block(lastBlock, frames_left, frames_right, sndFileOut);
    }

    return 0;
}
