#include "Encoder.hpp"


#include <cstdio>
#include <numeric>
#include <math.h>
#include <cmath>
#include "Frame.hpp"

Encoder::Encoder(const string & in_file, const string & out_file):
    infile(in_file.c_str()),w(out_file.c_str()){}

int WriteFile(std::string fname, std::map<int,int> *m) {
    int count = 0;
    if (m->empty())
        return 0;

    FILE *fp = fopen(fname.c_str(), "w");
    if (!fp)
        return -errno;

    for(std::map<int, int>::iterator it = m->begin(); it != m->end(); it++) {
        fprintf(fp, "%d %d\n", it->first, it->second);
        count++;
    }

    fclose(fp);
    return count;
}

int Encoder::get_best_k( vector<int> * residuals , int frame, int tck){

    /* This Function can be reworked */

    int k = 0;
    //double average = std::accumulate( residuals->begin(), residuals->end(), 0.0)/residuals->size();

    //double E=0;
    //double inverse = 1.0 / static_cast<double>(residuals->size());

    //std::map<int, int> freq;

    //for(unsigned int i=0;i<residuals->size();i++){
    //  freq[residuals->at(i)]++;
    //  E+=pow(static_cast<double>(residuals->at(i)) - average, 2);
    //}

    //printf("%d", frame);
    //string fname = "./histograms/rawdata/hist"+ to_string(frame);
    //printf("%s\n", fname.c_str());
    //WriteFile( fname, &freq);

    double tck_a = tck/residuals->size();
    k = std::ceil(std::log2(tck_a));
    //printf("K used-> %d \n", k);
    //printf("Average -> %f | Deviation-> %f | K-> %f\n", average, sqrt(inverse*E), k );
    return k;
}

int Encoder::get_residual_uniform( uint8_t previous_pixel_value, uint8_t real_pixel_value ){

    int residual = real_pixel_value - previous_pixel_value;
    return residual;

}

int Encoder::get_residual_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, uint8_t real_pixel_value){

    uint8_t pixel_prevision, maxAB, minAB;

    maxAB = std::max(pixel_A, pixel_B);
    minAB = std::min(pixel_A, pixel_B);

    if( pixel_C >= maxAB){
        pixel_prevision = minAB;
    }else if ( pixel_C <= minAB ){
        pixel_prevision = maxAB;
    }else{
        pixel_prevision = pixel_A + pixel_B - pixel_C;
    }

    return get_residual_uniform( pixel_prevision, real_pixel_value);
}


void Encoder::parse_header(  map<char,string> & header,
                    string header_line,
                    int delimiter(int)){

    string token;

    auto e=header_line.end();
    auto i=header_line.begin();

    while(i!=e){
        i=find_if_not(i,e, delimiter);
        if(i==e) break;
        auto j=find_if(i,e, delimiter);
        token = string(i,j);
        cout << token << endl;
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

int Encoder::get_residuals_from_matrix(cv::Mat * matrix, vector<int> * residuals){


    int x = 0, y=0,residual = 0,to_calculate_k = 0;
    int width = matrix->cols;
    int height= matrix->rows;
    uint8_t last_real = matrix->at<uint8_t>(0,0);

    for( y = 0; y < height; y++){
        if( y == 0 ){
            // First row
            for( x = 1; x < width; x++){
                residual = get_residual_uniform(last_real, matrix->at<uint8_t>(x,y));
                printf("%d -> %02x, %d\n",last_real, matrix->at<uint8_t>(x,y), residual);
                residuals->push_back(residual);
                //to_calculate_k += std::abs(residual);
                if(residual >= 0)
                  to_calculate_k += residual * 2;
                else
                  to_calculate_k += -2*residual-1;

                last_real = matrix->at<uint8_t>(x,y);
            }
        }else{
            // Other rows
            residual = get_residual_uniform(matrix->at<uint8_t>(0,y-1), matrix->at<uint8_t>(0,y));
            residuals->push_back(residual);
            //to_calculate_k += std::abs(residual);
            if(residual >= 0)
                to_calculate_k += residual * 2;
            else
                to_calculate_k += -2*residual-1;

            for( x = 1; x < width; x++){
                residual = get_residual_LOCO(
                        matrix->at<uint8_t>(x-1,y),
                        matrix->at<uint8_t>(x,y-1),
                        matrix->at<uint8_t>(x-1,y-1),
                        matrix->at<uint8_t>(x,y));

                residuals->push_back(residual);
                //to_calculate_k += std::abs(residual);
                if(residual >= 0)
                    to_calculate_k += residual * 2;
                else
                    to_calculate_k += -2*residual-1;

            }
        }
    }

    return to_calculate_k;
}

/* TODO melhorar isto; Branco*/

void Encoder::encode_and_write_frame(Frame * frame, int f_counter, Golomb * g){

    vector<int> residuals = {};
    cv::Mat matrix = frame->get_y();
    int to_calculate_k = 0;

    /* encode the Luminance Matrix */
    uint8_t seed = frame->get_y().at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    int k = get_best_k(&residuals, f_counter, to_calculate_k);

    /* Write Frame Header */
    this->w.writeFrameHeader(k, seed);
    /* Encode Residuals */

    g->set_m(k);
    for(unsigned int i = 0; i < residuals.size(); i++){
        printf("%d -> %02x\n",i, residuals.at(i));
        // g->encode_and_write(residuals.at(i), w);
    }
    exit(1);


    /* encode the U Matrix */
    residuals.clear();
    matrix = frame->get_u();
    seed = matrix.at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    k = get_best_k(&residuals, f_counter, to_calculate_k);

    this->w.writeFrameHeader(k, seed);
    g->set_m(k);
    for(unsigned int i = 0; i < residuals.size(); i++){
        g->encode_and_write(residuals.at(i), w);
    }

    /* encode the V Matrix */
    residuals.clear();
    matrix = frame->get_v();
    seed = matrix.at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    k = get_best_k(&residuals, f_counter, to_calculate_k);

    this->w.writeFrameHeader(k, seed);
    g->set_m(k);
    for(unsigned int i = 0; i < residuals.size(); i++){
        g->encode_and_write(residuals.at(i), w);
    }

    printf("Done %d\n", f_counter);
};

void Encoder::encode_and_write(){
    Golomb g;

    string line;
    int cols, rows,frame_counter =0;
    vector<unsigned char> imgData;
    Frame * f;

    getline(this->infile, line);

    map<char, string> header;

    parse_header(header, line);

    cols = stoi(header['W']);
    rows = stoi(header['H']);


    // printf("Writing Header To Compressed File...");
    this->w.writeHeader(cols,rows,stoi(header['C']));

    switch(stoi(header['C'])){
        case 444:{
            f = new Frame444 (rows, cols);
            imgData.resize(cols * rows * 3);
            break;
        }
        case 422:{
            f = new Frame422 (rows, cols);
            imgData.resize(cols * rows * 2);
            break;
        }
        case 420:{
            f = new Frame420 (rows, cols);
            imgData.resize(cols * rows * 3/2);
            break;
        }
        default:
            exit(1);
    }

    f->print_type();

    while(1){
        getline (this->infile,line); // Skipping word FRAME
        this->infile.read((char *) imgData.data(), imgData.size());
        f->set_frame_data(imgData.data());
        if(this->infile.gcount() == 0){
            break;
        }
        encode_and_write_frame(f, frame_counter, & g);
        frame_counter +=1;
    }

    this->w.flush();

    delete f;
};


