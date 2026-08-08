#pragma once


#include <iostream>
#include <vector>
#include <string>

#include "../helpers.hpp"
#include "rules.hpp"


namespace cpasm {

    struct CharWord {
        std::vector<char> chars = {};
        WordType type = WordType::NONE;
        int lineno = 0;
    };

    struct LineTableEntry {
        size_t index;  // index of the char where the line number changed
        int lineno;
    };

    class Wordizer {
        std::istream& _input_data;
        std::vector<CharWord> _result;
        bool _done;
        int _lineno;
        // TODO: Add linetable with binary search access based on char index instead of each word storing a line number
        // line table would have one entry for every time the line number changes.

        CharWord _latest_word() const;

    public:
        Wordizer(std::istream& input);
        bool next();
        bool done() const;
        array_view<CharWord> view() const;
    };
}

