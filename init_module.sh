#!/bin/sh

module_name=$1
test_directory=tests
src_directory=src
includes_directory=includes
header_filename=${1}.h
header_guard=$(printf $header_filename | tr '[:lower:]' '[:upper:]' | tr '.' '_')

mkdir -p $src_directory/$module_name

cat <<EOF >$src_directory/$module_name/${module_name}.c
#include "$header_filename"

EOF

cat <<EOF >$includes_directory/$header_filename
#ifndef $header_guard
#define $header_guard

#include "d_types.h"


#endif
EOF

mkdir -p $test_directory/$module_name

cat <<EOF >$test_directory/$module_name/test.c
#include "d_test.h"

int main()
{

}
EOF
