#include "Assembler.hpp"
#include "NASM.hpp"


namespace cpasm {
	std::map<std::string_view, const AssemblerStruct*> Assemblers = {
		{ "NASM", &NASM::Asm },
	};
}

