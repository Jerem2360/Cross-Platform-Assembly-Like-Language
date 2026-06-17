#pragma once
#include <map>
#include <string_view>

#include "../helpers.hpp"
#include "../Operand.hpp"


namespace cpasm {
	class Program;
	class AssemblyWriter;

#define PROP inline static


	struct EnvironmentImpl {
		PROP std::string_view supported_callconvs[] = {
			"",
		};
		PROP std::string_view syscallconv_priority[] = {
			"",
		};
		PROP std::string_view entry_name = "";

		PROP void prog_init(Program*) {}

		// This might need access to the code object
		PROP bool exit_process(AssemblyWriter&, const Code* owner, const Operand& exitcode) {
			return false;
		}
		PROP void env_init(Code* owner) {}
		// TODO: find a better way of extending instructions
		PROP std::string_view instr_extension(std::string_view instr_name, std::string_view arch_name, const std::vector<Operand>& operands, const Program* prog) { return ""; }
	};

	struct EnvironmentFuncs {
		decltype(&EnvironmentImpl::prog_init) prog_init;
		decltype(&EnvironmentImpl::exit_process) exit_process;
		decltype(&EnvironmentImpl::env_init) env_init;
		decltype(&EnvironmentImpl::instr_extension) instr_extension;
	};


	struct EnvironmentStruct {
		array_view<std::string_view> supported_callconvs;
		array_view<std::string_view> syscallconv_priority;
		decltype(EnvironmentImpl::entry_name) entry_name;
		EnvironmentFuncs funcs;
	};

	template<class T>
		requires std::is_base_of_v<EnvironmentImpl, T>
	EnvironmentStruct build_env() {
		return {
			.supported_callconvs = T::supported_callconvs,
			.syscallconv_priority = T::syscallconv_priority,
			.entry_name = T::entry_name,
			.funcs = EnvironmentFuncs{
				.prog_init = &T::prog_init,
				.exit_process = &T::exit_process,
				.env_init = &T::env_init,
				.instr_extension = &T::instr_extension,
			},
		};
	}

	extern std::map<std::string_view, const EnvironmentStruct*> Environments;
}
