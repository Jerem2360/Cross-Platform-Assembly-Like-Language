#pragma once
#include "token_base.hpp"


namespace cpasm {
    struct StringLiteralToken : public Token {
        using Token::Token;

        std::string value;
        
        static inline constexpr SupportType support() {
            return SUPPORTS_COMPTIME;
        }

        bool parse(Parser& parser);
    };

    struct IdentStringToken : public Token {
        using Token::Token;

        std::string value;
        
        static inline constexpr SupportType support() {
            return SUPPORTS_BOTH;
        }

        bool parse(Parser& parser);
    };
}

