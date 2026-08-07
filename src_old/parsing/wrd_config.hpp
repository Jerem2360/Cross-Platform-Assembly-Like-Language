#pragma once
#include <algorithm>
#include "CharGrouper.hpp"


namespace cpasm {
	static constexpr char_condition _SPACE_CHARS = char_specific<' ', '\t', '\r'>;
	static constexpr char_condition _UALPHA_CHARS = char_range<'A', 'Z'>;
	static constexpr char_condition _LALPHA_CHARSR = char_range<'a', 'z'>;
	static constexpr char_condition _ALPHA_CHARS = char_union<_UALPHA_CHARS, _LALPHA_CHARSR>;
	static constexpr char_condition _NAME_CHARS = char_union<_ALPHA_CHARS, char_specific<'_', '.'>>;
	static constexpr char_condition _DIGIT_CHARS = char_range<'0', '9'>;
	static constexpr char_condition _DIGITEX_CHARS = char_union<char_range<'a', 'w'>, char_range<'A', 'W'>>;
	static constexpr char_condition _BINDEL_CHARS = char_specific<'{', '}', '[', ']', '(', ')'>;
	static constexpr char_condition _SINGDEL_CHARS = char_specific<';', ':', ','>;
	static constexpr char_condition _OPERATOR_CHARS = char_specific<'+', '-', '=', '/', '*', '^', '|', '&', '<', '>', '!', '%', '@', '$', '#'>;
	static constexpr char_condition _QUOTE_CHARS = char_specific<'"', '\'', '`'>;


	static CharGroupDef _chargroup_rules[] = {
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // UNKNOWN
			return false;
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // EOF
			return false;
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // SPACE
			return _SPACE_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // NUMBER
			if (data.length() == 1) {  // we're the first char of the word
				return _DIGIT_CHARS.check(data.current());
			}
			if ((data == "0x") || (data == "0o") || (data == "0b")) {
				return true;
			}
			if (word_type != CharGroupType::NUMBER)
				return false;
			if (std::count(data.begin(), data.end(), '.') > 1)
				return false;
			if (data.current() == '.')
				return true;

			return _DIGITEX_CHARS.check(data.current()) || _DIGIT_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // NAME
			if (data.length() == 1)  // if we're the first char of the word
				return _NAME_CHARS.check(data.current());

			if (word_type != CharGroupType::NAME)
				return false;
			
			return _NAME_CHARS.check(data.current()) || _DIGIT_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // DOUBLE_QUOTE
			return (data.length() == 1) && (data.current() == '"');
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // SINGLE_QUOTE
			return (data.length() == 1) && (data.current() == '\'');
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // BACKTICK
			return (data.length() == 1) && (data.current() == '`');
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // BIN_DELIMITER
			return (data.length() == 1) && _BINDEL_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // SINGLE_DELIMITER
			return (data.length() == 1) && _SINGDEL_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // OPERATOR
			return _OPERATOR_CHARS.check(data.current());
		},
		[](CharGrouperData data, CharGroupType word_type) -> bool {  // NEWLINE
			return data.current() == '\n';
		},
	};
}

