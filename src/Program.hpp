#pragma once
#include <vector>
#include <optional>
#include <map>
#include <iostream>
#include <memory>

#include "helpers.hpp"
#include "CPU.hpp"
#include "RegisterOccupation.hpp"
#include "Implementation.hpp"
#include "StackTrace.hpp"
#include "Operator.hpp"
#include "Operand.hpp"
#include "AsmWriter.hpp"


namespace cpasm {

	struct Import {
		std::string name;
		std::vector<std::string> symbols;

		Result check() const;

		bool generate(const AssemblyWriter&) const;
	};

	struct VarName {
		bool is_box;
		std::string name;  // todo: decide whether to store this inside the program

		Result check() const;
	};

	struct FuncAttrProperties {
		const CallingConvention* callconv;
		// add additional properties that are dictated by attributes here
	};

	struct Instruction {
		enum class Type : uint8_t {
			INVALID,
			SIMPLE,
			GOTO,
			IF_GOTO,
			CALL,
			EXIT,
			RETURN,
		};

		Type type;
		std::vector<Operand> operands;
		intptr_t extra;
		int lineno;

		void post_decode(Program* prog);
		Result check(const Program*, const Code*) const;

		bool generate(AssemblyWriter&, std::string_view epilog_lbl, const Code*, const Program*) const;
	};


	struct CustomInstruction {
		std::string name;
		std::vector<Operand> operands;
		int lineno;

		bool generate(AssemblyWriter&, const Code*) const;
	};

	enum class CodeElementType : uint8_t {
		DECLARATION,
		LABEL,
		INSTRUCTION,
		CUSTOM_INSTR,
	};

	struct Box {
		const CPURegister* reg = nullptr;
		DataType type = { DataType::INVALID, 0 };
		bool is_param = false;
	};

	struct FuncParam {
		VarName name;
		DataType type;

		Result check(int lineno) const;
	};

	struct Code {
		std::vector<Selector<CodeElementType>> elem_order;

		std::vector<Instruction> instructions;
		std::vector<std::string> labels;
		std::vector<CustomInstruction> custom_instrs;

		RegisterOccupation integral_regs;
		RegisterOccupation float_regs;
		RegisterOccupation mixed_regs;

		std::map<std::string, Box> boxes;

		std::optional<Box> result_box = std::nullopt;

		FuncAttrProperties funcattr_props;

		Operand reserve_param(const CPURegister* where, CodeRef code, const FuncParam& param, int lineno);
		Operand reserve_register(const CPURegister*, CodeRef, int lineno);
		Operand reserve_box(std::string_view name, DataType ty, CodeRef, int lineno);
		void set_result_location(const CPURegister* where, DataType ty);

		bool box_exists(std::string_view name) const;

		bool label_exists(std::string_view name) const;

		void post_decode(Program*);
		Result check(const Program*) const;

		bool instruction(Instruction::Type type, const std::vector<Operand>& operands, int lineno, intptr_t extra = 0);
		bool label(std::string_view name, int lineno);
		bool custom_instruction(std::string_view name, const std::vector<Operand>& operands, int lineno);

		bool generate(AssemblyWriter&, size_t num_params, CodeRef ref, bool entry) const;

		bool gen_function_call(AssemblyWriter&, const Operand& target, const Operand& return_location, array_view<Operand> args, bool never_returns = false, std::string_view callconv = "default") const;
		void set_func_attrs(FuncAttrProperties attr_props);

		Code();
	};

	struct Function {
		CodeRef code;
		std::vector<std::string> attributes;
		std::vector<FuncParam> params;
		std::string name;

		void post_decode();
		Result check(const Program*, int lineno) const;
		bool generate(AssemblyWriter&) const;
	};

	struct DataDeclaration {
		DataType datatype;
		SimpleOperand value;  // must be a constant

		Result check(const Program*, int lineno) const;
		bool generate(const AssemblyWriter&) const;
	};

	enum class StatementType : uint8_t {
		IMPORT,
		FUNCTION,
		DATA_DECL_INIT,
		DATA_DECL_UNINIT,
		ENTRY_POINT,
		INCLUDE,
		SYMBOL,
		SECTION,
	};


	struct EntryPoint {
		CodeRef code;
		std::string_view name;

		EntryPoint(Program* owner, bool use_libc);
		EntryPoint();

		Code* operator->();
		const Code* operator->() const;

		void post_decode();
		Result check(const Program*) const;
		bool generate(AssemblyWriter&, bool use_libc) const;

		explicit operator bool() const;
	};


	class Program {
		friend struct CodeRef;

		std::vector<Selector<StatementType>> _stmt_order;

		std::vector<Import> _imports;
		std::vector<std::string> _exports;
		std::vector<std::string> _symbols;
		std::vector<Function> _funcs;
		std::vector<std::string> _includes;

		std::vector<DataDeclaration> _initialized_decls;
		std::vector<DataType> _uninitialized_decls;

		std::vector<std::string> _sections;

		EntryPoint _entry;
		bool _use_libc;
		std::vector<Code> _code_objects;
		std::string _filename;

	public:
		Program(std::string_view filename, bool use_libc);
		void add_import(std::string_view name, array_view<std::string> symbols, int lineno);
		void add_symbol(std::string_view name, int lineno);
		void add_function(CodeRef code, std::string_view name, array_view<std::string> attributes, array_view<FuncParam> params, DataType return_type, int lineno);
		void add_include(std::string name, int lineno);  // TODO: change this to an std::string_view ?
		void add_decl(DataType ty, SimpleOperand init, int lineno);
		//void set_entry_point(CodeRef source);
		CodeRef add_entry_point();
		void add_export(std::string_view name);
		void add_section(std::string_view name, int lineno = 1);

		bool symbol_exists(std::string_view name) const;
		bool get_function(std::string_view name, const Function** out) const;

		void post_decode();
		Result check() const;
		bool generate(AssemblyWriter&) const;

		CodeRef create_empty_code();
		inline const Code* _entry_point() const {
			return this->_entry.operator->();
		}
		//inline Code* _entry_point() {
		//	if (this->_entry_point_idx >= 0)
		//		return &this->_code_objects[this->_entry_point_idx];
		//	return nullptr;
		//}
	};
}

