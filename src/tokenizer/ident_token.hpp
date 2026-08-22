#pragma once
#include <variant>
#include "token_base.hpp"
#include "string_token.hpp"


namespace cpasm {
    struct IdentifierToken : public Token {
        using Token::Token;

        std::string value = "";

        bool parse(Parser& parser);
    };

    struct SymbolNameToken : public Token {
        using Token::Token;

        std::variant<IdentifierToken, IdentStringToken> value;

        bool parse(Parser& parser);
    };

    struct LabelUseToken : public Token {
        using Token::Token;

        SymbolNameToken base;
        IdentifierToken nested;

        bool parse(Parser& parser);
    };
}

