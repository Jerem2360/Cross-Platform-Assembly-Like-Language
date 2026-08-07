#pragma once
#include <utility>
#include <sstream>
#include <map>

#include "Wordizer.hpp"
#include "../StackTrace.hpp"

/*
NOTE: This parser must be changed to improve error reporting.
- line number and filenames must be tracked
- proper error stacktraces must be maintianed
*/

namespace cpasm {
	class ParseResult;
	class ParserView;

	class Parser {
		const std::vector<CharGroup>* _data;
		unsigned int _cursor;

	public:
		class state {
			unsigned int _val;
			Parser* _src;

		public:
			explicit state(Parser&);
			void restore();
		};

		Parser(Wordizer& src);
		array_view<CharGroup> before() const;
		array_view<CharGroup> after() const;
		const CharGroup& operator[](int index) const;
		bool consume(int cnt = 1);
		bool consume_chars(int cnt);
		state save();
		ParserView view();


		// helper functions
		size_t remaining() const;
		bool skip_spaces();
		bool skip_newlines();
		bool skip_blanks();  // skip spaces and newlines
		bool keyword(std::string_view kwd, int* plineno = nullptr, bool skip = true);
		bool operator_(const Operator* op, int* plineno = nullptr, bool skip = true);
		const Operator* operator_of_type(OperationType type, int* plineno = nullptr, bool skip = true);
		bool delimiter(std::string_view del, int* plineno = nullptr, bool skip = true);
		std::string_view name(int* plineno = nullptr, bool skip = true);
		Result fail(std::string_view msg, state st, const std::source_location where = std::source_location::current());
		Result fail(const std::string& msg, state st, const std::source_location where = std::source_location::current());
		template<size_t Len>
		inline Result fail(const char(&msg)[Len], state st, const std::source_location where = std::source_location::current()) {
			return this->fail(sview(msg), st, where);
		}

		Result propagate(std::string_view msg, Result& res, state st, const std::source_location where = std::source_location::current());
		Result propagate(const std::string& msg, Result& res, state st, const std::source_location where = std::source_location::current());
		template<size_t Len>
		inline Result propagate(const char(&msg)[Len], Result& res, state st, const std::source_location where = std::source_location::current()) {
			return this->propagate(sview(msg), res, st, where);
		}
	};


	class ParseErrorStack {
		std::map<std::string, ParseErrorStack> _subelements;

	public:
		ParseErrorStack(std::string_view msg);
		ParseErrorStack();
		void add(std::string_view name, const ParseErrorStack& other);
		bool is_empty() const;
		std::string compute_msg(int tablevel = 0) const;
		explicit operator bool() const;
	};

	class ParseResult {
		std::string _msg;
		ParseErrorStack _stack;


	public:
		ParseResult(std::string_view msg, const ParseErrorStack& st);
		ParseResult();

		std::string message() const;
		explicit operator bool() const;
		void push(std::string_view name, const ParseResult&);

		template<class ...TArgs>
		inline static ParseResult fail(Parser::state st, TArgs&& ...args) {
			std::stringstream text_fs;
			(text_fs << ... << std::forward<TArgs>(args));

			st.restore();

			return ParseResult(text_fs.view(), {});
		}
		static ParseResult success();
		static ParseResult& propagate(Parser::state st, ParseResult& res);
	};

	class ParserView {
		Parser* _parser;
		int _cursor;

		ParserView(Parser&, int cursor);

	public:
		ParserView(Parser&);
		char operator[](int idx) const;
		size_t size() const;
		ParserView advance(int amount) const;
		int pos() const;
	};
}

