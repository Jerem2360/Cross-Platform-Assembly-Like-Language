#include "Tokens.hpp"
#include "StringCoding.hpp"

#include <typeinfo>
#include <iostream>
#include <cmath>


namespace cpasm {
	static char _eval_escape_sequence(char esc) {
		switch (esc) {
		case '\\':
			return '\\';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'b':
			return '\b';
		case '"':
			return '"';
		case '\'':
			return '\'';
		case '0':
			return '\0';
		default:
			return esc;
		}
	}

	constexpr auto _HEXDIGITS = char_union<char_range<'0', '9'>, char_range<'a', 'f'>, char_range<'A', 'F'>>;

	size_t _digit_from_char(char c) {
		if (c >= 'a')
			return c - 'a';
		if (c >= 'A')
			return c - 'A';
		return c - '0';
	}

	static size_t _int_from_str(std::string_view s, unsigned int base = 10) {
		size_t res = 0;
		
		size_t power = (size_t)std::pow(base, s.size());

		for (size_t i = s.size(); i >= 0; --i) {
			power /= base;

			size_t dig = _digit_from_char(s[i]);
			if (dig >= base)
				return 0;
			res += power * dig;
		}
		return res;
	}

	static char _eval_ascii_escape(std::string_view s, int index) {
		if (s[index] == 'x') {
			if (s.size() <= (index + 2))
				return 0;
			char str_num[2] = { s[index + 1], s[index + 2] };
			return static_cast<char>(_int_from_str({ str_num, 2 }, 16));
		}
		return _eval_escape_sequence(s[index]);
	}

	static Result _parse_attributes(Parser& parser, std::vector<std::string>* out, Parser::state st) {

		if (!parser.delimiter("["))
			return {};

		while (1) {
			std::string_view _name = parser.name();
			if (_name.size())
				out->push_back(scopy(_name));

			if (parser.delimiter("]"))
				break;
			if (!parser.delimiter(","))
				return parser.fail("Missing ',' between two elements of function attributes.", st);
		}
		return {};
	}

	static Result _parse_args(Parser& parser, std::vector<OperandToken>* out, Parser::state st) {
		if (!parser.delimiter("("))
			return {};

		Result res;

		while (1) {
			auto& arg = out->emplace_back();
			res = OperandToken::parse(parser, &arg);
			if (!res)
				out->pop_back();

			if (parser.delimiter(")"))
				break;
			if (!parser.delimiter(","))
				return parser.fail("Missing ',' between two elements of function arguments.", st);
		}
		return {};
	}

	Result StringToken::parse(Parser& parser, StringToken* out, char delimiter) {
		auto st = parser.save();
		parser.skip_blanks();

		char del_str[] = { delimiter, 0 };

		if (parser[0].chars != del_str) {
			return parser.fail("failed to parse a string: no quote at beginning.", st);
		}
		out->lineno = parser[0].lineno;
		parser.consume();
		std::stringstream res;

		int cnt = parse_string_chars(parser.view(), res, delimiter);
		if (cnt < 0) {
			std::stringstream errmsg;
			write_string_error(cnt, errmsg);
			return parser.fail(errmsg.str(), st);
		}

		parser.consume_chars(cnt + 1);
		*out = { res.str() };
		return {};
	}


	Result ConstantToken::parse(Parser& parser, ConstantToken* out) {
		auto st = parser.save();

		StringToken stok;
		Result res = StringToken::parse(parser, &stok, '"');
		if (res) {
			out->value = stok.value;
			out->type = ConstantTokenType::STRING;
			out->lineno = stok.lineno;
			return {};
		}
		st.restore();
		res = StringToken::parse(parser, &stok, '`');
		if (res) {
			out->value = stok.value;
			out->type = ConstantTokenType::NAME;
			out->lineno = stok.lineno;
			return {};
		}
		st.restore();

		// todo: add support for character literals.

		parser.skip_blanks();
		auto& word = parser[0];

		switch (word.type) {
		case CharGroupType::NUMBER:
			out->type = ConstantTokenType::NUMBER;
			out->value = word.chars;
			out->lineno = word.lineno;
			break;
		case CharGroupType::NAME:
			out->type = ConstantTokenType::NAME;
			out->value = word.chars;
			out->lineno = word.lineno;
			break;
		default:
			return parser.fail(sfmt("Invalid word type ", word.type, " for a constant."), st);
		}

		parser.consume();

		return {};
	}


	SimpleOperand ConstantToken::decode(CodeRef owner) const {
		switch (this->type) {
		case ConstantTokenType::NAME:
			if (owner && owner->result_box.has_value() && (this->value == "result"))
				return SimpleOperand::from_register(
					owner->result_box->reg,
					owner,
					this->lineno
				);
			return SimpleOperand::from_const_label(sview(this->value), owner, this->lineno);
		case ConstantTokenType::STRING:
			return SimpleOperand::from_const_str(sview(this->value), owner, this->lineno);
		case ConstantTokenType::CHAR:
			return SimpleOperand::from_const_char(this->value[0], this->lineno);
		case ConstantTokenType::NUMBER:
		{
			size_t n_dots = std::count(this->value.begin(), this->value.end(), '.');
			if (!n_dots)
				return SimpleOperand::from_const_int(std::stoull(this->value), this->lineno);
			if (n_dots == 1)
				return SimpleOperand::from_const_float(std::stod(this->value), this->lineno);
			return {};
		}
		default:
			return {};
		}
	}


	Result DataTypeToken::parse(Parser& parser, DataTypeToken* out) {
		auto st = parser.save();

		std::string_view fullname = parser.name(&out->lineno);
		if (!fullname.size())
			return parser.fail("Expected a name here.", st);

		size_t cnt = std::count(fullname.begin(), fullname.end(), '.');
		if (cnt > 1)
			return parser.fail("No more than one '.' is allowed in a type.", st);

		size_t off = fullname.find('.');

		std::string_view type_name = ssubview(fullname, 0, cnt ? off : fullname.size());

		out->size = 0;

		if (cnt) {  // there is a '.' in the name
			std::string_view str_size = ssubview(fullname, off + 1, fullname.size() - off - 1);
			if (str_size == "8")
				out->size = 1;
			else if (str_size == "16")
				out->size = 2;
			else if (str_size == "32")
				out->size = 4;
			else if (str_size == "64")
				out->size = 8;
			else
				return parser.fail(sfmt("Invalid size specifier '", str_size, "' for type: size must be 8, 16, 32 or 64."), st);
		}
		out->type_name = type_name;

		return {};
	}

	DataType DataTypeToken::decode() const {
		if (this->type_name == "uint")
			return { DataType::UINT, this->size };
		if (this->type_name == "int")
			return { DataType::INT, this->size };
		if (this->type_name == "ptr")
			return { DataType::PTR, this->size ? (uint8_t)0 : CurrentImpl.pointer_size() };
		if (this->type_name == "float")
			return { DataType::FLOAT, this->size };
		return { DataType::INVALID, 0 };
	}


	Result DeclarationToken::parse(Parser& parser, DeclarationToken* out) {
		auto st = parser.save();

		std::string_view name = parser.name(&out->lineno);
		if (!name.size())
			return parser.fail("Missing name for declaration.", st);

		if (!parser.delimiter(":"))
			return parser.fail("Missing ':' after name in declaration.", st);

		Result res = DataTypeToken::parse(parser, &out->type);
		if (!res)
			return parser.propagate("Invalid datatype in declaration", res, st);
		out->name = scopy(name);
		return {};
	}


	Result VarDeclarationToken::parse(Parser& parser, VarDeclarationToken* out) {
		auto st = parser.save();

		const Operator* op = parser.operator_of_type(OperationType::SEMANTIC, &out->lineno);
		if (!op)
			return parser.fail("Not a variable, missing '$' or '@'.", st);
		if (op == Operator::BOX)
			out->is_box = true;
		else if (op == Operator::REG)
			out->is_box = false;
		else
			return parser.fail(sfmt("Invalid name prefix '", op->name, "' in this context."), st);

		Result res = DeclarationToken::parse(parser, &out->decl);
		if (!res)
			return parser.propagate("Invalid declaration for variable.", res, st);

		return {};
	}

	bool VarDeclarationToken::decode(CodeRef owner) const {
		if (this->is_box) {
			Operand op = owner->reserve_box(this->decl.name, this->decl.type.decode(), owner, this->lineno);
			return !op.is_empty();
		}
		const CPURegister* reg = CurrentImpl.reg_by_name(this->decl.name);
		if (!reg)
			return false;
		owner->reserve_register(reg, owner, this->lineno);
		return true;
	}

	Result VarRefToken::parse(Parser& parser, VarRefToken* out) {
		auto st = parser.save();

		const Operator* op = parser.operator_of_type(OperationType::SEMANTIC, &out->lineno);
		if (!op)
			return parser.fail("Not a variable, missing '$' or '@'.", st);
		if (op == Operator::BOX)
			out->is_box = true;
		else if (op == Operator::REG)
			out->is_box = false;
		else
			return parser.fail(sfmt("Invalid name prefix '", op->name, "' in this context."), st);

		std::string_view _storage_kind = out->is_box ? "box" : "register";

		std::string_view name = parser.name();
		if (!name.size())
			return parser.fail(sfmt("Expected a ", _storage_kind, " name after prefix '", op->name, "'."), st);

		out->name = name;

		return {};
	}

	SimpleOperand VarRefToken::decode(CodeRef owner) const {
		if (this->is_box) {
			return SimpleOperand::from_box(sview(this->name), owner, this->lineno);
		}
		const CPURegister* reg = CurrentImpl.reg_by_name(sview(this->name));
		if (!reg)
			return {};
		return SimpleOperand::from_register(reg, owner, this->lineno);
	}


	template<class T>
	static Result _parse_operand_alt(Parser& parser, SimpleOperandToken* out, Parser::state st, ResultBunch& final_result) {
		out->value = T();
		T& val = std::get<T>(out->value);
		Result res = T::parse(parser, &val);
		if (!res) {
			final_result += res;
			return parser.fail("", st);
		}
		out->lineno = val.lineno;
		return {};
	}


	Result SimpleOperandToken::parse(Parser& parser, SimpleOperandToken* out) {
		auto st = parser.save();
		ResultBunch final_res;

		Result res;
		res = _parse_operand_alt<VarRefToken>(parser, out, st, final_res);
		if (res)
			return {};
		res = _parse_operand_alt<ConstantToken>(parser, out, st, final_res);
		if (res)
			return {};
		return final_res.push("Failed to parse operand.");
	}

	SimpleOperand SimpleOperandToken::decode(Program* prog, CodeRef owner) const {
		return std::visit(
			overloaded{
				[&](const ConstantToken& tok) -> SimpleOperand {
					return tok.decode(owner);
				},
				[&](const VarRefToken& tok) -> SimpleOperand {
					return tok.decode(owner);
				},
			},
			this->value
		);
	}

	static Result _parse_op_dereferee(Parser& parser, OperandToken* out, Parser::state st, bool* has_paren) {
		if (parser.delimiter("(")) {
			Result res = SimpleOperandToken::parse(parser, &out->base);
			if (!res)
				//return ParseResult::propagate(st, res);
				return parser.propagate("Failed to parse simple operand", res, st);
			
			const Operator* offset_op = parser.operator_of_type(OperationType::ADDRESSING);
			if (offset_op) {
				if (offset_op == Operator::ADDR_MUL)
					return parser.fail(sfmt("Invalid addressing operand '", offset_op->name, "'. Only '+' and '-' are allowed here"), st);

				out->offset = SimpleOperandToken();
				res = SimpleOperandToken::parse(parser, &out->offset.value());
				if (!res)
					return parser.propagate("Missing offset operand after '+' in dereference", res, st);

				out->offset_negative = offset_op == Operator::ADDR_SUB;

				const Operator* scale_op = parser.operator_of_type(OperationType::ADDRESSING);
				if (scale_op) {
					if (scale_op != Operator::ADDR_MUL)
						return parser.fail(sfmt("Invalid addressing operand '", scale_op->name, "'. Only '*' is allowed here"), st);

					res = ConstantToken::parse(parser, &out->scale);
					if (!res)
						return parser.propagate("Missing scale operand after '*' in dereference", res, st);
				}
				else {
					out->scale = ConstantToken{ "", ConstantTokenType::NUMBER };
				}
			}
			else {
				out->offset = {};
				out->scale = ConstantToken{ "", ConstantTokenType::NUMBER };
			}

			if (!parser.delimiter(")"))
				return parser.fail("Missing ')' in dereference", st);
			*has_paren = true;
			return {};
		}
		st.restore();
		Result res = SimpleOperandToken::parse(parser, &out->base);
		if (!res)
			return parser.propagate("Base operand not found", res, st);

		*has_paren = false;
		return {};
	}

	static Result _parse_op_deref_type(Parser& parser, OperandToken* out, bool* found_deref_op) {
		auto st = parser.save();

		if (!parser.delimiter("["))
			return parser.fail("Deref operator is required when an offset and/or scale is specified", st);

		*found_deref_op = true;

		out->deref_type = DataTypeToken();
		Result res = DataTypeToken::parse(parser, &out->deref_type.value());
		if (!res)
			return parser.propagate("Missing dereference type after '['", res, st);

		if (!parser.delimiter("]"))
			return parser.fail("Missing ']' at end of dereference operator", st);
		return {};
	}

	Result OperandToken::parse(Parser& parser, OperandToken* out) {
		auto st = parser.save();

		bool has_paren;

		Result res = _parse_op_dereferee(parser, out, st, &has_paren);
		if (!res)
			return parser.propagate("Dereferee not found", res, st);

		bool found_deref_op = false;
		res = _parse_op_deref_type(parser, out, &found_deref_op);
		if (!res) {
			if (found_deref_op)
				return parser.propagate("Failed to parse deref expression", res, st);

			if (has_paren)
				return parser.propagate("Missing dereference type in deref expression", res, st);

			out->deref_type = {};
		}

		out->lineno = out->base.lineno;

		return {};
	}

	Operand OperandToken::decode(Program* prog, CodeRef owner) const {
		if (!this->deref_type.has_value()) {
			return Operand(this->base.decode(prog, owner));
		}
		return Operand(
			this->base.decode(prog, owner),
			this->deref_type->decode(),
			this->offset.has_value() ? this->offset->decode(prog, owner) : SimpleOperand(),
			this->scale.value.size() ? this->scale.decode(owner) : SimpleOperand(),
			this->lineno
		);
	}


	FuncAttrProperties FunctionAttributesToken::decode() const {
		FuncAttrProperties res = {
			.callconv = nullptr,
		};

		for (auto& attrname : this->attrs) {
			if (attrname == "callconv")
				res.callconv = CurrentImpl.dflt_calling_convention();
			else {
				auto cc = CurrentImpl.calling_convention(sview(attrname));
				if (cc)
					res.callconv = cc;
			}
		}

		return res;
	}


	Result SimpleOperationInstructionToken::parse(Parser& parser, SimpleOperationInstructionToken* out) {
		auto st = parser.save();

		Result res = OperandToken::parse(parser, &out->op0);
		if (!res)
			return parser.propagate("First operand of simple instruction not found", res, st);

		const Operator* op = parser.operator_of_type(OperationType::INSTRUCTION);
		if (!op)
			return parser.fail("Invalid instruction operand", st);

		if (op->arity == Arity::BINARY) {
			out->op1 = OperandToken();
			res = OperandToken::parse(parser, &out->op1.value());
			if (!res)
				return parser.propagate(sfmt("Second operand of simple operation '", op->name, "' not found"), res, st);
		}
		else
			out->op1 = {};

		out->operation = op;

		out->lineno = out->op0.lineno;

		return {};
	}

	bool SimpleOperationInstructionToken::decode(Program* prog, CodeRef owner, int lineno) const {
		std::vector<Operand> operands;
		operands.reserve(this->op1.has_value() ? 2 : 1);
		operands.emplace_back(this->op0.decode(prog, owner));
		if (this->op1.has_value())
			operands.emplace_back(this->op1->decode(prog, owner));

		return owner->instruction(
			Instruction::Type::SIMPLE,
			operands,
			lineno,
			reinterpret_cast<intptr_t>(this->operation)
		);
	}


	Result GotoInstructionToken::parse(Parser& parser, GotoInstructionToken* out) {
		auto st = parser.save();

		if (!parser.keyword("goto", &out->lineno))
			return parser.fail("Missing 'goto' keyword.", st);

		Result res = OperandToken::parse(parser, &out->target);
		if (!res)
			return parser.propagate("Instruction 'goto' is missing a target operand", res, st);

		return {};
	}

	bool GotoInstructionToken::decode(Program* prog, CodeRef owner, int lineno) const {
		return owner->instruction(
			Instruction::Type::GOTO,
			{
				this->target.decode(prog, owner)
			},
			lineno
		);
	}


	Result IfGotoInstructionToken::parse(Parser& parser, IfGotoInstructionToken* out) {
		auto st = parser.save();

		if (!parser.keyword("if", &out->lineno))
			return parser.fail("Missing 'if' keyword", st);

		Result res = OperandToken::parse(parser, &out->op0);
		if (!res)
			return parser.propagate("Instruction 'if ... goto ...' missing first comparison operand", res, st);

		const Operator* op = parser.operator_of_type(OperationType::COMPARISON);
		if (!op)
			return parser.fail("Operator of 'if ... goto ...' must be a comparison operator", st);

		out->operation = op;

		out->op1 = OperandToken();
		res = OperandToken::parse(parser, &out->op1.value());
		if (!res)
			return parser.propagate("Instruction 'if ... goto ...' missing second comparison operand", res, st);

		if (!parser.keyword("goto"))
			return parser.fail("Missing 'goto' keyword.", st);

		res = OperandToken::parse(parser, &out->target);
		if (!res)
			return parser.propagate("Instruction 'if ... goto ...' missing target operand", res, st);

		return {};
	}

	bool IfGotoInstructionToken::decode(Program* prog, CodeRef owner, int lineno) const {
		std::vector<Operand> operands = {
			this->target.decode(prog, owner),
			this->op0.decode(prog, owner)
		};
		if (this->op1.has_value())
			operands.push_back(this->op1->decode(prog, owner));

		return owner->instruction(Instruction::Type::IF_GOTO, operands, lineno, reinterpret_cast<intptr_t>(this->operation));
	}


	Result CallInstructionToken::parse(Parser& parser, CallInstructionToken* out) {
		auto st = parser.save();

		if (!parser.keyword("call", &out->lineno))
			return parser.fail("Missing 'call' keyword", st);

		Result res = _parse_attributes(parser, &out->attributes.attrs, st);
		if (!res)  // does not fail if there is no attributes specifier
			return res;

		res = OperandToken::parse(parser, &out->target);
		if (!res)
			res;

		res = _parse_args(parser, &out->args, st);
		if (!res)
			return res;

		std::unique_ptr<OperandToken> retval;
		if (parser.operator_(Operator::RETURNS)) {
			out->result = OperandToken();
			res = OperandToken::parse(parser, &out->result.value());
			if (!res)
				return parser.fail("'->' requires a return target to be specified", st);
		}
		else
			out->result = {};

		return {};
	}

	bool CallInstructionToken::decode(Program* prog, CodeRef owner, int lineno) const {
		Operand result = this->result.has_value() ? this->result->decode(prog, owner) : Operand();
		std::vector<Operand> operands;
		operands.reserve(this->args.size() + 2);
		vec_extend(operands, {
			this->target.decode(prog, owner),
			result
		});

		for (auto& arg : this->args) {
			operands.push_back(arg.decode(prog, owner));
		}

		const CallingConvention* callconv = this->attributes.decode().callconv;

		return owner->instruction(Instruction::Type::CALL, operands, lineno, reinterpret_cast<intptr_t>(callconv));
	}


	Result ExitInstructionToken::parse(Parser& parser, ExitInstructionToken* out) {
		auto st = parser.save();

		if (!parser.keyword("exit", &out->lineno))
			return parser.fail("Missing 'exit' keyword", st);

		Result res = ConstantToken::parse(parser, &out->exitcode);  // TODO: make this support a non-constant operand
		if (!res) {
			out->exitcode.value = "0";
			out->exitcode.type = ConstantTokenType::NUMBER;
		}

		return {};
	}

	bool ExitInstructionToken::decode(CodeRef owner, int lineno) const {
		return owner->instruction(Instruction::Type::EXIT, { this->exitcode.decode(owner) }, lineno);
	}

	Result LabelToken::parse(Parser& parser, LabelToken* out) {
		auto st = parser.save();

		std::string_view name = parser.name(&out->lineno);
		if (!name.size())
			return parser.fail("Invalid name for label", st);
		if (!parser.delimiter(":"))
			return parser.fail("Missing ':' at end of label declaration", st);
		out->name = scopy(name);
		return {};
	}

	bool LabelToken::decode(CodeRef owner, int lineno) const {
		return owner->label(sview(this->name), lineno);
	}

	Result ReturnInstructionToken::parse(Parser& parser, ReturnInstructionToken* out) {
		auto st = parser.save();

		if (!parser.keyword("return", &out->lineno))
			return parser.fail("Missing 'return' keyword", st);

		return {};
	}

	bool ReturnInstructionToken::decode(CodeRef owner, int lineno) const {
		return owner->instruction(Instruction::Type::RETURN, {}, lineno);
	}

	Result CustomInstructionToken::parse(Parser& parser, CustomInstructionToken* out) {
		auto st = parser.save();

		if (!parser.operator_(Operator::CUSTOM_INSTR))
			return parser.fail("Missing '#' before custom instruction name", st);

		std::string_view name = parser.name(&out->lineno);
		if (!name.size())
			return parser.fail("Missing instruction name after '#'", st);

		out->operands = {};

		for (int i = 0;; i++) {
			out->operands.emplace_back();
			bool found_comma = true;
			if (i)
				found_comma = parser.delimiter(",");

			if (!found_comma || !OperandToken::parse(parser, &vec_last(out->operands))) {
				out->operands.pop_back();
				break;
			}
		}

		out->name = scopy(name);

		return {};
	}

	bool CustomInstructionToken::decode(CodeRef owner) const {
		std::vector<Operand> decoded_operands;
		decoded_operands.reserve(this->operands.size());
		for (auto& op : this->operands) {
			decoded_operands.emplace_back(op.decode(owner.prog, owner));
		}

		return owner->custom_instruction(sview(this->name), decoded_operands, this->lineno);
	}

	template<class T>
	static Result _parse_instruction(Parser& parser, InstructionToken* out, ResultBunch& final_result, Parser::state st) {
		out->value = T();  // initialize lifetime of the object before using it
		T& tok = std::get<T>(out->value);
		Result res = T::parse(parser, &tok);
		if (!res) {
			final_result += res;
			return parser.fail("", st);
		}
		else {
			if constexpr (!std::same_as<T, LabelToken>) {
				if (!parser.delimiter(";"))
					return parser.fail("Missing ';' at end of instruction", st);
			}
			out->lineno = tok.lineno;
		}
		return res;
	}


	Result InstructionToken::parse(Parser& parser, InstructionToken* out) {
		auto st = parser.save();

		ResultBunch final_res;
		Result res;

		res = _parse_instruction<SimpleOperationInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<GotoInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<IfGotoInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<CallInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<ExitInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<LabelToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<ReturnInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};
		res = _parse_instruction<CustomInstructionToken>(parser, out, final_res, st);
		if (res)
			return {};

		return final_res.push("Failed to parse instruction");
	}

	bool InstructionToken::decode(Program* prog, CodeRef owner) const {
		return std::visit(
			overloaded{
				[&](const SimpleOperationInstructionToken& tok) -> bool {
					return tok.decode(prog, owner, this->lineno);
				},
				[&](const GotoInstructionToken& tok) -> bool {
					return tok.decode(prog, owner, this->lineno);
				},
				[&](const IfGotoInstructionToken& tok) -> bool {
					return tok.decode(prog, owner, this->lineno);
				},
				[&](const CallInstructionToken& tok) -> bool {
					return tok.decode(prog, owner, this->lineno);
				},
				[&](const ExitInstructionToken& tok) -> bool {
					return tok.decode(owner, this->lineno);
				},
				[&](const LabelToken& tok) -> bool {
					return tok.decode(owner, this->lineno);
				},
				[&](const ReturnInstructionToken& tok) -> bool {
					return tok.decode(owner, this->lineno);
				},
				[&](const CustomInstructionToken& tok) -> bool {
					return tok.decode(owner);
				},
			}, 
			this->value
		);
	}


	Result FunctionBodyToken::parse(Parser& parser, FunctionBodyToken* out) {
		auto st = parser.save();

		if (!parser.delimiter("{", &out->lineno))
			return parser.fail("Function body must start with '{'", st);


		Result res;
		out->declarations = {};

		while (1) {
			auto& decl = out->declarations.emplace_back();
			res = VarDeclarationToken::parse(parser, &decl);
			if (!res) {
				out->declarations.pop_back();
				break;
			}
			if (!parser.delimiter(";"))
				return parser.fail("Missing ';' at end of instruction", st);
		}

		out->instructions = {};

		while (1) {
			if (parser.delimiter("}"))
				break;

			auto& instr = out->instructions.emplace_back();
			res = InstructionToken::parse(parser, &instr);
			if (!res) {
				out->instructions.pop_back();
				return parser.propagate("Instruction not found", res, st);
			}
		}
		return {};
	}

	bool FunctionBodyToken::decode(Program* prog, CodeRef out) const {

		for (auto& decl : this->declarations) {
			if (!decl.decode(out))
				return false;
		}
		for (auto& instr : this->instructions) {
			if (!instr.decode(prog, out))
				return false;
		}
		return true;
	}


	Result CompileTimeConditionToken::parse(Parser& parser, CompileTimeConditionToken* out) {
		auto st = parser.save();

		if (!parser.operator_(Operator::CMP_TIME_VAR, &out->lineno))
			return parser.fail("First operand of compile-time condition must be a compile-time variable ('%' prefix missing)", st);

		std::string_view name = parser.name();
		if (!name.size())
			return parser.fail("Invalid compile time variable name as first operand of compile time condition", st);

		auto op = parser.operator_of_type(OperationType::COMPARISON);
		if (!op)
			return parser.fail("Operation of compile time condition must be a comparison operation", st);

		Result res = ConstantToken::parse(parser, &out->value);
		if (!res)
			return res;

		out->prop = name;
		out->comparison = op;

		return {};
	}

	bool CompileTimeConditionToken::decode() const {
		std::string_view prop_val = CurrentImpl.cmptime_var(sview(this->prop));
		if (!prop_val.size())
			return false;
		if (this->comparison == Operator::EQ) {
			return this->value.value == prop_val;
		}
		if (this->comparison == Operator::NEQ) {
			return this->value.value != prop_val;
		}

		// TODO: parse integers to support other comparison operators

		return false;
	}


	Result IfStatementToken::parse(Parser& parser, IfStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("if", &out->lineno))
			return parser.fail("Missing 'if' keyword", st);
		return CompileTimeConditionToken::parse(parser, &out->condition);
	}

	bool IfStatementToken::decode() const {
		return this->condition.decode();
	}


	Result ElseIfStatementToken::parse(Parser& parser, ElseIfStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("elif", &out->lineno))
			return parser.fail("Missing 'elif' keyword", st);
		return CompileTimeConditionToken::parse(parser, &out->condition);
	}

	bool ElseIfStatementToken::decode() const {
		return this->condition.decode();
	}


	Result ElseStatementToken::parse(Parser& parser, ElseStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("else", &out->lineno))
			return parser.fail("Missing 'else' keyword", st);
		*out = {};
		return {};
	}


	Result EndIfStatementToken::parse(Parser& parser, EndIfStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("endif", &out->lineno))
			return parser.fail("Missing 'endif' keyword", st);
		*out = {};
		return {};
	}


	Result ImportStatementToken::parse(Parser& parser, ImportStatementToken* out) {
		auto st = parser.save();

		if (parser.keyword("from", &out->lineno)) {
			StringToken stok;
			if (!StringToken::parse(parser, &stok))
				return parser.fail("Missing import source string", st);

			if (!parser.keyword("import"))
				return parser.fail("Missing 'import' keyword", st);

			std::vector<std::string> symbols;

			for (int i = 0;; i++) {
				std::string_view name = parser.name();
				if (!name.size())
					break;
				symbols.emplace_back(scopy(name));
			}

			out->source = std::move(stok.value);
			out->symbols = std::move(symbols);
			return {};
		}

		if (!parser.keyword("import", &out->lineno))
			return parser.fail("Missing 'import' or 'from' keyword", st);

		StringToken stok;
		if (!StringToken::parse(parser, &stok))
			return parser.fail("Missing import source string", st);

		out->source = stok.value;
		out->symbols = {};
		return {};
	}

	Import ImportStatementToken::decode() const {
		return Import{
			.name = this->source,
			.symbols = this->symbols,
		};
	}

	static Result _parse_params(Parser& parser, std::vector<VarDeclarationToken>* out, Parser::state st) {

		if (!parser.delimiter("("))
			return {};

		while (1) {
			auto& val = out->emplace_back();
			Result res = VarDeclarationToken::parse(parser, &val);
			if (!res) {
				out->pop_back();
				return res;
			}

			if (parser.delimiter(")"))
				break;
			if (!parser.delimiter(","))
				return parser.fail("Missing ',' between two elements of function parameters", st);
		}
		return {};
	}
	

	Result FunctionStatementToken::parse(Parser& parser, FunctionStatementToken* out) {
		auto st = parser.save();

		if (!parser.keyword("fn", &out->lineno))
			return parser.fail("Missing keyword 'fn'", st);

		out->attributes = {};
		auto res = _parse_attributes(parser, &out->attributes.attrs, st);
		if (!res)
			return parser.propagate("Failed to parse function attributes", res, st);

		std::string_view name = parser.name();
		// if there is no name, name is empty. we're fine with that, so skip checks.

		StringToken stok;
		if (!name.size()) {
			res = StringToken::parse(parser, &stok, '`');
			if (res) {
				name = sview(stok.value);
			}
		}

		out->params = {};
		res = _parse_params(parser, &out->params, st);
		if (!res)
			return parser.propagate("Failed to parse function parameters", res, st);

		/*
		For a statement like "fn export {...}", 'export' would be treated as the name of the function, and not as an 
		instruction to export the function.
		This is OK because it doesn't make sense to export anonymous functions.
		*/
		if (parser.keyword("export"))
			out->export_ = true;
		else
			out->export_ = false;

		if (parser.operator_(Operator::RETURNS)) {
			out->result_type = DataTypeToken();
			res = DataTypeToken::parse(parser, &out->result_type.value());
			if (!res)
				return parser.propagate("Missing function return type after '->'", res, st);
		}
		else
			out->result_type = {};

		res = FunctionBodyToken::parse(parser, &out->body);
		if (!res)
			return parser.propagate("Failed to parse function body", res, st);

		out->name = scopy(name);
		return {};
	}
	bool FunctionStatementToken::decode(Program* prog, int lineno) const {
		CodeRef code = prog->create_empty_code();

		code->set_func_attrs(this->attributes.decode());
		
		std::vector<FuncParam> params;
		params.reserve(this->params.size());
		for (auto& param : this->params) {
			params.emplace_back(FuncParam{
				.name = { param.is_box, param.decl.name },
				.type = param.decl.type.decode()
			});
		}

		DataType ret_ty = {};
		if (this->result_type.has_value()) {
			ret_ty = this->result_type->decode();
		}

		prog->add_function(
			code,
			this->name,
			vec2aview(this->attributes.attrs),
			vec2aview(params),
			ret_ty,
			lineno
		);
		if (this->export_)
			prog->add_export(this->name);
		return this->body.decode(prog, code);
	}

	Result DataDeclarationStatementToken::parse(Parser& parser, DataDeclarationStatementToken* out) {
		auto st = parser.save();

		if (!parser.keyword("data", &out->lineno))
			return parser.fail("Missing keyword 'data'", st);

		Result res = DataTypeToken::parse(parser, &out->type);
		if (!res)
			return parser.fail("Missing data type for data declaration", st);

		res = ConstantToken::parse(parser, &out->value);
		if (!res) {
			out->value = { "", ConstantTokenType::NUMBER };
		}
		return {};
	}
	bool DataDeclarationStatementToken::decode(Program* prog, int lineno) const {
		prog->add_decl(
			this->type.decode(),
			this->value.value.size() ? this->value.decode(nullptr) : SimpleOperand{},
			lineno
		);
		return true;
	}

	Result EntryPointStatementToken::parse(Parser& parser, EntryPointStatementToken* out) {
		auto st = parser.save();

		if (!parser.keyword("entry", &out->lineno))
			return parser.fail("Missing keyword 'entry'", st);

		Result res = FunctionBodyToken::parse(parser, &out->body);
		if (!res)
			return parser.propagate("", res, st);
		return {};
	}
	bool EntryPointStatementToken::decode(Program* prog) const {
		//CodeRef code = prog->create_empty_code();
		//if (!this->body.decode(prog, code))
		//	return false;
		//prog->set_entry_point(code);
		CodeRef code = prog->add_entry_point();
		return this->body.decode(prog, code);
	}

	Result IncludeStatementToken::parse(Parser& parser, IncludeStatementToken* out) {
		auto st = parser.save();

		if (!parser.keyword("include", &out->lineno))
			return parser.fail("Missing keyword 'include'", st);
		StringToken stok;
		Result res = StringToken::parse(parser, &stok);
		if (!res)
			return parser.fail("Include source must be string", st);
		out->path = stok.value;
		return {};
	}


	Result SymbolStatementToken::parse(Parser& parser, SymbolStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("sym", &out->lineno))
			return parser.fail("Failed to parse a symbol: missing keyword 'sym'", st);
		std::string_view name = parser.name();
		StringToken stok;
		if (!name.size()) {
			auto res = StringToken::parse(parser, &stok, '`');
			if (!res)
				return parser.fail("Failed to parse a symbol: a name is required", st);

			name = sview(stok.value);
		}

		if (parser.keyword("export"))
			out->export_ = true;
		else
			out->export_ = false;

		if (!parser.delimiter(":"))
			return parser.fail("Missing ':' after symbol definition", st);

		out->name = scopy(name);
		return {};
	}

	Result SectionStatementToken::parse(Parser& parser, SectionStatementToken* out) {
		auto st = parser.save();
		if (!parser.keyword("section", &out->lineno))
			return parser.fail("Failed to parse section statement: missing keyword 'section'", st);

		std::string_view name = parser.name();
		if (!name.size())
			return parser.fail("Failed to parse section statement: a name is required", st);

		if (!parser.delimiter(":"))
			return parser.fail("Missing ':' after section definition", st);

		out->name = scopy(name);
		return {};
	}

	bool SectionStatementToken::decode(Program* prog) const {
		prog->add_section(this->name, this->lineno);
		return true;
	}

	static Result _require_semicolon(Parser& parser, Parser::state st) {
		if (parser[0].chars != ";")
			return parser.fail("Expected ';' at end of statement", st);
		parser.consume();
		return {};
	}

	template<class T>
	static Result try_parse_stmt(Parser& parser, StatementToken* st_out, ResultBunch& final_res) {
		auto st = parser.save();
		st_out->value = T();
		T& val = std::get<T>(st_out->value);
		Result res = T::parse(parser, &val);
		st_out->lineno = val.lineno;
		if (!res) {
			final_res += res;
			return parser.fail("", st);
		}
		return res;
	}

	Result StatementToken::parse(Parser& parser, StatementToken* out) {
		auto st = parser.save();
		ResultBunch final_res;

		Result res = try_parse_stmt<IfStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<ElseIfStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<ElseStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<EndIfStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<ImportStatementToken>(parser, out, final_res);
		if (res) {
			res = _require_semicolon(parser, st);
			return res;
		}

		res = try_parse_stmt<FunctionStatementToken>(parser, out, final_res);
		if (res) {
			//res = _require_semicolon(parser, st);
			return res;
		}

		res = try_parse_stmt<DataDeclarationStatementToken>(parser, out, final_res);
		if (res) {
			res = _require_semicolon(parser, st);
			return res;
		}

		res = try_parse_stmt<EntryPointStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<IncludeStatementToken>(parser, out, final_res);
		if (res) {
			res = _require_semicolon(parser, st);
			return res;
		}

		res = try_parse_stmt<SymbolStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		res = try_parse_stmt<SectionStatementToken>(parser, out, final_res);
		if (res) {
			return res;
		}

		return final_res.push("Failed to parse a statement");
	}

	bool StatementToken::is_else_stmt() const {
		return std::holds_alternative<ElseStatementToken>(this->value);
	}
	bool StatementToken::is_elif_stmt() const {
		return std::holds_alternative<ElseIfStatementToken>(this->value);
	}
	bool StatementToken::is_if_stmt() const {
		return std::holds_alternative<IfStatementToken>(this->value);
	}
	bool StatementToken::is_endif_stmt() const {
		return std::holds_alternative<EndIfStatementToken>(this->value);
	}

	bool StatementToken::decode(Program* prog) const {
		return std::visit(
			overloaded{
				[&](const IfStatementToken& tok) -> bool {
					return tok.decode();
				},
				[&](const ElseIfStatementToken& tok) -> bool {
					return tok.decode();
				},
				[&](const ElseStatementToken& tok) -> bool {
					return true;
				},
				[&](const EndIfStatementToken& tok) -> bool {
					return true;
				},
				[&](const ImportStatementToken& tok) -> bool {
					Import imp = tok.decode();
					prog->add_import(imp.name, vec2aview(imp.symbols), this->lineno);
					return true;
				},
				[&](const FunctionStatementToken& tok) -> bool {
					return tok.decode(prog, this->lineno);
				},
				[&](const DataDeclarationStatementToken& tok) -> bool {
					return tok.decode(prog, this->lineno);
				},
				[&](const EntryPointStatementToken& tok) -> bool {
					return tok.decode(prog);
				},
				[&](const IncludeStatementToken& tok) -> bool {
					prog->add_include(tok.path, this->lineno);
					return true;
				},
				[&](const SymbolStatementToken& tok) -> bool {
					prog->add_symbol(tok.name, this->lineno);
					if (tok.export_)
						prog->add_export(tok.name);
					return true;
				},
				[&](const SectionStatementToken& tok) -> bool {
					return tok.decode(prog);
				}
			},
			this->value
		);
	}

	Result ProgramToken::parse(Parser& parser, ProgramToken* out) {
		auto st = parser.save();

		out->statements = {};

		while (1) {
			auto& stmt = out->statements.emplace_back();
			Result res = StatementToken::parse(parser, &stmt);
			if (!res) {
				out->statements.pop_back();
				return parser.propagate("Failed to parse statement", res, st);
			}
			//std::cout << "remaining: " << parser.remaining() << '\n';
			parser.skip_blanks();
			if (!parser.remaining())
				break;
		}

		return {};
	}

	Program* ProgramToken::decode(std::string_view name, bool use_libc) const {
		static Program result(name, use_libc);

		std::vector<ConditionElement> cmptime_conditions;

		for (auto& stmt : this->statements) {
			if (stmt.is_if_stmt()) {
				cmptime_conditions.push_back(stmt.decode(&result));
				continue;
			}
			if (stmt.is_elif_stmt()) {
				if (!cmptime_conditions.size())
					return nullptr;  // TODO: make clearer error messages
				if (!vec_last(cmptime_conditions).already_met())
					vec_last(cmptime_conditions).set(stmt.decode(&result));
				else
					vec_last(cmptime_conditions).set(false);
				continue;
			}
			if (stmt.is_else_stmt()) {
				if (!cmptime_conditions.size())
					return nullptr;  // TODO: make clearer error messages
				if (!vec_last(cmptime_conditions).already_met())
					vec_last(cmptime_conditions).set(true);
				else
					vec_last(cmptime_conditions).set(false);
				continue;
			}
			if (stmt.is_endif_stmt()) {
				if (!cmptime_conditions.size())
					return nullptr;  // TODO: make clearer error messages
				cmptime_conditions.pop_back();
				continue;
			}
			if (!vec_all(cmptime_conditions))
				continue;
			if (!stmt.decode(&result))
				return nullptr;
		}

		return &result;
	}
}

