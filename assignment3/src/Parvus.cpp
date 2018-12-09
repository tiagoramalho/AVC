#include "cxxopts.hpp"

#include "fstreamBits.h"
#include "Encoder.hpp"
#include "Decoder.hpp"
#include "Golomb.hpp"

using namespace std;
int main(int argc, char** argv)
{
    bool mode_decode = false;
    /* store the filename */
    string file;

    /* parse arguments */
    try{
        cxxopts::Options options("Parvus", "Video Compressor");

        options.add_options()
            ("h,help", "Print help")
            ("d,decode", "Decode Mode")
            ("f,file", "File (obrigatory)", cxxopts::value<std::string>())
            ;

        auto result = options.parse(argc, argv);

        if (result.count("help")){
            cout << options.help() << endl;
            exit(0);
        }

        if (result.count("f") != 1){
            cout << endl << "You always need to specify a file" << endl << endl;
            cout << options.help() << endl;
            exit(1);
        }else{
            file = result["f"].as<string>();
        }

        if (result.count("d")){
            mode_decode = true;
        }

    }catch(const cxxopts::OptionException& e){
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    /* Rest of the Code */

    if(!mode_decode){
        string out_file = file + ".pv";
        Encoder enc (file, out_file);
        enc.encode_and_write();
    }else{
        /* TODO: see again out_file and such */
        string out_file = file + ".y4m";
        Decoder dec ( file, out_file );
        dec.read_and_decode();
    }

    return 0;
}
