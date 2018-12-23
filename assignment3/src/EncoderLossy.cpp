#include "Encoder.hpp"

#include <cstdio>
#include <numeric>
#include <math.h>
#include <cmath>
#include "Frame.hpp"


using namespace cv;
using namespace std;

Encoder::Encoder(const string & in_file, const string & out_file, int p, int peri, int blocksize, int searcharea):
    infile(in_file.c_str()),w(out_file.c_str()),profile(p), periodicity(peri), block_size(blocksize), search_area(searcharea){} 




void zig_zag(Mat & arr, vector<int16_t> & v)
{
    int x,y ,z;
    for(z = 0 ; z <arr.cols ; z++)
    {
        x = 0;
        y = z;
        while(x<=z && y>=0)
        {
            if(z%2)
                v.push_back(arr.at<int16_t>(x++,y--));
            else
            	v.push_back(arr.at<int16_t>(y--,x++));
        }
    }
    for(z = 1 ; z<arr.cols ; z++)
    {
        x = z;
        y = arr.cols -1;
        while(x<arr.cols && y>=z)
        {
            if((arr.cols+z)%2)
            	v.push_back(arr.at<int16_t>(y--,x++));
            else
            	v.push_back(arr.at<int16_t>(x++,y--));
        }
    }
}


int pow_my_k(int val){
	int val_to_return;
	if(val >= 0) 	val_to_return = val * 2;
	else			val_to_return = -2* val -1;
	return val_to_return;
}

void Encoder::write_frame_component_lossy(Golomb & g, Golomb & g_zeros, vector<tuple<int, int16_t>> & write_vector){
	tuple<int, int16_t> x;
	for (uint32_t i = 0; i < write_vector.size(); ++i)
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

void Encoder::encode_lossy(Mat & matrix, Golomb & g, Golomb & g_zeros, int frame_matrix){
	Mat divisor;
	vector<int> residuals = {}, zero_residuals = {};
    int to_calculate_k = 0, zero_to_calculate_k = 0 ;
    vector<tuple<int, int16_t>> write_vector;
    int n_residuals = 0;

	if(frame_matrix == 0){
		divisor = mat_luminance;
	} else if(frame_matrix == 1){
		divisor = mat_chrominance;
	} else if(frame_matrix == 2){
		divisor = mat_chrominance;
	}

    Mat macroblock, tmp_matrix;
    for( int y_curr_frame = 0; y_curr_frame < matrix.rows; y_curr_frame +=this->block_size ){
        for( int x_curr_frame = 0; x_curr_frame < matrix.cols; x_curr_frame +=this->block_size ){

            macroblock = matrix(cv::Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));
            macroblock.convertTo(tmp_matrix, CV_64FC1);

            
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
				cout << "macroblock = "<< endl << " "  << macroblock << endl << endl;
			}
			

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
			

			macroblock = Mat(8, 8, CV_16SC1);
			int value, n_zeros = 0;

            for (int y = 0; y < macroblock.rows; ++y)
            {
            	for (int x = 0; x < macroblock.cols; ++x)
                {	
                	macroblock.at<int16_t>(y,x) = cvRound(tmp_matrix.at<double>(y,x));
                }
            }
            
            
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
				cout << "macroblock = "<< endl << " "  << macroblock << endl << endl;
			}
			

            vector<int16_t> v = {};
			zig_zag(macroblock, v);

			/*
			for (auto i = v.begin(); i != v.end(); ++i)
    			cout << *i << ' ';
			*/
			for (uint32_t i = 0; i < v.size(); ++i)
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

    write_frame_component_lossy(g, g_zeros, write_vector);


}

void Encoder::encode_intra_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix){
	Mat matrix;

	if(frame_matrix == 0){
		matrix = frame->get_y();
	} else if(frame_matrix == 1){
		matrix = frame->get_u();
	} else if(frame_matrix == 2){
		matrix = frame->get_v();
	}

	matrix.convertTo(matrix, CV_16SC1);

	encode_lossy(matrix, g, g_zeros, frame_matrix);
}


void Encoder::encode_non_intra_lossy(Frame * frame, Frame * previous_frame, Golomb & g, Golomb & g_zeros, int frame_matrix){
	Mat residuals, matrix, previous_matrix, macroblock, tmp_matrix;
	
	if(frame_matrix == 0){
		matrix = frame->get_y();
		previous_matrix = previous_frame->get_y();
	} else if(frame_matrix == 1){
		matrix = frame->get_u();
		previous_matrix = previous_frame->get_u();
	} else if(frame_matrix == 2){
		matrix = frame->get_v();
		previous_matrix = previous_frame->get_v();
	}
	/*
	cv::Mat diff = frame->get_y() != previous_frame->get_y();
	bool eq = cv::countNonZero(diff) == 0;
	if (eq){
		printf("Exited on 1\n");
		exit(1);
	}
	*/
	matrix.convertTo(matrix, CV_16SC1);
	previous_matrix.convertTo(previous_matrix, CV_16SC1);
	/*
    macroblock = matrix(cv::Rect(80, 80, this->block_size, this->block_size));
	cout << "matrix = "<< endl << " "  << macroblock << endl << endl;
    macroblock = previous_matrix(cv::Rect(80, 80, this->block_size, this->block_size));
	cout << "previous_matrix = "<< endl << " "  << macroblock << endl << endl;
	
	diff = matrix != previous_matrix;
	eq = cv::countNonZero(diff) == 0;
	if (eq){
		printf("Exited on 2\n");
		exit(1);
	}
	*/



	residuals = previous_matrix - matrix;

	encode_lossy(residuals, g, g_zeros, frame_matrix);
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
    Frame * current_frame;
    Frame * previous_frame;

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
			this->w.write_header_type(0);
			encode_intra_lossy(current_frame, g, g_zeros, 0);
			encode_intra_lossy(current_frame, g, g_zeros, 1);
			encode_intra_lossy(current_frame, g, g_zeros, 2);
		}else{
			this->w.write_header_type(1);

			encode_non_intra_lossy(current_frame, previous_frame, g, g_zeros, 0);
			encode_non_intra_lossy(current_frame, previous_frame, g, g_zeros, 1);
			encode_non_intra_lossy(current_frame, previous_frame, g, g_zeros, 2);
		}

        std::swap(current_frame, previous_frame);

		printf("Done frame %d\n", frame_counter);
		frame_counter += 1 ;


	}
    

    this->w.write_header_type(7);
    this->w.flush();

    delete current_frame;
    delete previous_frame;

} 
