#!/bin/sh

set -eu

module_name=$1



test_directory=tests
src_directory=src
includes_directory=includes
header_filename=${1}.h
header_guard=$(printf $header_filename | tr '[:lower:]' '[:upper:]' | tr '.' '_')

create_file() {
    local file_name=$1
    local file_content=$2
    local file_parent_dir="$(dirname "$file_name")"
    mkdir -p $file_parent_dir
cat << EOF >$file_name
$file_content
EOF
}

create_file $src_directory/$module_name/${module_name}.c  "#include \"$header_filename\""
create_file $includes_directory/$header_filename  "$(cat <<EOF
#ifndef $header_guard
#define $header_guard
#include "d_types.h"
#include "d_result.h"
   
#endif
EOF
)"

create_file $test_directory/$module_name/test.c "$(cat <<EOF 
#include "d_test.h"
int main() 
{
    
}
EOF
)"




