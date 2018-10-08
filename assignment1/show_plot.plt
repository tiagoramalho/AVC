#!/usr/bin/gnuplot -persist

clear
reset
unset key

set terminal png
set output "./histogram.png"

set title "WAVE Audio Histogram"
set grid
set xtics rotate

DATAFILE = "hist"

set style data histogram
set style fill solid border

set style histogram clustered

plot DATAFILE using 2:xtic(int($0)%1000 == 0 ? stringcolumn(1) : '') 

