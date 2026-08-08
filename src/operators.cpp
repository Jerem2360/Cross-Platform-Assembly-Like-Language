#include "operators.hpp"
#include "wordizer/rules.hpp"


namespace cpasm {
    static Registry<Operator> _operators = {};
    RegistryView<Operator> Operators = {&_operators};

    
    Operator::Operator(std::string_view name, u8 arity, OperationType type, OperatorPositioning position) :
        name(name), arity(arity), type(type), position(position)
    {}

    
    const Operator* Operator::add(std::string_view name, u8 arity, OperationType type, OperatorPositioning position) {
        const Operator* res = &_operators.emplace(name, arity, type, position);
        ParsingRules::register_symbol_chars(name);
        return res;
    }


    /*
    Define operators in order of increasing name length to avoid short operators masking longer operators. See Parser::get_operator
    for why. The registry stores items in reverse order of insertion.
    */
    const Operator* Operator::CUSTOM_INSTR = Operator::add("#",  1, OperationType::INSTRUCTION, OperatorPositioning::PREFIX);
    const Operator* Operator::SEMICOLON =    Operator::add(";",  0, OperationType::SEMANTIC);
    const Operator* Operator::BTICK =        Operator::add("`",  0, OperationType::SEMANTIC);
    const Operator* Operator::DQUOTE =       Operator::add("\"", 0, OperationType::SEMANTIC);
    const Operator* Operator::DOT =          Operator::add(".",  0, OperationType::SEMANTIC);
    const Operator* Operator::COLON =        Operator::add(":",  0, OperationType::SEMANTIC);
    const Operator* Operator::CLOSE_CURLY =  Operator::add("}",  0, OperationType::SEMANTIC);
    const Operator* Operator::OPEN_CURLY =   Operator::add("{",  0, OperationType::SEMANTIC);
    const Operator* Operator::CLOSE_SQUARE = Operator::add("]",  0, OperationType::SEMANTIC);
    const Operator* Operator::OPEN_SQUARE =  Operator::add("[",  0, OperationType::SEMANTIC);
    const Operator* Operator::CLOSE_BR =     Operator::add(")",  0, OperationType::SEMANTIC);
    const Operator* Operator::OPEN_BR =      Operator::add("(",  0, OperationType::SEMANTIC);
    const Operator* Operator::COMMA =        Operator::add(",",  0, OperationType::SEMANTIC);
    const Operator* Operator::ADDR_SUB =     Operator::add("-",  2, OperationType::ADDRESSING);
    const Operator* Operator::ADDR_MUL =     Operator::add("*",  2, OperationType::ADDRESSING);
    const Operator* Operator::ADDR_ADD =     Operator::add("+",  2, OperationType::ADDRESSING);
    const Operator* Operator::CMP_TIME_VAR = Operator::add("%",  1, OperationType::STORAGE,     OperatorPositioning::PREFIX);
    const Operator* Operator::REG =          Operator::add("@",  1, OperationType::STORAGE,     OperatorPositioning::PREFIX);
    const Operator* Operator::BOX =          Operator::add("$",  1, OperationType::STORAGE,     OperatorPositioning::PREFIX);
    const Operator* Operator::LT =           Operator::add("<",  2, OperationType::CONDITION);
    const Operator* Operator::GT =           Operator::add(">",  2, OperationType::CONDITION);
    const Operator* Operator::EQ =           Operator::add("==", 2, OperationType::CONDITION);
    const Operator* Operator::NEQ =          Operator::add("!=", 2, OperationType::CONDITION);
    const Operator* Operator::LEQ =          Operator::add("<=", 2, OperationType::CONDITION);
    const Operator* Operator::GEQ =          Operator::add(">=", 2, OperationType::CONDITION);
    const Operator* Operator::OPEN_CMT =     Operator::add("/*", 0, OperationType::SEMANTIC);
    const Operator* Operator::CLOSE_CMT =    Operator::add("*/", 0, OperationType::SEMANTIC);
    const Operator* Operator::LINE_CMT =     Operator::add("//", 0, OperationType::SEMANTIC);
    const Operator* Operator::RETURNS =      Operator::add("->", 2, OperationType::SEMANTIC);
    const Operator* Operator::IMOD =         Operator::add("%=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IBOR =         Operator::add("|=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IBAND =        Operator::add("&=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IBXOR =        Operator::add("^=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::DEC =          Operator::add("--", 1, OperationType::INSTRUCTION, OperatorPositioning::POSTFIX);
    const Operator* Operator::INC =          Operator::add("++", 1, OperationType::INSTRUCTION, OperatorPositioning::POSTFIX);
    const Operator* Operator::MOVE =         Operator::add("<-", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IDIV =         Operator::add("/=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IMUL =         Operator::add("*=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::ISUB =         Operator::add("-=", 2, OperationType::INSTRUCTION);
    const Operator* Operator::IADD =         Operator::add("+=", 2, OperationType::INSTRUCTION);
}
