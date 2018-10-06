#ifndef FSTREAMBITS_H
#define FSTREAMBITS_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class READBits: public ifstream {

    private:
        char buff = 0;
        int shamnt = -1;
        ifstream f;

    public:
        READBits() : f ("codedAudio.bin", ios::binary){} 

        uint8_t readBits(){
            if(shamnt == -1){
                f.read(&buff, 1);
                shamnt = 7;

            }
            uint8_t val = buff >> shamnt & 1U;
            shamnt--;
            return val;
        }

        short readItem(uint32_t bits){
            short frame = 0;
            uint32_t i = 0; 
            while(i < bits){
                frame = frame <<1;
                frame = frame | readBits();
                i++;
            }
            if(i < 16){
                frame = (frame << 1) | 1U; 
                i++;
            }
            //frame = frame << (16-bits);
            while(i < 16){
                frame = frame << 1;
                i++;
            }
            return frame;
        }

        string readHeader(){
            string header;
            std::getline(f, header);
            return header;
        }
};

class WRITEBits: public ofstream {
    private:
        char buff = 0;
        int shamnt = 7;
        ofstream f;

    public:
        WRITEBits (): f("codedAudio.bin", ios::binary){}
        void writeBits(char val){
            buff = buff | (val << shamnt);
            shamnt--;
            if(shamnt==-1){
                f.write( &buff, 1);
                shamnt = 7;
                buff = 0;
            }
        }

        void preWrite(short write, uint32_t bits){
            int count = bits-1;
            while(count != -1){
                char val = (write >> count) & 1U;
                writeBits(val);
                count--;
            }

        }
        void writeHeader(uint32_t frames, uint32_t channels, uint32_t samplerates, uint32_t format){
            f << frames << ";" << channels << ";" << samplerates << ";" << format << ";\n";
        }

        void flush(){
            while(shamnt != 7){
                writeBits(0);
            }
            f.close();
        }
};

#endif
