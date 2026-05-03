#!/bin/sh

directory_name=$1
test_directory=test
src_directory=src
includes_directory=includes

mkdir -p $src_directory/$directory_name && \
touch $src_directory/$directory_name/${directory_name}.c && \
touch $includes_directory/${directory_name}.h && \
mkdir -p $test_directory 


