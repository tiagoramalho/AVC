#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <iostream>
#include <string.h>
#include <vector>
#include "fstreamBits.h"

using namespace std;

class Predictor {

    private:
        uint32_t max_order, block_size;

        vector<double> averages;

        vector<vector<int>> block_all_residuals ;

        // Entropy Calculation
        // It will also be able to output histogram data
        // because the histogram calculations are the same
        // we must calculate the frequency of each residual
        // TODO: save_freqs
        vector<float> calculate_entropies();

        // Average Calculation
        // Calculates the Average of the saved residuals
        // Average is usefull because the value can be used
        // to set the m on Golomb coding
        vector<double> calculate_averages();


        // Average Calculation
        // Calculates the Average of the saved residuals
        // Average is usefull because the value can be used
        // to set the m on Golomb coding
        vector<double> get_averages();



    public:

        // Constructor
        Predictor(uint32_t max_order, uint32_t block_size);

        // Function that will build de matrix with all the residuals
        void populate_v(vector<int> & samples);

        // Recursive function that generates the residual values
        // up to max_order
        int gen_residuals(vector<int> & samples, uint32_t index, uint32_t order);

        // Get Id of the best predictor setting
        // 0 - id of the best predictor
        // 1 - value for m in GOlomb encoding
        // 2 - is frame constant
        vector<short> get_best_predictor_settings();

        // Get residual values of the best predictor
        vector<int> get_residuals(uint32_t predictor_index);

        //                  HELPER fUNCTIONS

        //Setter, change block size
        // Also cleans
        void set_block_size_and_clean(uint32_t size);

        // Print a matrix
        void print_matrix( vector<vector<int>> & matrix );

};

#endif
