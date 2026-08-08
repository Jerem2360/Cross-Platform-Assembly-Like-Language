#pragma once
#include "registry.hpp"


namespace cpasm {
    enum class OperationType : u8 {
        NONE = 0,
		INSTRUCTION,
		CONDITION,
        STORAGE,
		SEMANTIC,
		ADDRESSING,
	};

    enum class OperatorPositioning : u8 {
        NONE = 0,
        PREFIX,
        POSTFIX,
        BOTH,
    };

    struct Operator {
        std::string_view name;
        u8 arity;
        OperationType type;
        OperatorPositioning position;

    //private:
        Operator(std::string_view name, u8 arity, OperationType type, OperatorPositioning position);

    //public:
        static const Operator* add(std::string_view name, u8 arity, OperationType type, OperatorPositioning position = OperatorPositioning::NONE);
        
        static const Operator* CUSTOM_INSTR;
        static const Operator* SEMICOLON;
        static const Operator* BTICK;
        static const Operator* DQUOTE;
        static const Operator* DOT;
        static const Operator* COLON;
        static const Operator* CLOSE_CURLY;
        static const Operator* OPEN_CURLY;
        static const Operator* CLOSE_SQUARE;
        static const Operator* OPEN_SQUARE;
        static const Operator* CLOSE_BR;
        static const Operator* OPEN_BR;
        static const Operator* COMMA;
        static const Operator* ADDR_SUB;
        static const Operator* ADDR_MUL;
        static const Operator* ADDR_ADD;
        static const Operator* CMP_TIME_VAR;
        static const Operator* REG;
        static const Operator* BOX;
        static const Operator* LT;
        static const Operator* GT;
        static const Operator* EQ;
        static const Operator* NEQ;
        static const Operator* LEQ;
        static const Operator* GEQ;
        static const Operator* OPEN_CMT;
        static const Operator* CLOSE_CMT;
        static const Operator* LINE_CMT;
        static const Operator* RETURNS;
        static const Operator* IMOD;
        static const Operator* IBOR;
        static const Operator* IBAND;
        static const Operator* IBXOR;
        static const Operator* DEC;
        static const Operator* INC;
        static const Operator* MOVE;
        static const Operator* IDIV;
        static const Operator* IMUL;
        static const Operator* ISUB;
        static const Operator* IADD;
    };

    extern RegistryView<Operator> Operators;
}

