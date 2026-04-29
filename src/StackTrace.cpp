#include "StackTrace.hpp"

#include <utility>

#include "helpers.hpp"


namespace cpasm {
	struct Tabs {
		int tablevel;
	};

	static std::ostream& operator <<(std::ostream& fs, Tabs tabs) {
		for (int i = 0; i < tabs.tablevel; i++) {
			fs << "|   ";
		}
		return fs;
	}

	static std::string_view _simplified_funcname(std::string_view funcname) {
		size_t idx = funcname.find('(', 0);
		funcname = ssubview(funcname, 0, idx);
		idx = funcname.rfind(' ', funcname.size()-1);
		return ssubview(funcname, idx+1, funcname.size()-idx-1);
	}




	StackTrace::StackTrace(std::string_view message, std::vector<std::unique_ptr<StackTrace>> children, std::source_location where) :
		_msg(message), _where(where), _src_lineno(0), _children(std::move(children))
	{ }

	StackTrace::StackTrace(std::string_view message, int lineno, std::source_location where) :
		_msg(message), _where(where), _src_lineno(lineno), _children()
	{ }

	void StackTrace::write(std::ostream& out, std::string_view file_name, int tablevel) const {
		out << Tabs(tablevel) << "File '" << this->_where.file_name() << "', line " << this->_where.line() << ", in '";
		out << _simplified_funcname(this->_where.function_name()) << "':\n" << Tabs(tablevel) << "|>" << this->_msg << ", caused by";

		if (this->_src_lineno)
			out << " '" << file_name << "', line " << this->_src_lineno << ".\n";
		else
			out << ":\n";

		for (auto& child : this->_children) {
			child->write(out, file_name, tablevel + 1);
		}
	}


	ResultBunch::ResultBunch() :
		_traces()
	{ }
	void ResultBunch::add(Result& src) {
		this->_traces.emplace_back(std::move(src._stacktrace));
	}
	ResultBunch& ResultBunch::operator+=(Result& src) {
		if (!src)
			this->add(src);
		return *this;
	}
	ResultBunch& ResultBunch::operator+=(Result&& src) {
		if (!src)
			this->add(src);
		return *this;
	}
	Result ResultBunch::push(std::string_view msg, const std::source_location where) {
		if (!this->_traces.size())
			return {};
		return Result(msg, std::move(this->_traces), where);
	}
	Result ResultBunch::push(const std::string& msg, const std::source_location where) {
		if (!this->_traces.size())
			return {};
		return this->push(sview(msg), where);
	}


	Result::Result() :
		_stacktrace(nullptr), _filename("<unknown>")
	{ }
	Result::Result(std::string_view msg, int lineno, std::source_location where) :
		 _filename("<unknown>")
	{ 
		this->_stacktrace = std::make_unique<StackTrace>(msg, lineno, where);
	}
	Result::Result(std::string_view msg, std::vector<std::unique_ptr<StackTrace>> children, std::source_location where) :
		_filename("<unknown>")
	{
		this->_stacktrace = std::make_unique<StackTrace>(msg, std::move(children), where);
	}
	
	Result Result::fail(std::string_view msg, int lineno, const std::source_location where) {
		return Result(msg, lineno, where);
	}
	Result Result::fail(const std::string& msg, int lineno, const std::source_location where) {
		return Result(sview(msg), lineno, where);
	}

	Result Result::push(std::string_view msg, const std::source_location where) {
		if (!this->_stacktrace)
			return {};

		std::vector<std::unique_ptr<StackTrace>> tmp;
		tmp.reserve(1);
		tmp.push_back(std::move(this->_stacktrace));

		return Result(msg, std::move(tmp), where);
	}
	Result Result::push(const std::string& msg, const std::source_location where) {
		return this->push(sview(msg), where);
	}
	Result Result::with_filename(std::string_view filename) {
		this->_filename = scopy(filename);
		return std::move(*this);
	}
	void Result::write(std::ostream& fs) const {
		if (!this->_stacktrace) {
			fs << "Empty stacktrace.\n";
			return;
		}
		fs << "Error detected in the compiler. Full stacktrace:\n";
		this->_stacktrace->write(fs, sview(this->_filename));
	}
	Result::operator bool() const {
		return !this->_stacktrace;
	}
}

