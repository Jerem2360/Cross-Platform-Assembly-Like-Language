#pragma once
#include <variant>
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

    struct StorageToken : public Token {
        using Token::Token;

        std::variant<BoxToken, RegisterToken> value;

        bool parse(Parser& parser);
    };
}

