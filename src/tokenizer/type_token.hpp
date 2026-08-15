#pragma once
#include "ident_token.hpp"
#include "number_token.hpp"


namespace cpasm {
    struct DataTypeToken : public Token {
        using Token::Token;

        IdentifierToken name;
        IntLiteralToken bits;

        bool parse(Parser& parser);
    };
}

