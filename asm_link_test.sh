#!/bin/bash
nasm -f elf64 test/$1/$1-linux.asm -o test/$1/$1.o
nasm -f elf64 test/lib/lib-linux.asm -o test/lib/lib.o
gcc test/$1/$1.o test/lib/lib.o -o test/$1/$1.elf
