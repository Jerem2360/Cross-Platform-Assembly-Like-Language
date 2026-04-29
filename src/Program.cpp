#include "Program.hpp"
#include <iostream>


namespace cpasm {
	Result Import::check() const {
		return {};
	}

	Result VarName::check() const {
		return {};
	}

	bool Code::box_exists(std::string_view name) const {
		return this->boxes.contains(scopy(name));
	}

	bool Code::label_exists(std::string_view name) const {
		for (auto& lbl : this->labels) {
			if (lbl == name)
				return true;
		}
		return false;
	}
	void Code::post_decode(Program* prog) {
		for (auto& instr : this->instructions) {
			instr.post_decode(prog);
		}
	}
	Result Code::check(const Program* prog) const {
		ResultBunch res;

		for (auto& sel : this->elem_order) {
			if (sel.selector == CodeElementType::INSTRUCTION)
				res += this->instructions[sel.index].check(prog, this);
		}

		for (auto& [k, v] : this->boxes) {
			res += v.type.check(true, 0);
		}

		return res.push("Failed to parse code block");
	}

	bool Code::instruction(Instruction::Type type, const std::vector<Operand>& operands, int lineno, intptr_t extra) {
		this->elem_order.emplace_back(Selector<CodeElementType>{
			CodeElementType::INSTRUCTION, static_cast<int>(this->instructions.size()), 0
		});
		this->instructions.emplace_back(type, operands, extra, lineno);
		return true;
	}

	bool Code::label(std::string_view name, int lineno) {
		this->elem_order.emplace_back(Selector<CodeElementType>{
			CodeElementType::LABEL, static_cast<int>(this->labels.size()), lineno
		});
		this->labels.emplace_back(scopy(name));
		return true;
	}

	bool Code::custom_instruction(std::string_view name, const std::vector<Operand>& operands, int lineno) {
		this->elem_order.emplace_back(Selector<CodeElementType>{
			CodeElementType::CUSTOM_INSTR, static_cast<int>(this->custom_instrs.size()), lineno
		});
		this->custom_instrs.emplace_back(scopy(name), operands, lineno);
		return true;
	}

	Code::Code() :
		integral_regs(CurrentImpl.int_registers()),
		float_regs(CurrentImpl.float_registers()),
		mixed_regs(CurrentImpl.mixed_registers()),
		funcattr_props()
	{ }

	static Result _check_all_instr_operands(const Program* prog, const Code* code, const std::vector<Operand>& operands) {
		
		int i = 0;
		for (auto& op : operands) {
			i++;
			if (!op)
				continue;
			auto res = op.check(true, prog, code);
			if (!res)
				return res.push(sfmt("Failed to check operand ", i));
		}

		return {};
	}


	static Result _check_binary_instr(const Instruction* instr, const Operator* op, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands[0].is_constant())
			return Result::fail(sfmt("Instruction '", op->name, "': target operand cannot be a constant."), lineno);
		bool op0_float = instr->operands[0].type().type == DataType::Type::FLOAT;
		bool op1_float = instr->operands[1].type().type == DataType::Type::FLOAT;
		if (op0_float != op1_float)
			return Result::fail(sfmt("Instruction '", op->name, "': one operand is a float but not the other."), lineno);
		return {};
	}

	static Result _check_unary_instr(const Instruction* instr, const Operator* op, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands[0].is_constant())
			return Result::fail(sfmt("Instruction '", op->name, "': operand cannot be a constant."), lineno);

		if (op == Operator::INC || op == Operator::DEC) {
			if (instr->operands[0].type().type == DataType::Type::FLOAT)
				return Result::fail(sfmt("Instruction '", op->name, "': float operands are not supported."), lineno);
		}

		return {};
	}

	static Result _check_move_instr(const Instruction* instr, const Operator*, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands[0].is_constant())
			return Result::fail("Instruction '<-': operand cannot be a constant.", lineno);

		return {};
	}


	static Result(*_simple_checkers[])(const Instruction*, const Operator*, const Program*, const Code*, int) = {
		&_check_binary_instr,  // +=
		&_check_binary_instr,  // -=
		&_check_binary_instr,  // *=
		&_check_binary_instr,  // /=
		&_check_move_instr,  // <-
		&_check_unary_instr,  // ++
		&_check_unary_instr,  // --
		&_check_binary_instr,  // ^=
		&_check_binary_instr,  // &=
		&_check_binary_instr,  // |=
		&_check_binary_instr,  // %=
	};


	static bool _generate_iadd(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.add(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_isub(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.sub(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_imul(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.mul(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_idiv(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.div(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_mov(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.move(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_inc(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.inc(lhs.resolve(owner));
	}
	static bool _generate_dec(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.dec(lhs.resolve(owner));
	}
	static bool _generate_ixor(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.bxor(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_iand(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.band(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_ior(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.bor(lhs.resolve(owner), rhs.resolve(owner));
	}
	static bool _generate_imod(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, const Code* owner) {
		return out.mod(lhs.resolve(owner), rhs.resolve(owner));
	}


	static bool (*_simple_generators[])(AssemblyWriter&, const Operand& lhs, const Operand& rhs, const Code* owner) = {
		&_generate_iadd,  // +=
		&_generate_isub,  // -=
		&_generate_imul,  // *=
		&_generate_idiv,  // /=
		&_generate_mov,  // <-
		&_generate_inc,  // ++
		&_generate_dec,  // --
		&_generate_ixor,  // ^=
		&_generate_iand,  // &=
		&_generate_ior,  // |=
		&_generate_imod,  // %=
	};


	static Result _check_branch_target(const Operand& target, const Program* prog, const Code* owner, int lineno) {
		if (target.type().type != DataType::Type::PTR)
			return Result::fail("branching: branch target must be pointer-like", lineno);

		std::string_view target_name;
		if (target.as_const_label(&target_name)) {
			if (!owner->label_exists(target_name) && !prog->symbol_exists(target_name))
				return Result::fail(sfmt("branching: unknown target label or symbol '", target_name, "'"), lineno);
		}
		return {};
	}

	static Result _check_invalid_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		return Result::fail("Invalid instruction", lineno);
	}
	static Result _check_simple_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		const Operator* op = reinterpret_cast<const Operator*>(instr->extra);
		if (!op)
			return Result::fail("Simple instruction: invalid instruction operator '<NULL>'", lineno);
		if (op->type != OperationType::INSTRUCTION)
			return Result::fail(sfmt("Simple instruction: invalid instruction operator '", op->name, "'"), lineno);
		if (instr->operands.size() != get_underlying(op->arity))
			return Result::fail(sfmt("Instruction '", op->name, "': expected ", get_underlying(op->arity), " operands, got ", instr->operands.size(), " instead"), lineno);


		if (!instr->operands[0].supports_runtime())
			return Result::fail(sfmt("Instruction '", op->name, "': operand 0 does not support runtime operations"), lineno);

		if (instr->operands.size() > 1 && instr->operands[1] && !instr->operands[1].supports_runtime())
			return Result::fail(sfmt("Instruction '", op->name, "': operand 1 does not support runtime operations"), lineno);

		size_t offset = op - Operator::IADD;
		if (offset < array_len(_simple_checkers)) {
			Result res = _simple_checkers[offset](instr, op, prog, owner, lineno);
			if (!res)
				return res.push("Failed to validate simple instruction.");
		}
		else { 
			std::cout << "Warning: unchecked simple instruction '" << op->name << "' at line " << lineno << ".\n";
		}

		return {};
	}
	static Result _check_goto_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands.size() != 1)
			return Result::fail(sfmt("'goto' accepts exactly one operand, but found ", instr->operands.size()), lineno);
		return _check_branch_target(instr->operands[0], prog, owner, lineno);
	}
	static Result _check_ifgoto_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		const Operator* op = reinterpret_cast<const Operator*>(instr->extra);
		if (!op)
			return Result::fail("'if goto': invalid comparison operator", lineno);

		if (op->type != OperationType::COMPARISON)
			return Result::fail("'if goto': condition must be a comparison", lineno);

		if ((instr->operands.size() - 1) != get_underlying(op->arity))
			return Result::fail(sfmt("'if goto': comparison '", op->name, "' expected ", get_underlying(op->arity), " operands, but found ", instr->operands.size()), lineno);

		if (!instr->operands[1].supports_runtime())
			return Result::fail("Instruction 'if goto': operand 1 does not support runtime operations", lineno);

		if (!instr->operands[2].supports_runtime())
			return Result::fail("Instruction 'if goto': operand 2 does not support runtime operations", lineno);

		return _check_branch_target(instr->operands[0], prog, owner, lineno);
	}
	static Result _check_call_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands.size() < 2)
			return Result::fail(sfmt("Instruction 'call' requires at least one operand, but found ", instr->operands.size()), lineno);
		if (instr->operands[0].type().type != DataType::PTR)
			return Result::fail("Instruction 'call': callee must be a pointer-like object", lineno);
		std::string_view callee_name;
		if (instr->operands[0].as_const_label(&callee_name)) {
			if (!prog->symbol_exists(callee_name))
				return Result::fail(sfmt("Instruction 'call': unknown symbol '", callee_name, "'"), lineno);
		}
		if (!instr->operands[0].supports_runtime())
			return Result::fail("Instruction 'call': operand 0 does not support runtime operations", lineno);
		if (instr->operands[1]) {  // return location
			if (!instr->extra)  // no calling convention
				return Result::fail("Instruction 'call': return location not allowed when no calling convention is used", lineno);
			if (instr->operands[1].is_constant())
				return Result::fail("Instruction 'call': return location cannot be a constant", lineno);
			if (!instr->operands[1].supports_runtime())
				return Result::fail("Instruction 'call': operand 1 does not support runtime operations", lineno);
		}
		size_t arg_count = instr->operands.size() - 2;
		if (arg_count && !instr->extra)  // if args are provided and not using a calling convention
			return Result::fail("Instruction 'call': cannot pass arguments when no calling convention is used", lineno);
		const Function* callee;
		if (callee_name.size() && prog->get_function(callee_name, &callee)) {
			if (arg_count != callee->params.size())
				return Result::fail(sfmt("Instruction 'call': function '", callee_name, "' accepts ", callee->params.size(), " arguments, but found ", arg_count), lineno);

			for (int i = 0; i < callee->params.size(); i++) {
				/* 
				If an argument is smaller than its parameter, a move instruction would only move the bytes of the argument,
				thus leaving garbage in the higher bits of the parameter. Should that quirk be left to the user ?
				*/
				if (instr->operands[i + 2].type().size && instr->operands[i + 2].type().size < callee->params[i].type.size) {
					return Result::fail(sfmt("Instruction 'call': argument ", i+1, " of size ", instr->operands[i + 2].type().size, "is incompatible with parameter of size ", callee->params[i].type.size, "; argument size must be >= parameter size"), lineno);
				}
			}
		}
		return {};

	}
	static Result _check_exit_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		if (instr->operands.size() > 1)
			return Result::fail("Instruction 'exit' cannot accept more than one argument", lineno);
		if (!instr->operands.size())
			return {};

		if (!instr->operands[0].as_const_int(nullptr))
			return Result::fail("Instruction 'exit' argument 1 must be a constant integer.", lineno);  // TODO: make this instruction support non-constants
		return {};
	}
	static Result _check_return_instr(const Instruction* instr, const Program* prog, const Code* owner, int lineno) {
		return {};
	}

	static constexpr Result(*_instr_check_table[])(const Instruction*, const Program*, const Code*, int lineno) = {
		&_check_invalid_instr,
		&_check_simple_instr,
		&_check_goto_instr,
		&_check_ifgoto_instr,
		&_check_call_instr,
		&_check_exit_instr,
		&_check_return_instr,
	};

	void Instruction::post_decode(Program* prog) {
		if (this->type != Type::CALL)
			return;

		if (this->extra)  // calling convention already specified by user, nothing to do
			return;

		std::string_view callee_name;
		if (!this->operands[0].as_const_label(&callee_name))
			return;  // callee is not a label, nothing to do

		const Function* callee;
		if (!prog->get_function(callee_name, &callee))
			return;  // callee is not a function defined in this file, nothing to do

		// if we can find the callee function, use its calling convention as the default for the call instruction
		this->extra = reinterpret_cast<intptr_t>(callee->code->funcattr_props.callconv);
	}
	Result Instruction::check(const Program* program, const Code* owner) const {
		auto res = _check_all_instr_operands(program, owner, this->operands);
		if (!res)
			return res.push("Invalid instruction operands");

		return _instr_check_table[get_underlying(this->type)](this, program, owner, this->lineno);
	}

	static bool _call_function(
		AssemblyWriter& out,
		const Operand& target,
		const Operand& return_location,
		array_view<Operand> args,
		const Code* owner,
		const Program* prog,
		const CallingConvention* callconv,
		bool never_returns = false
	) {
		Operand retloc_resolved = return_location.resolve(owner);

		//auto callconv = CurrentImpl.dflt_calling_convention();

		std::stringstream comment_txt;

		// push our local boxes that are caller-saved
		std::vector<Operand> pushed_locals;
		size_t args_size = 0;
		size_t align_missing;

		out.comment("    { PUSH LOCALS }");
		if (callconv) {
			for (auto& [name, box] : owner->boxes) {

				if (CurrentImpl.register_callconv(box.reg, callconv).caller_saved()) {
					comment_txt.clear();
					comment_txt << "    -> " << name << ": ";

					if (!retloc_resolved.is_related_register(box.reg)) {
						comment_txt << "OK";
						out.comment(comment_txt.view());

						auto& op = pushed_locals.emplace_back(Operand::from_register(box.reg));
						out.push(op);
					}
					else {
						comment_txt << "SKIP; this is the return location";
						out.comment(comment_txt.view());
					}
				}
			}

			align_missing = remaining_for_align(out.stack_offset(), callconv->stk_ptr_align);
			comment_txt = {};
			comment_txt << "    { ALIGN stack to " << (int)callconv->stk_ptr_align << " }";
			out.comment(comment_txt.view());
			out.push_amount(align_missing);

			out.comment("    { PUSH SHADOW SPACE (if applicable) }");
			// push shadow space
			out.push_amount(callconv->shadow_space);

			int limit = (int)min(args.size(), callconv->arg_registers.size());
			out.comment("    { WRITE REG ARGS }");
			// set up register parameters
			for (int i = 0; i < limit; i++) {
				// for register params, move the arg into the correct register

				if (args[i].type().type == DataType::Type::FLOAT) {
					auto arg_reg = callconv->fp_arg_registers[i];
					if (args[i].type().size)
						arg_reg = arg_reg->smallest_to_fit(args[i].type().size);
					if (!args[i].resolve(owner).is_related_register(arg_reg)) {
						out.move(Operand::from_register(arg_reg), args[i]);
					}
				}
				else {
					auto arg_reg = callconv->arg_registers[i];
					if (args[i].type().size)
						arg_reg = arg_reg->smallest_to_fit(args[i].type().size);
					if (!args[i].resolve(owner).is_related_register(arg_reg)) {
						out.move(Operand::from_register(arg_reg), args[i]);
					}
				}
			}

			out.comment("    { WRITE STACK ARGS }");
			// set up stack parameters: push them in reverse order; they must satisfy the callconv's padding requirements
			if (args.size() > callconv->arg_registers.size()) {
				for (int i = (int)args.size(); i >= callconv->arg_registers.size(); --i) {
					size_t padding = remaining_for_align(out.stack_offset(), callconv->stk_args_align);
					out.push_amount(padding);
					out.push(args[i].resolve(owner));
					args_size += (padding + args[i].type().size);
				}
			}

			//comment_txt = {};
			//comment_txt << "    { ALIGN STACK to " << (int)callconv->stk_ptr_align << " }";
			//out.comment(comment_txt.view());
			//size_t stack_padding = remaining_for_align(out.stack_offset(), callconv->stk_ptr_align);
			//out.push_amount(stack_padding);  // make the stack pointer aligned according to the callconv
		}

		out.comment("    { CALL }");
		out.call(target);

		if (never_returns)
			return true;

		if (callconv) {
			out.comment("    { POP STACK ARGS }");
			out.pop_amount(args_size);  // pop args from stack

			out.comment("    { POP SHADOW SPACE (if applicable) }");
			// pop shadow space
			out.pop_amount(callconv->shadow_space);

			out.comment("    { POP ALIGN PADDING }");
			out.pop_amount(align_missing);

			const CPURegister* ret_reg;
			if (return_location.type().type == DataType::Type::FLOAT)
				ret_reg = callconv->fp_return_reg;
			else if (return_location.is_empty())
				ret_reg = nullptr;
			else
				ret_reg = callconv->return_reg;

			out.comment("    { FETCH RESULT } ");
			// move the return value into the target variable if needed
			if (!return_location.is_empty()) {
				if (return_location.type().type == DataType::Type::FLOAT) {
					//auto ret_reg = callconv->fp_return_reg;
					if (!retloc_resolved.is_related_register(ret_reg))
						out.move(return_location, Operand::from_register(ret_reg));
				}
				else {
					//auto ret_reg = callconv->return_reg;
					if (!retloc_resolved.is_related_register(ret_reg))
						out.move(return_location, Operand::from_register(ret_reg));
				}
			}

			out.comment("    { POP LOCALS }");
			// pop back local boxes that are caller-saved and are not the return location
			while (pushed_locals.size()) {
				auto& var = vec_last(pushed_locals);
				out.pop(var);  // comparison with return register is done in pre-call phase
				pushed_locals.pop_back();
			}
		}

		return true;
	}

	bool Instruction::generate(AssemblyWriter& out, std::string_view epilog_name, const Code* owner, const Program* prog) const {
		const Operator* op;
		size_t offset;

		std::stringstream ss;
		std::stringstream comment_txt;

		switch (this->type) {
		case Type::SIMPLE:
			op = reinterpret_cast<const Operator*>(this->extra);
			comment_txt.clear();
			comment_txt << "  (INSTR '" << op->name << "'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			offset = op - Operator::IADD;
			if (offset >= array_len(_simple_generators))
				return false;

			return _simple_generators[offset](out, this->operands[0], this->operands.size() > 1 ? this->operands[1] : Operand{}, owner);
		case Type::GOTO:
			comment_txt.clear();
			comment_txt << "  (INSTR 'goto'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			return out.jump(this->operands[0].resolve(owner));
		case Type::IF_GOTO:
			comment_txt.clear();
			comment_txt << "  (INSTR 'if goto'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			return out.jump_if(
				this->operands[0].resolve(owner),
				reinterpret_cast<const Operator*>(this->extra),
				this->operands[1].resolve(owner),
				this->operands[2].resolve(owner)
			);
		case Type::CALL:
			comment_txt.clear();
			comment_txt << "  (INSTR 'call'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			return _call_function(
				out,
				this->operands[0].resolve(owner),
				this->operands[1].resolve(owner),
				vec_slice(this->operands, 2, this->operands.size() - 2),
				owner,
				prog,
				reinterpret_cast<const CallingConvention*>(this->extra)
			);
		case Type::EXIT:
			comment_txt.clear();
			comment_txt << "  (INSTR 'exit'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			return out.exit(owner, this->operands[0].resolve(owner));
		case Type::RETURN:
			comment_txt.clear();
			comment_txt << "  (INSTR 'return'; line " << this->lineno << ')';
			out.comment(comment_txt.view());

			if (!epilog_name.size())
				return out.return_();

			ss << '.' << epilog_name;
			return out.jump(Operand::from_const_label(ss.view(), nullptr, 0));
		default:
			return false;
		}
	}

	bool CustomInstruction::generate(AssemblyWriter& out, const Code* owner) const {
		std::vector<Operand> resolved;
		resolved.reserve(this->operands.size());
		for (auto& op : this->operands) {
			resolved.emplace_back(op.resolve(owner));
		}

		std::stringstream cmt_str;
		cmt_str << '#' << this->name;
		return out.custom_instruction(sview(this->name), resolved, cmt_str.view());
	}

	Result FuncParam::check(int lineno) const {
		return this->type.check(true, lineno).push("Failed to validate function parameter");
	}

	void Function::post_decode() {
		this->code->post_decode(this->code.prog);
	}

	Result Function::check(const Program* prog, int lineno) const {
		if (!this->code->funcattr_props.callconv) {
			if (this->params.size())
				return Result::fail(sfmt("Function '", this->name, "' with no calling convention cannot define params"), lineno);
		}
		if (this->code->result_box.has_value() && !this->code->funcattr_props.callconv) {
			return Result::fail(sfmt("Function '", this->name, "' with no calling convention cannot define a result type"), lineno);
		}

		ResultBunch res;

		for (auto& var : this->params) {
			res += var.check(lineno);
		}
		res += this->code->check(prog);
		
		return res.push(sfmt("Invalid function definition for '", this->name.size() ? this->name : "<anonymous>", "'"));
	}

	bool Function::generate(AssemblyWriter& out) const {
		std::string rand_name;
		std::string_view vname = sview(this->name);
		if (!vname.size()) {
			rand_name = out.random_identifier("func");
			vname = sview(rand_name);
		}

		if (!out.def_global_label(vname))
			return false;

		return this->code->generate(out, this->params.size(), this->code, false);
	}

	Result DataDeclaration::check(const Program* program, int lineno) const {
		ResultBunch res;

		res += this->datatype.check(false, lineno);
		res += this->value.check(false, program, nullptr);
		if (!this->value.is_constant())
			res += Result::fail("Inital value of global data must be a constant.", lineno);

		return res.push("Invalid global data declaration.");
	}

	bool DataDeclaration::generate(const AssemblyWriter& out) const {
		return out.define_bytes(this->datatype.size, this->value);
	}

	bool Code::gen_function_call(AssemblyWriter& out, const Operand& target, const Operand& return_location, array_view<Operand> args, bool never_returns, std::string_view callconv_str) const {
		const CallingConvention* callconv;

		if (callconv_str == "default") {
			callconv = CurrentImpl.dflt_calling_convention();
		}
		else {
			callconv = CurrentImpl.calling_convention(callconv_str);
		}
		
		return _call_function(
			out, target, return_location, args, this, nullptr, callconv, never_returns
		);
	}

	void Code::set_func_attrs(FuncAttrProperties attr_props) {
		this->funcattr_props = attr_props;
		//this->float_regs.release_all();
		//this->integral_regs.release_all();

		if (this->funcattr_props.callconv) {
			for (auto reg : this->funcattr_props.callconv->temp_rules->tmp_registers) {
				this->integral_regs.reserve(reg);
				this->mixed_regs.reserve(reg);
			}
			for (auto reg : this->funcattr_props.callconv->temp_rules->fp_tmp_registers) {
				this->float_regs.reserve(reg);
				this->mixed_regs.reserve(reg);
			}
		}
	}

	Operand Code::reserve_register(const CPURegister* reg, CodeRef ref, int lineno) {
		if (this->integral_regs.reserve(reg))
			return Operand::from_register(reg, ref, lineno);
		if (this->float_regs.reserve(reg))
			return Operand::from_register(reg, ref, lineno);
		if (this->mixed_regs.reserve(reg))
			return Operand::from_register(reg, ref, lineno);
		return {};
	}

	Operand Code::reserve_param(const CPURegister* where, CodeRef code, const FuncParam& param, int lineno) {
		auto res = this->reserve_register(where, code, lineno);
		this->boxes[param.name.name] = Box{ .reg = where, .type = param.type, .is_param = true };
		return res;
	}

	Operand Code::reserve_box(std::string_view name, DataType ty, CodeRef ref, int lineno) {
		const CPURegister* allocated = this->mixed_regs.alloc(ty.size);
		if (allocated) {
			this->boxes[scopy(name)] = { .reg = allocated, .type = ty };
			return Operand::from_register(allocated, ref, lineno);
		}

		if (ty.type == DataType::Type::FLOAT) {
			
			allocated = this->float_regs.alloc(ty.size);
			if (allocated)
				this->boxes[scopy(name)] = { .reg = allocated, .type = ty };
			return allocated ? Operand::from_register(allocated, ref, lineno) : Operand{};
		}
		
		allocated = this->integral_regs.alloc(ty.size);
		if (allocated)
			this->boxes[scopy(name)] = { .reg = allocated, .type = ty };
		return allocated ? Operand::from_register(allocated, ref, lineno) : Operand{};
	}
	void Code::set_result_location(const CPURegister* where, DataType ty) {
		this->result_box = Box{ .reg = where, .type = ty };
	}


	bool Code::generate(AssemblyWriter& out, size_t num_params, CodeRef ref, bool entry) const {
		/*
		Generate a code block.
		It has the form:

			<prolog>
			<user code>
		.__epilog@1:
			<epilog>
			ret

		
		Where a return instruction simply translates to 'jmp .__epilogue@1'.
		Note that the epilogue label is uniquely generated to minimize naming conflicts with user labels
		*/
		const CallingConvention* callconv = this->funcattr_props.callconv;

		std::string epilog_lbl_name = out.random_identifier("epilog");

		std::vector<Operand> pushed_operands = {};

		std::stringstream comment_txt;

		//size_t stack_offset = 0;

		out.enter_code_block(callconv);

		if (callconv) {

			out.comment("[PROLOG]");

			out.comment("  (push temp registers)");
			for (auto reg : callconv->temp_rules->tmp_registers) {
				auto& op = pushed_operands.emplace_back(Operand::from_register(reg, ref, 0));
				out.push(op);
			}
			for (auto reg : callconv->temp_rules->fp_tmp_registers) {
				auto& op = pushed_operands.emplace_back(Operand::from_register(reg, ref, 0));
				out.push(op);
			}

			// prologue
			out.comment("  (push user boxes)");

			for (auto& [name, box] : this->boxes) {
				if (!box.is_param && CurrentImpl.register_callconv(box.reg, callconv).callee_saved()) {
					comment_txt.clear();
					comment_txt << "    {PUSH '" << name << "'}";
					out.comment(comment_txt.view());
					auto& op = pushed_operands.emplace_back(Operand::from_register(box.reg, ref, 0));
					out.push(op);
				}

				// TODO: move the params into the shadow space if applicable
			}
		}

		out.comment("[USER CODE]");
		// Actual instructions
		for (auto& [sel, idx, _] : this->elem_order) {
			bool res;
			switch (sel) {
			case CodeElementType::LABEL:
				res = out.def_local_label(this->labels[idx]);
				break;
			case CodeElementType::INSTRUCTION:
				res = this->instructions[idx].generate(out, callconv? epilog_lbl_name : "", this, ref.prog);
				break;
			case CodeElementType::CUSTOM_INSTR:
				res = this->custom_instrs[idx].generate(out, ref.ptr());
				break;
			default:
				return false;
			}
			if (!res)
				return false;
		}

		if (callconv) {
			out.comment("[EPILOG]");
			// epilogue
			out.def_local_label(sview(epilog_lbl_name));
			if (!entry) {
				//out.pop_amount(stack_padding);

				out.comment("  (POP TMP+LOCALS)");
				while (pushed_operands.size()) {
					auto& op = vec_last(pushed_operands);
					out.pop(op);
					pushed_operands.pop_back();
				}

				out.leave_code_block();
				out.comment("  ( RETURN )");
				return out.return_();
			}
			else {
				out.comment("  ( EXIT )");
				if (!out.exit(this, Operand::from_const_int(0)))
					return false;

				out.leave_code_block();
				return true;
			}
		}
		out.leave_code_block();
		return out.return_();
	}


	EntryPoint::EntryPoint(Program* owner, bool use_libc) {
		this->name = use_libc ? "main" : CurrentImpl.entry_name();
		this->code = owner->create_empty_code();
		this->code->set_func_attrs({
			.callconv = CurrentImpl.dflt_calling_convention(),
		});

		owner->add_export(this->name);
		CurrentImpl.call_env_init(this->code.ptr());
	}
	EntryPoint::EntryPoint() :
		code(nullptr)
	{ }

	Code* EntryPoint::operator->() {
		return this->code.operator->();
	}
	const Code* EntryPoint::operator->() const {
		return this->code.operator->();
	}

	void EntryPoint::post_decode() {
		if (this->code)
			this->code->post_decode(this->code.prog);
	}
	bool EntryPoint::generate(AssemblyWriter& out, bool use_libc) const {
		if (!out.def_global_label(this->name))
			return false;
		return this->code->generate(out, 0, this->code, true);
	}
	Result EntryPoint::check(const Program* prog) const {
		if (this->code)
			return this->code->check(prog);
		return {};
	}

	EntryPoint::operator bool() const {
		return this->code.operator bool();
	}

	void Program::post_decode() {
		for (auto& sel : this->_stmt_order) {
			switch (sel.selector) {
			case StatementType::FUNCTION:
				this->_funcs[sel.index].post_decode();
				break;
			//case StatementType::ENTRY_POINT:
			//	this->_entry.post_decode();
			//	break;
			default:
				break;
			}
		}
		this->_entry.post_decode();
	}

	Result Program::check() const {
		ResultBunch res;

		for (auto& sel : this->_stmt_order) {
			switch (sel.selector) {
			case StatementType::FUNCTION:
				res += this->_funcs[sel.index].check(this, sel.lineno);
				break;
			case StatementType::IMPORT:
				res += this->_imports[sel.index].check();
				break;
			case StatementType::DATA_DECL_INIT:
				res += this->_initialized_decls[sel.index].check(this, sel.lineno);
				break;
			case StatementType::DATA_DECL_UNINIT:
				res += this->_uninitialized_decls[sel.index].check(false, sel.lineno);
				break;
			//case StatementType::ENTRY_POINT:
			//	res += this->_entry_point()->check(this);
			//	break;
			case StatementType::INCLUDE:
				break;
			case StatementType::SYMBOL:
				break;
			default:
				break;
			}
		}
		res += this->_entry.check(this);

		return res.push("Failed to validate the program").with_filename(sview(this->_filename));
	}

	bool Program::generate(AssemblyWriter& out) const {
		// TODO: make this a little more modular
		out.set_bitness(CurrentImpl.pointer_size() * 8);
		if (CurrentImpl.pointer_size() == 8)
			out.enable_relative_addr();

		for (auto& exp : this->_exports) {
			out.export_symbol(sview(exp));
		}

		for (auto& [sel, idx, _] : this->_stmt_order) {
			bool res;
			switch (sel) {
			case StatementType::IMPORT:
				res = true;
				for (auto& name : this->_imports[idx].symbols) {
					res &= out.import_symbol(sview(name));
				}
				break;
			case StatementType::FUNCTION:
				res = this->_funcs[idx].generate(out);
				break;
			case StatementType::DATA_DECL_INIT:
				res = this->_initialized_decls[idx].generate(out);
				break;
			case StatementType::DATA_DECL_UNINIT:
				res = out.reserve_bytes(this->_uninitialized_decls[idx].size);
				break;
			case StatementType::INCLUDE:
				res = out.include_file(this->_includes[idx]);
				break;
			case StatementType::SYMBOL:
				res = out.def_global_label(this->_symbols[idx]);
				break;
			case StatementType::SECTION:
				res = out.section(sview(this->_sections[idx]));
				break;
			default:
				return false;
			}
			if (!res)
				return false;
		}
		if (this->_entry) {
			return this->_entry.generate(out, this->_use_libc);
		}
		
		//if (this->_entry_point_idx >= 0) { // TODO: make this defined by the environment
		//	out.def_global_label("_start");
		//	return this->_code_objects[this->_entry_point_idx].generate(out, 0, CodeRef{ const_cast<Program*>(this), _entry_point_idx });
		//}
		return true;
	}

	bool Program::symbol_exists(std::string_view name) const {
		for (auto& sym : this->_symbols) {
			if (sym == name)
				return true;
		}
		for (auto& imp : this->_imports) {
			for (auto& sym : imp.symbols) {
				if (sym == name)
					return true;
			}
		}
		for (auto& func : this->_funcs) {
			if (func.name == name)
				return true;
		}
		return false;
	}


	CodeRef Program::create_empty_code() {
		ssize_t idx = this->_code_objects.size();
		auto& code = this->_code_objects.emplace_back();
		return { this, idx };
	}

	Program::Program(std::string_view filename, bool use_libc) :
		_stmt_order(), _imports(), _symbols(), _funcs(), _includes(), _initialized_decls(), _uninitialized_decls(), _code_objects(), _filename(filename), _entry(), _use_libc(use_libc)
	{ 
		CurrentImpl.call_prog_init(this);
	}


	template<class TST, class TV, class ...TArgs>
	static TV& _add_stmt(
		std::vector<Selector<TST>>& vorder,
		std::vector<TV>& vtarget,
		const TST& selector,
		int lineno,
		TArgs&& ...args
	) {
		int idx = static_cast<int>(vtarget.size());
		vorder.emplace_back(selector, idx, lineno);
		return vtarget.template emplace_back<TArgs...>(std::forward<TArgs>(args)...);
	}


	void Program::add_import(std::string_view name, array_view<std::string> symbols, int lineno) {
		std::vector<std::string> syms = aview2vec<std::string>(symbols);
		
		_add_stmt(this->_stmt_order, this->_imports, StatementType::IMPORT, lineno, Import{
			.name = scopy(name),
			.symbols = std::move(syms),
		});
	}

	void Program::add_symbol(std::string_view name, int lineno) {
		_add_stmt(this->_stmt_order, this->_symbols, StatementType::SYMBOL, lineno, scopy(name));
	}

	void Program::add_function(CodeRef code, std::string_view name, array_view<std::string> attributes, array_view<FuncParam> params, DataType return_type, int lineno) {
		auto attrs = aview2vec<std::string>(attributes);
		auto _params = aview2vec<FuncParam>(params);


		auto& func = _add_stmt(this->_stmt_order, this->_funcs, StatementType::FUNCTION, lineno, Function{
			.code = code,
			.attributes = attrs,
			.params = _params,
			.name = scopy(name),
		});

		const CallingConvention* callconv = func.code->funcattr_props.callconv;

		if (callconv) {
			for (int i = 0; i < min(params.size(), callconv->arg_registers.size(), callconv->fp_arg_registers.size()); i++) {
				const CPURegister* param_reg;

				if (params[i].type.type == DataType::Type::FLOAT)
					param_reg = callconv->fp_arg_registers[i];
				else
					param_reg = callconv->arg_registers[i];

				param_reg = param_reg->smallest_to_fit(params[i].type.size);

				func.code->reserve_param(param_reg, func.code, params[i], lineno);
			}

			// LATER: reserve stack parameters here


			if (return_type) {
				const CPURegister* ret_reg;
				if (return_type.type == DataType::Type::FLOAT)
					ret_reg = callconv->fp_return_reg;
				else
					ret_reg = callconv->return_reg;

				ret_reg = ret_reg->smallest_to_fit(return_type.size);

				func.code->set_result_location(ret_reg, return_type);
				func.code->reserve_register(ret_reg, func.code, lineno);
			}
		}
	}

	void Program::add_include(std::string name, int lineno) {
		_add_stmt(this->_stmt_order, this->_includes, StatementType::INCLUDE, lineno, name);
	}

	void Program::add_decl(DataType ty, SimpleOperand init, int lineno) {
		if (init.is_empty()) {
			_add_stmt(this->_stmt_order, this->_uninitialized_decls, StatementType::DATA_DECL_UNINIT, lineno, ty);
			return;
		}
		_add_stmt(this->_stmt_order, this->_initialized_decls, StatementType::DATA_DECL_INIT, lineno, DataDeclaration{
			.datatype = ty,
			.value = init,
		});
	}



	CodeRef Program::add_entry_point() {
		this->_entry = EntryPoint(this, this->_use_libc);

		return this->_entry.code;
	}
	//void Program::set_entry_point(CodeRef source) {
	//	this->_entry_point_idx = source.idx;
	//}
	void Program::add_export(std::string_view name) {
		this->_exports.emplace_back(scopy(name));
	}
	void Program::add_section(std::string_view name, int lineno) {
		_add_stmt(
			this->_stmt_order,
			this->_sections,
			StatementType::SECTION,
			lineno,
			scopy(name)
		);
	}

	bool Program::get_function(std::string_view name, const Function** out) const {
		for (auto& func : this->_funcs) {
			if (func.name == name) {
				if (out) *out = &func;
				return true;
			}
		}
		return false;
	}
}

