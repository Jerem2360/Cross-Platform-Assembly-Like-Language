#pragma once
#include <variant>
#include <optional>
#include "token_base.hpp"
#include "type_token.hpp"
#include "ident_token.hpp"
#include "operand_tokens.hpp"
#include "../operators.hpp"


namespace cpasm {
    struct LabelToken : public Token {
        using Token::Token;

        IdentifierToken name;

        bool parse(Parser& parser);
    };
    struct DeclarationToken : public Token {
        using Token::Token;

        StorageToken location;
        DataTypeToken type;

        bool parse(Parser& parser);
    };

    struct FunctionSigToken : public Token {
        using Token::Token;

        std::vector<DeclarationToken> params;
        DataTypeToken ret_type;

        bool parse(Parser& parser);
    };

    struct SimpleInstructionToken : public Token {
        using Token::Token;

        const Operator* operation;
        OperandToken lhs;
        std::optional<OperandToken> rhs;

        bool parse(Parser& parser);
    };

    struct GotoInstructionToken : public Token {
        using Token::Token;

        OperandToken target;

        bool parse(Parser& parser);
    };

    struct IfGotoInstructionToken : public Token {
        using Token::Token;

        ConditionToken condition;
        OperandToken target;

        bool parse(Parser& parser);
    };

    struct CallInstructionToken : public Token {
        using Token::Token;

        OperandToken target;
        std::optional<OperandToken> return_target;
        std::vector<IdentifierToken> attributes;
        std::vector<OperandToken> args;

        bool parse(Parser& parser);
    };

    struct ExitInstructionToken : public Token {
        using Token::Token;

        std::optional<StaticOperandToken> status;

        bool parse(Parser& parser);
    };

    struct ReturnInstructionToken : public Token {
        using Token::Token;

        bool parse(Parser& parser);
    };

    struct CustomInstructionToken : public Token {
        using Token::Token;

        IdentifierToken name;
        std::vector<OperandToken> args;

        bool parse(Parser& parser);
    };


    struct InstructionToken : public Token {
        using Token::Token;

        std::variant<
            SimpleInstructionToken,
            GotoInstructionToken,
            IfGotoInstructionToken,
            CallInstructionToken,
            ExitInstructionToken,
            ReturnInstructionToken,
            CustomInstructionToken
        > value;

        bool parse(Parser& parser);
    };

    struct FunctionBodyToken : public Token {
        using Token::Token;

        enum def_type : uint8_t {
            DEF_INSTR,
            DEF_LABEL,
        };

        std::vector<LabelToken> labels;
        std::vector<InstructionToken> instructions;
        std::vector<DeclarationToken> declarations;
        std::vector<std::tuple<def_type, size_t>> order;

        bool parse(Parser& parser);
    };
}

