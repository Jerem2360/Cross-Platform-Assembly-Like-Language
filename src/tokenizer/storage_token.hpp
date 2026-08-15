#pragma once
#include "ident_token.hpp"


namespace cpasm {
    struct BoxToken : public Token {
        using Token::Token;

        IdentifierToken name;

        bool parse(Parser& parser);
    };

    struct RegisterToken : public Token {
        using Token::Token;

        IdentifierToken name;

        bool parse(Parser& parser);
    };
}

