#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>


#include "Operator.hpp"
#include "parsing/Parser.hpp"
#include "Program.hpp"


namespace cpasm {
	enum class ConstantTokenType {
		NAME,
		STRING,
		CHAR,
		NUMBER,
	};

	struct StringToken {
		std::string value;
		int lineno = 1;

		static Result parse(Parser&, StringToken* out, char delimiter = '"');
	};

	struct ConstantToken {
		std::string value;
		ConstantTokenType type;
		int lineno = 1;

		static Result parse(Parser&, ConstantToken* out);

		SimpleOperand decode(CodeRef) const;
	};

	struct DataTypeToken {
		std::string type_name;
		uint8_t size;
		int lineno = 1;

		static Result parse(Parser&, DataTypeToken* out);
		DataType decode() const;
	};

	struct DeclarationToken {
		std::string name;
		DataTypeToken type;
		int lineno = 1;

		static Result parse(Parser&, DeclarationToken* out);
	};

	struct VarDeclarationToken {
		bool is_box;  // true: BOX, false: REGISTER
		DeclarationToken decl;
		int lineno = 1;

		static Result parse(Parser&, VarDeclarationToken* out);

		bool decode(CodeRef) const;
	};

	struct VarRefToken {
		bool is_box;
		std::string name;
		int lineno = 1;

		static Result parse(Parser&, VarRefToken* out);

		SimpleOperand decode(CodeRef) const;
	};

	struct SimpleOperandToken {
		std::variant<VarRefToken, ConstantToken> value;
		int lineno = 1;

		static Result parse(Parser&, SimpleOperandToken* out);

		SimpleOperand decode(Program*, CodeRef) const;
	};

	struct OperandToken {
		SimpleOperandToken base;
		std::optional<SimpleOperandToken> offset;
		ConstantToken scale;
		std::optional<DataTypeToken> deref_type;
		bool offset_negative;
		int lineno = 1;

		static Result parse(Parser&, OperandToken* out);
		Operand decode(Program*, CodeRef) const;
	};

	/*
	Provides overrides for common function attributes such as calling convention.
	*/
	struct FunctionAttributesToken {
		std::vector<std::string> attrs;

		FuncAttrProperties decode() const;
	};

	struct SimpleOperationInstructionToken {
		OperandToken op0;
		const Operator* operation;
		std::optional<OperandToken> op1; // may be null
		int lineno = 1;

		static Result parse(Parser&, SimpleOperationInstructionToken* out);

		bool decode(Program*, CodeRef, int lineno) const;
	};

	struct GotoInstructionToken {
		OperandToken target;
		int lineno = 1;

		static Result parse(Parser&, GotoInstructionToken* out);
		bool decode(Program*, CodeRef, int lineno) const;
	};

	struct IfGotoInstructionToken {
		OperandToken target;
		OperandToken op0;
		const Operator* operation;
		std::optional<OperandToken> op1;  // may be null
		int lineno = 1;

		static Result parse(Parser&, IfGotoInstructionToken* out);
		bool decode(Program*, CodeRef, int lineno) const;
	};

	struct CallInstructionToken {
		OperandToken target;
		std::vector<OperandToken> args;
		std::optional<OperandToken> result;  // may be null
		FunctionAttributesToken attributes;
		int lineno = 1;

		static Result parse(Parser&, CallInstructionToken* out);
		bool decode(Program*, CodeRef, int lineno) const;
	};

	struct ExitInstructionToken {
		ConstantToken exitcode;
		int lineno = 1;

		static Result parse(Parser&, ExitInstructionToken* out);
		bool decode(CodeRef, int lineno) const;
	};

	struct LabelToken {
		std::string name;
		int lineno = 1;

		static Result parse(Parser&, LabelToken* out);
		bool decode(CodeRef, int lineno) const;
	};

	struct ReturnInstructionToken {
		int lineno = 1;

		static Result parse(Parser&, ReturnInstructionToken* out);
		bool decode(CodeRef, int lineno) const;
	};

	struct CustomInstructionToken {
		std::string name;
		std::vector<OperandToken> operands;
		int lineno;

		static Result parse(Parser&, CustomInstructionToken* out);
		bool decode(CodeRef) const;
	};

	struct InstructionToken {
		using _variant_t = std::variant<
			SimpleOperationInstructionToken,
			GotoInstructionToken,
			IfGotoInstructionToken,
			CallInstructionToken,
			ExitInstructionToken,
			LabelToken,
			ReturnInstructionToken,
			CustomInstructionToken
		>;

		_variant_t value;
		int lineno = 1;

		static Result parse(Parser&, InstructionToken* out);
		bool decode(Program*, CodeRef) const;
	};

	struct FunctionBodyToken {
		std::vector<VarDeclarationToken> declarations;
		std::vector<InstructionToken> instructions;
		int lineno = 1;

		static Result parse(Parser&, FunctionBodyToken* out);
		bool decode(Program*, CodeRef) const;
	};

	struct CompileTimeConditionToken {
		std::string prop;
		const Operator* comparison;
		ConstantToken value;
		int lineno = 1;

		static Result parse(Parser&, CompileTimeConditionToken* out);
		bool decode() const;  // detect if a condition is true
	};

	struct IfStatementToken {
		CompileTimeConditionToken condition;
		int lineno = 1;

		static Result parse(Parser&, IfStatementToken* out);
		bool decode() const;
	};

	struct ElseIfStatementToken {
		CompileTimeConditionToken condition;
		int lineno = 1;

		static Result parse(Parser&, ElseIfStatementToken* out);
		bool decode() const;
	};

	struct ElseStatementToken {
		int lineno = 1;

		static Result parse(Parser&, ElseStatementToken* out);
	};

	struct EndIfStatementToken {
		int lineno = 1;

		static Result parse(Parser&, EndIfStatementToken* out);
	};

	struct ImportStatementToken {  // tell the linker that the program depends on a binary file.
		std::string source;
		std::vector<std::string> symbols;  // may be empty
		int lineno = 1;

		static Result parse(Parser&, ImportStatementToken* out);
		Import decode() const;
	};

	struct FunctionStatementToken {
		std::string name;  // may be empty
		std::vector<VarDeclarationToken> params;  // may be empty
		FunctionBodyToken body;
		std::optional<DataTypeToken> result_type;  // may be null
		FunctionAttributesToken attributes;
		int lineno = 1;
		bool export_;

		static Result parse(Parser&, FunctionStatementToken* out);
		bool decode(Program*, int lineno) const; 
	};

	struct DataDeclarationStatementToken {
		DataTypeToken type;
		ConstantToken value;
		int lineno = 1;

		static Result parse(Parser&, DataDeclarationStatementToken* out);
		bool decode(Program*, int lineno) const;
	};

	struct EntryPointStatementToken {
		FunctionBodyToken body;
		int lineno = 1;

		static Result parse(Parser&, EntryPointStatementToken* out);
		bool decode(Program*) const;
	};

	struct IncludeStatementToken {  // includes the binary content of a file into this code
		std::string path;
		int lineno = 1;

		static Result parse(Parser&, IncludeStatementToken* out);

		bool decode(Program*) const;
	};

	struct SymbolStatementToken {
		std::string name;
		int lineno = 1;
		bool export_;

		static Result parse(Parser&, SymbolStatementToken* out);

		bool decode(Program*) const;
	};

	struct SectionStatementToken {
		std::string name;
		int lineno = 1;

		static Result parse(Parser&, SectionStatementToken* out);
		bool decode(Program*) const;
	};

	struct StatementToken {
		using _variant_t = std::variant<
			IfStatementToken,
			ElseIfStatementToken,
			ElseStatementToken,
			EndIfStatementToken,
			ImportStatementToken,
			FunctionStatementToken,
			DataDeclarationStatementToken,
			EntryPointStatementToken,
			IncludeStatementToken,
			SymbolStatementToken,
			SectionStatementToken
		>;

		_variant_t value;
		int lineno = 1;

		static Result parse(Parser&, StatementToken* out);

		bool is_else_stmt() const;
		bool is_elif_stmt() const;
		bool is_if_stmt() const;
		bool is_endif_stmt() const;

		bool decode(Program*) const;  // for compile time conditions, this returns the result of evaluating the condition
	};

	struct ProgramToken {
		std::vector<StatementToken> statements;

		static Result parse(Parser&, ProgramToken* out);

		Program* decode(std::string_view name, bool use_libc) const;
	};
}

