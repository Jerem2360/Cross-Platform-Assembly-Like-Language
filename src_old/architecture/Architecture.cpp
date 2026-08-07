#include "Architecture.hpp"
#include "x86_64.hpp"


namespace cpasm {
	std::map<std::string_view, const ArchitectureStruct*> Architectures = {
		{ "x86_64", &x86_64::Arch },
	};
}

