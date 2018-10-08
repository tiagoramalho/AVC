#ifndef WAVHIST_H
#define WAVHIST_H

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>
#include <fstream>

class WAVHist {
    private:
        std::vector<std::map<short, size_t>> counts;
        std::map<short, size_t> countmono;

    public:
        WAVHist(const SndfileHandle& sfh) {
            counts.resize(sfh.channels());
        }

        void update(const std::vector<short>& samples) {
            size_t n { }; 
            size_t sum = 0;
            size_t index = 0;
            for(auto s : samples) {
                index = n++ % counts.size();
                sum = sum +s;
                counts[index][s]++; 

                if(index == counts.size()-1){
                    countmono[sum/counts.size()]++;;
                    sum = 0;
                }
            }
        }

        void dump(const size_t channel) const {
            std::ofstream myfile;
            auto name = "channel";
            auto s1 = name + std::to_string(channel) + ".txt";
            myfile.open(s1);
            for(auto [value, counter] : counts[channel]){
                std::cout << value << '\t' << counter << '\n';
                myfile << value << '\t' << counter << '\n';

            }
        }

        void mono() {
            std::ofstream myfile;
            myfile.open ("mono.txt"); 
            for(auto [value, counter] : countmono){
                std::cout << value << '\t' << counter<< '\n';
                myfile << value << '\t' << counter<< '\n';
            }

        }
};

#endif

