#include "Implementation.hpp"
#include "architecture/Architecture.hpp"
#include "assembler/Assembler.hpp"
#include "environment/Environment.hpp"
#include "Program.hpp"


namespace cpasm {
	Implementation CurrentImpl;

	Implementation::Implementation() :
		_ptr_sz(0), _all_regs(), _integer_regs(), _float_regs(), _mixed_regs(), _cmptime_vars(), _asm_funcs(), _arch_funcs(), _instr_names(), _env_funcs()
	{ }

	void Implementation::init(std::string_view arch_name, std::string_view assembler, std::string_view environment) {
		this->_integer_regs = {};
		this->_float_regs = {};
		this->_mixed_regs = {};
		this->_callconvs = {};
		std::map<std::string_view, const CallingConvention*> _raw_callconvs = {};

		auto parch = map_get<std::string_view, const ArchitectureStruct*>(Architectures, arch_name, nullptr);
		if (parch) {
			this->_cmptime_vars["ARCH"] = arch_name;
			this->_cmptime_vars["BITS"] = std::to_string(parch->pointer_size * 8);

			this->_all_regs = parch->registers;
			this->_ptr_sz = parch->pointer_size;

			for (auto reg : this->_all_regs) {
				if (reg->support.supportsIntVar() && reg->support.supportsFloatVar())
					this->_mixed_regs.push_back(reg);
				else if (reg->support.supportsIntVar())
					this->_integer_regs.push_back(reg);
				else if (reg->support.supportsFloatVar())
					this->_float_regs.push_back(reg);
			}

			for (auto& cconv : parch->calling_conventions) {
				_raw_callconvs[cconv->name] = cconv;
			}

			this->_instr_names = parch->instruction_names;
			this->_arch_funcs = &parch->funcs;
		}
		else {
			this->_all_regs = {};
			this->_cmptime_vars["ARCH"] = "?";
			this->_cmptime_vars["BITS"] = "?";
		}

		auto pasm = map_get<std::string_view, const AssemblerStruct*>(Assemblers, assembler, nullptr);
		if (pasm) {
			this->_asm_funcs = &pasm->funcs;
			this->_cmptime_vars["ASM"] = assembler;
		}
		else {
			this->_cmptime_vars["ASM"] = "?";
		}

		auto penv = map_get<std::string_view, const EnvironmentStruct*>(Environments, environment, nullptr);
		if (penv) {
			this->_cmptime_vars["ENV"] = environment;
			for (auto& envname : penv->supported_callconvs) {
				auto cc = map_get<std::string_view, const CallingConvention*>(_raw_callconvs, envname, nullptr);
				if (!cc)
					continue;
				this->_callconvs[envname] = cc;
			}
			this->_env_funcs = &penv->funcs;
			this->_entry_name = penv->entry_name;
		}
		else {
			this->_cmptime_vars["ENV"] = "?";
		}
	}


	array_view<const CPURegister*> Implementation::registers() const {
		return this->_all_regs;
	}
	array_view<const CPURegister*> Implementation::int_registers() const {
		return vec2aview(this->_integer_regs);
	}
	array_view<const CPURegister*> Implementation::float_registers() const {
		return vec2aview(this->_float_regs);
	}
	array_view<const CPURegister*> Implementation::mixed_registers() const {
		return vec2aview(this->_mixed_regs);
	}
	uint8_t Implementation::pointer_size() const {
		return this->_ptr_sz;
	}
	const CPURegister* Implementation::reg_by_name(std::string_view name) const {
		for (const CPURegister* reg : this->_all_regs) {
			if (reg->name == name)
				return reg;
		}
		return nullptr;
	}
	std::string_view Implementation::cmptime_var(std::string_view name) const {
		return sview(map_get<std::string_view, std::string>(this->_cmptime_vars, name, ""));
	}
	//const std::map<std::string_view, const CallingConvention*>& Implementation::calling_conventions() const {
	//	return this->_callconvs;
	//}
	const CallingConvention* Implementation::calling_convention(std::string_view name) const {
		return map_get<std::string_view, const CallingConvention*>(this->_callconvs, name, nullptr);
	}
	const CallingConvention* Implementation::dflt_calling_convention() const {
		for (auto& [name, cconv] : this->_callconvs) {
			return cconv;
		}
		return nullptr;
	}
	const AssemblerFuncs* Implementation::assembler_functions() const {
		return this->_asm_funcs;
	}
	const ArchitectureFuncs* Implementation::arch_functions() const {
		return this->_arch_funcs;
	}
	const EnvironmentFuncs* Implementation::env_functions() const {
		return this->_env_funcs;
	}
	std::string_view Implementation::instruction_name(CPUInstruction instr) const {
		if (instr >= this->_instr_names.size())
			return "";
		return this->_instr_names[instr];
	}
	RegConventionFlags Implementation::register_callconv(const CPURegister* reg, const CallingConvention* callconv) const {
		auto where = std::find(this->_all_regs.begin(), this->_all_regs.end(), reg->toplevel());
		if (where == this->_all_regs.end())
			return RegConventionFlags(false, false);

		size_t offset = where - this->_all_regs.begin();

		array_view<RegConventionFlags> conv_flags = callconv->reg_convflags;

		if (offset >= conv_flags.size())
			return RegConventionFlags(false, false);

		return conv_flags[offset];
	}
	void Implementation::call_prog_init(Program* program) const {
		if (this->_env_funcs && this->_env_funcs->prog_init) this->_env_funcs->prog_init(program);
	}
	void Implementation::call_env_init(Code* owner) const {
		if (this->_env_funcs && this->_env_funcs->env_init) this->_env_funcs->env_init(owner);
	}
	std::string_view Implementation::entry_name() const {
		return this->_entry_name;
	}
}

