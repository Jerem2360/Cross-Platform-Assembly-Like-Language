#pragma once
#include <map>

#include "../helpers.hpp"
#include "../CPU.hpp"
#include "../Operand.hpp"
#include "../Operator.hpp"


#define PROP inline static


namespace cpasm {

	class AssemblyWriter;


	struct TempRulesImpl {
		PROP const CPURegister* temp_regs[] = {
			nullptr,
		};
		PROP const CPURegister* float_temp_regs[] = {
			nullptr,
		};
	};

	template<class T>
		requires std::is_base_of_v<TempRulesImpl, T>
	const TemporaryRules* build_temprules() {
		static TemporaryRules res = {
			.tmp_registers = T::temp_regs,
			.fp_tmp_registers = T::float_temp_regs,
		};
		return &res;
	}


	struct CallConvImpl {
		PROP const CPURegister* return_reg = nullptr;
		PROP const CPURegister* float_return_reg = nullptr;
		PROP const CPURegister* argument_regs[] = {
			nullptr,
		};
		PROP const CPURegister* float_argument_regs[] = {
			nullptr,
		};
		PROP const TemporaryRules* temp_rules = nullptr;
		PROP uint8_t stack_args_align = 1;
		PROP RegConventionFlags reg_saving_flags[] = {
			{}
		};
		PROP uint8_t shadow_space = 0;
		PROP uint8_t stack_pointer_align = 1;
		PROP std::string_view name = "<unknown>";
		PROP uint8_t funcentry_stack_align = 1;
		PROP const CPURegister* argcount_reg = nullptr;
		PROP CallArgsMethod args_method = CallArgsMethod::NONE;
	};

	template<class T>
		requires std::is_base_of_v<CallConvImpl, T>
	inline const CallingConvention* build_callconv() {
		static CallingConvention res = {
			.return_reg = T::return_reg,
			.fp_return_reg = T::float_return_reg,
			.arg_registers = T::argument_regs,
			.fp_arg_registers = T::float_argument_regs,
			.temp_rules = T::temp_rules,
			.stk_args_align = T::stack_args_align,
			.reg_convflags = T::reg_saving_flags,
			.shadow_space = T::shadow_space,
			.stk_ptr_align = T::stack_pointer_align,
			.name = T::name,
			.funcentry_stack_align = T::funcentry_stack_align,
			.argcount_reg = T::argcount_reg,
			.args_method = T::args_method,
		};
		return &res;
	}

	struct SyscallConvImpl {
		PROP const CPURegister* return_reg = nullptr;
		PROP const CPURegister* code_reg = nullptr;
		PROP const CPURegister* argument_regs[] = {
			nullptr,
		};
		PROP RegConventionFlags reg_saving_flags[] = {
			{}
		};
		PROP uint8_t shadow_space = 0;
		PROP uint8_t stack_pointer_align = 1;
		PROP std::string_view name = "<unknown>";
		PROP std::map<std::string_view, int> call_numbers = {};
		PROP int call_number_mask = 0;
	};

	template<class T>
		requires std::is_base_of_v<SyscallConvImpl, T>
	inline const SyscallConvention* build_syscallconv() {
		static SyscallConvention res = {
			.return_reg = T::return_reg,
			.code_reg = T::code_reg,
			.arg_registers = T::argument_regs,
			.reg_convflags = T::reg_saving_flags,
			.shadow_space = T::shadow_space,
			.stk_ptr_align = T::stack_pointer_align,
			.name = T::name,
			.call_numbers = T::call_numbers,
			.call_number_mask = T::call_number_mask,
		};
		return &res;
	}

	struct ArchitectureImpl {
		PROP uint8_t pointer_size = 0;
		PROP const CPURegister* registers[] = {
			nullptr,
		};
		PROP std::string_view instruction_names[] = {
			"",
		};
		PROP const CallingConvention* calling_conventions[] = {
			nullptr,
		};
		PROP const SyscallConvention* syscall_conventions[] = {
			nullptr,
		};
		PROP const CPURegister* stack_pointer = nullptr;

		PROP bool push(AssemblyWriter& out, const Operand& op, int* out_cnt) { return false; }
		PROP bool pop(AssemblyWriter& out, const Operand& target, int* out_cnt) { return false; }
		PROP bool push_amount(AssemblyWriter& out, size_t cnt) { return false; }
		PROP bool pop_amount(AssemblyWriter& out, size_t cnt) { return false; }
		PROP bool move(AssemblyWriter& out, const Operand& target, const Operand& src, OpFlags flags) { return false; }
		PROP bool add(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool sub(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool mul(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool div(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool inc(AssemblyWriter& out, const Operand& target, OpFlags flags) { return false; }
		PROP bool dec(AssemblyWriter& out, const Operand& target, OpFlags flags) { return false; }
		PROP bool bxor(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool band(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool bor(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool mod(AssemblyWriter& out, const Operand& target, const Operand& op, OpFlags flags) { return false; }
		PROP bool jump(AssemblyWriter& out, const Operand& target) { return false; }
		PROP bool jump_if(AssemblyWriter& out, const Operand& target, const Operand& lhs, const Operand& rhs, const Operator* op, OpFlags flags) { return false; }
		PROP bool call(AssemblyWriter& out, const Operand& target) { return false; }
		PROP bool return_(AssemblyWriter& out) { return false; }
		// Maybe make syscall_code an Operand for flexibility ?
		PROP bool syscall(AssemblyWriter& out, int syscall_code) { return false; }
	};

	struct ArchitectureFuncs {
		decltype(&ArchitectureImpl::push) push;
		decltype(&ArchitectureImpl::pop) pop;
		decltype(&ArchitectureImpl::push_amount) push_amount;
		decltype(&ArchitectureImpl::pop_amount) pop_amount;
		decltype(&ArchitectureImpl::move) move;
		decltype(&ArchitectureImpl::add) add;
		decltype(&ArchitectureImpl::sub) sub;
		decltype(&ArchitectureImpl::mul) mul;
		decltype(&ArchitectureImpl::div) div;
		decltype(&ArchitectureImpl::inc) inc;
		decltype(&ArchitectureImpl::dec) dec;
		decltype(&ArchitectureImpl::bxor) bxor;
		decltype(&ArchitectureImpl::band) band;
		decltype(&ArchitectureImpl::bor) bor;
		decltype(&ArchitectureImpl::mod) mod;
		decltype(&ArchitectureImpl::jump) jump;
		decltype(&ArchitectureImpl::jump_if) jump_if;
		decltype(&ArchitectureImpl::call) call;
		decltype(&ArchitectureImpl::return_) return_;
		decltype(&ArchitectureImpl::syscall) syscall;
	};

	struct ArchitectureStruct {
#define SAME(NAME) decltype(ArchitectureImpl::NAME) NAME
#define SAME_ARR(NAME) array_view<std::remove_reference_t<decltype(ArchitectureImpl::NAME[0])>> NAME

		SAME(pointer_size);
		SAME_ARR(registers);
		SAME_ARR(instruction_names);
		SAME_ARR(calling_conventions);
		SAME_ARR(syscall_conventions);
		SAME(stack_pointer);
		ArchitectureFuncs funcs;

#undef SAME
#undef SAME_ARR
	};

	template<class T>
		requires std::is_base_of_v<ArchitectureImpl, T>
	const ArchitectureStruct build_arch() {
		return {
			.pointer_size = T::pointer_size,
			.registers = T::registers,
			.instruction_names = T::instruction_names,
			.calling_conventions = T::calling_conventions,
			.syscall_conventions = T::syscall_conventions,
			.stack_pointer = T::stack_pointer,
			.funcs = ArchitectureFuncs{
				.push = T::push,
				.pop = T::pop,
				.push_amount = T::push_amount,
				.pop_amount = T::pop_amount,
				.move = T::move,
				.add = T::add,
				.sub = T::sub,
				.mul = T::mul,
				.div = T::div,
				.inc = T::inc,
				.dec = T::dec,
				.bxor = T::bxor,
				.band = T::band,
				.bor = T::bor,
				.mod = T::mod,
				.jump = T::jump,
				.jump_if = T::jump_if,
				.call = T::call,
				.return_ = T::return_,
				.syscall = T::syscall,
			}
		};
	}

	extern std::map<std::string_view, const ArchitectureStruct*> Architectures;
}

