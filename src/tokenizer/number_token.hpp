#pragma once
#include "token_base.hpp"


namespace cpasm {
    enum class IntParseStatus : u8 {
        NONE = 0,
        BAD_DIGIT,
        BAD_PREFIX,
        BAD_SYNTAX,
        OVERFLOW,
    };

    struct IntParseError {
        IntParseStatus status;
        char c = 0;
        u16 position = 0;
        u8 base = 0;

        std::string format();

        static constexpr IntParseError success() {
            return {.status = IntParseStatus::NONE};
        }

        operator bool();
    };


    IntParseError parse_digits(Parser& parser, size_t* out, int base);

    struct IntLiteralToken : public Token {
        using Token::Token;

        size_t value = 0;
        i8 sign = 0;

        static inline constexpr SupportType support() {
            return SUPPORTS_BOTH;
        }

        bool parse(Parser& parser);
    };

    struct FloatLiteralToken : public Token {
        using Token::Token;

        double value = 0;
        
        static inline constexpr SupportType support() {
            return SUPPORTS_BOTH;
        }

        bool parse(Parser& parser);
    };


    struct NumberLiteralToken : public Token {
        using Token::Token;

        std::variant<IntLiteralToken, FloatLiteralToken> value;
        
        static inline constexpr SupportType support() {
            return SUPPORTS_BOTH;
        }

        bool parse(Parser& parser);
    };
}

