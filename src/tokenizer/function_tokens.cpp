#include "function_tokens.hpp"


namespace cpasm {
    bool LabelToken::parse(Parser& parser) {
        this->Token::parse(parser);

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, &this->name, "label must start with an identifier");

        parser.consume_blanks();
        
        if (!parser.get_exact_operator(Operator::COLON))
            return fail(this, "label requires a ':' after its name.");
        parser.advance_operator(Operator::COLON);

        return true;
    }

    bool DeclarationToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        this->location = {this};
        if (!parser.parse_token(&this->location, false))
            return fail(this, &this->location, "missing a storage for declaration.");
        parser.consume_blanks();

        if (!parser.get_exact_operator(Operator::COLON))
            return fail(this, "missing ':' after the declaration's storage.");

        parser.advance_operator(Operator::COLON);
        parser.consume_blanks();

        this->type = {this};
        if (!parser.parse_token(&this->type, false))
            return fail(this, &this->type, "missing type in declaration.");

        return true;
    }

    bool FunctionSigToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        this->params = {};

        bool res = parse_list(this, parser, [&](Parser& _){
            DeclarationToken& tok = this->params.emplace_back(this);
            bool res = parser.parse_token(&tok, true);
            if (!res)
                this->params.pop_back();
            return res;
        }, Operator::COMMA, Operator::OPEN_BR, Operator::CLOSE_BR);

        if (!res)
            return false;

        parser.consume_blanks();

        if (!parser.get_exact_operator(Operator::RETURNS))
            return true;

        parser.consume_blanks();

        this->ret_type = {this};
        if (!parser.parse_token(&this->ret_type, false))
            return fail(this, &this->ret_type, "missing return type after '->' in function declaration.");

        return true;
    }

    bool SimpleInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        // not too sure what to do, that code could become an almost duplicate of ConditionToken::parse ...
        return fail(this, "not implemented"); 
    }

    bool GotoInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("goto", WordType::NAME))
            return fail(this, "Missing 'goto' keyword at start of statement");

        parser.advance_word(1);
        parser.consume_blanks();

        this->target = {this};
        if (!parser.parse_token(&this->target, false))
            return fail(this, &this->target, "Missing target operand after keyword 'goto'");

        return true;
    }

    bool IfGotoInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("if", WordType::NAME))
            return fail(this, "Missing 'if' keyword at start of statement");

        parser.advance_word(1);
        parser.consume_blanks();

        this->condition = {this};
        if (!parser.parse_token(&this->condition, false))
            return fail(this, &this->condition, "missing condition after 'if' keyword.");

        parser.consume_blanks();

        if (!parser.get_keyword("goto", WordType::NAME))
            return fail(this, "missing 'goto' keyword after condition");

        parser.advance_word(1);
        parser.consume_blanks();

        this->target = {this};
        if (!parser.parse_token(&this->target, false))
            return fail(this, &this->target, "Missing target operand after keyword 'goto'");

        return true;
    }

    bool CallInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("call", WordType::NAME))
            return fail(this, "Missing keyword 'call' at beginning of statement.");
        
        parser.advance_word(1);
        parser.consume_blanks();

        parse_list(this, parser, [&](Parser& _) {
            IdentifierToken& id_tok = this->attributes.emplace_back(this);
            bool res = parser.parse_token(&id_tok, true);
            if (!res)
                this->attributes.pop_back();
            return res;
        }, Operator::COMMA, Operator::OPEN_SQUARE, Operator::CLOSE_SQUARE);

        this->target = {this};
        if (!parser.parse_token(&this->target, false))
            return fail(this, &this->target, "Missing target operand after keyword 'call'");

        parser.consume_blanks();

        bool res = parse_list(this, parser, [&](Parser& _){
            OperandToken& tok = this->args.emplace_back(this);
            bool res = parser.parse_token(&tok, true);
            if (!res)
                this->args.pop_back();
            return res;
        }, Operator::COMMA, Operator::OPEN_BR, Operator::CLOSE_BR);

        parser.consume_blanks();

        if (!res)
            return true;

        if (!parser.get_exact_operator(Operator::RETURNS))
            return true;
        parser.advance_word(1);
        parser.consume_blanks();

        OperandToken& res_tok = this->return_target.emplace(this);
        if (!parser.parse_token(&res_tok, false))
            return fail(this, &res_tok, "Missing return location after '->'");
        
        return true;
    }

    bool ExitInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("exit", WordType::NAME))
            return fail(this, "Missing keyword 'exit' at beginning of instruction");

        parser.advance_word(1);
        parser.consume_blanks();

        StaticOperandToken& tok = this->status.emplace(this);
        if (!parser.parse_token(&tok, true))
            this->status = std::nullopt;
        
        return true;
    }

    bool ReturnInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("return", WordType::NAME))
            return fail(this, "Missing 'return' keyword");
        return true;
    }

    bool CustomInstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_exact_operator(Operator::CUSTOM_INSTR))
            return fail(this, "Missing '#' at beginning of custom instruction");
        parser.advance_operator(Operator::CUSTOM_INSTR);

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, &this->name, "Missing custom instruction name after '#'");
        
        parser.consume_blanks();

        return parse_list(this, parser, [&](Parser& _) {
            OperandToken& tok = this->args.emplace_back(this);
            if (parser.parse_token(&tok, true))
                return true;
            this->args.pop_back();
            return false;            
        }, Operator::COMMA);
    }

    bool InstructionToken::parse(Parser& parser) {
        this->Token::parse(parser);

        TokenErrorGroup g;

        int res = _FOR_EACH_VAR_TYPE(decltype(this->value)) {
            LoopType& tok = this->value.emplace<LoopType>(this);
            if (parser.parse_token(&tok, true)) {
                loop.break_(1);
                return;
            }
            g.add(&tok);
        };

        if (!res)
            return g.fail(this, "Unable to find a valid instruction");

        
        parser.consume_blanks();
        if (!parser.get_exact_operator(Operator::SEMICOLON))
            return fail(this, "Missing ';' at end of instruction");
        parser.advance_operator(Operator::SEMICOLON);

        return true;
    }

    bool FunctionBodyToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_exact_operator(Operator::OPEN_CURLY))
            return fail(this, "Function body must start with '{'");
        parser.advance_operator(Operator::OPEN_CURLY);
        parser.consume_blanks();
        
        // declarations go first
        while (1) {
            DeclarationToken& decl = this->declarations.emplace_back(this);
            if (!parser.parse_token(&decl, true)) {
                this->declarations.pop_back();
                break;
            }
            parser.consume_blanks();
            if (!parser.get_exact_operator(Operator::SEMICOLON))
                return fail(this, "Missing ';' at end of declaration");
            parser.advance_operator(Operator::SEMICOLON);
            parser.consume_blanks();
        }

        // then follows a mix of instructions and labels
        while (1) {
            /*
            Should we parse the labels or the instructions first ?
            - Labels first means that parsing labels is faster and doesnt need to check for every single instruction type first
            - Instructions first means parsing instructions will be slightly faster because no need to check for a label first

            Will there be more instructions or more labels per function on average ?
            Maybe labels should be parsed as a type of instruction ?
            */

            size_t idx = this->labels.size();
            LabelToken& lbl_tok = this->labels.emplace_back(this);
            if (parser.parse_token(&lbl_tok, true)) {
                this->order.push_back({ DEF_LABEL, idx });
                parser.consume_blanks();
                continue;
            }
            this->labels.pop_back();

            idx = this->instructions.size();
            InstructionToken& instr_tok = this->instructions.emplace_back(this);
            if (parser.parse_token(&instr_tok, true)) {
                this->order.push_back({ DEF_INSTR, idx });
                parser.consume_blanks();
                continue;
            }
            this->instructions.pop_back();
            break;
        }

        if (!parser.get_exact_operator(Operator::CLOSE_CURLY))
            return fail(this, "Function body must end with '}'");
        parser.advance_operator(Operator::CLOSE_CURLY);

        return true;
    }
}

