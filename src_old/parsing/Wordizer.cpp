#include "Wordizer.hpp"
#include "../Operator.hpp"


namespace cpasm {
	Wordizer::Wordizer(std::istream& fs) :
		_initial_groups(fs), _distinguished_operators(), _commentsRemoved()
	{ }

	static const Operator* _find_operator(std::string_view prefixed) {
		for (auto& op : Operator::Operators) {
			if (prefixed.starts_with(op.name)) {
				return &op;
			}
		}
		return nullptr;
	}

	static std::string_view _advance_view(std::string_view view, size_t amount) {
		if (amount >= view.size())
			return {};
		return { view.data() + amount, view.size() - amount };
	}

	bool Wordizer::_distinguish_operators(const CharGroup& grp) {
		std::string_view data = sview(grp.chars);
		std::vector<char> _unknown_acc = {};

		while (data.size()) {
			const Operator* op = _find_operator(data);

			if (!op) {
				_unknown_acc.push_back(data[0]);
				data = _advance_view(data, 1);
				continue;
			}
			if (_unknown_acc.size()) {
				this->_distinguished_operators.emplace_back(CharGroup{
					.chars = scopy(vec2str(_unknown_acc)),
					.type = CharGroupType::UNKNOWN,
					.lineno = grp.lineno,
				});
				_unknown_acc.clear();
			}
			this->_distinguished_operators.emplace_back(CharGroup{
				.chars = scopy(ssubview(data, 0, op->name.size())),
				.type = CharGroupType::OPERATOR,
				.extra = reinterpret_cast<intptr_t>(op),
				.lineno = grp.lineno,
			});
			data = _advance_view(data, op->name.size());
		}

		if (_unknown_acc.size()) {
			this->_distinguished_operators.emplace_back(CharGroup{
				.chars = scopy(vec2str(_unknown_acc)),
				.type = CharGroupType::UNKNOWN,
				.lineno = grp.lineno,
				});
		}

		return true;
	}

	static bool _is_operator(const CharGroup& grp, const Operator* op) {
		if (grp.type != CharGroupType::OPERATOR)
			return false;

		const Operator* actual_op = reinterpret_cast<const Operator*>(grp.extra);
		return actual_op == op;
	}

	bool Wordizer::parse() {
		if (!this->_initial_groups.parse())
			return false;

		// separate the different operators
		for (auto& grp : this->_initial_groups.result()) {
			if (grp.type != CharGroupType::OPERATOR) {
				this->_distinguished_operators.push_back(grp);
				continue;
			}
			if (!this->_distinguish_operators(grp))
				return false;
		}


		// get rid of the comments ... except this also requires that we know when we are inside a string.
		bool squote_string = false;
		bool dquote_string = false;
		bool btick_string = false;
		bool ml_comment = false;
		bool sl_comment = false;

		CharGroup* prev = nullptr;

		for (auto& grp : this->_distinguished_operators) {
			if (ml_comment) {
				if (_is_operator(grp, Operator::END_CMT))
					ml_comment = false;
				prev = &grp;
				continue;
			}
			if (sl_comment) {
				if (grp.type == CharGroupType::NEW_LINE)
					sl_comment = false;
				prev = &grp;
				continue;
			}
			if (!dquote_string && !btick_string && (grp.type == CharGroupType::SINGLE_QUOTE)) {
				if (squote_string && prev && sendwith(prev->chars, "\\")) {
					prev = &grp;
					continue;
				}
				squote_string = !squote_string;
			}
			
			if (!squote_string && !btick_string && (grp.type == CharGroupType::DOUBLE_QUOTE)) {
				if (dquote_string && prev && sendwith(prev->chars, "\\")) {
					prev = &grp;
					continue;
				}
				dquote_string = !dquote_string;
			}

			if (!squote_string && !dquote_string && (grp.type == CharGroupType::BACKTICK)) {
				if (btick_string && prev && sendwith(prev->chars, "\\")) {
					prev = &grp;
					continue;
				}
				btick_string = !btick_string;
			}

			if (!dquote_string && !squote_string && !btick_string && _is_operator(grp, Operator::BEG_CMT)) {
				ml_comment = true;
				prev = &grp;
				continue;
			}
			if (!dquote_string && !squote_string && !btick_string && _is_operator(grp, Operator::LINE_CMT)) {
				sl_comment = true;
				prev = &grp;
				continue;
			}
			this->_commentsRemoved.push_back(grp);
			prev = &grp;
		}

		return true;
	}

	const std::vector<CharGroup>& Wordizer::result() const {
		return this->_commentsRemoved;
	}
}

