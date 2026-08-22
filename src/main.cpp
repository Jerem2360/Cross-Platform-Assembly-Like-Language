#include "wordizer/wordizer.hpp"
#include "wordizer/rules.hpp"
#include "helpers.hpp"
#include "operators.hpp"
#include "parser/parser.hpp"
#include <cstring>
#include <fstream>
#include <utility>
#include <string_view>
#include "inline_kv_registry.hpp"
// #include "formatter.hpp"


struct CompileRules {
    bool use_libc = true;
    std::string_view input_file = "";
    std::string_view output_file = "";
    std::string_view asm_name = "";
    std::string_view arch_name = "";
    std::string_view env_name = "";
};

// struct Register;
// struct RegisterRef {
//     std::string_view name;

//     Register* operator *();
//     Register* operator ->();
// };

// struct Register {
//     std::string_view name;
//     uint8_t size;
//     std::string_view parent_name;
//     std::string_view hi_name;
//     std::string_view lo_name;

//     RegisterRef register_();
// };

// cpasm::InlineKeyRegistry<std::string_view, Register, &Register::name> registers;

// Register* RegisterRef::operator *() {
//     return registers.get(this->name);
// }
// Register* RegisterRef::operator ->() {
//     return registers.get(this->name);
// }

// RegisterRef Register::register_() {
//     if (registers.push(*this))
//         return RegisterRef(this->name);
//     return RegisterRef("");
// }


// auto RAX = Register("RAX", 8, "", "", "EAX").register_();
// auto EAX = Register("EAX", 4, "RAX", "", "AX").register_();
// auto AX = Register("AX", 2, "EAX", "AH", "AL").register_();
// auto AL = Register("AL", 1, "AX", "", "").register_();
// auto AH = Register("AH", 1, "AX", "", "").register_();

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

class CA {};
class CB {};
class CD {};

using V = std::variant<CA, CB, CD>;


int main(int argc, char** argv) {
    init_compiler();

    CompileRules rules;

    if (!parse_cmdline(&rules, argc, argv))
        return 1;

    auto fs = std::ifstream(rules.input_file.data(), std::ios::in);

    if (!wordize(fs, rules.input_file))
        return false;

    cpasm::for_var_types<V>([&]<class T>() {
        std::cout << typeid(T).name() << '\n';
    });

    cpasm::for_var_types2<V>([&]<class T>(cpasm::StaticLoopCtx& ctx) {
        std::string_view name = typeid(T).name();
        std::cout << name << '\n';
        if (name == "2CB") {
            ctx.break_();
            return;
        }
    });

    // std::cout << cpasm::format("First={0} && Second={1}\n", 10ull, 'c');

    return 0;
}

