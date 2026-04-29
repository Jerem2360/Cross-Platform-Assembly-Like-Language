#include "Parser.hpp"


namespace cpasm {
	Parser::Parser(Wordizer& src) :
		_data(&src.result()), _cursor(0)
	{ }
	array_view<CharGroup> Parser::before() const {
		return { this->_data->data(), this->_cursor };
	}
	array_view<CharGroup> Parser::after() const {
		return { this->_data->data() + this->_cursor, this->_data->size() - this->_cursor };
	}
	const CharGroup& Parser::operator[](int idx) const {
		int off = this->_cursor + idx;
		return this->_data->at(static_cast<size_t>(off));
	}
	bool Parser::consume(int cnt) {
		if (this->_cursor + cnt > this->_data->size())
			return false;
		this->_cursor += cnt;
		return true;
	}
	bool Parser::consume_chars(int cnt) {
		int grp_cnt = 0;
		for (int i = 0; i < this->remaining(); i++) {
			auto& grp = this->operator[](i);
			cnt -= (int)grp.chars.size();
			grp_cnt += 1;
			if (cnt <= 0)
				break;
		}
		return this->consume(grp_cnt);
	}
	Parser::state Parser::save() {
		return Parser::state(*this);
	}
	ParserView Parser::view() {
		return ParserView(*this);
	}

	size_t Parser::remaining() const {
		return this->_data->size() - this->_cursor;
	}
	bool Parser::skip_spaces() {
		if (!this->remaining())
			return false;
		bool res = false;
		while ((*this)[0].type == CharGroupType::SPACE) {
			this->consume();
			res = true;
		}
		return res;
	}
	bool Parser::skip_newlines() {
		if (!this->remaining())
			return false;
		bool res = false;
		while ((*this)[0].type == CharGroupType::NEW_LINE) {
			this->consume();
			res = true;
		}
		return res;
	}
	bool Parser::skip_blanks() {
		if (!this->remaining())
			return false;
		bool res = false;
		while (this->skip_spaces() || this->skip_newlines()) {
			res = true;
		};
		return res;
	}
	bool Parser::keyword(std::string_view kwd, int* plineno, bool skip) {
		if (skip) this->skip_blanks();
		if (!this->remaining())
			return false;
		bool res = ((*this)[0].type == CharGroupType::NAME) && ((*this)[0].chars == kwd);
		if (res) { 
			if (plineno) *plineno = (*this)[0].lineno;
			this->consume();
		}
		return res;
	}
	bool Parser::operator_(const Operator* op, int* plineno, bool skip) {
		if (skip) this->skip_blanks();
		if (!this->remaining())
			return false;
		if ((*this)[0].type != CharGroupType::OPERATOR)
			return false;
		const Operator* actual_op = reinterpret_cast<const Operator*>((*this)[0].extra);
		if (op == actual_op) {
			if (plineno) *plineno = (*this)[0].lineno;
			this->consume(); 
		}
		return op == actual_op;
	}
	const Operator* Parser::operator_of_type(OperationType type, int* plineno, bool skip) {
		if (skip) this->skip_blanks();
		if (!this->remaining())
			return nullptr;
		if ((*this)[0].type != CharGroupType::OPERATOR)
			return nullptr;
		const Operator* actual_op = reinterpret_cast<const Operator*>((*this)[0].extra);
		if (actual_op->type == type) {
			if (plineno) *plineno = (*this)[0].lineno;
			this->consume();
			return actual_op;
		}
		return nullptr;
	}

	bool Parser::delimiter(std::string_view del, int* plineno, bool skip) {
		if (skip) this->skip_blanks();
		if (!this->remaining())
			return false;
		if (((*this)[0].type != CharGroupType::BINARY_DELIMITER) && ((*this)[0].type != CharGroupType::SINGLE_DELIMITER))
			return false;
		bool res = (*this)[0].chars == del;
		if (res) {
			if (plineno) *plineno = (*this)[0].lineno;
			this->consume(); 
		}
		return res;
	}
	std::string_view Parser::name(int* plineno, bool skip) {
		std::string_view res = {};
		if (skip) this->skip_blanks();
		if (!this->remaining())
			return {};
		if ((*this)[0].type == CharGroupType::NAME) {
			res = sview((*this)[0].chars);
			if (plineno) *plineno = (*this)[0].lineno;
			this->consume();
		}
		return res;
	}

	Parser::state::state(Parser& p) {
		this->_src = &p;
		this->_val = p._cursor;
	}
	void Parser::state::restore() {
		this->_src->_cursor = this->_val;
	}


	Result Parser::fail(std::string_view msg, state st, const std::source_location where) {
		int lineno = (*this)[0].lineno;
		st.restore();
		return Result::fail(msg, lineno, where);
	}
	Result Parser::fail(const std::string& msg, state st, const std::source_location where) {
		return this->fail(sview(msg), st, where);
	}
	Result Parser::propagate(std::string_view msg, Result& res, state st, const std::source_location where) {
		st.restore();
		return res.push(msg, where);
	}
	Result Parser::propagate(const std::string& msg, Result& res, state st, const std::source_location where) {
		return this->propagate(sview(msg), res, st, where);
	}



	ParseResult::ParseResult(std::string_view view, const ParseErrorStack& st) {
		this->_msg = scopy(view);
		this->_stack = st;
	}
	ParseResult::ParseResult() : ParseResult("", {})
	{ }
	std::string ParseResult::message() const {
		std::string res = this->_msg;
		if (!this->_stack)
			res += this->_stack.compute_msg();
		return res;
	}
	ParseResult::operator bool() const {
		return this->_msg.size() == 0;
	}

	ParseResult ParseResult::success() {
		return ParseResult("", {});
	}
	ParseResult& ParseResult::propagate(Parser::state st, ParseResult& res) {
		st.restore();
		return res;
	}
	void ParseResult::push(std::string_view name, const ParseResult& src) {
		if (src._stack)
			this->_stack.add(name, { sview(src._msg) });
		else 
			this->_stack.add(name, src._stack);
	}

	ParseErrorStack::ParseErrorStack(std::string_view msg) {
		this->_subelements[scopy(msg)] = {};
	}
	ParseErrorStack::ParseErrorStack() {}
	void ParseErrorStack::add(std::string_view name, const ParseErrorStack& other) {
		this->_subelements[scopy(name)] = other;
	}
	bool ParseErrorStack::is_empty() const {
		return !this->_subelements.size();
	}
	ParseErrorStack::operator bool() const {
		return this->_subelements.size() == 0;
	}

	std::string ParseErrorStack::compute_msg(int tablevel) const {
		if (this->_subelements.size() == 1) {
			auto& pair = this->_subelements.begin().operator*();
			if (pair.second.is_empty()) {
				return pair.first;  // this is not a complete stack, but just a single error
			}
		}

		std::stringstream fs;
		fs << "could be:\n";

		for (auto& [k, v] : this->_subelements) {
			fs << times(' ', 2 * tablevel) << "- '" << k << "', but ";
			fs << v.compute_msg(tablevel + 1);
			fs << '\n';
		}

		return scopy(fs.view());
	}


	ParserView::ParserView(Parser& parser, int cursor) :
		_parser(&parser), _cursor(cursor)
	{ }

	ParserView::ParserView(Parser& parser) : ParserView(parser, 0)
	{ }

	char ParserView::operator[](int idx) const {
		int offset = this->_cursor + idx;
		for (int i = 0; i < this->_parser->remaining(); i++) {
			auto& grp = this->_parser->operator[](i);
			if (offset >= grp.chars.size()) {
				offset -= (int)grp.chars.size();
				continue;
			}
			return grp.chars[offset];
		}
		return 0;
	}
	size_t ParserView::size() const {
		return this->_parser->remaining() - this->_cursor;
	}
	ParserView ParserView::advance(int amount) const {
		amount = std::min(amount, (int)this->size());
		return ParserView(*this->_parser, this->_cursor + amount);
	}
	int ParserView::pos() const {
		return this->_cursor;
	}
}

