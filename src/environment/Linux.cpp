#include "Linux.hpp"
#include "../Implementation.hpp"
#include "../Program.hpp"


namespace cpasm::linux {

    struct LinuxImpl : EnvironmentImpl {
        PROP std::string_view supported_callconvs[] = {
			"systemv_x64",
			"systemv_arm64",
            "varargs",
		};
		PROP std::string_view syscallconv_priority[] = {
			"systemv_x64",
            "systemv_arm64",
		};
		PROP std::string_view entry_name = "_start";
		PROP bool exit_process(AssemblyWriter& out, const Code* owner, const Operand& exitcode) {
			return owner->gen_syscall(
                out, "exit_group", {}, obj2aview(&exitcode), true
            );
		}
    };

    extern const EnvironmentStruct env = build_env<LinuxImpl>();
}

