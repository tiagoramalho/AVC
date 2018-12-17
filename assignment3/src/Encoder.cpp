#include "Encoder.hpp"


#include <cstdio>
#include <numeric>
#include <math.h>
#include <cmath>
#include "Frame.hpp"

using namespace cv;

Encoder::Encoder(const string & in_file, const string & out_file, int p, int peri, int blocksize, int searcharea):
    infile(in_file.c_str()),w(out_file.c_str()),profile(p), periodicity(peri), block_size(blocksize), search_area(searcharea){} 

int WriteFile(std::string fname, std::map<int,int> *m) {
    int count = 0;
    if (m->empty())
        return 0;

    FILE *fp = fopen(fname.c_str(), "w");
    if (!fp)
        return -errno;

    for(std::map<int, int>::iterator it = m->begin(); it != m->end(); it++) {
        //fprintf(fp, "%d %d\n", it->first, it->second);
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
    printf("----width %d\n", width);
    int height= matrix->rows;
    printf("----height %d\n", height);
    uint8_t last_real = matrix->at<uint8_t>(0,0);

    for( y = 0; y < height; y++){
        if( y == 0 ){
            // First row
            for( x = 1; x < width; x++){
                residual = get_residual_uniform(last_real, matrix->at<uint8_t>(y,x));
                // printf("%02x -> %02x, %d\n",last_real, matrix->at<uint8_t>(y,x), residual);
                residuals->push_back(residual);
                //to_calculate_k += std::abs(residual);
                if(residual >= 0)
                  to_calculate_k += residual * 2;
                else
                  to_calculate_k += -2*residual-1;

                last_real = matrix->at<uint8_t>(y,x);
            }
        }else{
            // Other rows
            residual = get_residual_uniform(matrix->at<uint8_t>(y-1,0), matrix->at<uint8_t>(y,0));
            residuals->push_back(residual);
            //to_calculate_k += std::abs(residual);
            if(residual >= 0)
                to_calculate_k += residual * 2;
            else
                to_calculate_k += -2*residual-1;

            for( x = 1; x < width; x++){
                residual = get_residual_LOCO(
                        matrix->at<uint8_t>(y,x-1),
                        matrix->at<uint8_t>(y-1,x),
                        matrix->at<uint8_t>(y-1,x-1),
                        matrix->at<uint8_t>(y,x));

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

void Encoder::get_best_fit( cv::Mat macroblock, cv::Mat searchingArea, vector<int> * to_encode){
    Mat img_display;
    searchingArea.copyTo( img_display );

    cv::Mat result;

    int result_cols =  searchingArea.cols - macroblock.cols + 1;
    int result_rows = searchingArea.rows - macroblock.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );


    cv::matchTemplate( searchingArea, macroblock, result, TM_SQDIFF );

    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    matchLoc = minLoc;

    //macroblock.convertTo(macroblock, CV_8U);
    //searchingArea.convertTo(searchingArea, CV_8U);

    rectangle(img_display, matchLoc, Point( matchLoc.x + macroblock.cols , matchLoc.y + macroblock.rows ), Scalar::all(0), 1, 8, 0 ); //rectangle( result, matchLoc, Point( matchLoc.x + searchingArea.cols , matchLoc.y + searchingArea.rows ), Scalar::all(0), 2, 8, 0 );
    imshow( "Pequeno", macroblock );
    imshow( "Area to search", img_display );
    imshow( "Grande", result );

    cv::waitKey(10);
};

void Encoder::encode_and_write_frame_inter(Frame * frame, Frame * previous_frame,int f_counter, Golomb * g){
    int y_curr_frame = 0;
    int x_curr_frame = 0;
    int x_searching_area_top_left = 0;
    int x_searching_area_bot_right = 0;
    int y_searching_area_top_left = 0;
    int y_searching_area_bot_right = 0;
    cv::Mat macroblock;
    cv::Mat searching_area;

    cv::Mat y_previous = previous_frame->get_y();
    cv::Mat y_frame = frame->get_y();

    imshow( "Previous Frame", y_previous );
    cv::waitKey(100);
    int height = y_frame.rows;
    int width = y_frame.cols;

    vector<int> to_encode = {};

    for( y_curr_frame = 0; y_curr_frame < height; y_curr_frame +=this->block_size ){

        for( x_curr_frame = 0; x_curr_frame < width; x_curr_frame +=this->block_size ){

            //printf("x %d y %d\n", x_curr_frame, y_curr_frame);

            macroblock = y_frame(cv::Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));

            x_searching_area_top_left = std::max(0, x_curr_frame - this->search_area);
            y_searching_area_top_left = std::max(0, y_curr_frame - this->search_area);

            x_searching_area_bot_right = std::min(width,
                    x_curr_frame + this->block_size + this->search_area);
            y_searching_area_bot_right = std::min(height,
                    y_curr_frame + this->block_size + this->search_area);

            int area = ( y_searching_area_bot_right - y_searching_area_top_left ) * (x_searching_area_bot_right - x_searching_area_top_left);
            //printf("Search Area: %d\n", area);

            //printf("%d, %d | %d, %d\n", x_searching_area_top_left, y_searching_area_top_left,x_searching_area_bot_right,y_searching_area_bot_right);

            searching_area = y_previous(cv::Rect(cv::Point(x_searching_area_top_left, y_searching_area_top_left),
                        cv::Point(x_searching_area_bot_right,y_searching_area_bot_right)));

            get_best_fit( macroblock, searching_area, & to_encode );

            //codificar_macro_bloco( vector, residuais , golomb)

        }
    }

    printf("Done %d\n", f_counter);
};

/* TODO melhorar isto; Branco*/

void Encoder::encode_and_write_frame_intra(Frame * frame, int f_counter, Golomb * g){

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

    int m = pow(2,k);
    g->set_m(m);
    for(unsigned int i = 0; i < residuals.size(); i++){
        // printf("%d -> %02x\n",i, residuals.at(i));
        g->encode_and_write(residuals.at(i), w);
    }


    /* encode the U Matrix */
    residuals.clear();
    matrix = frame->get_u();
    seed = matrix.at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    k = get_best_k(&residuals, f_counter, to_calculate_k);

    this->w.writeFrameHeader(k, seed);
    m = pow(2,k);
    g->set_m(m);
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
    m = pow(2,k);
    g->set_m(m);
    for(unsigned int i = 0; i < residuals.size(); i++){
        g->encode_and_write(residuals.at(i), w);
        //printf("%d, %d\n", i, residuals.at(i));
    }

    printf("Done %d\n", f_counter);
};

void Encoder::encode_and_write(){
    Golomb g;

    string line;
    int cols, rows,frame_counter =0;
    vector<unsigned char> imgData;
    vector<unsigned char> previous_imgData;
    Frame * f;
    Frame * previous_frame;

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
            previous_frame = new Frame444 (rows, cols); 
            imgData.resize(cols * rows * 3);
            break;
        }
        case 422:{
            f = new Frame422 (rows, cols);
            previous_frame = new Frame422 (rows, cols); 
            imgData.resize(cols * rows * 2);
            break;
        }
        case 420:{
            f = new Frame420 (rows, cols);
            previous_frame = new Frame420 (rows, cols); 
            imgData.resize(cols * rows * 3/2);
            break;
        }
        default:
            exit(1);
    }

    //f->print_type();

    if ( this->profile == 0){
      while(1){
          getline (this->infile,line); // Skipping word FRAME
          this->infile.read((char *) imgData.data(), imgData.size());
          f->set_frame_data(imgData.data());
          if(this->infile.gcount() == 0){
              break;
          }
          encode_and_write_frame_intra(f, frame_counter, & g);
          frame_counter +=1;
      }
    }else{
      while(1){
          f = new Frame444 (rows, cols);
          getline (this->infile,line); // Skipping word FRAME
          this->infile.read((char *) imgData.data(), imgData.size());
          f->set_frame_data(imgData.data());

          if(this->infile.gcount() == 0){
              break;
          }
          Mat y = f->get_y();
          //printf("rows: %d, cols: %d\n", y.rows, y.cols);

          if( frame_counter % this->periodicity == 0){
              encode_and_write_frame_intra(f, frame_counter, & g);
              //encode_and_write_frame_inter(f, previous_frame, frame_counter, & g);
              //printf("=============================================================\n\n\n\n\nPILAAAAAAAAAAAAAAAAA");
              //previous_imgData = imgData;
              //previous_frame->set_frame_data(previous_imgData.data());
              previous_frame = f ;
          }else{
              encode_and_write_frame_inter(f, previous_frame, frame_counter, & g);
          }

          frame_counter += 1 ;
          //previous_frame = f ;
      }
    }

    this->w.flush();

    delete f;
};
