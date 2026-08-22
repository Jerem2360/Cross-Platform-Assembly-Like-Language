#pragma once
#include <variant>
#include <optional>
#include "string_token.hpp"
#include "number_token.hpp"
#include "ident_token.hpp"
#include "storage_token.hpp"
#include "type_token.hpp"


namespace cpasm {
    struct LabelUseToken : public Token {
        using Token::Token;

        IdentifierToken base;
        std::optional<IdentifierToken> sublabel;
        
        bool parse(Parser& parser);
    };

    struct ComptimeConstToken : public Token {
        using Token::Token;

        IdentifierToken name;

        bool parse(Parser& parser);
    };

    struct DereferableOperandToken : public Token {
        using Token::Token;

        std::variant<
            IntLiteralToken,
            LabelUseToken,
            BoxToken,
            RegisterToken
        > value;
        
        bool parse(Parser& parser);
    };

    struct StaticOperandToken : public Token {
        using Token::Token;

        std::variant<
            StringLiteralToken,
            FloatLiteralToken,
            ComptimeConstToken
        > value;
        
        bool parse(Parser& parser);
    };

    struct DereferenceToken : public Token {
        using Token::Token;

        DereferableOperandToken base;
        std::optional<DereferableOperandToken> index;
        std::optional<IntLiteralToken> scale;
        DataTypeToken type;
        
        bool parse(Parser& parser);
    };

    struct OperandToken : public Token {
        using Token::Token;

        std::variant<
            DereferableOperandToken,
            DereferenceToken,
            StaticOperandToken
        > value;
        
        bool parse(Parser& parser);
    };

    struct ConditionToken : public Token {
        using Token::Token;

        Operator* operation;
        OperandToken lhs;
        std::optional<OperandToken> rhs;

        bool parse(Parser& parser);
    };
}

