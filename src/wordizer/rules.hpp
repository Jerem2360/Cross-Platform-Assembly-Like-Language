#pragma once
#include <string_view>
#include <vector>

#include "../helpers.hpp"
#include "../registry.hpp"


namespace cpasm {

    enum class WordType {
        NONE = 0,
        NAME,
        NUMERIC,
        SYMBOLS,
        SPACES,
        UNKNOWN,
    };

    std::string_view WordType_name(WordType value);

    extern RegistryView<char> SymbolChars;

    struct ParsingRule {
        using definition_t = bool (*) (char c, std::string_view prev_chars, WordType prev_type);

        const definition_t _def;

        inline consteval ParsingRule(definition_t definition) :
            _def(definition)
        {}
        inline constexpr bool check(char c, std::string_view prev_chars, WordType prev_type) const {
            return this->_def(c, prev_chars, prev_type);
        }
        inline constexpr bool check(char c) const {
            return this->check(c, "", WordType::NONE);
        }
    };

    // singleton
    class ParsingRules {

        template<ParsingRule ...conds>
        static constexpr ParsingRule rule_intersect = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return (conds.check(c, prev_chars, prev_type) && ...); 
            } 
        };

        template<ParsingRule ...conds>
        static constexpr ParsingRule rule_union = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return (conds.check(c, prev_chars, prev_type) || ...); 
            } 
        };

        template<ParsingRule base>
        static constexpr ParsingRule rule_not = {
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return !base.check(c, prev_chars, prev_type);
            } 
        };

        static constexpr ParsingRule rule_all = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return true; 
            } 
        };

        static constexpr ParsingRule rule_none = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return false; 
            } 
        };

        template<size_t pos>
        static constexpr ParsingRule rule_exact_position = {
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return prev_chars.size() == pos; 
            } 
        };

        template<size_t pos>
        static constexpr ParsingRule rule_min_position = {
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return prev_chars.size() >= pos; 
            } 
        };

        template<size_t pos>
        static constexpr ParsingRule rule_max_position = {
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return prev_chars.size() <= pos; 
            } 
        };

        template<char C>
        static constexpr ParsingRule rule_prev_contains = {
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return coll_contains(prev_chars, C); 
            } 
        };
        
        template<char min, char max>
        static constexpr ParsingRule rule_char_range = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return (c >= min) && (c <= max); 
            } 
        };

        template<char ...targets>
        static constexpr ParsingRule rule_specific_chars = { 
            [](char c, std::string_view prev_chars, WordType prev_type) { 
                return ((c == targets) || ...); 
            } 
        };

        static constexpr ParsingRule _decimal_point = rule_intersect<
            rule_specific_chars<'.'>,
            rule_not<rule_prev_contains<'.'>>
        >;

    public:
        static constexpr ParsingRule decimal_digits = rule_char_range<'0', '9'>;
    
    private:
        static constexpr ParsingRule _hexonly_digits = rule_union<
            rule_char_range<'a', 'f'>,
            rule_char_range<'A', 'F'>
        >;

        static constexpr ParsingRule _num_base_spec = rule_specific_chars<'b', 'o', 'x', 'd'>;

        static constexpr ParsingRule _asciiletters = rule_union<
            rule_char_range<'a', 'z'>,
            rule_char_range<'A', 'Z'>
        >;

        static constexpr ParsingRule _basic_identifier = rule_union<
            _asciiletters,
            rule_specific_chars<'_'>
        >;

        static constexpr ParsingRule _ident_with_number = rule_union<
            _basic_identifier,
            decimal_digits
        >;
        
    public:
        static void register_symbol_char(char c);
        static inline void register_symbol_chars(std::string_view chars) {
            for (char c : chars) {
                register_symbol_char(c);
            }
        }

        static constexpr ParsingRule numeric = rule_union<
            rule_intersect<
                rule_exact_position<0>,
                decimal_digits
            >,
            rule_intersect<
                rule_exact_position<1>,
                _num_base_spec
            >,
            rule_intersect<
                rule_min_position<1>,
                rule_union<
                    decimal_digits,
                    _hexonly_digits
                >
            >
        >;

        static constexpr ParsingRule identifier = rule_union<
            rule_intersect<
                rule_exact_position<0>,
                _basic_identifier
            >,
            rule_intersect<
                rule_min_position<1>,
                _ident_with_number
            >
        >;

        
        static constexpr ParsingRule symbols = { [](char c, std::string_view prev_chars, WordType prev_type) { return coll_contains(SymbolChars, c); } };
        
        static constexpr ParsingRule spaces = rule_specific_chars<' ', '\t', '\n', '\r'>;

    private:
        static constexpr ParsingRule _rule_map[] = {
            rule_none,  // NONE
            identifier,  // NAME
            numeric,  // NUMERIC
            symbols,  // SYMBOLS
            spaces,  // SPACES
            rule_none,  // UNKNOWN
        };

    public:
        static array_view<ParsingRule> rules();
    };
}

