#include "Environment.hpp"
#include "Win64.hpp"
#include "Linux.hpp"


namespace cpasm {
	std::map<std::string_view, const EnvironmentStruct*> Environments = {
		{ "win64", &win64::env },
		{ "linux", &linux::env },
	};
}

