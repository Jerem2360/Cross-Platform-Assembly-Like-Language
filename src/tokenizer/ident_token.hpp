#pragma once
#include "token_base.hpp"


namespace cpasm {
    struct IdentifierToken : public Token {
        using Token::Token;

        std::string value = "";

        bool parse(Parser& parser);
    };

    struct LabelUseToken : public Token {
        using Token::Token;

        IdentifierToken base;
        IdentifierToken nested;

        bool parse(Parser& parser);
    };
}

