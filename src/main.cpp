#include "wordizer/wordizer.hpp"
#include "wordizer/rules.hpp"
#include "helpers.hpp"
#include "operators.hpp"
#include "parser/parser.hpp"
#include <cstring>
#include <fstream>
#include <utility>


struct CompileRules {
    bool use_libc = true;
    std::string_view input_file = "";
    std::string_view output_file = "";
    std::string_view asm_name = "";
    std::string_view arch_name = "";
    std::string_view env_name = "";
};

enum class ArgFlag : cpasm::u8 {
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
    //if (!out->output_file.size()) {
    //    std::cout << "FATAL: no output file (-o).";
    //    return false;
    //}
    //if (!out->asm_name.size()) {
    //    std::cout << "FATAL: missing assembler name (-asm).";
    //    return false;
    //}
    //if (!out->arch_name.size()) {
    //    std::cout << "FATAL: missing architecture name (-arch).";
    //    return false;
    //}
    //if (!out->env_name.size()) {
    //    std::cout << "FATAL: missing environment name (-env).";
    //    return false;
    //}

    return true;
}


static void print_charword(const cpasm::CharWord* w) {
    using namespace cpasm;

    std::cout << "[TEXT] |" << sslice(vec2sview(w->chars), 0, 100) << "| [TYPE] " << WordType_name(w->type) << " [LINENO] " << w->lineno;
}


static bool wordize(std::istream& in_file, std::string_view filename) {
    using namespace cpasm;

    Wordizer wordizer(in_file);
    
    while (wordizer.next()) {}

    //for (auto& word : wordizer.view()) {
    //    print_charword(&word);
    //    std::cout << '\n';
    //}

    Parser parser = {wordizer.view(), filename};

    std::cout << "File '" << filename << "':\n";

    while (!parser.exhausted()) {
        parser.consume_comment();
        if (parser.exhausted())
            break;
        char c = (char)parser.get_char();

        if (c == '\r')
            std::cout << "[CR]";
        else if (c == '\n')
            std::cout << "[LF]" << c;
        else
            std::cout << c;

        parser.advance_char(1);
    }

    return true;
}

static constexpr bool DEBUG = true;


template<class T>
static consteval T _debug_conditional(T _debug, T _nodebug) {
    if (DEBUG)
        return _debug;
    else 
        return _nodebug;
}


static void init_compiler() {
    using namespace cpasm;

    //ParsingRules::register_symbol_chars("\"'`.;,:/!%\\[](){}$=+-*^|&!");
    SymbolChars.freeze(_debug_conditional("symbol chars", ""));
    Operators.freeze(_debug_conditional("operator", ""));
}


int main(int argc, char** argv) {
    init_compiler();

    CompileRules rules;

    if (!parse_cmdline(&rules, argc, argv))
        return 1;

    auto fs = std::ifstream(rules.input_file.data(), std::ios::in);

    if (!wordize(fs, rules.input_file))
        return false;

    return 0;
}

