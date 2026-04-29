#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <functional>
#include <ostream>

#include "helpers.hpp"
#include "StackTrace.hpp"
#include "CPU.hpp"


namespace cpasm {
	struct Code;
	class Program;
	class AssemblyWriter;

	using writer_t = std::function<void(std::ostream&)>;

	static inline void empty_writer(std::ostream& fs) { fs << "<invalid>"; }


	struct CodeRef {
		Program* prog;
		ssize_t idx;

		inline constexpr CodeRef(Program* prog, ssize_t idx) :
			prog(prog), idx(idx)
		{
		}

		inline constexpr CodeRef(std::nullptr_t) :
			CodeRef(nullptr, 0)
		{
		}

		inline constexpr CodeRef() : CodeRef(nullptr) {}

		Code* ptr() const;
		Code* operator->() const;
		Code& operator *() const;

		explicit operator bool() const;
	};


	struct DataType {
		enum Type : uint8_t {
			INVALID,  // object is invalid inside CPU instructions
			INT,
			UINT,
			PTR,
			FLOAT,
		};

		Type type;
		uint8_t size;

		explicit operator bool() const;


		Result check(bool runtime, int lineno) const;

		bool operator ==(const DataType& other) const;
	};

	class SimpleOperand {
		/*
		Type of an operand. The three least significant bits are reserved for constant types,
		while the rest is used for non-constant types
		*/
		enum _Type : uint8_t {
			TY_EMPTY = 0,
			TY_CONST_INT = 0b001,
			TY_CONST_FLOAT = 0b010,
			TY_CONST_STR = 0b011,
			TY_CONST_CHAR = 0b100,
			TY_CONST_LABEL = 0b101,
			TY_BOX = 0b01 << 3,
			TY_REGISTER = 0b10 << 3,
		};

		static constexpr uint8_t _CONST_MASK = 0b111;

		union {
			size_t _integer;
			double _floating;
			std::string _string;
			const CPURegister* _reg;
		};

		_Type _type;
		CodeRef _owner;
		int lineno = 1;

		explicit SimpleOperand(size_t, _Type, int lineno);
		explicit SimpleOperand(double, _Type, int lineno);
		explicit SimpleOperand(std::string_view, _Type, CodeRef owner, int lineno);
		explicit SimpleOperand(const CPURegister*, _Type, CodeRef owner, int lineno);

		constexpr void _copy_from(const SimpleOperand& op, bool construct) {
			switch (op._type) {
			case TY_CONST_STR:
			case TY_CONST_LABEL:
			case TY_BOX:
				break;
			default:
				if (!construct) this->_destroy();
				break;
			}

			auto _old_type = this->_type;
			this->_type = op._type;
			this->_owner = op._owner;
			switch (this->_type) {
			case TY_EMPTY:
				this->_integer = 0;
				break;
			case TY_CONST_INT:
			case TY_CONST_CHAR:
				this->_integer = op._integer;
				break;
			case TY_CONST_FLOAT:
				this->_floating = op._floating;
				break;
			case TY_CONST_STR:
			case TY_CONST_LABEL:
			case TY_BOX:
				if (!construct && (_old_type == TY_CONST_STR))
					this->_string = op._string;
				else
					std::construct_at(&this->_string, op._string);
				break;
			case TY_REGISTER:
				this->_reg = op._reg;
				break;
			}
		}

		constexpr void _move_from(SimpleOperand& op, bool construct) noexcept {
			switch (op._type) {
			case TY_CONST_STR:
			case TY_CONST_LABEL:
			case TY_BOX:
				break;
			default:
				if (!construct) this->_destroy();
				break;
			}

			auto _old_type = this->_type;
			this->_type = op._type;
			this->_owner = op._owner;
			switch (this->_type) {
			case TY_EMPTY:
				this->_integer = 0;
				break;
			case TY_CONST_INT:
			case TY_CONST_CHAR:
				this->_integer = op._integer;
				break;
			case TY_CONST_FLOAT:
				this->_floating = op._floating;
				break;
			case TY_CONST_STR:
			case TY_CONST_LABEL:
			case TY_BOX:
				if (!construct && (_old_type == TY_CONST_STR))
					this->_string = std::move(op._string);
				else
					std::construct_at(&this->_string, std::move(op._string));
				break;
			case TY_REGISTER:
				this->_reg = op._reg;
				break;
			}
		}

		constexpr void _destroy() {
			switch (_type) {
			case TY_CONST_STR:
			case TY_CONST_LABEL:
			case TY_BOX:
				std::destroy_at(&this->_string);
				break;
			default:
				break;
			}
		}

	public:
		constexpr SimpleOperand() :
			_type(TY_EMPTY), _integer(0), _owner(nullptr)
		{
		}

		constexpr SimpleOperand(const SimpleOperand& op) {
			this->_copy_from(op, true);
		}
		constexpr SimpleOperand(SimpleOperand&& op) noexcept {
			this->_move_from(op, true);
		}
		SimpleOperand& operator =(const SimpleOperand&);
		SimpleOperand& operator =(SimpleOperand&&) noexcept;

		static SimpleOperand from_const_int(size_t, int lineno);
		static SimpleOperand from_const_float(double, int lineno);
		static SimpleOperand from_const_str(std::string_view, CodeRef owner, int lineno);
		static SimpleOperand from_const_label(std::string_view, CodeRef owner, int lineno);
		static SimpleOperand from_const_char(char c, int lineno);
		static SimpleOperand from_box(std::string_view, CodeRef owner, int lineno);
		static SimpleOperand from_register(const CPURegister*, CodeRef owner, int lineno);

		bool as_const_int(size_t* out) const;
		bool as_const_float(double* out) const;
		bool as_const_str(std::string_view* out) const;
		bool as_const_label(std::string_view* out) const;
		bool as_const_char(char* out) const;
		bool as_box(std::string_view* out) const;
		bool as_register(const CPURegister** out) const;

		bool is_empty() const;
		bool is_constant() const;
		bool supports_runtime() const;
		bool supports_comptime() const;
		explicit operator bool() const;

		DataType type() const;

		Result check(bool runtime, const Program* prog, const Code* owner) const;

		writer_t writer(const AssemblyWriter*) const;

		bool is_given_register(const CPURegister* reg) const;

		bool is_related_register(const CPURegister* reg) const;

		SimpleOperand toplevel_register() const;

		bool operator ==(const SimpleOperand& other) const;

		SimpleOperand resolve(const Code* ctx) const;

		constexpr ~SimpleOperand() {
			this->_destroy();
		}
	};

	class Operand {
		SimpleOperand _base;
		SimpleOperand _index;
		SimpleOperand _scale;  // must be a constant
		DataType _type;
		int lineno = 1;

	public:
		Operand(SimpleOperand simple);
		Operand();
		Operand(SimpleOperand base, DataType ty, SimpleOperand idx = {}, SimpleOperand scale = {}, int lineno = 1);

		bool is_simple() const;


		static Operand from_const_int(size_t, int lineno = 0);
		static Operand from_const_float(double, int lineno);
		static Operand from_const_str(std::string_view, CodeRef owner, int lineno);
		static Operand from_const_label(std::string_view, CodeRef owner, int lineno);
		static Operand from_const_char(char c, int lineno);
		static Operand from_box(std::string_view name, CodeRef owner, int lineno);
		static Operand from_register(const CPURegister*, CodeRef owner = nullptr, int lineno = 0);
		static Operand from_deref_register(const CPURegister*, CodeRef owner = nullptr, int lineno = 0);

		bool as_const_int(size_t* out) const;
		bool as_const_float(double* out) const;
		bool as_const_str(std::string_view* out) const;
		bool as_const_label(std::string_view* out) const;
		bool as_const_char(char* out) const;
		bool as_box(std::string_view* out) const;
		bool as_register(const CPURegister** out) const;
		bool as_memop(SimpleOperand* base, SimpleOperand* idx, SimpleOperand* scale) const;

		bool is_empty() const;
		bool is_constant() const;
		bool supports_runtime() const;
		bool supports_comptime() const;
		Operand toplevel_register() const;
		explicit operator bool() const;

		DataType type() const;

		Result check(bool runtime, const Program*, const Code* code) const;

		writer_t writer(const AssemblyWriter*) const;

		bool is_given_register(const CPURegister* reg) const;
		bool is_related_register(const CPURegister* reg) const;

		bool operator ==(const Operand& other) const;

		Operand resolve(const Code* ctx) const;
	};
}

