#include "StringCoding.hpp"

#include <ostream>
#include <bit>

#include "helpers.hpp"


namespace cpasm {

	static std::string_view _error_msgs[] = {
		"Invalid character in string definition",
		"Invalid character escape sequence",
		"End of file unexpectedly enountered in string",
		"Invalid escape character code",
	};

	static std::string_view _get_errmsg(int code) {
		int idx = code + (int)array_len(_error_msgs);
		if ((idx < 0) || (idx >= array_len(_error_msgs)))
			return "Unknown error";
		return _error_msgs[idx];
	}

	static inline constexpr int _make_error(int code, int position) {
		return code * 1000 + position;
	}
	static inline constexpr int _decode_error(int err, int* out_pos) {
		*out_pos = (-err) % 1000;
		int err_mul = err + *out_pos;
		return err_mul / 1000;
	}

	static inline int _fail(int code, const ParserView& view) {
		return _make_error(code, view.pos());
	}


	static std::string_view _advance_str(std::string_view src, int count) {
		return ssubview(src, count, src.size() - count);
	}

	static bool _is_printable(char c) {
		return c >= ' ';
	}

	static int _get_digit(char c, int base) {
		int idx = c - '0';
		if (idx < 0)
			return E_STR_INVALID_ESCAPE_CHARCODE;
		if (idx < 10) {
			if (idx >= base)
				return E_STR_INVALID_ESCAPE_CHARCODE;
			return idx;
		}
		int g_idx = c - 'A';
		if (g_idx < 0)
			return E_STR_INVALID_ESCAPE_CHARCODE;
		if (g_idx >= 26) {
			g_idx = c - 'a';
			if (g_idx < 0)
				return E_STR_INVALID_ESCAPE_CHARCODE;
			if (g_idx >= 26)
				return E_STR_INVALID_ESCAPE_CHARCODE;
		}
		if ((g_idx + 10) >= base)
			return E_STR_INVALID_ESCAPE_CHARCODE;
		return g_idx + 10;
	}

	static int _parse_number(ParserView chars, int base, int* dig_cnt, int max_len = INT_MAX) {
		int res = 0;

		if (!chars.size())
			return E_STR_ESCAPE_EOF;

		for (int i = 0; i < max_len; i++) {
			int dig;
			if (chars.size() > i)
				dig = _get_digit(chars[i], base);
			else
				dig = E_STR_ESCAPE_EOF;

			if (dig < 0) {
				if (!i)
					return _fail(dig, chars);
				break;
			}
			
			res *= base;
			res += dig;
			*dig_cnt += 1;
		}

		return res;
	}

	static int _get_octal_escape(ParserView chars, char* out) {
		int dig_cnt = 0;

		int number = _parse_number(chars, 8, &dig_cnt, 3);
		if (number > UCHAR_MAX)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		if (number < 0)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		*out = (char)number;
		return dig_cnt + 2;  // accounting for the '\' and the 'o'
	}

	static int _get_decimal_escape(ParserView chars, char* out) {
		int dig_cnt = 0;

		int number = _parse_number(chars, 10, &dig_cnt, 3);
		if (number > UCHAR_MAX)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		if (number < 0)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		*out = (char)number;
		return dig_cnt + 1;  // accounting for the '\'
	}

	static int _get_bin_escape(ParserView chars, char* out) {
		int dig_cnt = 0;

		int number = _parse_number(chars, 2, &dig_cnt, 7);
		if (number > UCHAR_MAX)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		if (number < 0)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		*out = (char)number;
		return dig_cnt + 2;  // accounting for the '\' and the 'B'
	}

	static int _get_hex_escape(ParserView chars, char* out) {
		int dig_cnt = 0;

		int number = _parse_number(chars, 16, &dig_cnt, 2);
		if (number > UCHAR_MAX)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		if (number < 0)
			return _fail(E_STR_INVALID_ESCAPE_CHARCODE, chars);

		*out = (char)number;
		return dig_cnt + 2;  // accounting for the '\' and the 'x'
	}

	static int _get_escape(ParserView chars, char* out) {
		if (!chars.size())
			return 0;
		switch (chars[0]) {
		case 'a':
			*out = '\a';
			return 2;
		case 'b':
			*out = '\b';
			return 2;
		case 'f':
			*out = '\f';
			return 2;
		case 'n':
			*out = '\n';
			return 2;
		case 'r':
			*out = '\r';
			return 2;
		case 't':
			*out = '\t';
			return 2;
		case 'v':
			*out = '\v';
			return 2;
		case '\\':
			*out = '\\';
			return 2;
		case '\'':
			*out = '\'';
			return 2;
		case '"':
			*out = '"';
			return 2;
		case '`':
			*out = '`';
			return 2;
		case 'o':
			return _get_octal_escape(chars.advance(1), out);
		case 'x':
			return _get_hex_escape(chars.advance(1), out);
		case 'B':
			return _get_bin_escape(chars.advance(1), out);
		}

		return _get_decimal_escape(chars, out);
	}

	static int _get_char(ParserView data, char* out) {
		if (!data.size())
			return 0;
		if (data[0] == '\\')
			return _get_escape(data.advance(1), out);
		if (_is_printable(data[0])) {
			*out = data[0];
			return 1;
		}
		return _fail(E_INVALID_STR_CHAR, data);  // invalid string char
	}

	int parse_string_chars(ParserView data, std::stringstream& out, char delimiter) {
		
		while (1) {
			if (data[0] == delimiter)  // end of the string
				return data.pos();
			char c;
			int cnt = _get_char(data, &c);
			if (cnt < 0)  // failed to get a character
				return cnt;
			out << c;
			data = data.advance(cnt);
		}
	}

	void write_string_error(int err, std::ostream& out) {
		int pos;
		int code = _decode_error(err, &pos);
		out << _get_errmsg(code) << " at position " << pos;
	}
}

