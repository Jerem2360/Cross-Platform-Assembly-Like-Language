#!/bin/bash

LIST=()

build_one() {
    if [ -z "$2" ]; then
        echo src/$1.cpp to bin/obj/$1.o... 
        g++ -c -std=c++20 src/$1.cpp -o bin/obj/$1.o || exit 1
        LIST+=("bin/obj/$1.o")
    else 
        echo src/$2/$1.cpp to bin/obj/$2-$1.o... 
        g++ -c -std=c++20 src/$2/$1.cpp -o bin/obj/$2-$1.o || exit 1
        LIST+=("bin/obj/$2-$1.o")
    fi
}


build_one AsmWriter
build_one Implementation
build_one Main
build_one Operand
build_one Operator
build_one Program
build_one FuncCall
build_one RegisterOccupation
build_one StackTrace
build_one StringCoding
build_one Tokens
build_one Architecture architecture
build_one x86_64 architecture
build_one Assembler assembler
build_one NASM assembler
build_one Environment environment
build_one Win64 environment
build_one Linux environment
build_one CharGrouper parsing
build_one Parser parsing
build_one Wordizer parsing


echo "linking bin/cropall.elf..."
g++ -std=c++20 "${LIST[@]}" -o bin/cropall.elf

