#include "Encoder.hpp"

#include <cstdio>
#include <numeric>
#include <math.h>
#include <cmath>
#include "Frame.hpp"

using namespace cv;
using namespace std;


double dataLuminance[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

Mat mat_luminance = Mat(8, 8, CV_64FC1, &dataLuminance);

double dataChrominance[8][8] = {
    {17, 18, 24, 27, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};

Mat mat_chrominance = Mat(8, 8, CV_64FC1, &dataChrominance);

bool buble (std::vector<int> i, std::vector<int> j) { return  (i[0] == j[0]) ? (i[1] < j[1]) : (i[0] < j[0]); }


// void zigZagMatrix(int arr[][C], int n, int m) 
void zigZagMatrix(Mat & arr, vector<int> & v)
{ 
	int n = arr.cols;
	int m = arr.rows;

    int row = 0, col = 0; 
  
    // Boolean variable that will true if we 
    // need to increment 'row' value otherwise 
    // false- if increment 'col' value 
    bool row_inc = 0; 
  
    // Print matrix of lower half zig-zag pattern 
    int mn = min(m, n); 
    for (int len = 1; len <= mn; ++len) { 
        for (int i = 0; i < len; ++i) {
        	v.push_back(arr.at<int>(row,col));
  
            if (i + 1 == len) 
                break; 
            // If row_increment value is true 
            // increment row and decrement col 
            // else decrement row and increment 
            // col 
            if (row_inc) 
                ++row, --col; 
            else
                --row, ++col; 
        } 
  
        if (len == mn) 
            break; 
  
        // Update row or col vlaue according 
        // to the last increment 
        if (row_inc) 
            ++row, row_inc = false; 
        else
            ++col, row_inc = true; 
    } 
  
    // Update the indexes of row and col variable 
    if (row == 0) { 
        if (col == m - 1) 
            ++row; 
        else
            ++col; 
        row_inc = 1; 
    } 
    else { 
        if (row == n - 1) 
            ++col; 
        else
            ++row; 
        row_inc = 0; 
    } 
  
    // Print the next half zig-zag pattern 
    int MAX = max(m, n) - 1; 
    for (int len, diag = MAX; diag > 0; --diag) { 
  
        if (diag > mn) 
            len = mn; 
        else
            len = diag; 
  
        for (int i = 0; i < len; ++i) { 
        	v.push_back(arr.at<int>(row,col));

            if (i + 1 == len) 
                break; 
  
            // Update row or col vlaue according 
            // to the last increment 
            if (row_inc) 
                ++row, --col; 
            else
                ++col, --row; 
        } 
  
        // Update the indexes of row and col variable 
        if (row == 0 || col == m - 1) { 
            if (col == m - 1) 
                ++row; 
            else
                ++col; 
  
            row_inc = true; 
        } 
  
        else if (col == 0 || row == n - 1) { 
            if (row == n - 1) 
                ++col; 
            else
                ++row; 
  
            row_inc = false; 
        } 
    } 
} 

int pow_my_k(int val){
	int val_to_return;
	if(val >= 0) 	val_to_return = val * 2;
	else			val_to_return -2* val -1;
	return val_to_return;
}

void Encoder::write_frame_component_lossless(Golomb & g, Golomb & g_zeros, vector<tuple<int, uint8_t>> & write_vector){
	tuple<int, uint8_t> x;
	for (int i = 0; i < write_vector.size(); ++i)
	{
		x = write_vector.at(i);

		/* if golomb do tipo 0 */
		if(get<1>(x) == 0){
			g_zeros.encode_and_write(get<0>(x), w);
		} else{ /* if golomb do tipo 1 */
			g.encode_and_write(get<0>(x), w);
		}
	}
}

void Encoder::encode_and_write_frame_intra_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix){
	Mat matrix, divisor;
	vector<int> residuals = {}, zero_residuals = {};
    int to_calculate_k = 0, zero_to_calculate_k = 0 ;
    vector<tuple<int, uint8_t>> write_vector;
    int n_residuals = 0;


	if(frame_matrix == 0){
		matrix = frame->get_y();
		divisor = mat_luminance;
	} else if(frame_matrix == 1){
		matrix = frame->get_u();
		divisor = mat_chrominance;
	} else if(frame_matrix == 2){
		matrix = frame->get_v();
		divisor = mat_chrominance;
	}

    Mat macroblock, tmp_matrix;
    for( int y_curr_frame = 0; y_curr_frame < matrix.rows; y_curr_frame +=this->block_size ){
        for( int x_curr_frame = 0; x_curr_frame < matrix.cols; x_curr_frame +=this->block_size ){

            macroblock = matrix(cv::Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));
            macroblock.convertTo(tmp_matrix, CV_64FC1);

			dct(tmp_matrix, tmp_matrix);
			
			/*
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
				cout << "tmp_matrix = "<< endl << " "  << tmp_matrix << endl << endl;
				cout << "divisor = "<< endl << " "  << divisor << endl << endl;
			}
			*/
			divide(tmp_matrix, divisor, tmp_matrix, 1, CV_64FC1);

			/*
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
				cout << "tmp_matrix = "<< endl << " "  << tmp_matrix << endl << endl;
			}
			*/
			

			macroblock = Mat(8, 8, CV_32SC1);
			int value, n_zeros = 0;



            for (int y = 0; y < macroblock.rows; ++y)
            {
            	for (int x = 0; x < macroblock.cols; ++x)
                {	
                	macroblock.at<int32_t>(y,x) = cvRound(tmp_matrix.at<double>(y,x));
                }
            }
            /*
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
				cout << "macroblock = "<< endl << " "  << macroblock << endl << endl;
			}
			*/
            vector<int> v = {};
			zigZagMatrix(macroblock, v);


			for (int i = 0; i < v.size(); ++i)
			{
				value = v.at(i);

				if (i == 0)
				{
					// TODO fazer menos 

					write_vector.push_back(make_tuple(value, 1));
					to_calculate_k += pow_my_k(value);
					n_residuals++;

				} else if (value == 0) {
					n_zeros++;
            	}
            	else{
					write_vector.push_back(make_tuple(n_zeros, 0));
					zero_to_calculate_k += pow_my_k(n_zeros);

					write_vector.push_back(make_tuple(value, 1));
					to_calculate_k += pow_my_k(value);
					n_residuals++;

					n_zeros=0;
            	}
			}

            if (n_zeros != 0)
            {
				write_vector.push_back(make_tuple(n_zeros, 0));
				zero_to_calculate_k += pow_my_k(n_zeros);

            }
		}
	}


    int k = get_best_k(n_residuals, to_calculate_k);
    int m = pow(2,k);
    g.set_m(m);



    int k0 = get_best_k(write_vector.size() - n_residuals, zero_to_calculate_k);
    int m0 = pow(2,k0);
    g_zeros.set_m(m0);
    /*
	printf("n_zeros: %d; zero_to_calculate_k: %d\n", write_vector.size() - n_residuals, zero_to_calculate_k);
	printf("n_residuals: %d; to_calculate_k: %d\n", n_residuals, to_calculate_k);
	*/
    printf("Type of frame: %d; k: %d; k0: %d\n", frame_matrix, k, k0);
	
    this->w.write_header_k(k);
    this->w.write_header_k(k0);

    write_frame_component_lossless(g, g_zeros, write_vector);


}



void Encoder::encode_and_write_lossy(){

	/* Get basic variables */
    string line;
    getline(this->infile, line);
    map<char, string> header;
    parse_header(header, line);

    this->cols = stoi(header['W']);
    this->rows = stoi(header['H']);
    this->color_space = stoi(header['C']);


    Golomb g, g_zeros;

    int frame_counter =0;
    Frame * current_frame, * previous_frame;

    vector<unsigned char> imgData;
    vector<unsigned char> previous_imgData;

    switch(this->color_space){
        case 444:{
            current_frame = new Frame444 (rows, cols);
            previous_frame = new Frame444 (rows, cols); 
            imgData.resize(cols * rows * 3);
            break;
        }
        case 422:{
            current_frame = new Frame422 (rows, cols);
            previous_frame = new Frame422 (rows, cols); 
            imgData.resize(cols * rows * 2);
            break;
        }
        case 420:{
            current_frame = new Frame420 (rows, cols);
            previous_frame = new Frame420 (rows, cols); 
            imgData.resize(cols * rows * 3/2);
            break;
        }
        default:
            exit(1);
    }

    this->w.writeHeader(this->cols, this->rows, this->color_space, this->block_size);


	while(1){
		current_frame->clear();
		getline (this->infile,line); // Skipping word FRAME
		this->infile.read((char *) imgData.data(), imgData.size());
		current_frame->set_frame_data(imgData.data());

		if(this->infile.gcount() == 0){
			break;
		}

		if( this->periodicity == 0 || frame_counter % this->periodicity == 0){
			encode_and_write_frame_intra_lossy(current_frame, g, g_zeros, 0);
			encode_and_write_frame_intra_lossy(current_frame, g, g_zeros, 1);
			encode_and_write_frame_intra_lossy(current_frame, g, g_zeros, 2);
		}else{
			// encode_and_write_frame_inter(current_frame, previous_frame, frame_counter, g, g_zeros );
		}

        std::swap(current_frame, previous_frame);

		printf("Done frame %d\n", frame_counter);
		frame_counter += 1 ;


	}
    

    this->w.flush();

    delete current_frame;
    delete previous_frame;

} 
