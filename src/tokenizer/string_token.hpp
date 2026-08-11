#pragma once
#include "token_base.hpp"


namespace cpasm {
    struct StringLiteralToken : public Token {
        using Token::Token;

        std::string value;

        bool parse(Parser& parser);
    };

    struct IdentStringToken : public Token {
        using Token::Token;

        std::string value;

        bool parse(Parser& parser);
    };
}

