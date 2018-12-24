#include "cxxopts.hpp"
#include <fstream>
#include "Frame.hpp"
#include <math.h>

using namespace std;

double mean_squared_error( Frame * og_frame , Frame * cp_frame){

    double mse = 0.0;
    int difference;

    // MSD with luminance
    cv::Mat og_matrix = og_frame->get_y();
    cv::Mat cp_matrix = cp_frame->get_y();

    for(int i=0; i<og_matrix.rows; i++){
        for(int j=0; j<og_matrix.cols; j++){
            difference = og_matrix.at<uint8_t>(i,j) - cp_matrix.at<uint8_t>(i,j);
            mse = mse + difference*difference;
        }
    }

    mse = mse /(og_matrix.rows*og_matrix.cols);
    printf("%f\n", mse);

    return mse;
};

// ReUsed Code
void parse_header(  map<char,string> & header,
                    string header_line,
                    int delimiter(int) = ::isspace){

    string token;

    auto e=header_line.end();
    auto i=header_line.begin();

    while(i!=e){
        i=find_if_not(i,e, delimiter);
        if(i==e) break;
        auto j=find_if(i,e, delimiter);
        token = string(i,j);
        if(token.at(0) == 'W'){
            cout << "Width " << token.substr(1) << endl;
            header['W'] = token.substr(1);
        }
        else if(token.at(0) == 'H'){
            cout << "Height " << token.substr(1) <<endl;
            header['H'] = token.substr(1);
        }
        else if(token.at(0) == 'F'){
            cout << "Frame Rate " << token.substr(1) << endl;
            header['F'] = token.substr(1);
        }
        else if(token.at(0) == 'I'){
            cout << "Interlacing not parsed" << endl;
        }
        else if(token.at(0) == 'A'){
            cout << "Aspect Ratio not parsed" << endl;
        }
        else if(token.at(0) == 'C'){
            cout << "Colour Space " << token.substr(1) << endl;
            header['C'] = token.substr(1);
        }
        i=j;
    }
    if (header.find('C') == header.end()){
        cout << "Colour Space " << token.substr(1) << endl;
        header['C'] = "420";
    }
}


int main(int argc, char** argv)
{
    string original_file;
    string compressed_file;

   /* parse arguments */
    try{
        cxxopts::Options options("Video Compare", "Video Comparer With Source");

        options.add_options()
            ("h,help", "Print help")
            ("o,original", "Original File", cxxopts::value<std::string>())
            ("c,compressed", "Compressed File", cxxopts::value<std::string>())
            ;

        options.parse_positional({"o", "c"});

        auto result = options.parse(argc, argv);

        if (result.count("help")){
            cout << options.help() << endl;
            exit(0);
        }

        if (result.count("o") == 0 || result.count("c") == 0){
            cout << options.help() << endl;
            exit(1);
        }

        original_file = result["o"].as<string>();
        compressed_file = result["c"].as<string>();


    }catch(const cxxopts::OptionException& e){
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    ifstream og_file (original_file);
    ifstream cp_file (compressed_file);

    Frame * og_f;
    Frame * cp_f;

    string line, line2;
    int cols, rows,frame_counter = 0,color_space =0;
    vector<unsigned char> og_imgData;
    vector<unsigned char> cp_imgData;
    map<char, string> header;

    getline(og_file, line);
    parse_header(header, line);
    cols = stoi(header['W']);
    rows = stoi(header['H']);
    color_space = stoi(header['C']);

    getline(cp_file, line2);
    if( line != line2) {
        printf("The Files Don't Share the Same Header");
        exit(1);
    }

    switch(color_space){
        case 444:{
            og_f = new Frame444 (rows, cols);
            cp_f = new Frame444 (rows, cols);
            og_imgData.resize(cols * rows * 3);
            cp_imgData.resize(cols * rows * 3);
            break;
        }
        case 422:{
            og_f = new Frame422 (rows, cols);
            cp_f = new Frame422 (rows, cols);
            og_imgData.resize(cols * rows * 2);
            cp_imgData.resize(cols * rows * 2);
            break;
        }
        case 420:{
            og_f = new Frame420 (rows, cols);
            cp_f = new Frame420 (rows, cols);
            og_imgData.resize(cols * rows * 3/2);
            cp_imgData.resize(cols * rows * 3/2);
            break;
        }
        default:
            exit(1);
    }

    double m = 0.0;
    while(1){
        // Read Original Frame
        getline (og_file,line); // Skipping word FRAME
        og_file.read((char *) og_imgData.data(), og_imgData.size());
        og_f->set_frame_data(og_imgData.data());

        // Read Reconstructed Frame
        getline (cp_file,line2); // Skipping word FRAME
        cp_file.read((char *) cp_imgData.data(), cp_imgData.size());
        cp_f->set_frame_data(cp_imgData.data());

        if(og_file.gcount() == 0){
            if(cp_file.gcount() == 0){
                break;
            }
            printf("Files have a different frame number");
            exit(1);
        }
        //printf("Analysing frame %d\n", frame_counter);

        m += mean_squared_error(og_f, cp_f);


        frame_counter +=1;
    }

    m = m/frame_counter;
    printf("Average MSE: %f\n", m);
    double psnr = 10 * log10((255*255)/(m*m));
    printf("PSNR : %f\n", psnr);

    /* Rest of the Code */
    return 0;
}
