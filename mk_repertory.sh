#!/bin/sh

directory_name=$1

(mkdir $directory_name && cd $directory_name && mkdir include test src && touch Makefile test/test.c src/d_$directory_name.c include/d_$directory_name.h)