#include "wordizer.hpp"
#include "rules.hpp"
#include <iostream>


namespace cpasm {
    static WordType _char_new_wordtype(char c) {
        using under = std::underlying_type_t<WordType>;
        auto rules = ParsingRules::rules();

        for (under i = 0; i < rules.size(); i++) {
            if (rules[i].check(c))
                return (WordType)i;
        }
        return WordType::UNKNOWN;
    }

    static WordType _char_wordtype(char c, CharWord previous) {
        auto rules = ParsingRules::rules();

        if (!previous.chars.size()) {
            return _char_new_wordtype(c);
        }
        auto prev_num = get_underlying(previous.type);

        if (prev_num >= rules.size())
            return WordType::UNKNOWN;
        
        if (rules[prev_num].check(c, vec2sview(previous.chars), previous.type))
            return previous.type;
        
        previous.chars = {};
        previous.type = WordType::NONE;
        return _char_new_wordtype(c);
    }

    Wordizer::Wordizer(std::istream& input) :
        _input_data(input), _result(), _done(false), _lineno(1)
    {}

    CharWord Wordizer::_latest_word() const {
        if (!this->_result.size())
            return {.chars={}, .type=WordType::NONE};

        return coll_last(this->_result);
    }

    bool Wordizer::next() {
        CharWord latest = this->_latest_word();

        int c_i = this->_input_data.get();
        if (this->_input_data.eof()) {
            this->_done = true;
            return false;
        }

        char c = (char)c_i;

        if (c == '\n')
            this->_lineno++;

        //WordType newWordType = _char_wordtype(c, {});
        WordType newWordType = _char_wordtype(c, latest);

        if (latest.type != newWordType) {
            //std::cout << "(new word) last = |" << vec2sview(latest.chars) << "|\n";

            this->_result.emplace_back(CharWord{
                .chars = { c },
                .type = newWordType,
                .lineno = this->_lineno
            });
            return true;
        }
        //std::cout << "last = |" << vec2sview(latest.chars) << "|\n";

        coll_last(this->_result).chars.push_back(c);
        //std::cout << "-> after push back |" << sslice(vec2sview(this->_latest_word().chars), 0, 100) << "|..." << '\n';
        return true;
    }

    bool Wordizer::done() const {
        return this->_done;
    }

    array_view<CharWord> Wordizer::view() const {
        return this->_result;
    }
}
