#include "../include/Decoder.hpp"

using namespace cv;


void reverse_zig_zag(Mat & arr, vector<int> & v)
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
            	arr.at<int>(x++,y--) = v.at(i);
                i++;
            }
            else{
            	arr.at<int>(y--,x++) = v.at(i);
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
            	arr.at<int>(y--,x++) = v.at(i);
                i++;
            }
            else{
            	arr.at<int>(x++,y--) = v.at(i);
                i++;
            }
        }
    }
}

void Decoder::decode_intra_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix) {
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

	Mat macroblock;
	Mat tmp_matrix = Mat(8, 8, CV_32SC1);
	Mat tmp_matrix2;


    for( int y_curr_frame = 0; y_curr_frame < matrix.rows; y_curr_frame +=this->block_size ){
        for( int x_curr_frame = 0; x_curr_frame < matrix.cols; x_curr_frame +=this->block_size ){
			vector<int> residuals;
			
			/* Ler para um array de tamanho  8*8 */
        	while(true){
        		residuals.push_back(g.read_and_decode(this->r));
        		if (residuals.size() == 64) break;
 
        		int zeros = g_zeros.read_and_decode(this->r);
        		while (zeros != 0){
        			residuals.push_back(0);
        			zeros--;
        		}
        		if (residuals.size() == 64) break;
        	}

        	/*
			for (auto i = residuals.begin(); i != residuals.end(); ++i)
    			cout << *i << ' ';
    		cout << endl;
			*/

			/* passar de assaray para zigzaged matrix */
    		reverse_zig_zag(tmp_matrix, residuals);

			/* multiplicar por matrizes constantes e inverse dct*/
            tmp_matrix.convertTo(macroblock, CV_64FC1);
    		macroblock = macroblock.mul(divisor);
			/* Inverse dct */
    		dct(macroblock, macroblock, DCT_INVERSE);


			/* back to 32 representation */
			tmp_matrix2 = Mat(8, 8, CV_8UC1);
            for (int y = 0; y < macroblock.rows; ++y)
            {
            	for (int x = 0; x < macroblock.cols; ++x)
                {	
                	tmp_matrix2.at<uint8_t>(y,x) = cvRound(macroblock.at<double>(y,x));
                }
            }
            macroblock = tmp_matrix2;
            //Rect srcRect = Rect(0, 0, this->block_size, this->block_size);
            // Rect tmp_matrix(Point(0, 0), Size(this->block_size-1, this->block_size-1));

            // matrix(Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size));
            //Rect dstRect = Rect(x_curr_frame, y_curr_frame, this->block_size, this->block_size);
            // tmp_matrix(srcRect).copyTo(matrix(dstRect));


            for (int x = x_curr_frame; x < x_curr_frame + this->block_size; ++x)
            {
                for (int y = y_curr_frame; y < y_curr_frame + this->block_size; ++y)
                {
                    matrix.at<uint8_t>(y,x) = tmp_matrix2.at<uint8_t>(y-y_curr_frame,x-x_curr_frame); 
                }
            }
			// cout << "tmp_matrix = "<< endl << " "  << tmp_matrix << endl << endl;	        
            /*
            printf("%d, %d\n", x_curr_frame, y_curr_frame);
            printf("%d\n", matrix.at<uint8_t>(1,0));
            
			
			if (y_curr_frame == 0 && x_curr_frame == 0)
			{
	            printf("%d\n", matrix.at<uint8_t>(1,0));
				cout << "macroblock = "<< endl << " "  << macroblock << endl << endl;
			}
			exit(1);
			*/




    	}
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


    Frame * current_frame;
    Frame * last_frame;


    switch(this->color_space){
        case 444:{
            current_frame = new Frame444 (this->height, this->width);
            last_frame = new Frame444 (this->height, this->width);
            break;
        }
        case 422:{
            current_frame = new Frame422 (this->height, this->width);
            last_frame = new Frame422 (this->height, this->width);
            break;
        }
        case 420:{
            current_frame = new Frame420 (this->height, this->width);
            last_frame = new Frame420 (this->height, this->width);
            break;
        }
        default:
            exit(1);
    }

    while(1){

        type = this->r.read_type();
        if(type == 7){
            break;
        }

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
        }
    	printf("done %d\n", frame_counter);
    	frame_counter++;
    }
}