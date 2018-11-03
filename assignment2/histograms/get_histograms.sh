#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"

if echo $* | grep -e "-h" -q
then
	echo "Usage: ./get_histograms.sh <regex to list of .dat files>"
	exit 0
elif echo $* | grep -e "--help" -q
then 
	echo "Usage: ./get_histograms.sh <regex to list of .dat files>"
	exit 0
fi

current_dir=$(pwd)
pushd . >> /dev/null
cd $DIR 
for f in "$@"
do
	echo $f
    filename=$(basename -- "$f")
    extension="${filename##*.}"
    filename="${filename%.*}"
    ./show_plot.plt $current_dir/$f $filename
done
popd
