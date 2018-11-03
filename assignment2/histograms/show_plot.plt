#!/usr/bin/gnuplot -c 

clear
reset
unset key

set terminal png

set output sprintf("./%s.png", ARG2)

set title "WAVE Audio Histogram"
set grid
set xtics rotate

set style data histogram
set style fill solid border

set style histogram clustered

set logscale y 2

if (ARGC != 2) {
    set print "-"
    print "Usage: ./show_plot.plt <path/to/histogram> <output_name>"
} else {
    plot ARG1 using 2:xtic(int($0)%1000 == 0 ? stringcolumn(1) : '') 
}
