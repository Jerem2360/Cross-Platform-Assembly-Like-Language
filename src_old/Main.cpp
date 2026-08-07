// Name ideas:
/*
- aerogel
- CALM - Cross-platform Assembly Like Model
*/


#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <cstring>

#include "parsing/Wordizer.hpp"
#include "Operator.hpp"
#include "parsing/Parser.hpp"
#include "Tokens.hpp"
#include "helpers.hpp"
#include "AsmWriter.hpp"


static int compile(std::string_view src_filename, std::istream& fs_in, std::ostream& fs_out, std::string_view arch, std::string_view asm_, std::string_view env, bool use_libc = true) {

    cpasm::CurrentImpl.init(arch, asm_, env);

    cpasm::Wordizer wordizer{ fs_in };
    if (!wordizer.parse()) {
        std::cout << "Failed to parse words!";
        return 1;
    }

    cpasm::Parser parser{ wordizer };
    cpasm::ProgramToken program_tok;

    cpasm::Result res = cpasm::ProgramToken::parse(parser, &program_tok).with_filename(src_filename);
    if (!res) {
        std::cout << "Failed to parse tokens!\n";
        res.write(std::cout);
        return 1;
    }


    cpasm::Program* prog = program_tok.decode(src_filename, use_libc);

    prog->post_decode();

    auto err = prog->check();

    if (!err) {
        err.write(std::cout);
        return 1;
    }

    cpasm::AssemblyWriter writer(fs_out);
    if (!prog->generate(writer))
        return 1;

    return 0;
}


static int compile(std::string_view src_filename, std::string_view out_filename, std::string_view arch, std::string_view asm_, std::string_view env, bool use_libc = true) {
    std::ifstream fs_in(cpasm::scopy(src_filename), std::ios::in);
    std::ofstream fs_out(cpasm::scopy(out_filename), std::ios::out);

    int res = compile(src_filename, fs_in, fs_out, arch, asm_, env, use_libc);
    fs_out.close();
    fs_in.close();
    return res;
}

struct CompileRules {
    bool use_libc = true;
    std::string_view input_file = "";
    std::string_view output_file = "";
    std::string_view asm_name = "";
    std::string_view arch_name = "";
    std::string_view env_name = "";
};

enum class ArgFlag : uint8_t {
    NONE,
    OUT,
    ASM,
    ARCH,
    ENV,
};


static bool parse_cmdline(CompileRules* out, int argc, char** argv) {
    ArgFlag flag = ArgFlag::NONE;

    for (int i = 1; i < argc; i++) {
        std::string_view arg = { argv[i], std::strlen(argv[i]) };

        switch (flag) {
        case ArgFlag::NONE:
            if (arg == "--nolibc") {
                out->use_libc = false;
                continue;
            }
            if (arg == "-asm") {
                flag = ArgFlag::ASM;
                continue;
            }
            if (arg == "-arch") {
                flag = ArgFlag::ARCH;
                continue;
            }
            if (arg == "-env") {
                flag = ArgFlag::ENV;
                continue;
            }
            if (arg == "-o") {
                flag = ArgFlag::OUT;
                continue;
            }
            out->input_file = arg;
            break;
        case ArgFlag::OUT:
            out->output_file = arg;
            flag = ArgFlag::NONE;
            break;
        case ArgFlag::ASM:
            out->asm_name = arg;
            flag = ArgFlag::NONE;
            break;
        case ArgFlag::ARCH:
            out->arch_name = arg;
            flag = ArgFlag::NONE;
            break;
        case ArgFlag::ENV:
            out->env_name = arg;
            flag = ArgFlag::NONE;
            break;
        }
    }

    if (!out->input_file.size()) {
        std::cout << "FATAL: no input file.";
        return false;
    }
    if (!out->output_file.size()) {
        std::cout << "FATAL: no output file (-o).";
        return false;
    }
    if (!out->asm_name.size()) {
        std::cout << "FATAL: missing assembler name (-asm).";
        return false;
    }
    if (!out->arch_name.size()) {
        std::cout << "FATAL: missing architecture name (-arch).";
        return false;
    }
    if (!out->env_name.size()) {
        std::cout << "FATAL: missing environment name (-env).";
        return false;
    }

    return true;
}


int main(int argc, char** argv)
{
    if (argc == 1)
        return compile("test_funcptr.txt", "test_funcptr.asm", "x86_64", "NASM", "win64");

    CompileRules rules;

    if (!parse_cmdline(&rules, argc, argv))
        return 1;

    return compile(rules.input_file, rules.output_file, rules.arch_name, rules.asm_name, rules.env_name, rules.use_libc);
}
