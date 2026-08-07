#pragma once
#include <string_view>
#include <sstream>


#include "parsing/Parser.hpp"


namespace cpasm {
	static constexpr int E_STR_INVALID_ESCAPE_CHARCODE = -1;
	static constexpr int E_STR_ESCAPE_EOF = -2;
	static constexpr int E_STR_INVALID_ESCAPE_CHAR = -3;
	static constexpr int E_INVALID_STR_CHAR = -4;
	static constexpr int E_INVALID_DIGIT = -5;
	static constexpr int E_INVALID_INT_LIT = -6;
	static constexpr int E_INT_OVERFLOW = -7;
	static constexpr int E_INVALID_FLOAT_LIT = -8;

	/*
	Parse a string, excluding the first delimiter character and 
	write it to the output stream.
	Upon success, return the number of characters read from the
	input, excluding the final delimiter character. 
	Upon fail, return a negative error status.
	*/
	int parse_string_chars(ParserView data, std::stringstream& out, char delimiter = '"');

	/*
	Parse an integer literal, and put the result in `*out`.
	Upon failure, return one of the E_* constants and don't write to `*out`.
	Upon success, return the number of characters read.
	*/
	int parse_integer_literal(std::string_view data, size_t* out);

	/*
	Parse a floating-point literal, and put the result in `*out`.
	`decimal_sep` is the character that separates the integral from the decimal part of the number.
	Upon failure, return one of the E_* constants and don't write to `*out`.
	Upon success, return the number of characters read.
	*/
	int parse_float_literal(std::string_view data, double* out, char decimal_sep = '.');

	/*
	Write to the provided output stream a string for the provided error code.
	This is used to decode the return value of `parse_string_chars`.
	*/
	void write_string_error(int err, std::ostream& out);
}

