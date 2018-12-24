#include "../include/Decoder.hpp"

using namespace cv;


void reverse_zig_zag(Mat & arr, vector<int16_t> & v)
{
	int i = 0;
    int x,y ,z;
    for(z = 0 ; z < 8 ; z++)
    {
        x = 0;
        y = z;
        while(x<=z && y>=0)
        {
            if(z%2){
            	arr.at<int16_t>(x++,y--) = v.at(i);
                i++;
            }
            else{
            	arr.at<int16_t>(y--,x++) = v.at(i);
                i++;
            }
        }
    }
    for(z = 1 ; z < 8 ; z++)
    {
        x = z;
        y = 8 -1;
        while( x < 8 && y>=z)
        {
            if((8+z)%2){
            	arr.at<int16_t>(y--,x++) = v.at(i);
                i++;
            }
            else{
            	arr.at<int16_t>(x++,y--) = v.at(i);
                i++;
            }
        }
    }
}

Mat Decoder::decode_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix) {
	Mat matrix, divisor;

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

    matrix.convertTo(matrix, CV_16SC1);

	Mat macroblock;
	Mat tmp_matrix = Mat(8, 8, CV_16SC1);

    for( int y_curr_frame = 0; y_curr_frame < matrix.rows; y_curr_frame +=this->block_size ){
        for( int x_curr_frame = 0; x_curr_frame < matrix.cols; x_curr_frame +=this->block_size ){
			vector<int16_t> residuals;
			
			/* Ler para um array de tamanho  8*8 */
        	while(true){
        		residuals.push_back(g.read_and_decode(this->r));
        		if (residuals.size() == 64) break;
 
        		int16_t zeros = g_zeros.read_and_decode(this->r);
        		while (zeros != 0){
        			residuals.push_back(0);
        			zeros--;
        		}
        		if (residuals.size() == 64) break;
        	}


			/* passar de array para zigzaged matrix */
    		reverse_zig_zag(tmp_matrix, residuals);
            if (y_curr_frame == 0 && x_curr_frame == 0)
            {
                for (auto i = residuals.begin(); i != residuals.end(); ++i)
                    cout << (int16_t) *i << ' ';
                cout <<endl;
                printf("%d\n", matrix.at<int16_t>(1,0));
                cout << "macroblock = "<< endl << " "  << tmp_matrix << endl << endl;
            }
			/* multiplicar por matrizes constantes e inverse dct*/
            tmp_matrix.convertTo(macroblock, CV_64FC1);
    		macroblock = macroblock.mul(divisor);
			/* Inverse dct */
    		dct(macroblock, macroblock, DCT_INVERSE);

            // Store in matrix
            for (int x = x_curr_frame; x < x_curr_frame + this->block_size; ++x)
            {
                for (int y = y_curr_frame; y < y_curr_frame + this->block_size; ++y)
                {
                    matrix.at<int16_t>(y,x) = cvRound(macroblock.at<double>(y-y_curr_frame,x-x_curr_frame)); 
                }
            }

            /*
            macroblock = matrix(Rect(0, 0, this->block_size, this->block_size));
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
	            printf("%d\n", matrix.at<int16_t>(1,0));
				cout << "macroblock = "<< endl << " "  << macroblock << endl << endl;
			}

            */


    	}
	}
    /*
    if(frame_matrix == 0){
        frame->get_y() = matrix;
    } else if(frame_matrix == 1){
        frame->get_u() = matrix;
    } else if(frame_matrix == 2){
        frame->get_v() = matrix;
    }
    */
    return matrix;
    printf("This %d\n", frame->get_y().at<int16_t>(1,0));
    printf("This %d\n", matrix.at<int16_t>(1,0));

}

void Decoder::decode_non_intra_lossy(Frame * frame, Frame * previous_frame, Golomb & g, Golomb & g_zeros, int frame_matrix) {
    Mat matrix, tmp_matrix;
    
    tmp_matrix = decode_lossy(frame, g, g_zeros, frame_matrix);
    tmp_matrix.convertTo(matrix, CV_8UC1);
    
    if(frame_matrix == 0){
        frame->get_y() = previous_frame->get_y() - matrix;
    } else if(frame_matrix == 1){
        frame->get_u() = previous_frame->get_u() - matrix;
    } else if(frame_matrix == 2){
        frame->get_v() = previous_frame->get_v() - matrix;
    }

    uint8_t * line = matrix.ptr(0);
    this->outfile.write( (char*) line, matrix.cols * matrix.rows);
}

void Decoder::decode_intra_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix) {
    Mat matrix, tmp_matrix;
    
    tmp_matrix = decode_lossy(frame, g, g_zeros, frame_matrix);
    tmp_matrix.convertTo(matrix, CV_8UC1);

    if(frame_matrix == 0){
        frame->get_y() = matrix;
    } else if(frame_matrix == 1){
        frame->get_u() = matrix;
    } else if(frame_matrix == 2){
        frame->get_v() = matrix;
    }

    uint8_t * line = matrix.ptr(0);
    this->outfile.write( (char*) line, matrix.cols * matrix.rows);
}

void Decoder::read_and_decode_lossy(){

    int frame_counter =0;
    int k, k0;
    int type;

    Golomb g, g_zeros;

    string line;

    map<char, string> header;
    line = this->r.readHeader();
    this->r.parse_header_pv(header, line);

    this->height = stoi(header['H']);
    this->width = stoi(header['W']);
    this->block_size = stoi(header['B']);
    this->color_space = stoi(header['C']);
    string frameRate = header['F'];
    string a = header['A'];
    string interlance = header['I'];

    Frame * current_frame;
    Frame * previous_frame;


    switch(this->color_space){
        case 444:{
            current_frame = new Frame444 (this->height, this->width);
            previous_frame = new Frame444 (this->height, this->width);
            break;
        }
        case 422:{
            current_frame = new Frame422 (this->height, this->width);
            previous_frame = new Frame422 (this->height, this->width);
            break;
        }
        case 420:{
            current_frame = new Frame420 (this->height, this->width);
            previous_frame = new Frame420 (this->height, this->width);
            break;
        }
        default:
            exit(1);
    }


    this->write_header_y4m(stoi(header['W']), stoi(header['H']), frameRate, stoi(header['C']), a, interlance);
    

    while(1){

        type = this->r.read_type();
        if(type == 7){
            break;
        }

        current_frame->clear();

        if(type == 0){
            this->write_header_frame();

            for (int i = 0; i < 3; ++i)
            {
	            k = this->r.read_k();
    			g.set_m(pow(2,k));
            	k0 = this->r.read_k();
            	g_zeros.set_m(pow(2,k0));
            	decode_intra_lossy(current_frame, g, g_zeros, i);
            }
        } else if(type == 1){
            /* Write Frame Header */
            this->write_header_frame();

            for (int i = 0; i < 3; ++i)
            {
                k = this->r.read_k();
                g.set_m(pow(2,k));
                k0 = this->r.read_k();
                g_zeros.set_m(pow(2,k0));
                decode_non_intra_lossy(current_frame, previous_frame, g, g_zeros, i);
            }

        }
        std::swap(current_frame, previous_frame);

    	printf("done %d\n", frame_counter);
    	frame_counter++;
    }
}

