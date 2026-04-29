# Cross-Platform Assembly-Like Language
A small language that wraps assembly with a modular cross-platform layer and a few convenience features.


## Overview

This repository hosts a compiler for a language called  Cross-PLatform Assembly-Like Language (CPlAL).
The compiler is developped in pure C++.


The language's syntax looks alot like that of assembly. For instance, here's what a HelloWorld program looks like:


```
from "c" import puts;  // requires linking with libc

sym message:
    // declares a string where each character is an int.8 (i.e. an 8-bit signed integer)
    data int.8 "HelloWorld!\0";


entry {
    /*
    calls the puts function. 
    "[callconv]" tells the compiler to use the current platform's default calling convention.
    without this, the compiler defaults to using just an assembly 'call' instruction, and doesn't 
    know how to pass the argument.
    */
    call [callconv] puts(message);
}

```

The main difference with assembly (for now) is that the compiler can handle calling conventions if explicitly told to.

The [wiki](https://github.com/Jerem2360/Cross-Platform-Assembly-Like-Language/wiki) contains an exhaustive description
of the currently supported syntax.


## Basic functioning

This compiler takes as an input source files of the CPlAL language, and outputs assembly source files, which the developer can
then assemble and link with the tools they want.

So far, the only supported assembler is NASM. The only supported architecture is x86-64. The only supported environment is
Windows.  

Support for other platforms/assemblers (such as ARM64) and environments (such as posix and bare-metal) are planned for the 
future.

## Usage

Binary distributions for Windows platforms are available in the releases section of this repository.
Support for other cross-compilation platforms (mainly linux) will be added in the future, also in the releases section.

The program is composed of four "parts". 
The source code is organized in such a way that adding support for any platform is a easy as possible, and from that
emerge the concepts of "Assembler", "Architecture" and "Environment".

### Assembler

The developper choses an assembler to turn the output of the compiler into a binary object (.obj on windows, .o elsewhere). 
For the generated assembly to be valid, the compiler thus needs to speak the same language as the assembler.

An "Assembler" module tells the compiler what language the assembler understands and how to generate it.

### Architecture

An "Architecture" module tells the compiler how exacly a specific CPU architecture works. It describes the CPU registers, 
the different instructions that are available, and so on.

### Environment

An "Environment" module tells the compiler how to handle a specific environment in which the program can run. This includes
how to exit the program, how to generate the entry point of the program, and the available calling conventions. 

If the program runs on an operating system, then the environment is almost always related to the operating system in 
question.

Examples of environments include:
- windows
- posix
- bare metal


### Core compiler


The core compiler is responsible for parsing the input files and using the architecture, assembler and environment modules 
to generate the appropriate assembly.


### Command-line syntax

Each module described above defines a name that can be used in the compiler's command line arguments to identify it.

The compiler takes five arguments:
- The input source file
- The assembler `-asm <name>`
- The architecture `-arch <name>`
- The environment `-env <name>`
- The output file `-o <name>`

An additional `--nolibc` flag can be used to tell the compiler that libc will not be linked to the program.
This is required because it dictates the entry point name. Without this flag, the entry point is always `main`.


Example:

```
cpasm.exe main.cpa -asm NASM -arch x86_64 -env win64 -o main.asm
```

The above command receives as input a `main.cpa` file and generates a `main.asm` file containing NASM syntax, built for Windows x86_64. 

After doing this, the developer needs to assemble and link the generated assembly source file in order to execute it.

For example:

```
nasm -f win64 main.asm -o main.obj
```
then,
```
link /entry:main /subsystem:console main.obj kernel32.lib ucrt.lib libcmt.lib
```

Linking with libc is only required if the program uses functions from the C stdlib. However, the program
always requires linkage with the operating system's API to support exiting the process. 

Note that the `main` entrypoint counts as part of libc as long as it follows the libc spec.

