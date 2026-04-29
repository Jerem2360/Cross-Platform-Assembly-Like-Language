#include "Win64.hpp"

#include <string_view>
#include "../Program.hpp"
#include "../AsmWriter.hpp"

namespace cpasm::win64 {
	static std::string _k32_imports[] = {
		"ExitProcess",
	};


	struct Win64Impl : EnvironmentImpl {
		PROP std::string_view supported_callconvs[] = {
			"win_x64",
			"win_arm64",
		};
		PROP std::string_view entry_name = "_start";
		PROP void prog_init(Program* prog) {
			prog->add_import("kernel32.lib", _k32_imports, 0);
		}
		PROP bool exit_process(AssemblyWriter& out, const Code* owner, const Operand& exitcode) {
			const CPURegister* reg = out.current_callconv()->arg_registers[0];
			return owner->gen_function_call(
				out,
				Operand::from_const_label("ExitProcess", nullptr, 0),
				{},
				obj2aview(&exitcode),
				true
			);
		}
		PROP void env_init(Code* owner) {}  // Do we need to call _CRT_MAIN_STARTUP here ?
	};


	extern const EnvironmentStruct env = build_env<Win64Impl>();
}

