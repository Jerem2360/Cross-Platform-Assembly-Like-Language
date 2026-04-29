#pragma once
#include <vector>
#include <string_view>
#include <string>
#include <map>

#include "CPU.hpp"
#include "helpers.hpp"


namespace cpasm {
	struct AssemblerFuncs;
	struct ArchitectureFuncs;
	struct EnvironmentFuncs;
	class Program;
	struct Code;

	class Implementation {
		uint8_t _ptr_sz;
		array_view<const CPURegister*> _all_regs;
		std::vector<const CPURegister*> _integer_regs;
		std::vector<const CPURegister*> _float_regs;
		std::vector<const CPURegister*> _mixed_regs;
		std::map<std::string_view, std::string> _cmptime_vars;
		std::map<std::string_view, const CallingConvention*> _callconvs;
		array_view<std::string_view> _instr_names;
		std::string_view _entry_name;
		const AssemblerFuncs* _asm_funcs;
		const ArchitectureFuncs* _arch_funcs;
		const EnvironmentFuncs* _env_funcs;

	public:
		Implementation();
		void init(std::string_view arch_name, std::string_view assembler, std::string_view environment);
		array_view<const CPURegister*> registers() const;
		array_view<const CPURegister*> int_registers() const;
		array_view<const CPURegister*> float_registers() const;
		array_view<const CPURegister*> mixed_registers() const;
		uint8_t pointer_size() const;
		const CPURegister* reg_by_name(std::string_view name) const;
		std::string_view cmptime_var(std::string_view name) const;
		//const std::map<std::string_view, const CallingConvention*>& calling_conventions() const;
		const CallingConvention* calling_convention(std::string_view name) const;
		const CallingConvention* dflt_calling_convention() const;
		const AssemblerFuncs* assembler_functions() const;
		const ArchitectureFuncs* arch_functions() const;
		const EnvironmentFuncs* env_functions() const;
		std::string_view instruction_name(CPUInstruction instr) const;
		RegConventionFlags register_callconv(const CPURegister* reg, const CallingConvention* callconv) const;
		void call_prog_init(Program* program) const;
		void call_env_init(Code* owner) const;
		std::string_view entry_name() const;
	};

	extern Implementation CurrentImpl;
}

