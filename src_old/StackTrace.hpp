#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <memory>
#include <source_location>

#include "helpers.hpp"

#define _SRC_LOCATION(name) const std::source_location name = std::source_location::current()

namespace cpasm {
	class StackTrace {
		std::string _msg;
		std::source_location _where;
		int _src_lineno;
		std::vector<std::unique_ptr<StackTrace>> _children;

	public:
		StackTrace(std::string_view message, std::vector<std::unique_ptr<StackTrace>> children, std::source_location where);
		StackTrace(std::string_view message, int lineno, std::source_location where);

		StackTrace(StackTrace&&) noexcept = default;
		StackTrace& operator =(StackTrace&&) noexcept = default;
		StackTrace(const StackTrace&) = delete;
		StackTrace& operator =(const StackTrace&) = delete;

		void write(std::ostream&, std::string_view file_name, int tablevel = 0) const;
	};


	class Result;


	class ResultBunch {
		std::vector<std::unique_ptr<StackTrace>> _traces;

	public:
		ResultBunch();
		ResultBunch(const ResultBunch&) = delete;
		ResultBunch& operator =(const ResultBunch&) = delete;
		ResultBunch(ResultBunch&&) = default;
		ResultBunch& operator =(ResultBunch&&) = default;

		void add(Result&);
		ResultBunch& operator +=(Result&);
		ResultBunch& operator +=(Result&&);

		Result push(std::string_view msg, _SRC_LOCATION(where));
		Result push(const std::string& msg, _SRC_LOCATION(where));
		template<size_t Len>
		inline auto push(const char(&msg)[Len], _SRC_LOCATION(where)) {
			return this->push(std::string_view(msg), where);
		}
	};


	class Result {
		friend class ResultBunch;

		std::unique_ptr<StackTrace> _stacktrace;
		std::string _filename;

		explicit Result(std::string_view msg, int lineno, std::source_location where);
		explicit Result(std::string_view msg, std::vector<std::unique_ptr<StackTrace>> children, std::source_location where);

	public:
		Result();

		Result(const Result&) = delete;
		Result& operator =(const Result&) = delete;
		Result(Result&&) noexcept = default;
		Result& operator =(Result&&) noexcept = default;

		static Result fail(std::string_view msg, int lineno, _SRC_LOCATION(where));
		static Result fail(const std::string& msg, int lineno, _SRC_LOCATION(where));
		template<size_t Len>
		inline static Result fail(const char(&msg)[Len], int lineno, _SRC_LOCATION(where)) {
			return Result(sview(msg), lineno, where);
		}

		Result push(std::string_view msg, _SRC_LOCATION(where));
		Result push(const std::string& msg, _SRC_LOCATION(where));
		template<size_t Len>
		inline Result push(const char(&msg)[Len], _SRC_LOCATION(where)) {
			return this->push(sview(msg), where);
		}

		Result with_filename(std::string_view filename);

		void write(std::ostream&) const;

		explicit operator bool() const;  // This is true if the object holds no error
	};
}


#undef _SRC_LOCATION

