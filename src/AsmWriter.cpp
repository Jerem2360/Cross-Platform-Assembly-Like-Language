#include "AsmWriter.hpp"
#include "Implementation.hpp"


namespace cpasm {
	AssemblyWriter::AssemblyWriter(std::ostream& out) :
		_output(out), 
		_asm_funcs(CurrentImpl.assembler_functions()), 
		_arch_funcs(CurrentImpl.arch_functions()),
		_env_funcs(CurrentImpl.env_functions()),
		_stack_offset(0),
		_callconv(nullptr)
	{ }
	void AssemblyWriter::enter_code_block(const CallingConvention* callconv) {
		if (!callconv)
			callconv = CurrentImpl.dflt_calling_convention();

		this->_tmp_regs = std::move(RegisterOccupation(callconv->temp_rules->tmp_registers));
		this->_fp_tmp_regs = std::move(RegisterOccupation(callconv->temp_rules->fp_tmp_registers));
		this->_callconv = callconv;
		this->_stack_offset = callconv->funcentry_stack_align;
	}
	TmpRegWrapper AssemblyWriter::wrap_tmp(const Operand& src, bool push, bool pull, TmpRegFlags accept, uint8_t size) {
		bool floatness = src.type().type == DataType::Type::FLOAT;
		bool copy = false;
		if (accept.backs_consts() && src.is_constant())
			copy = true;
		if (accept.backs_memory() && !src.is_simple())
			copy = true;
		if (accept.backs_diffreg() && src.as_register(nullptr) && (floatness != accept.is_FP_backed()))
			copy = true;
		auto res = TmpRegWrapper(
			*this, 
			accept.is_FP_backed() ? this->_fp_tmp_regs : this->_tmp_regs, 
			src, 
			nullptr, 
			copy, 
			push, 
			pull, 
			src.type().size ? src.type().size : size
		);

		const CPURegister* tmp = res.get_tmp();
		if (!tmp)
			return res;

		auto it = std::find(this->_tmp_usage.begin(), this->_tmp_usage.end(), tmp);
		if (it == this->_tmp_usage.end())
			this->_tmp_usage.push_back(tmp);

		return res;
	}
	TmpRegWrapper AssemblyWriter::wrap_given_reg(const Operand& src, const CPURegister* back, bool push, bool pull) {
		bool back_floatness = back->support.supportsFloatVar();
		auto res = TmpRegWrapper(
			*this, 
			back_floatness ? this->_fp_tmp_regs : this->_tmp_regs, 
			src, 
			back, 
			!src.is_given_register(back), 
			push, 
			pull, 
			back->size
		);
		const CPURegister* tmp = res.get_tmp();
		if (!tmp)
			return res;

		auto it = std::find(this->_tmp_usage.begin(), this->_tmp_usage.end(), tmp);
		if (it == this->_tmp_usage.end())
			this->_tmp_usage.push_back(tmp);

		return res;
	}
	void AssemblyWriter::leave_code_block() {
		this->_tmp_regs.release_all();
		this->_fp_tmp_regs.release_all();
		this->_callconv = nullptr;
	}
	
	const CallingConvention* AssemblyWriter::current_callconv() const {
		return this->_callconv;
	}
	unsigned int AssemblyWriter::stack_offset() const {
		return this->_stack_offset;
	}
	void AssemblyWriter::stack_offset(unsigned int to_add) {
		this->_stack_offset += to_add;
	}
	void AssemblyWriter::get_temp_usage(std::vector<const CPURegister*>* out_regs) const {
		*out_regs = this->_tmp_usage;
	}

	bool AssemblyWriter::cpu_instruction(CPUInstruction instr, std::vector<Operand> operands, std::string_view comment) const {
		if (!this->_asm_funcs->cpu_instruction)
			return false;
		std::string_view instr_name = CurrentImpl.instruction_name(instr);
		
		return custom_instruction(instr_name, operands, comment);
	}
	bool AssemblyWriter::custom_instruction(std::string_view name, std::vector<Operand> operands, std::string_view comment) const {
		std::vector<writer_t> op_writers = {};
		op_writers.reserve(operands.size());

		for (auto& op : operands) {
			op_writers.push_back(op.writer(this));
		}

		return this->_asm_funcs->cpu_instruction(this->_output, name, std::move(op_writers), comment);
	}
	bool AssemblyWriter::define_bytes(uint8_t size, const SimpleOperand& value) const {
		if (!this->_asm_funcs->define_bytes)
			return false;
		return this->_asm_funcs->define_bytes(this->_output, size, value.writer(this));
	}
	bool AssemblyWriter::reserve_bytes(uint8_t size) const {
		if (!this->_asm_funcs->reserve_bytes)
			return false;
		return this->_asm_funcs->reserve_bytes(this->_output, size);
	}
	bool AssemblyWriter::set_bitness(uint8_t bits) const {
		if (!this->_asm_funcs->set_bitness)
			return false;
		return this->_asm_funcs->set_bitness(this->_output, bits);
	}
	bool AssemblyWriter::def_global_label(std::string_view name) const {
		if (!this->_asm_funcs->def_global_label)
			return false;
		return this->_asm_funcs->def_global_label(this->_output, name);
	}
	bool AssemblyWriter::def_local_label(std::string_view name) const {
		if (!this->_asm_funcs->def_local_label)
			return false;
		return this->_asm_funcs->def_local_label(this->_output, name);
	}
	bool AssemblyWriter::use_global_label(std::string_view name) const {
		if (!this->_asm_funcs->use_global_label)
			return false;
		return this->_asm_funcs->use_global_label(this->_output, name);
	}
	bool AssemblyWriter::use_local_label(std::string_view name) const {
		if (!this->_asm_funcs->use_local_label)
			return false;
		return this->_asm_funcs->use_local_label(this->_output, name);
	}
	bool AssemblyWriter::include_file(std::string_view name) const {
		if (!this->_asm_funcs->include_file)
			return false;
		return this->_asm_funcs->include_file(this->_output, name);
	}
	bool AssemblyWriter::import_symbol(std::string_view name) const {
		if (!this->_asm_funcs->import_symbol)
			return false;
		return this->_asm_funcs->import_symbol(this->_output, name);
	}
	bool AssemblyWriter::export_symbol(std::string_view name) const {
		if (!this->_asm_funcs->export_symbol)
			return false;
		return this->_asm_funcs->export_symbol(this->_output, name);
	}
	bool AssemblyWriter::const_int(size_t value) const {
		if (!this->_asm_funcs->const_int)
			return false;
		return this->_asm_funcs->const_int(this->_output, value);
	}
	bool AssemblyWriter::const_float(double value) const {
		if (!this->_asm_funcs->const_float)
			return false;
		return this->_asm_funcs->const_float(this->_output, value);
	}
	bool AssemblyWriter::const_char(char value) const {
		if (!this->_asm_funcs->const_char)
			return false;
		return this->_asm_funcs->const_char(this->_output, value);
	}
	bool AssemblyWriter::const_str(std::string_view value) const {
		if (!this->_asm_funcs->const_str)
			return false;
		return this->_asm_funcs->const_str(this->_output, value);
	}

	bool AssemblyWriter::deref(const SimpleOperand& base, const SimpleOperand& index, const SimpleOperand& scale, uint8_t size) const {
		if (!this->_asm_funcs->deref)
			return false;
		return this->_asm_funcs->deref(
			this->_output, 
			base.writer(this), 
			index ? index.writer(this) : writer_t{},
			scale ? scale.writer(this) : writer_t{},
			size
		);
	}
	bool AssemblyWriter::register_(const CPURegister* reg) const {
		if (!this->_asm_funcs->register_)
			return false;
		return this->_asm_funcs->register_(this->_output, reg);
	}
	bool AssemblyWriter::comment(std::string_view text) const {
		if (!this->_asm_funcs->comment)
			return false;
		return this->_asm_funcs->comment(this->_output, text);
	}
	std::string AssemblyWriter::random_identifier(std::string_view base) const {
		if (!this->_asm_funcs->random_identifier)
			return "";
		return this->_asm_funcs->random_identifier(base);
	}
	bool AssemblyWriter::section(std::string_view name) const {
		if (!this->_asm_funcs->section)
			return false;
		return this->_asm_funcs->section(this->_output, name);
	}
	bool AssemblyWriter::enable_relative_addr() const {
		if (!this->_asm_funcs->enable_relative_addr)
			return false;
		return this->_asm_funcs->enable_relative_addr(this->_output);
	}


	static OpFlags _compute_flags(const Operand& lhs, const Operand& rhs) {
		uint8_t size = min(lhs.type().size, rhs.type().size);
		if (!size)
			size = max(lhs.type().size, rhs.type().size);

		std::underlying_type_t<OpFlags::_op_status> float_st = OpFlags::NONE;
		const CPURegister* reg;
		if (lhs.as_register(&reg) && reg->support.supportsFloatVar())
			float_st |= OpFlags::LEFT;
		if (rhs.as_register(&reg) && reg->support.supportsFloatVar())
			float_st |= OpFlags::RIGHT;

		std::underlying_type_t<OpFlags::_op_status> memory_st = OpFlags::NONE;
		if (!lhs.is_simple())
			memory_st |= OpFlags::LEFT;
		if (!rhs.is_simple())
			memory_st |= OpFlags::RIGHT;

		std::underlying_type_t<OpFlags::_op_status> signed_st = OpFlags::NONE;
		if (lhs.type().type == DataType::Type::INT)
			signed_st |= OpFlags::LEFT;
		if (rhs.type().type == DataType::Type::INT)
			signed_st |= OpFlags::RIGHT;

		return OpFlags{
			.size = size,
			.float_st = static_cast<OpFlags::_op_status>(float_st),
			.memory_st = static_cast<OpFlags::_op_status>(memory_st),
			.signed_st = static_cast<OpFlags::_op_status>(signed_st),
		};
	}

	static OpFlags _compute_flags(const Operand& op) {
		const CPURegister* reg;
		bool is_float = false;
		if (op.as_register(&reg))
			is_float = reg->support.supportsFloatVar();

		return OpFlags{
			.size = op.type().size,
			.float_st = is_float ? OpFlags::ALL : OpFlags::NONE,
			.memory_st = op.is_simple() ? OpFlags::NONE : OpFlags::ALL,
			.signed_st = op.type().type == DataType::Type::INT ? OpFlags::ALL : OpFlags::NONE,
		};
	}


	template<class ...AT>
	static bool _call_or_fail(
		bool (*fn)(AssemblyWriter&, AT...),
		AssemblyWriter& writer, AT&& ...args
	) {
		if (!fn)
			return false;
		return fn(writer, std::forward<AT>(args)...);
	}


	bool AssemblyWriter::push(const Operand& operand) {
		int cnt = 0;
		bool res = _call_or_fail(
			this->_arch_funcs->push,
			*this, operand, &cnt
		);
		if (!cnt)
			cnt = operand.type().size;
		// TODO: this assumes that no constants are pushed to the stack. Such a thing may happen if a constant is provided for a stack parameter of a function.
		this->_stack_offset += cnt;
		return res;
	}
	bool AssemblyWriter::pop(const Operand& operand) {
		int cnt = 0;
		bool res = _call_or_fail(
			this->_arch_funcs->pop,
			*this, operand, &cnt
		);
		if (!cnt)
			cnt = operand.type().size;
		// TODO: this assumes that no constants are pushed to the stack. Such a thing may happen if a constant is provided for a stack parameter of a function.
		this->_stack_offset -= cnt;
		return res;
	}
	bool AssemblyWriter::push_amount(size_t cnt, bool nested) {
		if (!nested)
			this->_stack_offset += (int)cnt;
		if (!cnt)
			return true;
		return _call_or_fail(
			this->_arch_funcs->push_amount,
			*this, std::move(cnt)
		);
	}
	bool AssemblyWriter::pop_amount(size_t cnt, bool nested) {
		if (!nested)
			this->_stack_offset += (int)cnt;
		if (!cnt)
			return true;
		return _call_or_fail(
			this->_arch_funcs->pop_amount,
			*this, std::move(cnt)
		);
	}
	bool AssemblyWriter::jump(const Operand& target) {
		return _call_or_fail(
			this->_arch_funcs->jump,
			*this, target
		);
	}
	bool AssemblyWriter::jump_if(const Operand& target, const Operator* op, const Operand& lhs, const Operand& rhs) {
		return _call_or_fail(
			this->_arch_funcs->jump_if,
			*this, target, lhs, rhs, std::move(op), _compute_flags(lhs, rhs)
		);
	}
	bool AssemblyWriter::call(const Operand& target) {
		return _call_or_fail(
			this->_arch_funcs->call,
			*this, target
		);
	}
	bool AssemblyWriter::return_() {
		return _call_or_fail(
			this->_arch_funcs->return_,
			*this
		);
	}
	bool AssemblyWriter::add(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->add,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::sub(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->sub,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::mul(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->mul,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::div(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->div,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::move(const Operand& target, const Operand& src) {
		return _call_or_fail(
			this->_arch_funcs->move,
			*this, target, src, _compute_flags(target, src)
		);
	}
	bool AssemblyWriter::inc(const Operand& target) {
		return _call_or_fail(
			this->_arch_funcs->inc,
			*this, target, _compute_flags(target)
		);
	}
	bool AssemblyWriter::dec(const Operand& target) {
		return _call_or_fail(
			this->_arch_funcs->dec,
			*this, target, _compute_flags(target)
		);
	}
	bool AssemblyWriter::bxor(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->bxor,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::band(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->band,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::bor(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->bor,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::mod(const Operand& target, const Operand& op) {
		return _call_or_fail(
			this->_arch_funcs->mod,
			*this, target, op, _compute_flags(target, op)
		);
	}
	bool AssemblyWriter::exit(const Code* owner, const Operand& exitcode) {
		return _call_or_fail(
			this->_env_funcs->exit_process,
			*this, std::move(owner), exitcode
		);
	}


	AssemblyGenerator::AssemblyGenerator() :
		_ss(), _writer(_ss), _instr_stack(), _labels(), _order()
	{ }

	bool AssemblyGenerator::cpu_instruction(CPUInstruction instr, std::vector<Operand> operands, std::string_view comment) {
		this->_order.emplace_back(ELEM_INSTR, (int)this->_instr_stack.size(), 0);
		this->_instr_stack.emplace_back(InstructionItem{
			.instr = instr,
			.operands = operands,
			.comment = comment
		});
		return true;
	}

	bool AssemblyGenerator::def_local_label(std::string_view name) {
		this->_order.emplace_back(ELEM_LBL, (int)this->_labels.size(), 0);
		this->_labels.emplace_back(scopy(name));
		return true;
	}

	TmpRegWrapper AssemblyGenerator::wrap_tmp(const Operand& src, bool push, bool pull, TmpRegFlags accept, uint8_t size) {
		throw 10;
	}
	TmpRegWrapper AssemblyGenerator::wrap_given_reg(const Operand& src, const CPURegister* back, bool push, bool pull) {
		throw 10;
	}

	void AssemblyGenerator::push() {
		throw 10;
	}


	static const CPURegister* _fit_operand(const CPURegister* reg, const Operand& op) {
		uint8_t sz = op.type().size;
		if (!sz)
			return reg->toplevel();
		return reg->toplevel()->smallest_to_fit(sz);
	}


	TmpRegWrapper::TmpRegWrapper(
		AssemblyWriter& writer, RegisterOccupation& occ, const Operand& src, const CPURegister* backing, bool copy, bool push, bool pull, uint8_t size
	) :
		_aw(&writer), _occupation(&occ), _src(&src), _pull(false), _back(nullptr)
	{
		if (copy) {
			this->_reg = occ.alloc(/*this->_src->type().size ? this->_src->type().size :*/ size);
			this->_status = ST_OWNED_TMP_REG;

			if (backing) {
				this->_aw->move(Operand::from_register(this->_reg->toplevel()), Operand::from_register(backing->toplevel()));
				this->_back = backing;
			}

			if (push) {

				if (backing)
					this->_aw->move(Operand::from_register(_fit_operand(backing, src)), src);
				else 
					this->_aw->move(Operand::from_register(_fit_operand(this->_reg, src)), src);
			}
			this->_pull = pull;
		}
		else {
			//if (backing) {
			//	this->_back = backing;
			//	if (push)
			//		this->_aw->move(Operand::from_register(this->_back), src);
			//	this->_pull = pull;
			//}

			if (this->_src->as_register(&this->_reg)) {
				this->_status = ST_EXISTING_REG;
			}
			else {
				this->_reg = nullptr;
				this->_status = ST_EXISTING_OP;
			}
		}
	}


	TmpRegWrapper::TmpRegWrapper(TmpRegWrapper&& src) noexcept :
		_reg(src._reg), _back(src._back), _src(src._src), _occupation(src._occupation), _aw(src._aw), _pull(src._pull), _status(src._status)
	{
		src._reg = nullptr;
		src._back = nullptr;
		src._src = nullptr;
		src._occupation = nullptr;
		src._aw = nullptr;
		src._pull = false;
		src._status = ST_EXISTING_OP;
	}

	TmpRegWrapper& TmpRegWrapper::operator =(TmpRegWrapper&& src) noexcept {
		if (this == &src)
			return *this;

		this->_reg = src._reg;
		this->_back = src._back;
		this->_src = src._src;
		this->_occupation = src._occupation;
		this->_aw = src._aw;
		this->_pull = src._pull;
		this->_status = src._status;

		src._reg = nullptr;
		src._back = nullptr;
		src._src = nullptr;
		src._occupation = nullptr;
		src._aw = nullptr;
		src._pull = false;
		src._status = ST_EXISTING_OP;

		return *this;
	}


	Operand TmpRegWrapper::get() const {
		if (this->_back)
			return Operand::from_register(_fit_operand(this->_back, *this->_src));
		if (this->_reg)
			return Operand::from_register(_fit_operand(this->_reg, *this->_src));
		return *this->_src;
	}

	const CPURegister* TmpRegWrapper::get_tmp() const {
		return this->_reg;
	}


	TmpRegWrapper& TmpRegWrapper::with_callback(decltype(_destroy_cb) callback) {
		this->_destroy_cb = callback;
		return *this;
	}

	TmpRegWrapper::~TmpRegWrapper() {
		if (this->_pull) {
			if (this->_back)
				this->_aw->move(*this->_src, Operand::from_register(_fit_operand(this->_back, *this->_src)));
			else
				this->_aw->move(*this->_src, Operand::from_register(_fit_operand(this->_reg, *this->_src)));
			this->_pull = false;
		}
		if (this->_status == ST_OWNED_TMP_REG) {
			if (this->_back)
				this->_aw->move(Operand::from_register(this->_back->toplevel()), Operand::from_register(this->_reg->toplevel()));

			this->_occupation->free(this->_reg);
			this->_status = ST_EXISTING_REG;
		}
	}
}

