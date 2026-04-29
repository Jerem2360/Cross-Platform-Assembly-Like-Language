#include "Operator.hpp"


namespace cpasm {
	static Operator _operators[] = {
		{"+=", Arity::BINARY, OperationType::INSTRUCTION },
		{"-=", Arity::BINARY, OperationType::INSTRUCTION },
		{"*=", Arity::BINARY, OperationType::INSTRUCTION },
		{"/=", Arity::BINARY, OperationType::INSTRUCTION },
		{"<-", Arity::BINARY, OperationType::INSTRUCTION },
		{"++", Arity::UNARY, OperationType::INSTRUCTION },
		{"--", Arity::UNARY, OperationType::INSTRUCTION },
		{"^=", Arity::BINARY, OperationType::INSTRUCTION },
		{"&=", Arity::BINARY, OperationType::INSTRUCTION },
		{"|=", Arity::BINARY, OperationType::INSTRUCTION },
		{"%=", Arity::BINARY, OperationType::INSTRUCTION },
		{"/*", Arity::NULLARY, OperationType::MARKER },
		{"*/", Arity::NULLARY, OperationType::MARKER },
		{"//", Arity::NULLARY, OperationType::MARKER },
		{"->", Arity::NULLARY, OperationType::MARKER },
		{">=", Arity::BINARY, OperationType::COMPARISON },
		{"<=", Arity::BINARY, OperationType::COMPARISON },
		{"!=", Arity::BINARY, OperationType::COMPARISON },
		{"==", Arity::BINARY, OperationType::COMPARISON },
		{">", Arity::BINARY, OperationType::COMPARISON },
		{"<", Arity::BINARY, OperationType::COMPARISON },
		{"$", Arity::UNARY, OperationType::SEMANTIC },
		{"@", Arity::UNARY, OperationType::SEMANTIC },
		{"%", Arity::UNARY, OperationType::SEMANTIC },
		{"+", Arity::BINARY, OperationType::ADDRESSING },
		{"*", Arity::BINARY, OperationType::ADDRESSING },
		{"-", Arity::BINARY, OperationType::ADDRESSING },
		{"#", Arity::UNARY, OperationType::SEMANTIC },
	};

	array_view<Operator> Operator::Operators = _operators;

	static const Operator* _next_op() {
		static int idx = 0;
		return &_operators[idx++];
	}


	const Operator* Operator::IADD = _next_op();
	const Operator* Operator::ISUB = _next_op();
	const Operator* Operator::IMUL = _next_op();
	const Operator* Operator::IDIV = _next_op();
	const Operator* Operator::MOVE = _next_op();
	const Operator* Operator::INC = _next_op();
	const Operator* Operator::DEC = _next_op();
	const Operator* Operator::IBXOR = _next_op();
	const Operator* Operator::IBAND = _next_op();
	const Operator* Operator::IBOR = _next_op();
	const Operator* Operator::IMOD = _next_op();
	const Operator* Operator::BEG_CMT = _next_op();
	const Operator* Operator::END_CMT = _next_op();
	const Operator* Operator::LINE_CMT = _next_op();
	const Operator* Operator::RETURNS = _next_op();
	const Operator* Operator::GEQ = _next_op();
	const Operator* Operator::LEQ = _next_op();
	const Operator* Operator::NEQ = _next_op();
	const Operator* Operator::EQ = _next_op();
	const Operator* Operator::GT = _next_op();
	const Operator* Operator::LT = _next_op();
	const Operator* Operator::BOX = _next_op();
	const Operator* Operator::REG = _next_op();
	const Operator* Operator::CMP_TIME_VAR = _next_op();
	const Operator* Operator::ADDR_ADD = _next_op();
	const Operator* Operator::ADDR_MUL = _next_op();
	const Operator* Operator::ADDR_SUB = _next_op();
	const Operator* Operator::CUSTOM_INSTR = _next_op();
}

