#pragma once
#include <string_view>
#include <sstream>


#include "parsing/Parser.hpp"


namespace cpasm {
	constexpr int E_STR_INVALID_ESCAPE_CHARCODE = -1;
	constexpr int E_STR_ESCAPE_EOF = -2;
	constexpr int E_STR_INVALID_ESCAPE_CHAR = -3;
	constexpr int E_INVALID_STR_CHAR = -4;

	/*
	Parse a string, excluding the first delimiter character and 
	write it to the output stream.
	Upon success, return the number of characters read from the
	input, excluding the final delimiter character. 
	Upon fail, return a negative error status.
	*/
	int parse_string_chars(ParserView data, std::stringstream& out, char delimiter = '"');
	/*
	Write to the provided output stream a string for the provided error code.
	This is used to decode the return value of `parse_string_chars`.
	*/
	void write_string_error(int err, std::ostream& out);
}

