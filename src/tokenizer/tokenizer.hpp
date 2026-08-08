#pragma once
#include <string_view>
#include "../parser/parser.hpp"


namespace cpasm {
    class Tokenizer {
        Parser* _parser;

    public:
        Tokenizer(Parser* parser);

        std::string_view filename() const;
    };
}

