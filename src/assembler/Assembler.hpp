#pragma once
#include <ostream>
#include <map>
#include <vector>

#include "../Operand.hpp"
#include "../CPU.hpp"

#define PROP inline static


namespace cpasm {
	struct AssemblerImpl {
		PROP bool cpu_instruction(std::ostream& out, std::string_view instr_name, std::vector<writer_t> operands, std::string_view comment) {
			return false;
		}
		PROP bool define_bytes(std::ostream& out, uint8_t size, writer_t value) {
			return false;
		}
		PROP bool reserve_bytes(std::ostream& out, uint8_t size) {
			return false;
		}
		PROP bool set_bitness(std::ostream& out, uint8_t bits) {
			return false;
		}
		PROP bool def_global_label(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool def_local_label(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool use_global_label(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool use_local_label(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool include_file(std::ostream& out, std::string_view filename) {
			return false;
		}
		PROP bool import_symbol(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool export_symbol(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool const_int(std::ostream& out, size_t value) {
			return false;
		}
		PROP bool const_float(std::ostream& out, double value) {
			return false;
		}
		PROP bool const_char(std::ostream& out, char value) {
			return false;
		}
		PROP bool const_str(std::ostream& out, std::string_view value) {
			return false;
		}
		PROP bool deref(std::ostream& out, writer_t base, writer_t index, writer_t scale, uint8_t size) {
			return false;
		}
		PROP bool register_(std::ostream& out, const CPURegister* reg) {
			return false;
		}
		PROP bool comment(std::ostream& out, std::string_view text) {
			return false;
		}
		/*
		Return a random and unique identifier based on the provided text string.
		It is acceptable for debugging to not make the identifier random. However,
		each identifier generated that way must remain unique within a given 
		object file.
		*/
		PROP std::string random_identifier(std::string_view base) {
			return "";
		}
		PROP bool section(std::ostream& out, std::string_view name) {
			return false;
		}
		PROP bool enable_relative_addr(std::ostream& out) {
			return false;
		}
	};


	struct AssemblerFuncs {
		decltype(&AssemblerImpl::cpu_instruction) cpu_instruction;
		decltype(&AssemblerImpl::define_bytes) define_bytes;
		decltype(&AssemblerImpl::reserve_bytes) reserve_bytes;
		decltype(&AssemblerImpl::set_bitness) set_bitness;
		decltype(&AssemblerImpl::def_global_label) def_global_label;
		decltype(&AssemblerImpl::def_local_label) def_local_label;
		decltype(&AssemblerImpl::use_global_label) use_global_label;
		decltype(&AssemblerImpl::use_local_label) use_local_label;
		decltype(&AssemblerImpl::include_file) include_file;
		decltype(&AssemblerImpl::import_symbol) import_symbol;
		decltype(&AssemblerImpl::export_symbol) export_symbol;
		decltype(&AssemblerImpl::const_int) const_int;
		decltype(&AssemblerImpl::const_float) const_float;
		decltype(&AssemblerImpl::const_char) const_char;
		decltype(&AssemblerImpl::const_str) const_str;
		decltype(&AssemblerImpl::deref) deref;
		decltype(&AssemblerImpl::register_) register_;
		decltype(&AssemblerImpl::comment) comment;
		decltype(&AssemblerImpl::random_identifier) random_identifier;
		decltype(&AssemblerImpl::section) section;
		decltype(&AssemblerImpl::enable_relative_addr) enable_relative_addr;
	};

	struct AssemblerStruct {
		AssemblerFuncs funcs;
	};

	template<class T>
		requires std::is_base_of_v<AssemblerImpl, T>
	inline AssemblerStruct build_assembler() {
		return {
			.funcs = AssemblerFuncs{
				.cpu_instruction = &T::cpu_instruction,
				.define_bytes = &T::define_bytes,
				.reserve_bytes = &T::reserve_bytes,
				.set_bitness = &T::set_bitness,
				.def_global_label = &T::def_global_label,
				.def_local_label = &T::def_local_label,
				.use_global_label = &T::use_global_label,
				.use_local_label = &T::use_local_label,
				.include_file = &T::include_file,
				.import_symbol = &T::import_symbol,
				.export_symbol = &T::export_symbol,
				.const_int = &T::const_int,
				.const_float = &T::const_float,
				.const_char = &T::const_char,
				.const_str = &T::const_str,
				.deref = &T::deref,
				.register_ = &T::register_,
				.comment = &T::comment,
				.random_identifier = &T::random_identifier,
				.section = &T::section,
				.enable_relative_addr = &T::enable_relative_addr,
			}
		};
	}

	extern std::map<std::string_view, const AssemblerStruct*> Assemblers;
}

