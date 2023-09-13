#!/bin/bash

dirs=()
gk_shallow="../build/bin/Ginkgo"
gk="../../build/bin/Ginkgo"
filename=""
success=0
fail=0

# no argument
check_and_build() {
    if ! [[ -d "../build" ]]; then
        mkdir ../build
        cd ../build
        cmake ..
        make all -j 10
        cd ../tests
    elif ! [[ -f "$gk_shallow" ]]; then
        cd ../build
        make all -j 10
        cd ../tests
    fi
}

# no argument
find_dir() {
    while IFS= read -r line; do
        dirs+=("$line")
    done < "dirs.txt"
}

# two arguments
# first argument: dir name
# second argument: test name
find_test() {
    cd "$1"
    if ! [[ -f "hints.txt" ]]; then
        cd ..
        return 0
    fi

    while IFS= read -r line; do
        local words=($line)
        local name="${words[0]}"
        local args="${words[@]:1}"
        if [[ "$name" != "$2" ]]; then
            continue
        fi

        if [[ -z "$args" ]]; then
            filename="${name}.c"
        else
            filename="$args"
        fi

        cd ..
        return 1
    done < "hints.txt"

    cd ..
    return 0
}

# no argument
report_success() {
    echo -e "${GREEN}OK${RESET}"
    success=$((success + 1))
}

# no argument
report_failure() {
    echo -e "${RED}FAILED${RESET}"
    fail=$((fail + 1))
}

# three arguments
# first argument: name of the test
# second argument: source files included by this test
# thrid argument: disable output of a.out or not
test_file() {
    GREEN="\033[0;32m"
    RED="\033[0;31m"
    RESET="\033[0;0m"

    echo -n "testing $1... "
    eval "$gk -I ../../tests $2 -o a.out"
    if ! [[ -f "a.out" ]]; then
        report_failure
        return
    fi

    if [[ $3 == 0 ]]; then
        eval $"./a.out &> /dev/null"
    else
        eval $"./a.out"
    fi

    if [[ $? == 0 ]]; then
        report_success 
    else
        report_failure
    fi
    rm a.out
}

# one argument
# first: directory name
test_dir() {
    cd "$1"
    if ! [[ -f "hints.txt" ]]; then
        cd ..
        return
    fi

    while IFS= read -r line; do
        local name="${line[0]}"
        local args="${word[@]:1}"
        local filename=""
        if [[ -z "$args" ]]; then
            filename="${name}.c"
        else
            filename="$args"
        fi
        test_file "$name" "$filename" 0
    done < "hints.txt"
    cd ..
}

# one argument
# The argument can be either a test name or
# a directory name. For the latter case, all
# tests in the directory is run.
run_ginkgo() {
    if [[ -d "$1" ]]; then
        test_dir "$1"
    else # in some subdirectory?
        find_dir
        for d in $dirs; do
            find_test "$d" "$1"
            if [[ $? == 1 ]]; then
                cd "$d"
                test_file "$1" "$filename" 1
                cd ..
                break
            fi
        done
    fi
}

# --------------- main logic -----------------

check_and_build

if ! [[ -f "dirs.txt" ]]; then
    echo Missing dirs.txt. Abort.
    exit 1
fi

# no argument - run all the tests
if [[ $# == 0 ]]; then
    find_dir
    for d in $dirs; do
        run_ginkgo "$d"
    done
else
    for name in $@; do
        run_ginkgo "$name"
    done
fi

total=$(($success + $fail))
echo "$total case(s) are tested, $success succeeded and $fail failed."
