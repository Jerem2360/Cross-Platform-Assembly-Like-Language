#pragma once
#include <string_view>
#include "helpers.hpp"


namespace cpasm {

	/*
	Number of operands taken by an operation
	*/
	enum class Arity : uint8_t {
		NULLARY = 0,
		UNARY,
		BINARY,
		TERNARY,
	};

	enum class OperationType : uint8_t {
		INSTRUCTION = 0,
		COMPARISON,
		SEMANTIC,
		MARKER,
		ADDRESSING,
	};

	struct Operator {
		std::string_view name;
		Arity arity;
		OperationType type;


		static array_view<Operator> Operators;

		static const Operator* IADD;
		static const Operator* ISUB;
		static const Operator* IMUL;
		static const Operator* IDIV;
		static const Operator* MOVE;
		static const Operator* INC;
		static const Operator* DEC;
		static const Operator* IBXOR;
		static const Operator* IBAND;
		static const Operator* IBOR;
		static const Operator* IMOD;
		static const Operator* BEG_CMT;
		static const Operator* END_CMT;
		static const Operator* LINE_CMT;
		static const Operator* RETURNS;
		static const Operator* GEQ;
		static const Operator* LEQ;
		static const Operator* NEQ;
		static const Operator* EQ;
		static const Operator* GT;
		static const Operator* LT;
		static const Operator* BOX;
		static const Operator* REG;
		static const Operator* CMP_TIME_VAR;
		static const Operator* ADDR_ADD;
		static const Operator* ADDR_MUL;
		static const Operator* ADDR_SUB;
		static const Operator* CUSTOM_INSTR;
	};
}

