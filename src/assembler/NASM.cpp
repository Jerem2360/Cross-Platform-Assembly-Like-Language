#include <string_view>
#include "Assembler.hpp"


namespace cpasm::NASM {
	/*
	------------------------------
	Helpers
	------------------------------
	*/
	static std::string_view _db_sizes[] = {
		"d?",
		"db",  // 1
		"dw",  // 2
		"d?",
		"dd",  // 4
		"d?",
		"d?",
		"d?",
		"dq",  // 8
	};

	static std::string_view _resb_sizes[] = {
		"res?",
		"resb",  // 1
		"resw",  // 2
		"res?",
		"resd",  // 4
		"res?",
		"res?",
		"res?",
		"resq",  // 8
	};


	static std::string_view _deref_sizes[] = {
		"????",
		"BYTE",  // 1
		"WORD",  // 2
		"????",
		"DWORD",  // 4
		"????",
		"????",
		"????",
		"QWORD",  // 8
	};

	static bool _is_printable(char c) {
		return (c >= ' ') && (c < 127);
	}

	struct StringWriter {
		std::string_view orig;
		int cursor;

		inline StringWriter(std::string_view src) :
			orig(src), cursor(0)
		{ }
		std::string_view next_sequence(bool* out_isprintable) const {
			if (this->cursor >= this->orig.size())
				return {};

			if (!_is_printable(this->orig[this->cursor])) {  // non printable character
				*out_isprintable = false;
				return ssubview(this->orig, this->cursor, 1);
			}

			int size = 0;

			for (int i = this->cursor; i < this->orig.size(); i++) {
				if (!_is_printable(this->orig[i]))  // non printable
					break;
				size += 1;
			}

			if (!size)
				return {};
			*out_isprintable = true;
			return ssubview(this->orig, this->cursor, size);
		}

		void write_sequence(std::string_view seq, bool is_printable, std::ostream& out) {
			if (is_printable) {
				out << '"';
				this->write_text(seq, out);
				out << '"';
				return;
			}
			out << (int)(uint8_t)seq[0];
		}

		void write_text(std::string_view s, std::ostream& out) {
			for (char c : s) {
				if (c == '\\')
					out << "\\\\";
				else
					out << c;
			}
		}
	};


	/*
	------------------------------------
	Implementation
	------------------------------------
	*/


	struct NasmImpl : AssemblerImpl {
		PROP bool cpu_instruction(std::ostream& out, std::string_view instr_name, std::vector<writer_t> operands, std::string_view comment) {
			out << "    " << instr_name << ' ';
			bool first = true;
			for (auto& writer : operands) {
				if (!first)
					out << ", ";
				first = false;
				writer(out);
			}
			if (comment.size())
				out << "; " << comment;

			out << '\n';
			return true;
		}
		PROP bool define_bytes(std::ostream& out, uint8_t size, writer_t value) {
			out << "    " << _db_sizes[size] << ' ';
			value(out);
			out << '\n';
			return true;
		}
		PROP bool reserve_bytes(std::ostream& out, uint8_t size) {
			out << "    " << _resb_sizes[size] << " 1\n";
			return true;
		}
		PROP bool set_bitness(std::ostream& out, uint8_t bits) {
			out << "bits " << (int)bits << '\n';
			return true;
		}
		PROP bool def_global_label(std::ostream& out, std::string_view name) {
			out << name << ":\n";
			return true;
		}
		PROP bool def_local_label(std::ostream& out, std::string_view name) {
			out << '.' << name << ":\n";
			return true;
		}
		PROP bool use_global_label(std::ostream& out, std::string_view name) {
			out << '$' << name;
			return true;
		}
		PROP bool use_local_label(std::ostream& out, std::string_view name) {
			out << '.' << name;
			return true;
		}
		PROP bool include_file(std::ostream& out, std::string_view filename) {
			out << "    incbin \"" << filename << "\"\n";
			return true;
		}
		PROP bool import_symbol(std::ostream& out, std::string_view name) {
			out << "extern " << name << '\n';
			return true;
		}
		PROP bool export_symbol(std::ostream& out, std::string_view name) {
			out << "global " << name << '\n';
			return true;
		}
		PROP bool const_int(std::ostream& out, size_t value) {
			out << value;
			return true;
		}
		PROP bool const_float(std::ostream& out, double value) {
			out << value;
			return true;
		}
		PROP bool const_char(std::ostream& out, char value) {
			out << '\'' << value << '\'';
			return true;
		}
		PROP bool const_str(std::ostream& out, std::string_view value) {
			StringWriter writer(value);

			for (int i = 0;; i++) {
				bool is_printable;
				std::string_view seq = writer.next_sequence(&is_printable);
				if (!seq.size())
					break;

				if (i)
					out << ", ";

				writer.cursor += (int)seq.size();
				writer.write_sequence(seq, is_printable, out);
			}

			return true;
		}
		PROP bool deref(std::ostream& out, writer_t base, writer_t index, writer_t scale, uint8_t size) {
			out << _deref_sizes[size] << " [";
			base(out);
			if (index) {
				out << " + ";
				index(out);
				if (scale) {
					out << '*';
					scale(out);
				}
			}
			out << ']';
			return true;
		}
		PROP bool register_(std::ostream& out, const CPURegister* reg) {
			out << reg->name;
			return true;
		}
		PROP bool comment(std::ostream& out, std::string_view text) {
			out << "; " << text << '\n';
			return true;
		}
		PROP std::string random_identifier(std::string_view base) {
			static int idx = 0;
			std::stringstream res;
			res << "__" << base << "@cf5f19" << idx++;
			return res.str();
		}
		PROP bool section(std::ostream& out, std::string_view name) {
			out << "SECTION " << name << '\n';
			return true;
		}
		PROP bool enable_relative_addr(std::ostream& out) {
			out << "default rel\n";
			return true;
		}
	};

	extern const AssemblerStruct Asm = build_assembler<NasmImpl>();
}

