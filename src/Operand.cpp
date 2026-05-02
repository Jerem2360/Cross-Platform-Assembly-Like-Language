#include "Operand.hpp"
#include "Program.hpp"
#include "Implementation.hpp"
#include "AsmWriter.hpp"


namespace cpasm {
	Code* CodeRef::ptr() const {
		if (!this->prog)
			return nullptr;
		//if (idx < 0)
		//	return &prog->_code_objects[this->prog->_entry_point_idx];
		return &prog->_code_objects[this->idx];
	}
	Code* CodeRef::operator->() const {
		return this->ptr();
	}
	Code& CodeRef::operator*() const {
		return *this->ptr();
	}
	CodeRef::operator bool() const {
		return this->prog != nullptr;
	}


	DataType::operator bool() const {
		return this->type != Type::INVALID;
	}

	Result DataType::check(bool runtime, int lineno) const {
		switch (this->type) {
		case Type::INVALID:
			if (runtime)
				return Result::fail("Undefined datatype is not allowed at runtime.", lineno);
			return {};
		case Type::INT:
		case Type::UINT:
			if (!this->size)
				return Result::fail("Int and UInt types require that a size be provided.", lineno);
			return {};
		case Type::FLOAT:
			if (this->size < 4)
				return Result::fail("Float type must be at least 4 bytes.", lineno);
			if (!this->size)
				return Result::fail("Float type requires that a size be provided.", lineno);
			return {};
		case Type::PTR:
			if (this->size != CurrentImpl.pointer_size())
				return Result::fail("Cannot specify a size for pointer type.", lineno);
			return {};
		default:
			return {};
		}
	}

	bool DataType::operator==(const DataType& other) const {
		return (this->type == other.type) && (this->size == other.size);
	}

	SimpleOperand::SimpleOperand(size_t val, _Type ty, int lineno) :
		_integer(val), _type(ty), _owner(nullptr), lineno(lineno)
	{
	}
	SimpleOperand::SimpleOperand(double val, _Type ty, int lineno) :
		_floating(val), _type(ty), _owner(nullptr), lineno(lineno)
	{
	}
	SimpleOperand::SimpleOperand(std::string_view val, _Type ty, CodeRef owner, int lineno) :
		_string(val.data(), val.size()), _type(ty), _owner(owner), lineno(lineno)
	{
	}
	SimpleOperand::SimpleOperand(const CPURegister* val, _Type ty, CodeRef owner, int lineno) :
		_reg(val), _type(ty), _owner(owner), lineno(lineno)
	{
	}

	SimpleOperand SimpleOperand::from_const_int(size_t value, int lineno) {
		return SimpleOperand(value, TY_CONST_INT, lineno);
	}
	SimpleOperand SimpleOperand::from_const_float(double value, int lineno) {
		return SimpleOperand(value, TY_CONST_FLOAT, lineno);
	}
	SimpleOperand SimpleOperand::from_const_str(std::string_view value, CodeRef owner, int lineno) {
		return SimpleOperand(value, TY_CONST_STR, owner, lineno);
	}
	SimpleOperand SimpleOperand::from_const_label(std::string_view name, CodeRef owner, int lineno) {
		return SimpleOperand(name, TY_CONST_LABEL, owner, lineno);
	}
	SimpleOperand SimpleOperand::from_const_char(char value, int lineno) {
		return SimpleOperand(static_cast<size_t>(value), TY_CONST_CHAR, lineno);
	}
	SimpleOperand SimpleOperand::from_box(std::string_view name, CodeRef owner, int lineno) {
		return SimpleOperand(name, TY_BOX, owner, lineno);
	}
	SimpleOperand SimpleOperand::from_register(const CPURegister* reg, CodeRef owner, int lineno) {
		return SimpleOperand(reg, TY_REGISTER, owner, lineno);
	}

	template<class T>
	static bool _get_val(T* dst, const T& src, uint8_t ty_src, uint8_t ty_expected) {
		if (ty_src != ty_expected)
			return false;
		if (dst)
			*dst = src;
		return true;
	}

	bool SimpleOperand::as_const_int(size_t* out) const {
		return _get_val(out, this->_integer, this->_type, TY_CONST_INT);
	}
	bool SimpleOperand::as_const_float(double* out) const {
		return _get_val(out, this->_floating, this->_type, TY_CONST_FLOAT);
	}
	bool SimpleOperand::as_const_str(std::string_view* out) const {
		if (this->_type != TY_CONST_STR)
			return false;
		return _get_val(out, sview(this->_string), this->_type, TY_CONST_STR);
	}
	bool SimpleOperand::as_const_label(std::string_view* out) const {
		if (this->_type != TY_CONST_LABEL)
			return false;
		return _get_val(out, sview(this->_string), this->_type, TY_CONST_LABEL);
	}
	bool SimpleOperand::as_const_char(char* out) const {
		return _get_val(out, static_cast<char>(this->_integer), this->_type, TY_CONST_CHAR);
	}
	bool SimpleOperand::as_box(std::string_view* out) const {
		if (this->_type != TY_BOX)
			return false;
		return _get_val(out, sview(this->_string), this->_type, TY_BOX);
	}
	bool SimpleOperand::as_register(const CPURegister** out) const {
		return _get_val(out, this->_reg, this->_type, TY_REGISTER);
	}

	bool SimpleOperand::is_empty() const {
		return this->_type == TY_EMPTY;
	}
	bool SimpleOperand::is_constant() const {
		return this->_type & _CONST_MASK;
	}
	bool SimpleOperand::supports_runtime() const {
		return !this->is_empty() && (this->_type != TY_CONST_STR);
	}
	bool SimpleOperand::supports_comptime() const {
		return this->is_constant();
	}
	SimpleOperand::operator bool() const {
		return !this->is_empty();
	}
	DataType SimpleOperand::type() const {
		switch (this->_type) {
		case TY_CONST_INT:
			return {
				DataType::UINT,
				0
			};
		case TY_CONST_CHAR:
			return {
				DataType::UINT,
				1
			};
		case TY_CONST_FLOAT:
			return {
				DataType::FLOAT,
				0
			};
		case TY_CONST_LABEL:
			return {
				DataType::PTR,
				CurrentImpl.pointer_size()
			};
		case TY_BOX:
		{
			const Box& box = map_get(this->_owner->boxes, this->_string, Box{ nullptr, {} });
			return box.type;
		}

		case TY_REGISTER:
			return {
				this->_reg->support.supportsFloatVar() ? DataType::FLOAT : DataType::UINT,
				this->_reg->size
			};
		default:
			return {
				DataType::INVALID,
				0
			};
		}
	}
	SimpleOperand& SimpleOperand::operator=(const SimpleOperand& op) {
		if (this == &op)
			return *this;
		this->_copy_from(op, false);
		return *this;
	}
	SimpleOperand& SimpleOperand::operator=(SimpleOperand&& op) noexcept {
		if (this == &op)
			return *this;
		this->_move_from(op, false);
		return *this;
	}

	Result SimpleOperand::check(bool runtime, const Program* prog, const Code* code) const {
		if (this->_type == TY_EMPTY)
			return Result::fail("Empty operand is not allowed here", this->lineno);

		if (runtime) {
			switch (this->_type) {
			case TY_CONST_LABEL:
				if (!prog->symbol_exists(sview(this->_string)) && !code->label_exists(sview(this->_string))) {
					if (this->_string == "result") {
						if (!code->funcattr_props.callconv)
							return Result::fail("Cannot access 'result' in a function that has no calling convention", this->lineno);
						// TODO: special error message for when function has no return value
					}
					return Result::fail(sfmt("Unknown symbol '", this->_string, "'"), this->lineno);
				}
				return {};
			case TY_CONST_STR:
				return Result::fail("String constants are not allowed inside instructions", this->lineno);
			case TY_BOX:
				if (!code->box_exists(this->_string))
					return Result::fail(sfmt("Unknown box '", this->_string, "'"), this->lineno);
				return {};
			case TY_REGISTER:
				if (!this->_reg)
					return Result::fail("Invalid register", this->lineno);
				return {};
			default:
				return {};
			}
		}
		if (this->_type == TY_CONST_LABEL && !prog->symbol_exists(this->_string))
			return Result::fail(sfmt("Unknown symbol '", this->_string, "'"), this->lineno);

		switch (this->_type) {
		case TY_BOX:
			return Result::fail("Box operands are not allowed at compile time.", this->lineno);
		case TY_REGISTER:
			return Result::fail("Register operands are not allowed at compile time.", this->lineno);
		default:
			return {};
		}
	}
	
	writer_t SimpleOperand::writer(const AssemblyWriter* writer) const {
		size_t int_val;
		if (this->as_const_int(&int_val)) {
			return [=](std::ostream&) {
				writer->const_int(int_val);
			};
		}
		double fl_val;
		if (this->as_const_float(&fl_val)) {
			return [=](std::ostream&) {
				writer->const_float(fl_val);
			};
		}
		std::string_view str_val;
		if (this->as_const_str(&str_val)) {
			std::string val0 = scopy(str_val);
			return [=](std::ostream&) {
				writer->const_str(sview(val0));
			};
		}
		if (this->as_const_label(&str_val)) {
			bool local = this->_owner ? this->_owner->label_exists(str_val) : false;
			std::string val1 = scopy(str_val);
			return [=](std::ostream&) {
				local ? writer->use_local_label(sview(val1)) : writer->use_global_label(sview(val1));
			};
		}
		if (this->as_box(&str_val)) {
			const CPURegister* reg = this->_owner->boxes[scopy(str_val)].reg;
			return [=](std::ostream&) {
				writer->register_(reg);
			};
		}
		char chr_val;
		if (this->as_const_char(&chr_val)) {
			return [=](std::ostream&) {
				writer->const_char(chr_val);
			};
		}
		const CPURegister* reg;
		if (this->as_register(&reg)) {
			return [=](std::ostream&) {
				writer->register_(reg);
			};
		}

		return empty_writer;
	}

	bool SimpleOperand::is_given_register(const CPURegister* reg) const {
		const CPURegister* my_reg;
		if (!this->as_register(&my_reg))
			return false;
		return my_reg == reg;
	}

	bool SimpleOperand::is_related_register(const CPURegister* reg) const {
		const CPURegister* my_reg;
		if (!this->as_register(&my_reg))
			return false;
		return my_reg->related(reg);
	}

	SimpleOperand SimpleOperand::toplevel_register() const {
		const CPURegister* my_reg;
		if (!this->as_register(&my_reg))
			return *this;
		return SimpleOperand::from_register(my_reg->toplevel(), this->_owner, this->lineno);
	}

	bool SimpleOperand::operator==(const SimpleOperand& other) const {
		if (this->_type != other._type)
			return false;

		switch (this->_type) {
		case TY_EMPTY:
			return true;
		case TY_CONST_INT:
			return this->_integer == other._integer;
		case TY_CONST_FLOAT:
			return this->_floating == other._floating;
		case TY_CONST_STR:
			return this->_string == other._string;
		case TY_CONST_CHAR:
			return this->_integer == other._integer;
		case TY_CONST_LABEL:
			return this->_string == other._string;
		case TY_BOX:
			return this->_string == other._string;
		case TY_REGISTER:
			return this->_reg == other._reg;
		default:
			return false;
		}
	}

	SimpleOperand SimpleOperand::resolve() const {
		std::string_view name;
		if (!this->as_box(&name))
			return *this;

		if (!this->_owner)
			return *this;

		const CPURegister* reg = map_get(this->_owner->boxes, scopy(name), Box{}).reg;
		return SimpleOperand::from_register(reg, this->_owner, this->lineno);
	}

	Operand::Operand(SimpleOperand simple) :
		_base(simple), _index(), _scale(), _type()
	{
	}

	Operand::Operand() :
		_base(), _index(), _scale(), _type()
	{
	}
	Operand::Operand(SimpleOperand base, DataType ty, SimpleOperand idx, SimpleOperand scale, int lineno) :
		_base(base), _type(ty), _index(idx), _scale(scale), lineno(lineno)
	{
	}
	bool Operand::is_simple() const {
		return !this->_type;
	}
	Operand Operand::from_const_int(size_t value, int lineno) {
		return Operand(SimpleOperand::from_const_int(value, lineno));
	}
	Operand Operand::from_const_float(double value, int lineno) {
		return Operand(SimpleOperand::from_const_float(value, lineno));
	}
	Operand Operand::from_const_str(std::string_view value, CodeRef owner, int lineno) {
		return Operand(SimpleOperand::from_const_str(value, owner, lineno));
	}
	Operand Operand::from_const_label(std::string_view name, CodeRef owner, int lineno) {
		return Operand(SimpleOperand::from_const_label(name, owner, lineno));
	}
	Operand Operand::from_const_char(char value, int lineno) {
		return Operand(SimpleOperand::from_const_char(value, lineno));
	}
	Operand Operand::from_box(std::string_view name, CodeRef owner, int lineno) {
		return Operand(SimpleOperand::from_box(name, owner, lineno));
	}
	Operand Operand::from_register(const CPURegister* value, CodeRef owner, int lineno) {
		return Operand(SimpleOperand::from_register(value, owner, lineno));
	}
	Operand Operand::from_deref_register(const CPURegister* reg, CodeRef owner, int lineno) {
		return Operand(
			SimpleOperand::from_register(reg, owner, lineno),
			{ reg->support.supportsFloatVar() ? DataType::FLOAT : DataType::INT, reg->size },
			{},
			{},
			lineno
		);
	}

	bool Operand::as_const_int(size_t* out) const {
		if (this->is_simple())
			return this->_base.as_const_int(out);
		return false;
	}
	bool Operand::as_const_float(double* out) const {
		if (this->is_simple())
			return this->_base.as_const_float(out);
		return false;
	}
	bool Operand::as_const_str(std::string_view* out) const {
		if (this->is_simple())
			return this->_base.as_const_str(out);
		return false;
	}
	bool Operand::as_const_label(std::string_view* out) const {
		if (this->is_simple())
			return this->_base.as_const_label(out);
		return false;
	}
	bool Operand::as_const_char(char* out) const {
		if (this->is_simple())
			return this->_base.as_const_char(out);
		return false;
	}
	bool Operand::as_box(std::string_view* out) const {
		if (this->is_simple())
			return this->_base.as_box(out);
		return false;
	}
	bool Operand::as_register(const CPURegister** out) const {
		if (this->is_simple())
			return this->_base.as_register(out);
		return false;
	}
	bool Operand::as_memop(SimpleOperand* base, SimpleOperand* index, SimpleOperand* scale) const {
		if (this->is_simple())
			return false;
		if (base)
			*base = this->_base;
		if (index)
			*index = this->_index;
		if (scale)
			*scale = this->_scale;
		return true;
	}

	bool Operand::is_empty() const {
		if (this->is_simple())
			return this->_base.is_empty();
		return false;
	}
	bool Operand::is_constant() const {
		if (this->is_simple())
			return this->_base.is_constant();
		return false;
	}
	bool Operand::supports_runtime() const {
		if (this->is_simple())
			return this->_base.supports_runtime();
		return true;
	}
	bool Operand::supports_comptime() const {
		if (this->is_simple())
			return this->_base.supports_comptime();
		return false;
	}
	Operand::operator bool() const {
		return !this->is_empty();
	}
	DataType Operand::type() const {
		if (this->is_simple())
			return this->_base.type();
		return this->_type;
	}

	static constexpr size_t _pow2(uint8_t n) {
		size_t val = 1;
		for (int i = 0; i < n; i++)
			val *= 2;
		return val;
	}

	static constexpr size_t _powers_of_2[] = {
		_pow2(0),
		_pow2(1),
		_pow2(2),
		_pow2(3),
		_pow2(4),
		_pow2(5),
		_pow2(6),
		_pow2(7),
		_pow2(8),
		_pow2(9),
		_pow2(10),
		_pow2(11),
		_pow2(12),
		_pow2(13),
		_pow2(14),
		_pow2(15),
		_pow2(16),
		_pow2(17),
		_pow2(18),
		_pow2(19),
		_pow2(20),
	};

	static bool _is_pow2(size_t num) {
		for (int i = 0; i < array_len(_powers_of_2); i++) {
			if (num == _powers_of_2[i])
				return true;
		}
		return false;
	}

	static bool _check_scale(size_t scale) {
		return (scale > 0) && (scale <= 8) && _is_pow2(scale);
	}

	Result Operand::check(bool runtime, const Program* program, const Code* code) const {
		if (this->is_simple()) {
			return this->_base.check(runtime, program, code);
		}
		if (!runtime)
			return Result::fail("Dereferences are only allowed inside code blocks", this->lineno);

		auto res = this->_base.check(runtime, program, code);
		if (!res)
			return res.push("Invalid deref base");
		if (this->_base.type().size != CurrentImpl.pointer_size())
			return Result::fail(sfmt("Can only dereference pointer-sized (", (int)CurrentImpl.pointer_size() * 8, "bits) operands, found ", (int)this->_base.type().size * 8, "bit operand instead"), this->lineno);
		if (this->_index)
			res = this->_index.check(runtime, program, code);
		if (!res)
			return res.push("Invalid deref index");
		if (this->_scale)
			res = this->_scale.check(runtime, program, code);
		if (!res)
			return res.push("Invalid deref scale");
		size_t val = 1;
		if (this->_scale && !this->_scale.as_const_int(&val)) {
			return Result::fail("Scale must be a constant integer", this->lineno);
		}
		if (!_check_scale(val)) {
			return Result::fail(sfmt("Invalid scale: scale must be a power of 2 between 1 and 8, not '", val, "'"), this->lineno);
		}
		res = this->_type.check(runtime, this->lineno);
		if (!res)
			return res;

		return {};
	}

	writer_t Operand::writer(const AssemblyWriter* writer) const {
		if (this->is_simple())
			return this->_base.writer(writer);

		const SimpleOperand base = this->_base;
		const SimpleOperand index = this->_index;
		const SimpleOperand scale = this->_scale;
		uint8_t sz = this->_type.size;

		return [=](std::ostream&) {
			writer->deref(base, index, scale, sz);
		};
	}

	bool Operand::is_given_register(const CPURegister* reg) const {
		if (!this->is_simple())
			return false;
		return this->_base.is_given_register(reg);
	}

	bool Operand::is_related_register(const CPURegister* reg) const {
		if (!this->is_simple())
			return false;
		return this->_base.is_related_register(reg);
	}
	bool Operand::operator==(const Operand& other) const {
		if (this->is_simple() && other.is_simple())
			return this->_base == other._base;

		if (this->_base != other._base)
			return false;
		if (this->_index != other._index)
			return false;
		if (this->_scale != other._scale)
			return false;
		if (this->_type != other._type)
			return false;
		return true;
	}

	Operand Operand::resolve() const {
		return Operand(
			this->_base.resolve(),
			this->_type,
			this->_index.resolve(),
			this->_scale.resolve(),
			this->lineno
		);
	}
	Operand Operand::toplevel_register() const {
		if (!this->is_simple())
			return *this;
		return Operand(this->_base.toplevel_register());
	}
}

