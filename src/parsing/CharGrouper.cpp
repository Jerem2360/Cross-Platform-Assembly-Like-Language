#include "CharGrouper.hpp"
#include "wrd_config.hpp"


namespace cpasm {

	static CharGroupType _get_chargrouptype(CharGrouperData data, CharGroupType word_type, array_view<CharGroupDef> rules) {
		for (uint16_t i = 0; i < rules.size(); i++) {
			if (rules[i](data, word_type)) {
				return static_cast<CharGroupType>(i);
			}
		}
		return CharGroupType::UNKNOWN;
	}

	static bool _is_chargrouptype(CharGrouperData data, CharGroupType word_type, array_view<CharGroupDef> rules) {
		auto idx = static_cast<uint16_t>(word_type);
		auto& rule = rules[idx];
		return rule(data, word_type);
	}


	CharGrouper::CharGrouper(std::istream& source) :
		_source(source)
	{ }
	bool CharGrouper::parse() {
		std::vector<char> word = {};
		CharGroupType gtype = CharGroupType::UNKNOWN;
		int lineno = 1;
		int grp_lineno = 1;
		
		while (1) {

			int i = this->_source.get();
			if (i == EOF)
				break;

			char c = static_cast<char>(i);
			if (c == '\n')
				lineno += 1;
			word.push_back(c);

			CharGrouperData st = { word.data(), word.size() };

			if (!_is_chargrouptype(st, gtype, _chargroup_rules)) {
				if (word.size() > 1) {
					this->_result.emplace_back(CharGroup{
						.chars = scopy(ssubview(st, 0, st.size() - 1)),
						.type = gtype,
						.lineno = grp_lineno,
						});
				}
				gtype = _get_chargrouptype({&c, 1}, CharGroupType::UNKNOWN, _chargroup_rules);
				word.clear();
				word.push_back(c);
				grp_lineno = lineno;
			}
			
		}
		return true;
	}

	const std::vector<CharGroup>& CharGrouper::result() const {
		return this->_result;
	}

	std::ostream& operator <<(std::ostream& fs, CharGroupType val) {
		uint16_t int_val = static_cast<uint16_t>(val);
		if (int_val >= array_len(_CharGroupType_names))
			fs << "<???> (" << int_val << ')';

		else
			fs << '<' << _CharGroupType_names[int_val] << "> (" << int_val << ')';

		return fs;
	}
}

