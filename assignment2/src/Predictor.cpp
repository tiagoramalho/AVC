#include "Predictor.hpp"

#include <algorithm>
#include <map>
#include <math.h>
#include <vector>


using namespace std;

Predictor::Predictor(uint32_t max_order, uint32_t block_size) :
    max_order(max_order),
    block_size(block_size),
    block_all_residuals(max_order, vector<short> (block_size))
{
    cout << "New Predictor" << endl;
}

void Predictor::populate_v(vector<short> & samples) {
    uint32_t i = 0;

    for (i = this->block_size - 1; i > 1; i--)
    {
        gen_residuals( samples, i, max_order-1);
    }

    print_matrix(this->block_all_residuals);

}

void Predictor::print_matrix( vector<vector<short>> & matrix){
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t max_order = matrix.size();
    uint32_t block_size = matrix.at(0).size();

    for (i = 0; i < max_order; i++)
    {
        cout << "Order " << i << " : ";

        for (j = 0; j < block_size; j++)
        {
            cout << matrix.at(i).at(j) << ", ";
        }

        cout << endl;
    }

}


short Predictor::gen_residuals(vector<short> & samples, uint32_t index, uint32_t order) {

    short rn = 0;
    if (order == 0){
        rn = samples.at(index);
    }
    else{

        if (index < order)
        {
            rn = 0;
        }else{
            rn = gen_residuals(samples, index, order-1) - gen_residuals(samples, index - 1, order-1);
        }
    }

    this->block_all_residuals.at(order).at(index) = rn;
    return rn;
}


vector<float> Predictor::calculate_entropies(bool save_freqs){

    vector<float> entropies (this->max_order, 0);

    float entropy = 0;

    map<short,long> counts;
    map<short,long>::iterator it;

    for( uint32_t i = 0; i < this->max_order; i++){

      entropy = 0;

      for (uint32_t residual_index = 0; residual_index < this->block_size; residual_index++) {
        counts[this->block_all_residuals.at(i)[residual_index]]++;
      }

      it = counts.begin();
      while(it != counts.end()){
        float p_x = (float)it->second/this->block_size;
        if (p_x>0) entropy-=p_x*log(p_x)/log(2);
          it++;
      }

      entropies.at(i) = entropy;
      counts.clear();
    }

    for( uint32_t i = 0; i < entropies.size(); i++){
        cout << "Order " << i << " entropy :" << entropies.at(i) << endl;
    }

    return entropies;

};


uint32_t Predictor::get_best_predictor(){
    vector<float> entropies = calculate_entropies(false);
    vector<float>::iterator result = min_element(begin(entropies), end(entropies));
    return distance(std::begin(entropies), result);
}

vector<short> Predictor::get_residuals(uint32_t predictor_index){
    return this->block_all_residuals.at(predictor_index);
}

float Predictor::calculate_entropy( vector<short> & matrix){

    float entropy = 0;

    map<short,long> counts;
    map<short,long>::iterator it;

    for (uint32_t residual_index = 0; residual_index < matrix.size(); residual_index++) {
      counts[matrix[residual_index]]++;
    }

    it = counts.begin();
    while(it != counts.end()){
      float p_x = (float)it->second/matrix.size();
      if (p_x>0) entropy-=p_x*log(p_x)/log(2);
        it++;
    }

    return entropy;
}
