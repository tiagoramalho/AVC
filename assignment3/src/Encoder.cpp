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
        count++;
    }

    fclose(fp);
    return count;
}

int Encoder::get_best_k( int size, int tck ){
    int k = 0;
    double tck_a = tck/size;
    if(tck_a < 1){
        tck_a = 1;
    }
    k = std::ceil(std::log2(tck_a));
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
                residual = get_residual_uniform(last_real, matrix->at<uint8_t>(y,x));
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

void Encoder::get_best_fit( cv::Mat macroblock, cv::Mat searchingArea, vector<Point> & to_encode){
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

    to_encode.push_back(matchLoc);
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

    vector<Point> to_encode_vector = {};
    int to_calculate_k = 0;
    int index = 0;
    for( y_curr_frame = 0; y_curr_frame < height; y_curr_frame +=this->block_size ){

        for( x_curr_frame = 0; x_curr_frame < width; x_curr_frame +=this->block_size ){


            macroblock = y_frame(cv::Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));

            x_searching_area_top_left = std::max(0, x_curr_frame - this->search_area);
            y_searching_area_top_left = std::max(0, y_curr_frame - this->search_area);

            x_searching_area_bot_right = std::min(width,
                    x_curr_frame + this->block_size + this->search_area);
            y_searching_area_bot_right = std::min(height,
                    y_curr_frame + this->block_size + this->search_area);


            searching_area = y_previous(cv::Rect(cv::Point(x_searching_area_top_left, y_searching_area_top_left),
                        cv::Point(x_searching_area_bot_right,y_searching_area_bot_right)));

            get_best_fit( macroblock, searching_area, to_encode_vector );

            //save list of point = motion vectors
            to_encode_vector.at(index)+=Point(x_searching_area_top_left, y_searching_area_top_left);
            to_encode_vector.at(index)-=Point(x_curr_frame, y_curr_frame);

            Point tmp = to_encode_vector.at(index);

            if(tmp.x >= 0)
                to_calculate_k += tmp.x * 2;
            else
                to_calculate_k += -2* tmp.x -1;
if(tmp.y >= 0)
                to_calculate_k += tmp.y * 2;
            else
                to_calculate_k += -2* tmp.y -1;

            index+=1;
        }
    }
    // escrever vetores na bitstream

    int k = get_best_k(to_encode_vector.size()*2, to_calculate_k);
    int m = pow(2,k);
    g->set_m(m);
    this->w.write_header_type(1);
    this->w.write_header_k(k);
    for(unsigned int i = 0; i < to_encode_vector.size(); i++){
        g->encode_and_write(to_encode_vector.at(i).x, w);
        g->encode_and_write(to_encode_vector.at(i).y, w);
    }

    cv::Mat u_previous = previous_frame->get_u();
    cv::Mat u_frame = frame->get_u();

    // code v
    cv::Mat v_previous = previous_frame->get_v();
    cv::Mat v_frame = frame->get_v();
    if(frame->print_type() == 444){
        // code y
        inter_encode_write_4(y_frame, g, to_encode_vector, y_previous);

        // code u
        inter_encode_write_4(u_frame, g, to_encode_vector, u_previous);
    
        // code v
        inter_encode_write_4(v_frame, g, to_encode_vector, v_previous);

    }else if(frame->print_type() == 422){
        // code y
        inter_encode_write_4(y_frame, g, to_encode_vector, y_previous);

        // code u
        inter_encode_write_2(u_frame, g, to_encode_vector, u_previous);
    
        // code v
        inter_encode_write_2(v_frame, g, to_encode_vector, v_previous);
    
    }else if(frame->print_type() == 420){
        // code y
        inter_encode_write_4(y_frame, g, to_encode_vector, y_previous);
        
        // code u
        inter_encode_write_0(u_frame, g, to_encode_vector, u_previous);
    
        // code v
        inter_encode_write_0(v_frame, g, to_encode_vector, v_previous);
        //exit(1);
    
    }


    printf("Done %d\n", f_counter);
};
void Encoder::inter_encode_write_4(Mat frame, Golomb * g, vector<Point> to_encode_vector, Mat previous){

    vector<int> to_encode_residuals = {};
    int to_calculate_k = 0;
    int y_curr_frame = 0;
    int x_curr_frame = 0;

    cv::Mat macroblock;
    cv::Mat match_area;
    int index = 0;
    for( y_curr_frame = 0; y_curr_frame < frame.rows; y_curr_frame +=this->block_size ){

        for( x_curr_frame = 0; x_curr_frame < frame.cols; x_curr_frame +=this->block_size ){


            macroblock = frame(cv::Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));

            Point tmp = to_encode_vector.at(index);
            match_area = previous(cv::Rect(x_curr_frame + tmp.x, y_curr_frame + tmp.y,this->block_size, this->block_size));

            macroblock.convertTo(macroblock, CV_32S);
            match_area.convertTo(match_area, CV_32S);

            cv::Mat residuals(macroblock.cols, macroblock.rows, CV_32S);

            //frame anterior - atual 
            subtract(
                match_area,
                macroblock,
                residuals
            );

            for (int x = 0; x < macroblock.cols; ++x)
            {
                for (int y = 0; y < macroblock.rows; ++y)
                {
                    to_encode_residuals.push_back(residuals.at<int32_t>(y,x));
                    if(residuals.at<int32_t>(y,x) >= 0)
                        to_calculate_k += residuals.at<int32_t>(y,x) * 2;
                    else
                        to_calculate_k += -2* residuals.at<int32_t>(y,x) -1;
                }
            }
            index+=1;
        }
    }

    int k = get_best_k(to_encode_residuals.size(), to_calculate_k);
    int m = pow(2,k);
    g->set_m(m);
    this->w.write_header_k(k);
    for(unsigned int i = 0; i < to_encode_residuals.size(); i++){
        g->encode_and_write(to_encode_residuals.at(i), w);
    }

}

void Encoder::inter_encode_write_2(Mat frame, Golomb * g, vector<Point> to_encode_vector, Mat previous){

    vector<int> to_encode_residuals = {};
    int to_calculate_k = 0;
    int y_curr_frame = 0;
    int x_curr_frame = 0;

    cv::Mat macroblock;
    cv::Mat match_area;
    int adjusted_size = this->block_size / 2;
    int index = 0;
    for( y_curr_frame = 0; y_curr_frame < frame.rows; y_curr_frame +=this->block_size ){

        for( x_curr_frame = 0; x_curr_frame < frame.cols; x_curr_frame +=adjusted_size ){


            macroblock = frame(cv::Rect(x_curr_frame, y_curr_frame, adjusted_size, this->block_size));

            Point tmp = to_encode_vector.at(index);

            match_area = previous(cv::Rect(x_curr_frame + tmp.x/2, y_curr_frame + tmp.y,adjusted_size, this->block_size));
            macroblock.convertTo(macroblock, CV_32S);
            match_area.convertTo(match_area, CV_32S);

            cv::Mat residuals(macroblock.cols, macroblock.rows, CV_32S);

            //frame anterior - atual 
            subtract(
                match_area,
                macroblock,
                residuals
            );

            for (int x = 0; x < macroblock.cols; ++x)
            {
                for (int y = 0; y < macroblock.rows; ++y)
                {
                    to_encode_residuals.push_back(residuals.at<int32_t>(y,x));
                    if(residuals.at<int32_t>(y,x) >= 0)
                        to_calculate_k += residuals.at<int32_t>(y,x) * 2;
                    else
                        to_calculate_k += -2* residuals.at<int32_t>(y,x) -1;
                }
            }
            index +=1;
        }
    }

    int k = get_best_k(to_encode_residuals.size(), to_calculate_k);
    int m = pow(2,k);
    g->set_m(m);
    this->w.write_header_k(k);
    for(unsigned int i = 0; i < to_encode_residuals.size(); i++){
        g->encode_and_write(to_encode_residuals.at(i), w);
    }

}
void Encoder::inter_encode_write_0(Mat frame, Golomb * g, vector<Point> to_encode_vector, Mat previous){

    vector<int> to_encode_residuals = {};
    int to_calculate_k = 0;
    int y_curr_frame = 0;
    int x_curr_frame = 0;

    cv::Mat macroblock;
    cv::Mat match_area;
    int adjusted_size = this->block_size / 2;
    int index = 0;
    for( y_curr_frame = 0; y_curr_frame < frame.rows; y_curr_frame +=adjusted_size ){

        for( x_curr_frame = 0; x_curr_frame < frame.cols; x_curr_frame +=adjusted_size ){


            macroblock = frame(cv::Rect(x_curr_frame, y_curr_frame, adjusted_size, adjusted_size));

            Point tmp = to_encode_vector.at(index);
            match_area = previous(cv::Rect(x_curr_frame + tmp.x/2, y_curr_frame + tmp.y/2,adjusted_size, adjusted_size));

            macroblock.convertTo(macroblock, CV_32S);
            match_area.convertTo(match_area, CV_32S);

            cv::Mat residuals(macroblock.cols, macroblock.rows, CV_32S);

            //frame anterior - atual 
            subtract(
                match_area,
                macroblock,
                residuals
            );
            
            /*if(y_curr_frame == x_curr_frame && y_curr_frame == 0){
                cout << "macroblock" << endl;
                cout << macroblock << endl;
            
            }*/

            for (int x = 0; x < macroblock.cols; ++x)
            {
                for (int y = 0; y < macroblock.rows; ++y)
                {
                    to_encode_residuals.push_back(residuals.at<int32_t>(y,x));
                    if(residuals.at<int32_t>(y,x) >= 0)
                        to_calculate_k += residuals.at<int32_t>(y,x) * 2;
                    else
                        to_calculate_k += -2* residuals.at<int32_t>(y,x) -1;
                }
            }

            index+=1;
        }
    }

    int k = get_best_k(to_encode_residuals.size(), to_calculate_k);
    int m = pow(2,k);
    g->set_m(m);
    this->w.write_header_k(k);
    for(unsigned int i = 0; i < to_encode_residuals.size(); i++){
        g->encode_and_write(to_encode_residuals.at(i), w);
    }

}
/* TODO melhorar isto; Branco*/

void Encoder::encode_and_write_frame_intra(Frame * frame, int f_counter, Golomb * g){

    vector<int> residuals = {};
    cv::Mat matrix = frame->get_y();
    int to_calculate_k = 0;

    /* encode the Luminance Matrix */
    uint8_t seed = frame->get_y().at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    int k = get_best_k(residuals.size(), to_calculate_k);

    /* Write Frame Header */
    this->w.write_header_type(0);
    this->w.write_header_k(k);
    this->w.write_header_seed(seed);
    /* Encode Residuals */

    int m = pow(2,k);
    g->set_m(m);
    for(unsigned int i = 0; i < residuals.size(); i++){
        g->encode_and_write(residuals.at(i), w);
    }


    /* encode the U Matrix */
    residuals.clear();
    matrix = frame->get_u();
    seed = matrix.at<uint8_t>(0,0);
    to_calculate_k = get_residuals_from_matrix( & matrix , & residuals);
    k = get_best_k(residuals.size(), to_calculate_k);

    this->w.write_header_k(k);
    this->w.write_header_seed(seed);
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
    k = get_best_k(residuals.size(), to_calculate_k);

    this->w.write_header_k(k);
    this->w.write_header_seed(seed);
    m = pow(2,k);
    g->set_m(m);
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
    vector<unsigned char> previous_imgData;
    Frame * f;
    Frame * previous_frame;

    getline(this->infile, line);

    map<char, string> header;

    parse_header(header, line);

    cols = stoi(header['W']);
    rows = stoi(header['H']);

    this->w.writeHeader(cols,rows,stoi(header['C']), this->block_size);
    this->color_space = stoi(header['C']);

    switch(this->color_space){
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
          switch(this->color_space){
              case 444:{
                           f = new Frame444 (rows, cols);
                           break;
                       }
              case 422:{
                           f = new Frame422 (rows, cols);
                           break;
                       }
              case 420:{
                           f = new Frame420 (rows, cols);
                           break;
                       }
          }
          getline (this->infile,line); // Skipping word FRAME
          this->infile.read((char *) imgData.data(), imgData.size());
          f->set_frame_data(imgData.data());

          if(this->infile.gcount() == 0){
              break;
          }

          if( frame_counter % this->periodicity == 0){
              encode_and_write_frame_intra(f, frame_counter, & g);
          }else{
              encode_and_write_frame_inter(f, previous_frame, frame_counter, & g);
          }
          previous_imgData = imgData;
          previous_frame->set_frame_data(previous_imgData.data());

          frame_counter += 1 ;
      }
    }

    this->w.flush();

    delete f;
};
