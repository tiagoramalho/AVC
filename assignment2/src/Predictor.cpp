#include "Predictor.hpp"
#include <vector>

using namespace std;

Predictor::Predictor(uint32_t max_order, uint32_t sample_size): max_order(max_order), sample_size(sample_size){
	vector< vector<short> > vec (max_order, vector<short> (sample_size));
	this->v = vec;
}

void Predictor::populate_v(vector<short> & samples) {
    uint32_t i = 0;
    uint32_t j = 0;

    cout << samples.at(1) << endl;

	for (i = sample_size - 1; i > 1; i--)
	{
		gen_residuals( samples, i, max_order-1);
	}

	for (i = 0; i < v.size(); ++i)
	{
		for (j = 0; j < sample_size; ++j)
		{
			cout << v.at(i).at(j) << ", ";
		}
		cout << endl;
	}
}

short Predictor::gen_residuals(vector<short> & samples, uint32_t index, uint32_t order) {

	short rn = 0;
	if (order == 0){

		cout << "ordem 0 " << index << endl;

		rn = samples.at(index);
    	cout << order << endl;

	}
	else{
		cout << "encavacou" << endl;

		if (index < order)
		{
			rn = 0;
		}else{
			rn = gen_residuals(samples, index, order-1) - gen_residuals(samples, index - 1, order-1);
		}
	}
	cout << "order "<< order << endl;
	cout << "index "<< index << endl;
	cout << "v_order_size "<< v.size() << endl;
	cout << "v_order_index_size "<< v.at(order).size() << endl;
	v.at(order).at(index) = rn;
	return rn;
}



