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
		PROP std::string_view instr_extension(std::string_view instr_name, std::string_view arch_name, const std::vector<Operand>& operands, const Program* prog) {
			//std::cout << "instruction=" << instr_name << " arch=" << arch_name << '\n';
			if (arch_name != "x86_64")
				return "";
			//std::cout << "Arch OK\n";
			if ((instr_name != "CALL") && (instr_name != "JMP"))
				return "";
			//std::cout << "Instr OK\n";
			if (operands.size() != 1)
				return "";
			//std::cout << "Operand count OK\n";
			const Operand& op = operands[0];
			std::string_view op_name;
			if (!op.as_const_label(&op_name))
				return "";
			//std::cout << "Operand name " << op_name << " OK\n";
			if (!prog->imports(op_name))
				return "";
			//std::cout << "Imported OK\n";
			return "wrt ..plt";
		}
    };

    extern const EnvironmentStruct env = build_env<LinuxImpl>();
}

