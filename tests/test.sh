#!/bin/bash

files=()
gk="../build/bin/Ginkgo"
success=0
fail=0

find_c_files() {
    for file in "$1"/*; do
        if [[ -d "$file" ]]; then
            find_c_files "$file"
        elif [[ "${file##*.}" == "c" ]]; then
            files+=("$file")
        fi
    done
}

check_and_build() {
    if ! [[ -d "../build" ]]; then
        mkdir ../build
        cd ../build
        cmake ..
        make all
        cd ../tests
    fi
}

test_file() {
    eval "$gk -I. ../include ../tests $1"
    if $(($? == 0)); then
        echo "testing $1... SUCCESS"
        success=$((success + 1))
    else
        echo "testing $1... FAILED"
        fail=$((fail + 1))
    fi
}

run_ginkgo() {
    if [[ -d "$1" ]]; then
        find_c_files "$1"
        for file in files; do
            test_file "$file"
        done
    elif [[ -f "$1" ]]; then
        test_file "$1"
    else # in some subdirectory?
        name=$(find . -name "$1" -quit 2>/dev/null)
        test_file "$name"
    fi
}

check_and_build

if [[ $# == 0 ]]; then
    run_ginkgo .
else
    for name in $@; do
        run_ginkgo "$name"
    done
fi

total=$($success + $fail)
echo "$total case(s) are tested, $success successed and $fail failed."
