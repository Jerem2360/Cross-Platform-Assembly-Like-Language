#pragma once
#include "token_base.hpp"


namespace cpasm {
    struct IntToken : public Token {
        using Token::Token;

        int value = 0;

        bool parse(Parser& parser);
    };

    struct FloatToken : public Token {
        using Token::Token;

        float value = 0;

        bool parse(Parser& parser);
    };


    struct NumberToken : public Token {
        using Token::Token;

        std::variant<IntToken, FloatToken> value;

        bool parse(Parser& parser);
    };
}

