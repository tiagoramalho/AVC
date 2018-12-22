#include "cxxopts.hpp"

#include "fstreamBits.h"
#include "Encoder.hpp"
#include "Decoder.hpp"
#include "Golomb.hpp"

using namespace std;
int main(int argc, char** argv)
{
    bool mode_decode = false, lossy=false;


    /* Profiles:
     * 0 - All Intra
     * 1 - Intra and Inter
     */
    int profile = 0;
    int block_size = 0;
    int periodicity = 0;
    int search_area = 0;
    int shamnt_y = 0;
    int shamnt_u = 0;
    int shamnt_v = 0;

    /* store the filename */
    string file;

    /* parse arguments */
    try{
        cxxopts::Options options("Parvus", "Video Compressor");

        options.add_options()
            ("h,help", "Print help")
            ("d,decode", "Decode Mode")
            ("l,lossy_dct", "Lossy Mode")
            ("m,mode", "Mode", cxxopts::value<int>())
            ("p,periodicity", "Periodicity", cxxopts::value<int>())
            ("b,blocksize", "Block Size", cxxopts::value<int>())
            ("s,searcharea", "Search Area Size", cxxopts::value<int>())
            ("y,shamntY", "shamnt Y", cxxopts::value<int>())
            ("u,shamntU", "shamnt U", cxxopts::value<int>())
            ("v,shamntV", "shamnt V", cxxopts::value<int>())
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

        if (result.count("m") != 1){
            cout << endl << "You always need to specify a mode" << endl << endl;
            cout << options.help() << endl;
            exit(1);
        }

        profile = result["m"].as<int>();


        if (result.count("d")){
            mode_decode = true;
        }

        if (result.count("l")){
            lossy = true;
        }

        if (profile == 1 && mode_decode==false)
        {
            periodicity = result["p"].as<int>();
            search_area = result["s"].as<int>();
            block_size = result["b"].as<int>();
            if (result.count("y") != 0){
                shamnt_y = result["y"].as<int>();
                if (shamnt_y >= 8){
                    cout << endl << "You need to specify a -y lesser than 8" << endl << endl;
                    cout << options.help() << endl;
                    exit(1);
                }
            }
            if (result.count("u") != 0){
                shamnt_u = result["u"].as<int>();
                if (shamnt_u >= 8){
                    cout << endl << "You need to specify a -u lesser than 8" << endl << endl;
                    cout << options.help() << endl;
                    exit(1);
                }
            }
            if (result.count("v") != 0){
                shamnt_v = result["v"].as<int>();
                if (shamnt_v >= 8){
                    cout << endl << "You need to specify a -v lesser than 8" << endl << endl;
                    cout << options.help() << endl;
                    exit(1);
                }
            }
        }else{
            periodicity = 0;
            search_area = 0;
            block_size = 0;
        }

    }catch(const cxxopts::OptionException& e){
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
    /* Rest of the Code */
    if (!mode_decode)
    {
        string out_file = file + ".pv";


        if(lossy){
            Encoder enc (file, out_file, profile, periodicity, block_size, search_area);
            enc.encode_and_write_lossy();
        } else {
            Encoder enc (file, out_file, profile, periodicity, block_size, search_area, shamnt_y, shamnt_u, shamnt_v);
            enc.encode_and_write();
        }

    }else{
        /* TODO: see again out_file and such */
        string out_file = file + ".y4m";
        Decoder dec ( file, out_file );

        if(lossy){
            dec.read_and_decode_lossy();
        } else {
            dec.read_and_decode();
        }

    }

    return 0;
}
