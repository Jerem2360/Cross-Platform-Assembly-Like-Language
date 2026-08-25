#pragma once
#include <variant>
#include <vector>
#include <optional>
#include "token_base.hpp"
#include "string_token.hpp"
#include "ident_token.hpp"
#include "type_token.hpp"
#include "function_tokens.hpp"


namespace cpasm {
    // used for functions and symbols
    enum prop_type : uint8_t {
        PROP_NONE = 0,
        PROP_EXPORT = 0b01,
    };

    struct IfStatementToken : public Token {
        using Token::Token;

        ConditionToken condition;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct ElifStatementToken : public Token {
        using Token::Token;

        ConditionToken condition;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct ElseStatementToken : public Token {
        using Token::Token;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct EndifStatementToken : public Token {
        using Token::Token;

        static constexpr bool NEED_SEMICOLON = true;

        bool parse(Parser& parser);
    };

    struct IncludeStatementToken : public Token {
        using Token::Token;

        StringLiteralToken name;

        static constexpr bool NEED_SEMICOLON = true;

        bool parse(Parser& parser);
    };

    struct EntryPointStatementToken : public Token {
        using Token::Token;

        FunctionBodyToken body;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct FunctionStatementToken : public Token {
        using Token::Token;

        std::optional<SymbolNameToken> name;
        std::optional<FunctionSigToken> signature;
        FunctionBodyToken body;
        std::vector<IdentifierToken> attributes;
        prop_type properties = PROP_NONE;  // NOTE: exporting is disallowed for anonymous functions
        

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct DataDeclStatementToken : public Token {
        using Token::Token;

        DataTypeToken type;
        std::optional<StaticOperandToken> value;
        std::optional<IntLiteralToken> length;

        static constexpr bool NEED_SEMICOLON = true;

        bool parse(Parser& parser);
    };

    struct ImportStatementToken : public Token {
        using Token::Token;

        StringLiteralToken source;
        std::vector<IdentifierToken> symbols;

        static constexpr bool NEED_SEMICOLON = true;

        bool parse(Parser& parser);
    };

    struct SymbolStatementToken : public Token {
        using Token::Token;

        IdentifierToken name;
        prop_type properties = PROP_NONE;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    struct SectionStatementToken : public Token {
        using Token::Token;

        IdentifierToken name;
        /*
        Normally this is a combination of 'r', 'w' and 'x' characters telling the OS if
        user code can read, write and/or execute code respectively from this section.
        If this is missing, the default value for that section is used.

        e.g. section mysection rw:
        declares a section called "mysection" with read and write access.

        e.g. section mycodesection rx:
        declares a section called "mycodesection" with read and execute access.

        Arbitrary sections have no access at all by default.
        */
        std::optional<IdentifierToken> access;
        bool has_dot;

        static constexpr bool NEED_SEMICOLON = false;

        bool parse(Parser& parser);
    };

    
    struct StatementToken : public Token {
        using Token::Token;

        std::variant<
            IfStatementToken,
            ElifStatementToken,
            ElseStatementToken,
            EndifStatementToken,
            IncludeStatementToken,
            EntryPointStatementToken,
            FunctionStatementToken,
            DataDeclStatementToken,
            ImportStatementToken,
            SymbolStatementToken,
            SectionStatementToken
        > value;

        bool parse(Parser& parser);
    };

    struct ProgramToken : public Token {
        using Token::Token;

        std::vector<StatementToken> statements;

        bool parse(Parser& parser);
    };
}

