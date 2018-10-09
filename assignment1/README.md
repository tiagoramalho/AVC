# Audio Files Manipulation and Lossy Compression Algorithms

## Dependencies
- CMake
- libsndfile

## Build:
```cmake . && make```

## Run

### Multi-Channel to mono

Usage:
```
./bin/wavhist <input file> <channel>
```

Using the following example the mono is both printed and stored in `mono.txt`:
```
./bin/wavhistsamples/sample01.wav mono
```

There was developed a gnuplot program to generate the `histogram.png` correspondent to the provided histogram:
```
./show_plot.plt <path/to/histogram> 
```

### Uniform linear quantization

Usage:
```
./bin/wavquant <input file> <output file> <operation (e/d)> <number of bits>
```

Encode example:
```
./bin/wavquant samples/sample01.wav sample01.bin e 14
```

After this we should decode: 
```
./bin/wavquant sample01.bin sample_decompressed.wav d 14
```

### Vector quantization

First step is the codebook creation
```
./bin/wavcb_2 samples/sample02.wav 3 2 512
```

The previous command will create a file in `output`  called `cb.txt`.
After this you can either enconde this file based on the codebook an generate an `index.txt`:
```
wavvq samples/sample02.wav index.txt output/cb.txt e 3 2
```

or decode a file from based on a previously created one:
```
wavvq index.txt newfile.wav output/cb.txt d 3 2
```

Full syntax here:
```
wavvq <input file> <output file> <codebook> <operation (e/d)> <block_size> <channels>
```
