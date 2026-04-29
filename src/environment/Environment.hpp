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
		PROP std::string_view entry_name = "";

		PROP void prog_init(Program*) {}

		// This might need access to the code object
		PROP bool exit_process(AssemblyWriter&, const Code* owner, const Operand& exitcode) {
			return false;
		}
		PROP void env_init(Code* owner) {}
	};

	struct EnvironmentFuncs {
		decltype(&EnvironmentImpl::prog_init) prog_init;
		decltype(&EnvironmentImpl::exit_process) exit_process;
		decltype(&EnvironmentImpl::env_init) env_init;
	};


	struct EnvironmentStruct {
		array_view<std::string_view> supported_callconvs;
		decltype(EnvironmentImpl::entry_name) entry_name;
		EnvironmentFuncs funcs;
	};

	template<class T>
		requires std::is_base_of_v<EnvironmentImpl, T>
	EnvironmentStruct build_env() {
		return {
			.supported_callconvs = T::supported_callconvs,
			.entry_name = T::entry_name,
			.funcs = EnvironmentFuncs{
				.prog_init = &T::prog_init,
				.exit_process = &T::exit_process,
				.env_init = &T::env_init,
			},
		};
	}

	extern std::map<std::string_view, const EnvironmentStruct*> Environments;
}
