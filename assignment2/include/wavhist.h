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
        string file_name;
        bool residuals;

    public:
        WAVHist(const SndfileHandle& sfh, string file_name) {
            counts.resize(sfh.channels());
            this->file_name = file_name.substr(0, file_name.find("."));
            residuals = false;
        }

        WAVHist(const SndfileHandle& sfh, string file_name, uint8_t size) {
            counts.resize(sfh.channels() * size);
            this->file_name = file_name.substr(0, file_name.find("."));
            residuals = true;
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

        void simple_update_index(uint8_t idx, const std::vector<short>& samples) {
            for(auto s : samples) {
                counts[idx][s]++; 
            }
        }

        void dump(const size_t channel) const {
            std::ofstream myfile;
            auto s1 = file_name + "_channel" + std::to_string(channel) + "_hist.dat";
            myfile.open(s1);
            for(auto [value, counter] : counts[channel]){
                myfile << value << '\t' << counter << '\n';

            }
        }

        void full_dump() const {
            if(residuals){
                for (uint8_t i = 0; i < counts.size(); ++i)
                {
                    std::ofstream myfile;
                    auto s1 = file_name + "_channel" + std::to_string(i%2) + "_residual" + std::to_string(i/2) + "_hist.dat";
                    myfile.open(s1);
                    for(auto [value, counter] : counts[i]){
                        myfile << value << '\t' << counter << '\n';
                    }
                    myfile.close();
                }
            } else {
                for (uint8_t i = 0; i < counts.size(); ++i)
                {
                    std::ofstream myfile;
                    auto s1 = file_name + "_best_channel" + std::to_string(i) + "_hist.dat";
                    myfile.open(s1);
                    for(auto [value, counter] : counts[i]){
                        myfile << value << '\t' << counter << '\n';
                    }
                    myfile.close();
                }
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

