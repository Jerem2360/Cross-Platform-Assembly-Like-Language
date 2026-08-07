#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <istream>
#include "../helpers.hpp"


namespace cpasm {
	enum class CharGroupType : uint16_t {
		UNKNOWN = 0,
		EOF,
		SPACE,
		NUMBER,
		NAME,
		DOUBLE_QUOTE,
		SINGLE_QUOTE,
		BACKTICK,
		BINARY_DELIMITER,  // delimiter with opening and closing symbols
		SINGLE_DELIMITER,  // delimiter that goes between two things
		OPERATOR,
		NEW_LINE,
	};

	inline constexpr std::string_view _CharGroupType_names[] = {
		"UNKNOWN",
		"EOF",
		"SPACE",
		"NUMBER",
		"NAME",
		"DOUBLE_QUOTE",
		"SINGLE_QUOTE",
		"BACKTICK",
		"BINARY_DELIMITER",
		"SINGLE_DELIMITER",
		"OPERATOR",
		"NEW_LINE",
	};

	std::ostream& operator <<(std::ostream&, CharGroupType);

	struct CharGroup {
		std::string chars;
		CharGroupType type;
		intptr_t extra = 0;
		int lineno = 1;
	};


	struct CharGrouperData : public std::string_view {
		using std::string_view::string_view;

		inline constexpr char current() const {
			if (!this->size())
				return 0;
			return this->at(this->size() - 1);
		}
	};

	using CharGroupDef = bool(*)(CharGrouperData data, CharGroupType word_type);

	class CharGrouper {
		std::istream& _source;
		std::vector<CharGroup> _result = {};

	public:

		CharGrouper(std::istream& source);
		bool parse();
		const std::vector<CharGroup>& result() const;

		inline void print(std::ostream& fs) const {
			for (auto& grp : this->_result) {
				fs << "token: \"" << grp.chars << "\"; type = " << grp.type << ".\n";
			}
		}
	};
}

