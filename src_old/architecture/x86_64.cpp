#include <bit>
#include <vector>
#include <string_view>
#include "x86_64.hpp"
#include "../AsmWriter.hpp"
#include "../Implementation.hpp"


namespace cpasm::x86_64 {

	/* 
	----------------------
	Register definitions
	----------------------
	*/
#define REG(NAME, PARENT, LOW, HIGH, SPRT, SZ) \
	static const CPURegister* NAME = [](){ \
		__ ## NAME = { \
			.name = #NAME, .parent = PARENT, .low = LOW, .high = HIGH, .support = SPRT, .size = SZ \
		}; \
		return &__ ## NAME; \
	}();

#define TL_REG(NAME, LOW, HIGH, SPRT, SZ) REG(NAME, nullptr, LOW, HIGH, SPRT, SZ)
#define BL_REG(NAME, PARENT, SPRT, SZ) REG(NAME, PARENT, nullptr, nullptr, SPRT, SZ)

#define GP_REG(NAME, PARENT, LOW, HIGH, SZ) REG(NAME, PARENT, LOW, HIGH, GP_REGISTER, SZ)
#define GP_TL_REG(NAME, LOW, HIGH, SZ) TL_REG(NAME, LOW, HIGH, GP_REGISTER, SZ)
#define GP_BL_REG(NAME, PARENT, SZ) BL_REG(NAME, PARENT, GP_REGISTER, SZ)
#define GP_BL_REG8(NAME, PARENT) BL_REG(NAME, PARENT, GP_REGISTER, 1)
#define GP_REG16(NAME, PARENT, LOW, HIGH) REG(NAME, PARENT, LOW, HIGH, GP_REGISTER, 2)
#define GP_REG32(NAME, PARENT, LOW, HIGH) REG(NAME, PARENT, LOW, HIGH, GP_REGISTER, 4)
#define GP_TL_REG64(NAME, LOW, HIGH) TL_REG(NAME, LOW, HIGH, GP_REGISTER, 8)

#define BASIC_REG(NAME, SPRT, SZ) REG(NAME, nullptr, nullptr, nullptr, SPRT, SZ)

#define FP_REG(NAME) BASIC_REG(NAME, FLOAT_REGISTER, 8)  // These are normally 128 bits, but for convenience we mark them as being 64

#define DECL_LETTERREG(LETTER) static CPURegister __R ## LETTER ## X; static CPURegister __E ## LETTER ## X; static CPURegister __ ## LETTER ## X; static CPURegister __ ## LETTER ## L; static CPURegister __ ## LETTER ## H;
#define DECL_NUMREG(NUM) static CPURegister __R ## NUM; static CPURegister __R ## NUM ## D; static CPURegister __R ## NUM ## W; static CPURegister __R ## NUM ## B;
#define DECL_LETTERSREG(LETTERS) static CPURegister __R ## LETTERS; static CPURegister __E ## LETTERS; static CPURegister __ ## LETTERS;

#define DEF_LETTERREG(LETTER) \
	GP_TL_REG64(R ## LETTER ## X, &__E ## LETTER ## X, nullptr) \
	GP_REG32(E ## LETTER ## X, &__R ## LETTER ## X, &__ ## LETTER ## X, nullptr) \
	GP_REG16(LETTER ## X, &__E ## LETTER ## X, &__ ## LETTER ## L, &__ ## LETTER ## H) \
	GP_BL_REG8(LETTER ## L, &__ ## LETTER ## X) \
	GP_BL_REG8(LETTER ## H, &__ ## LETTER ## X)

#define DEF_NUMREG(NUM) \
	GP_TL_REG64(R ## NUM, &__R ## NUM ## D, nullptr) \
	GP_REG32(R ## NUM ## D, &__R ## NUM, &__R ## NUM ## W, nullptr) \
	GP_REG16(R ## NUM ## W, &__R ## NUM ## D, &__R ## NUM ## B, nullptr) \
	GP_BL_REG8(R ## NUM ## B, &__R ## NUM ## W)

#define DEF_LETTERSREG(LTRS) \
	GP_TL_REG64(R ## LTRS, &__E ## LTRS, nullptr) \
	GP_REG32(E ## LTRS, &__R ## LTRS, &__ ## LTRS, nullptr) \
	GP_REG16(LTRS, &__E ## LTRS, nullptr, nullptr)

	constexpr CPURegisterSupport FLOAT_REGISTER = {
		false, true, false, false, true
	};
	constexpr CPURegisterSupport GP_REGISTER = {  // for general purpose registers
		true, false, false, false, true
	};
	constexpr CPURegisterSupport BP_REGISTER = {  // for the RBP registers & family
		false, false, true, false, true
	};
	constexpr CPURegisterSupport SP_REGISTER = {  // for the RSP registers & family
		false, false, false, true, true
	};
	constexpr CPURegisterSupport MISC_REGISTER = {  // for all other userland registers
		false, false, false, false, true
	};
	constexpr CPURegisterSupport KERNEL_REGISTER = {  // for all kernel registers
		false, false, false, false, false
	};

	DECL_LETTERREG(A);
	DECL_LETTERREG(B);
	DECL_LETTERREG(C);
	DECL_LETTERREG(D);
	DECL_NUMREG(8);
	DECL_NUMREG(9);
	DECL_NUMREG(10);
	DECL_NUMREG(11);
	DECL_NUMREG(12);
	DECL_NUMREG(13);
	DECL_NUMREG(14);
	DECL_NUMREG(15);
	DECL_LETTERSREG(SI);
	DECL_LETTERSREG(DI);

	DEF_LETTERREG(A);
	DEF_LETTERREG(B);
	DEF_LETTERREG(C);
	DEF_LETTERREG(D);

	DEF_NUMREG(8);
	DEF_NUMREG(9);
	DEF_NUMREG(10);
	DEF_NUMREG(11);
	DEF_NUMREG(12);
	DEF_NUMREG(13);
	DEF_NUMREG(14);
	DEF_NUMREG(15);

	DEF_LETTERSREG(SI);
	DEF_LETTERSREG(DI);


	static CPURegister __XMM0;
	static CPURegister __XMM1;
	static CPURegister __XMM2;
	static CPURegister __XMM3;
	static CPURegister __XMM4;
	static CPURegister __XMM5;
	static CPURegister __XMM6;
	static CPURegister __XMM7;
	static CPURegister __XMM8;
	static CPURegister __XMM9;
	static CPURegister __XMM10;
	static CPURegister __XMM11;
	static CPURegister __XMM12;
	static CPURegister __XMM13;
	static CPURegister __XMM14;
	static CPURegister __XMM15;


	FP_REG(XMM0);
	FP_REG(XMM1);
	FP_REG(XMM2);
	FP_REG(XMM3);
	FP_REG(XMM4);
	FP_REG(XMM5);
	FP_REG(XMM6);
	FP_REG(XMM7);
	FP_REG(XMM8);
	FP_REG(XMM9);
	FP_REG(XMM10);
	FP_REG(XMM11);
	FP_REG(XMM12);
	FP_REG(XMM13);
	FP_REG(XMM14);
	FP_REG(XMM15);

	static CPURegister __RSP;
	static CPURegister __RBP;
	static CPURegister __RIP;

	BASIC_REG(RSP, SP_REGISTER, 8);
	BASIC_REG(RBP, BP_REGISTER, 8);
	BASIC_REG(RIP, MISC_REGISTER, 8);

	static CPURegister __CS;
	static CPURegister __DS;
	static CPURegister __SS;
	static CPURegister __ES;
	static CPURegister __FS;
	static CPURegister __GS;


	BASIC_REG(CS, KERNEL_REGISTER, 2);
	BASIC_REG(DS, KERNEL_REGISTER, 2);
	BASIC_REG(SS, KERNEL_REGISTER, 2);
	BASIC_REG(ES, KERNEL_REGISTER, 2);
	BASIC_REG(FS, KERNEL_REGISTER, 2);
	BASIC_REG(GS, KERNEL_REGISTER, 2);

	static CPURegister __CR0;
	static CPURegister __CR2;
	static CPURegister __CR3;
	static CPURegister __CR4;
	static CPURegister __CR8;

	BASIC_REG(CR0, KERNEL_REGISTER, 8);
	BASIC_REG(CR2, KERNEL_REGISTER, 8);
	BASIC_REG(CR3, KERNEL_REGISTER, 8);
	BASIC_REG(CR4, KERNEL_REGISTER, 8);
	BASIC_REG(CR8, KERNEL_REGISTER, 8);

	static CPURegister __IA32_EFER;

	BASIC_REG(IA32_EFER, KERNEL_REGISTER, 8);


	/*
	--------------------------
	CPU instructions
	--------------------------
	*/


	static constexpr CPUInstruction MOV = 0;
	static constexpr CPUInstruction ADD = 1;
	static constexpr CPUInstruction ADDPD = 2;
	static constexpr CPUInstruction ADDPS = 3;
	static constexpr CPUInstruction ADDSD = 4;
	static constexpr CPUInstruction ADDSS = 5;
	static constexpr CPUInstruction INC = 6;
	static constexpr CPUInstruction SUB = 7;
	static constexpr CPUInstruction SUBPD = 8;
	static constexpr CPUInstruction SUBPS = 9;
	static constexpr CPUInstruction SUBSD = 10;
	static constexpr CPUInstruction SUBSS = 11;
	static constexpr CPUInstruction DEC = 12;
	static constexpr CPUInstruction DIV = 13;
	static constexpr CPUInstruction IDIV = 14;
	static constexpr CPUInstruction DIVPD = 15;
	static constexpr CPUInstruction DIVPS = 16;
	static constexpr CPUInstruction DIVSD = 17;
	static constexpr CPUInstruction DIVSS = 18;
	static constexpr CPUInstruction DPPD = 19;
	static constexpr CPUInstruction DPPS = 20;
	static constexpr CPUInstruction MUL = 21;
	static constexpr CPUInstruction IMUL = 22;
	static constexpr CPUInstruction MULPD = 23;
	static constexpr CPUInstruction MULPS = 24;
	static constexpr CPUInstruction MULSD = 25;
	static constexpr CPUInstruction MULSS = 26;
	static constexpr CPUInstruction JMP = 27;
	static constexpr CPUInstruction JE = 28;
	static constexpr CPUInstruction JNE = 29;
	static constexpr CPUInstruction JG = 30;
	static constexpr CPUInstruction JL = 31;
	static constexpr CPUInstruction JGE = 32;
	static constexpr CPUInstruction JLE = 33;
	static constexpr CPUInstruction JZ = 34;
	static constexpr CPUInstruction JNZ = 35;
	static constexpr CPUInstruction CMP = 36;
	static constexpr CPUInstruction XOR = 37;
	static constexpr CPUInstruction AND = 38;
	static constexpr CPUInstruction OR = 39;
	static constexpr CPUInstruction CBW = 40;
	static constexpr CPUInstruction CWDE = 41;
	static constexpr CPUInstruction CWQE = 42;
	static constexpr CPUInstruction PUSH = 43;
	static constexpr CPUInstruction POP = 44;
	static constexpr CPUInstruction MOVD = 45;
	static constexpr CPUInstruction MOVQ = 46;
	static constexpr CPUInstruction MOVSS = 47;
	static constexpr CPUInstruction MOVSD = 48;
	static constexpr CPUInstruction UCOMISS = 49;
	static constexpr CPUInstruction UCOMISD = 50;
	static constexpr CPUInstruction RET = 51;
	static constexpr CPUInstruction CALL = 52;
	static constexpr CPUInstruction JAE = 53;
	static constexpr CPUInstruction JBE = 54;
	static constexpr CPUInstruction JA = 55;
	static constexpr CPUInstruction JB = 56;
	static constexpr CPUInstruction SYSCALL = 57;
	static constexpr CPUInstruction LEA = 58;


	/*
	--------------------------
	Architecture properties
	--------------------------
	*/


	struct Winx64TmpRules : TempRulesImpl {
		PROP const CPURegister* temp_regs[] = {
			R15,
			RDI,
			RSI,
		};
		PROP const CPURegister* float_temp_regs[] = {
			XMM15,
			XMM14,
			XMM13,
		};
	};


	struct Winx64CallConv : CallConvImpl {
		PROP const CPURegister* return_reg = RAX;
		PROP const CPURegister* float_return_reg = XMM0;
		PROP const CPURegister* argument_regs[] = {
			RCX,
			RDX,
			R8,
			R9,
		};
		PROP const CPURegister* float_argument_regs[] = {
			XMM0,
			XMM1,
			XMM2,
			XMM3,
		};
		PROP const TemporaryRules* temp_rules = build_temprules<Winx64TmpRules>();
		PROP uint8_t stack_args_align = 8;
		PROP RegConventionFlags reg_saving_flags[] = {
			RegConventionFlags::CALLER_SAVED(),  // RAX
			RegConventionFlags::CALLEE_SAVED(),  // RBX
			RegConventionFlags::CALLER_SAVED(),  // RCX
			RegConventionFlags::CALLER_SAVED(),  // RDX
			RegConventionFlags::CALLER_SAVED(),  // R8
			RegConventionFlags::CALLER_SAVED(),  // R9
			RegConventionFlags::CALLER_SAVED(),  // R10
			RegConventionFlags::CALLER_SAVED(),  // R11
			RegConventionFlags::CALLEE_SAVED(),  // R12
			RegConventionFlags::CALLEE_SAVED(),  // R13
			RegConventionFlags::CALLEE_SAVED(),  // R14
			RegConventionFlags::CALLEE_SAVED(),  // R15
			RegConventionFlags::CALLEE_SAVED(),  // RSI
			RegConventionFlags::CALLEE_SAVED(),  // RDI
			RegConventionFlags::CALLER_SAVED(),  // XMM0
			RegConventionFlags::CALLER_SAVED(),  // XMM1
			RegConventionFlags::CALLER_SAVED(),  // XMM2
			RegConventionFlags::CALLER_SAVED(),  // XMM3
			RegConventionFlags::CALLER_SAVED(),  // XMM4
			RegConventionFlags::CALLER_SAVED(),  // XMM5
			RegConventionFlags::CALLEE_SAVED(),  // XMM6
			RegConventionFlags::CALLEE_SAVED(),  // XMM7
			RegConventionFlags::CALLEE_SAVED(),  // XMM8
			RegConventionFlags::CALLEE_SAVED(),  // XMM9
			RegConventionFlags::CALLEE_SAVED(),  // XMM10
			RegConventionFlags::CALLEE_SAVED(),  // XMM11
			RegConventionFlags::CALLEE_SAVED(),  // XMM12
			RegConventionFlags::CALLEE_SAVED(),  // XMM13
			RegConventionFlags::CALLEE_SAVED(),  // XMM14
			RegConventionFlags::CALLEE_SAVED(),  // XMM15
			RegConventionFlags::CALLEE_SAVED(),  // RSP
			RegConventionFlags::CALLEE_SAVED(),  // RBP
		};
		PROP uint8_t shadow_space = 32;
		PROP uint8_t stack_pointer_align = 16;
		PROP std::string_view name = "win_x64";
		PROP uint8_t funcentry_stack_align = 8;
		PROP CallArgsMethod args_method = CallArgsMethod::MAPPED;
	};

	struct Sysvx64TmpRules : TempRulesImpl {
		PROP const CPURegister* temp_regs[] = {
			R15,
			R14,
			R13,
		};
		PROP const CPURegister* float_temp_regs[] = {
			XMM15,
			XMM14,
			XMM13,
		};
	};


	struct Sysvx64CallConv : CallConvImpl {
		/*
		FAILING POINT - Argument registers: linux requires that the nth float argument be in XMMn, as opposed to windows, which requires the
		nth argument to be in XMMn if it is a float.
		example: (int, float, float, int)
		- systemv: (RDI, XMM0, XMM1, RSI)
		- win64: (RCX, XMM1, XMM2, R9)

		Also linux requires the number of args to be put in AL for variadic functions.
		*/
		PROP const CPURegister* return_reg = RAX;
		PROP const CPURegister* float_return_reg = XMM0;
		PROP const CPURegister* argument_regs[] = {
			RDI,
			RSI,
			RDX,
			RCX,
			R8,
			R9,
		};
		PROP const CPURegister* float_argument_regs[] = { 
			XMM0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
		};
		PROP const TemporaryRules* temp_rules = build_temprules<Sysvx64TmpRules>();
		PROP RegConventionFlags reg_saving_flags[] = {
			RegConventionFlags::CALLER_SAVED(),  // RAX
			RegConventionFlags::CALLEE_SAVED(),  // RBX
			RegConventionFlags::CALLER_SAVED(),  // RCX
			RegConventionFlags::CALLER_SAVED(),  // RDX
			RegConventionFlags::CALLER_SAVED(),  // R8
			RegConventionFlags::CALLER_SAVED(),  // R9
			RegConventionFlags::CALLER_SAVED(),  // R10
			RegConventionFlags::CALLER_SAVED(),  // R11
			RegConventionFlags::CALLEE_SAVED(),  // R12
			RegConventionFlags::CALLEE_SAVED(),  // R13
			RegConventionFlags::CALLEE_SAVED(),  // R14
			RegConventionFlags::CALLEE_SAVED(),  // R15
			RegConventionFlags::CALLER_SAVED(),  // RSI
			RegConventionFlags::CALLER_SAVED(),  // RDI
			RegConventionFlags::CALLER_SAVED(),  // XMM0
			RegConventionFlags::CALLER_SAVED(),  // XMM1
			RegConventionFlags::CALLER_SAVED(),  // XMM2
			RegConventionFlags::CALLER_SAVED(),  // XMM3
			RegConventionFlags::CALLER_SAVED(),  // XMM4
			RegConventionFlags::CALLER_SAVED(),  // XMM5
			RegConventionFlags::CALLER_SAVED(),  // XMM6
			RegConventionFlags::CALLER_SAVED(),  // XMM7
			RegConventionFlags::CALLER_SAVED(),  // XMM8
			RegConventionFlags::CALLER_SAVED(),  // XMM9
			RegConventionFlags::CALLER_SAVED(),  // XMM10
			RegConventionFlags::CALLER_SAVED(),  // XMM11
			RegConventionFlags::CALLER_SAVED(),  // XMM12
			RegConventionFlags::CALLER_SAVED(),  // XMM13
			RegConventionFlags::CALLER_SAVED(),  // XMM14
			RegConventionFlags::CALLER_SAVED(),  // XMM15
			RegConventionFlags::CALLEE_SAVED(),  // RSP
			RegConventionFlags::CALLEE_SAVED(),  // RBP
		};
		PROP uint8_t stack_pointer_align = 16;
		PROP std::string_view name = "systemv_x64";
		PROP uint8_t funcentry_stack_align = 8;
		PROP CallArgsMethod args_method = CallArgsMethod::NEXT_AVAILABLE;
	};

	struct Sysvx64VarargsCallconv : Sysvx64CallConv {
		PROP std::string_view name = "varargs";  // TODO: find a better name ?
		PROP const CPURegister* argcount_reg = AL;
	};

	struct Sysvx64SyscallConv : SyscallConvImpl {
		PROP const CPURegister* return_reg = RAX;
		PROP const CPURegister* code_reg = RAX;
		PROP const CPURegister* argument_regs[] = {
			RDI,
			RSI,
			RDX,
			R10,
			R8,
			R9,
		};
		PROP RegConventionFlags reg_saving_flags[] = {
			RegConventionFlags::CALLEE_SAVED(),  // RAX
			RegConventionFlags::CALLEE_SAVED(),  // RBX
			RegConventionFlags::CALLER_SAVED(),  // RCX
			RegConventionFlags::CALLEE_SAVED(),  // RDX
			RegConventionFlags::CALLEE_SAVED(),  // R8
			RegConventionFlags::CALLEE_SAVED(),  // R9
			RegConventionFlags::CALLEE_SAVED(),  // R10
			RegConventionFlags::CALLER_SAVED(),  // R11
			RegConventionFlags::CALLEE_SAVED(),  // R12
			RegConventionFlags::CALLEE_SAVED(),  // R13
			RegConventionFlags::CALLEE_SAVED(),  // R14
			RegConventionFlags::CALLEE_SAVED(),  // R15
			RegConventionFlags::CALLEE_SAVED(),  // RSI
			RegConventionFlags::CALLEE_SAVED(),  // RDI
		};
		PROP uint8_t stack_pointer_align = 16;
		PROP std::string_view name = "systemv_x64";
		PROP std::map<std::string_view, int> call_numbers = {
			{"exit", 60},
			{"exit_group", 231},
		};
	};

	/*
	-------------------------------
	Helpers
	-------------------------------
	*/

	/*
	Class that wraps the base of a dereference operand only if the base is
	a label and an index or scale are present.
	*/
	class _LabeledDerefWrapper {
		TmpRegWrapper _wrapper;
		SimpleOperand _index;
		SimpleOperand _scale;
		DataType _type;
		Operand _orig;

	public:
		_LabeledDerefWrapper() :
			_type(), _scale(), _index(), _wrapper(), _orig()
		{}

		inline _LabeledDerefWrapper(AssemblyWriter& out, const Operand& op) :
			_LabeledDerefWrapper()
		{
			SimpleOperand base;
			if (op.as_memop(&base, &this->_index, &this->_scale) && base.as_const_label(nullptr) && (!this->_index.is_empty() || !this->_scale.is_empty())) {
				this->_wrapper = out.wrap_tmp(base, true, false, TmpRegFlags::GP_backed(true, false, false, false));
				this->_type = op.type();
			} else {
				this->_orig = op;
			}
		}
		inline Operand get() {
			if (this->_orig)
				return this->_orig;
			const CPURegister* reg;
			if (!this->_wrapper.get().as_register(&reg))
				return this->_orig;
			return Operand(SimpleOperand::from_register(reg, nullptr, 0), this->_type, this->_index, this->_scale);
		}
	};


	/**
	* Wraps cpu instructions so that any memory operands with a label/symbol as a base that specifies an index or scale has
	* the base put into a temporary register beforehand.
	* This comes from the fact that x86-64 does not allow combining RIP-relative addressing with [base+index*scale+offset] style adressing.
	* e.g.:
	*
	* mov rax, [label + rbx*2]; illegal on x86, combines RIP-relative adressing with [base + index*scale]
	*
	* becomes:
	*
	* mov r15, label; fine, RIP-relative address calculated here
	* mov rax, [r15 + rbx*2]; fine, not RIP-relative
	*/
	static bool _cpu_instruction2(AssemblyWriter& out, CPUInstruction instr, std::vector<Operand> operands, std::string_view comment = "") {
		std::vector<_LabeledDerefWrapper> wrappers;
		wrappers.reserve(operands.size());

		int i = 0;
		for (auto& op : operands) {
			wrappers.emplace_back(out, op);
			operands[i] = wrappers[i].get();
			i++;
		}

		return out.cpu_instruction(instr, operands, comment);
	}


	/*
	Serialize a float constant into an int constant of the appropriate size.
	*/
	static Operand _serialize_const_float(const Operand& src, OpFlags* flags) {
		double val;
		if (!src.as_const_float(&val))
			return src;
		if (flags->size == 4) { // if we are supposed to intepret this float as 32-bit, it must be serialized into a 32-bit integer
			flags->float_st = static_cast<decltype(flags->float_st)>(flags->float_st & OpFlags::LEFT);
			return Operand::from_const_int(std::bit_cast<unsigned int>(static_cast<float>(val)));
		}

		if (flags->size == 8) {  // for 64-bits, we need to serialize into a 64-bit integer
			flags->float_st = static_cast<decltype(flags->float_st)>(flags->float_st & OpFlags::LEFT);
			return Operand::from_const_int(std::bit_cast<size_t>(val));
		}

		return src;
	}

	static bool _needs_membase_wrapping(const Operand& op) {
		SimpleOperand base;
		if (!op.as_memop(&base, nullptr, nullptr))
			return false;

		return base.as_const_label(nullptr);
	}

	static bool _mov_simple(AssemblyWriter& out, const Operand& target, const Operand& src) {
		return _cpu_instruction2(out, MOV, { target, src }, "move");
	}

	static bool _mov_mem2mem(AssemblyWriter& out, const Operand& target, const Operand& src) {
		auto backed_src = out.wrap_tmp(src, true, false, TmpRegFlags::GP_backed());
		return _mov_simple(out, target, backed_src.get());
	}
	static bool _mov_dq(AssemblyWriter& out, const Operand& target, const Operand& src, uint8_t size) {
		if (size == 4)
			return _cpu_instruction2(out, MOVD, { target, src }, "move");
		if (size == 8)
			return _cpu_instruction2(out, MOVQ, { target, src }, "move");
		return false;
	}
	static bool _mov_sssd(AssemblyWriter& out, const Operand& target, const Operand& src, uint8_t size) {
		if (size == 4)
			return _cpu_instruction2(out, MOVSS, { target, src }, "move");
		if (size == 8)
			return _cpu_instruction2(out, MOVSD, { target, src }, "move");
		return false;
	}
	// this assumes that src is not a constant
	static bool _mov_float2float(AssemblyWriter& out, const Operand& target, const Operand& src, OpFlags flags) {
		if (flags.memory_st & OpFlags::ALL)  // one of the two operands is memory
			return _mov_dq(out, target, src, flags.size);
		
		return _mov_sssd(out, target, src, flags.size);
	}
	// for when src xor target is not a float; also assumes that src is not a constant
	static bool _mov_int2float(AssemblyWriter& out, const Operand& target, const Operand& src, OpFlags flags) {
		if (flags.float_st != OpFlags::LEFT) {  // the target operand is not a float
			if (flags.memory_st == OpFlags::RIGHT)  // if the other is a memory, then no float operation is needed
				return _mov_simple(out, target, src); 

			return _mov_dq(out, target, src, flags.size);
		}
		// if src is not a float
		if (flags.memory_st == OpFlags::LEFT)  // if target is memory
			return _mov_simple(out, target, src);

		return _mov_dq(out, target, src, flags.size);
	}

	static bool _mov_float(AssemblyWriter& out, const Operand& target, const Operand& src, OpFlags flags) {
		// turning float constants into int constants is taken care of by the caller of this function (x86_64Impl::move)
		TmpRegWrapper backed_src;
		if (src.is_constant())
			backed_src = out.wrap_tmp(src, true, false, TmpRegFlags::GP_backed(true, false, false), flags.size);
		else
			backed_src = out.wrap_tmp(src, true, false, TmpRegFlags::FP_backed(false, false, false)); // TODO: find another solution than having an object backing nothing


		if (flags.float_st == OpFlags::ALL)
			return _mov_float2float(out, target, backed_src.get(), flags);

		return _mov_int2float(out, target, backed_src.get(), flags);
	}

	static bool _mov_label2reg(AssemblyWriter& out, const Operand& target, const SimpleOperand& src, OpFlags flags) {
		std::cout << "move label to reg\n";
		return out.cpu_instruction(LEA, { 
			target, 
			Operand(src, DataType(DataType::INT, CurrentImpl.pointer_size()), {}, {}, 1, AddressingMode::RELATIVE) 
		});
	}

	static bool _add_float(AssemblyWriter& out, const Operand& target, const Operand& op, uint8_t size) {
		auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(true, false, false));  // wrap potential constants with a GP register
		auto backed_op2 = out.wrap_tmp(backed_op.get(), true, false, TmpRegFlags::FP_backed(true, false, true));  // src needs to be an xmm reg or memory
		auto backed_target = out.wrap_tmp(target, true, true, TmpRegFlags::FP_backed(true, true, true));  // target must be an xmm reg

		if (size == 4)
			return _cpu_instruction2(out, ADDSS, { backed_target.get(), backed_op2.get() }, "add");
		if (size == 8)
			return _cpu_instruction2(out, ADDSD, { backed_target.get(), backed_op2.get() }, "add");
		return false;
	}

	static bool _add_int(AssemblyWriter& out, const Operand& target, const Operand& op) {
		return _cpu_instruction2(out, ADD, { target, op }, "add");
	}

	static bool _sub_float(AssemblyWriter& out, const Operand& target, const Operand& op, uint8_t size) {
		auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(true, false, false));  // wrap potential constants with a GP register
		auto backed_op2 = out.wrap_tmp(backed_op.get(), true, false, TmpRegFlags::FP_backed(true, false, true));  // src needs to be an xmm reg or memory
		auto backed_target = out.wrap_tmp(target, true, true, TmpRegFlags::FP_backed(true, true, true));  // target must be an xmm reg

		if (size == 4)
			return _cpu_instruction2(out, SUBSS, {backed_target.get(), backed_op2.get()}, "sub");
		if (size == 8)
			return _cpu_instruction2(out, SUBSD, {backed_target.get(), backed_op2.get()}, "sub");
		return false;
	}

	static bool _sub_int(AssemblyWriter& out, const Operand& target, const Operand& op) {
		return _cpu_instruction2(out, SUB, { target, op }, "sub");
	}

	static bool _mul_float(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
		Operand ser_op = _serialize_const_float(op, &flags);

		auto backed_op = out.wrap_tmp(ser_op, true, false, TmpRegFlags::FP_backed(true, false), flags.size);  // op must be memory or FP register
		auto backed_target = out.wrap_tmp(target, true, true, TmpRegFlags::FP_backed());  // target must be FP register

		if (flags.size == 4)
			return _cpu_instruction2(out, MULSS, { backed_target.get(), backed_op.get() }, "mul");
		if (flags.size == 8)
			return _cpu_instruction2(out, MULSD, { backed_target.get(), backed_op.get() }, "mul");

		return false;
	}

	static bool _mul_int(AssemblyWriter& out, const Operand& target, const Operand& op, uint8_t size) {
		auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(false, false), size);  // op must be memory or GP register
		auto backed_target = out.wrap_tmp(target, true, true, TmpRegFlags::GP_backed());  // target must be GP register

		if (op.is_constant())
			return _cpu_instruction2(out, IMUL, { backed_target.get(), backed_target.get(), backed_op.get() }, "mul");
		return _cpu_instruction2(out, IMUL, { backed_target.get(), backed_op.get() }, "mul");
	}

	static bool _div_float(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
		Operand ser_op = _serialize_const_float(op, &flags);

		auto backed_op = out.wrap_tmp(ser_op, true, false, TmpRegFlags::FP_backed(true, false), flags.size);  // op must be memory or FP register
		auto backed_target = out.wrap_tmp(target, true, true, TmpRegFlags::FP_backed());  // target must be FP register

		if (flags.size == 4)
			return _cpu_instruction2(out, DIVSS, { backed_target.get(), backed_op.get() }, "div");
		if (flags.size == 8)
			return _cpu_instruction2(out, DIVSD, { backed_target.get(), backed_op.get() }, "div");
		return false;
	}

	static bool _div_int(AssemblyWriter& out, const Operand& target, const Operand& op, uint8_t size, bool _signed) {
		const CPURegister* reg_tgt_lo, * reg_tgt_hi;
		if (size == 1) {
			reg_tgt_lo = AL;
			reg_tgt_hi = AH;
		}
		else {
			reg_tgt_lo = RAX->smallest_to_fit(size);
			reg_tgt_hi = RDX->smallest_to_fit(size);
		}

		// these may look unused, but here we depend on the destructor calls at the end of the function.
		auto backed_target = out.wrap_given_reg(target, reg_tgt_lo, true, true);
		auto backed_target_hi = out.wrap_given_reg(Operand::from_const_int(0), reg_tgt_hi, true, false);
		auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(true, false), size);

		if (_signed)
			return _cpu_instruction2(out, IDIV, { backed_op.get() }, "div");

		return _cpu_instruction2(out, DIV, { backed_op.get() }, "div");
	}

	static bool _bxor_int(AssemblyWriter& out, const Operand& target, const Operand& op) {
		return _cpu_instruction2(out, XOR, { target, op }, "bxor");
	}

	static bool _band_int(AssemblyWriter& out, const Operand& target, const Operand& op) {
		return _cpu_instruction2(out, AND, { target, op }, "band");
	}

	static bool _bor_int(AssemblyWriter& out, const Operand& target, const Operand& op) {
		return _cpu_instruction2(out, OR, { target, op }, "bor");
	}

	static bool _mod_int(AssemblyWriter& out, const Operand& target, const Operand& op, uint8_t size, bool _signed) {
		const CPURegister *reg_tgt_lo, *reg_tgt_hi;
		if (size == 1) {
			reg_tgt_lo = AL;
			reg_tgt_hi = AH;
		}
		else {
			reg_tgt_lo = RAX->smallest_to_fit(size);
			reg_tgt_hi = RDX->smallest_to_fit(size);
		}

		auto backed_target = out.wrap_given_reg(target, reg_tgt_lo, true, false);
		auto backed_target_hi = out.wrap_given_reg(target, reg_tgt_hi, false, true);
		auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(true, false), size);

		out.move(backed_target_hi.get(), Operand::from_const_int(0));

		if (_signed)
			return _cpu_instruction2(out, IDIV, { backed_op.get() }, "mod");

		return _cpu_instruction2(out, DIV, { backed_op.get() }, "mod");
	}

	static bool _cmp_simple(AssemblyWriter& out, const Operand& lhs, const Operand& rhs) {
		return _cpu_instruction2(out, CMP, { lhs, rhs }, "jump_if: compare");
	}

	static bool _cmp_mem2mem(AssemblyWriter& out, const Operand& lhs, const Operand& rhs) {
		auto backed_rhs = out.wrap_tmp(rhs, true, false, TmpRegFlags::GP_backed(false));
		return _cmp_simple(out, lhs, backed_rhs.get());
	}

	static bool _cmp_int(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, OpFlags flags) {
		if (flags.memory_st == OpFlags::ALL)
			return _cmp_mem2mem(out, lhs, rhs);
		return _cmp_simple(out, lhs, rhs);
	}

	static bool _cmp_float(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, OpFlags* pflags) {
		pflags->signed_st = OpFlags::NONE;  // on x86-64, float comparisons (UCOMISS, UCOMISD) only set unsigned comparison flags

		auto wrapped_lhs = out.wrap_tmp(lhs, true, false, TmpRegFlags::FP_backed());
		auto wrapped_rhs = out.wrap_tmp(rhs, true, false, TmpRegFlags::FP_backed(true, false));
		
		if (pflags->size == 4)
			return _cpu_instruction2(out, UCOMISS, { lhs, rhs }, "jump_if: compare");
		if (pflags->size == 8)
			return _cpu_instruction2(out, UCOMISD, { lhs, rhs }, "jump_if: compare");
		return false;
	}

	static bool _compare(AssemblyWriter& out, const Operand& lhs, const Operand& rhs, OpFlags* pflags) {
		if (pflags->float_st == OpFlags::ALL)
			return _cmp_float(out, lhs, rhs, pflags);
		return _cmp_int(out, lhs, rhs, *pflags);
	}


	static CPUInstruction _signed_jmp_if_table[] = {
		JGE,  // >=
		JLE,  // <=
		JNE,  // !=
		JE,  // ==
		JG,  // >
		JL,  // <
	};

	static CPUInstruction _unsigned_jmp_if_table[] = {
		JAE,  // >=
		JBE,  // <=
		JNE,  // !=
		JE,  // ==
		JA,  // >
		JB,  // <
	};


	/*
	-------------------------------
	Architecture implementation
	-------------------------------
	*/

	struct x86_64Impl : ArchitectureImpl {
		PROP uint8_t pointer_size = 8;
		PROP const CPURegister* registers[] = {
			RAX,
			RBX,
			RCX,
			RDX,
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15,
			RSI,
			RDI,
			XMM0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15,
			RSP,
			RBP,
			RIP,
			CS,
			DS,
			SS,
			ES,
			FS,
			GS,
			CR0,
			CR2,
			CR3,
			CR4,
			CR8,
			IA32_EFER,
		};
		PROP std::string_view instruction_names[] = {
			"MOV",
			"ADD",
			"ADDPD",
			"ADDPS",
			"ADDSD",
			"ADDSS",
			"INC",
			"SUB",
			"SUBPD",
			"SUBPS",
			"SUBSD",
			"SUBSS",
			"DEC",
			"DIV",
			"IDIV",
			"DIVPD",
			"DIVPS",
			"DIVSD",
			"DIVSS",
			"DPPD",
			"DPPS",
			"MUL",
			"IMUL",
			"MULPD",
			"MULPS",
			"MULSD",
			"MULSS",
			"JMP",
			"JE",
			"JNE",
			"JG",
			"JL",
			"JGE",
			"JLE",
			"JZ",
			"JNZ",
			"CMP",
			"XOR",
			"AND",
			"OR",
			"CBW",
			"CWDE",
			"CWQE",
			"PUSH",
			"POP",
			"MOVD",
			"MOVQ",
			"MOVSS",
			"MOVSD",
			"UCOMISS",
			"UCOMISD",
			"RET",
			"CALL",
			"JAE",
			"JBE",
			"JA",
			"JB",
			"SYSCALL",
			"LEA",
		};
		PROP const CallingConvention* calling_conventions[] = {
			build_callconv<Winx64CallConv>(),
			build_callconv<Sysvx64CallConv>(),
			build_callconv<Sysvx64VarargsCallconv>(),
		};
		PROP const SyscallConvention* syscall_conventions[] = {
			build_syscallconv<Sysvx64SyscallConv>(),
		};
		PROP const CPURegister* stack_pointer = RSP;

		PROP bool push(AssemblyWriter& out, const Operand& op, int* out_cnt) {
			const CPURegister* reg;
			if (op.as_register(&reg) && reg->support.supportsFloatVar()) {  // push instruction does not support floating point registers
				if (!out.push_amount(reg->size, true))
					return false;
				// NOTE: this only works because reg->size is defined as 8 in this file! if it is set back to 16, the 
				// correct instruction to use becomes MOVDQA!
				return out.cpu_instruction(MOVQ, { Operand::from_deref_register(RSP), op }, "push");
			}
			*out_cnt = op.toplevel_register().type().size;
			return _cpu_instruction2(out, PUSH, { op.toplevel_register() }, "push");
		}
		PROP bool pop(AssemblyWriter& out, const Operand& target, int* out_cnt) {
			const CPURegister* reg;
			if (target.as_register(&reg) && reg->support.supportsFloatVar()) {  // pop instruction does not support floating point registers
				// NOTE: this only works because reg->size is defined as 8 in this file! if it is set back to 16, the 
				// correct instruction to use becomes MOVDQA!
				if (!out.cpu_instruction(MOVQ, { target, Operand::from_deref_register(RSP) }, "pop"))
					return false;
				return out.pop_amount(reg->size, true);
			}
			*out_cnt = target.toplevel_register().type().size;
			return _cpu_instruction2(out, POP, { target.toplevel_register() }, "pop");
		}
		PROP bool push_amount(AssemblyWriter& out, size_t cnt) {
			return out.cpu_instruction(SUB, { Operand::from_register(RSP), Operand::from_const_int(cnt) }, "push_amount");
		}
		PROP bool pop_amount(AssemblyWriter& out, size_t cnt) {
			return out.cpu_instruction(ADD, { Operand::from_register(RSP), Operand::from_const_int(cnt) }, "pop_amount");
		}
		PROP bool move(AssemblyWriter& out, const Operand& target, const Operand& src, OpFlags flags) {
			Operand ser_src = _serialize_const_float(src, &flags);

			std::string_view label_name;
			if (target.as_register(nullptr) && src.as_const_label(&label_name)) {
				// on linux, PIC requires "lea target, [rel src]" to move a global symbol into a register, which also works on windows
				SimpleOperand s_src = SimpleOperand::from_const_label(label_name, nullptr, 1);
				return _mov_label2reg(out, target, s_src, flags);
			}

			if (flags.memory_st == OpFlags::ALL)  // both operands are memory
				return _mov_mem2mem(out, target, ser_src);

			if (!flags.float_st)  // none of the operands are float
				return _mov_simple(out, target, ser_src);

			return _mov_float(out, target, ser_src, flags);
		}
		PROP bool add(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 ADD does not support two memory operands in one instruction
				if (target.type().type == DataType::Type::FLOAT)  // for float addition with two memory operands, use addss and addsd
					return _add_float(out, target, op, flags.size);

				auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed());
				return _add_int(out, target, backed_op.get());
			}

			if (!flags.float_st)
				return _add_int(out, target, op);

			return _add_float(out, target, op, flags.size);
		}
		PROP bool sub(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 SUB does not support two memory operands in one instruction
				if (target.type().type == DataType::Type::FLOAT)  // for float subtraction with two memory operands, use subss and subsd
					return _sub_float(out, target, op, flags.size);

				auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed());
				return _sub_int(out, target, backed_op.get());
			}

			if (!flags.float_st)
				return _sub_int(out, target, op);

			return _sub_float(out, target, op, flags.size);
		}
		PROP bool mul(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 IMUL does not support two memory operands in one instruction
				if (target.type().type == DataType::Type::FLOAT)  // for float multiplication with two memory operands, use mulss and mulsd
					return _mul_float(out, target, op, flags);
			}

			if (!flags.float_st)
				return _mul_int(out, target, op, flags.size);

			return _mul_float(out, target, op, flags);
		}
		PROP bool div(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {
				if (target.type().type == DataType::Type::FLOAT)  // for float division with two memory operands, use divss and divsd
					return _div_float(out, target, op, flags);
			}

			if (!flags.float_st)  // float_st only tracks float registers, NOT memory locations of float type
				return _div_int(out, target, op, flags.size, flags.signed_st & OpFlags::LEFT);

			return _div_float(out, target, op, flags);
		}
		PROP bool inc(AssemblyWriter& out, const Operand& target, OpFlags flags) {
			return _cpu_instruction2(out, INC, { target }, "inc");
		}
		PROP bool dec(AssemblyWriter& out, const Operand& target, OpFlags flags) {
			return _cpu_instruction2(out, DEC, { target }, "dec");
		}
		PROP bool bxor(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 XOR does not support two memory operands in one instruction
				auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(false, true), flags.size);
				return _bxor_int(out, target, backed_op.get());
			}
			return _bxor_int(out, target, op);
		}
		PROP bool band(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 AND does not support two memory operands in one instruction
				auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(false, true), flags.size);
				return _band_int(out, target, backed_op.get());
			}
			return _band_int(out, target, op);
		}
		PROP bool bor(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			if (flags.memory_st == OpFlags::ALL) {  // x86 OR does not support two memory operands in one instruction
				auto backed_op = out.wrap_tmp(op, true, false, TmpRegFlags::GP_backed(false, true), flags.size);
				return _bor_int(out, target, backed_op.get());
			}
			return _bor_int(out, target, op);
		}
		PROP bool mod(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) {
			return _mod_int(out, target, op, flags.size, flags.signed_st & OpFlags::LEFT);
		}
		PROP bool jump(AssemblyWriter& out, const Operand& target) {
			return _cpu_instruction2(out, JMP, { target }, "jump");
		}
		PROP bool jump_if(AssemblyWriter& out, const Operand& target, const Operand& lhs, const Operand& rhs, const Operator* op, OpFlags flags) {
			if (!_compare(out, lhs, rhs, &flags))
				return false;

			ptrdiff_t diff = op - Operator::GEQ;
			if (diff < 0 || diff >= (ptrdiff_t)array_len(_signed_jmp_if_table))
				return false;
			
			// TODO: Should signed comparison be used also when only one of the two operands is signed ?
			CPUInstruction jmp_instr = flags.signed_st == OpFlags::ALL ? _signed_jmp_if_table[diff] : _unsigned_jmp_if_table[diff];

			return _cpu_instruction2(out, jmp_instr, { target }, "jump_if");
		}
		PROP bool call(AssemblyWriter& out, const Operand& target) {
			return _cpu_instruction2(out, CALL, { target }, "call");
		}
		PROP bool return_(AssemblyWriter& out) {
			return _cpu_instruction2(out, RET, {}, "return");
		}
		PROP bool syscall(AssemblyWriter& out, int syscall_code) {
			const CPURegister* code_reg = CurrentImpl.syscall_convention()->code_reg;
			if (!out.move(Operand::from_register(code_reg), Operand::from_const_int(syscall_code)))
				return false;

			return out.cpu_instruction(SYSCALL, {});
		}
	};

	extern const ArchitectureStruct Arch = build_arch<x86_64Impl>();
}

