#include "statement_tokens.hpp"


namespace cpasm {
    bool IfStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("if", WordType::NAME))
            return fail(this, "missing 'if' at start of statement");
        parser.advance_word(1);
        parser.consume_blanks();

        this->condition = {this};
        if (!parser.parse_token(&this->condition, false))
            return fail(this, &this->condition, "invalid condition for if statement");
        return true;
    }

    bool ElifStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("elif", WordType::NAME))
            return fail(this, "missing 'elif' at start of statement");
        parser.advance_word(1);
        parser.consume_blanks();

        this->condition = {this};
        if (!parser.parse_token(&this->condition, false))
            return fail(this, &this->condition, "invalid condition for elif statement");
        return true;
    }

    bool ElseStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("else", WordType::NAME))
            return fail(this, "missing 'else' at start of statement");

        parser.advance_word(1);
        return true;
    }
    
    bool EndifStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("endif", WordType::NAME))
            return fail(this, "missing 'endif' at start of statement");

        parser.advance_word(1);
        return true;
    }

    bool IncludeStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("include", WordType::NAME))
            return fail(this, "missing 'include' at start of statement");
        parser.advance_word(1);
        parser.consume_blanks();

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, &this->name, "invalid filename for include: must be a string literal");

        return true;
    }

    bool EntryPointStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("entry", WordType::NAME))
            return fail(this, "missing 'entry' at start of statement");
        parser.advance_word(1);
        parser.consume_blanks();

        this->body = {this};
        if (!parser.parse_token(&this->body, false))
            return fail(this, &this->body, "invalid function body");

        return true;
    }

    bool FunctionStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("fn", WordType::NAME))
            return fail(this, "missing 'fn' at start of function");
        parser.advance_word(1);
        parser.consume_blanks();

        parse_list(this, parser, [&](Parser& _) {
            IdentifierToken& tok = this->attributes.emplace_back(this);
            if (parser.parse_token(&tok, true))
                return true;
            this->attributes.pop_back();
            return false;
        }, Operator::COMMA, Operator::OPEN_SQUARE, Operator::CLOSE_SQUARE);

        parser.consume_blanks();

        SymbolNameToken& name_tok = this->name.emplace(this);
        if (!parser.parse_token(&name_tok, true))
            this->name = std::nullopt;

        parser.consume_blanks();
        
        FunctionSigToken& sig_tok = this->signature.emplace(this);
        if (!parser.parse_token(&sig_tok, true))
            this->signature = std::nullopt;

        parser.consume_blanks();

        if (parser.get_keyword("export", WordType::NAME)) {
            this->properties = PROP_EXPORT;  // should be "&=", but somehow cpp doesn't allow bitwise on enums 
            parser.advance_word(1);
            parser.consume_blanks();
        }

        this->body = {this};
        if (!parser.parse_token(&this->body, false))
            return fail(this, &this->body, "Missing function body");

        return true;
    }

    bool DataDeclStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("data", WordType::NAME))
            return fail(this, "missing 'data' at start of declaration");
        parser.advance_word(1);
        parser.consume_blanks();

        this->type = {this};
        if (!parser.parse_token(&this->type, false))
            return fail(this, &this->type, "missing datatype for declaration");

        parser.consume_blanks();

        if (parser.get_exact_operator(Operator::OPEN_SQUARE)) {
            parser.advance_operator(Operator::OPEN_SQUARE);
            parser.consume_blanks();

            IntLiteralToken& len_tok = this->length.emplace(this);
            if (!parser.parse_token(&len_tok, false))
                return fail(this, &len_tok, "Missing data length after '['");

            parser.consume_blanks();

            if (!parser.get_exact_operator(Operator::CLOSE_SQUARE))
                return fail(this, "missing ']' after data size specification");

            parser.advance_operator(Operator::CLOSE_SQUARE);

            parser.consume_blanks();
        } else 
            this->length = std::nullopt;

        StaticOperandToken& val_tok = this->value.emplace(this);
        if (!parser.parse_token(&val_tok, true))
            this->value = std::nullopt;
        
        return true;
    }

    bool ImportStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("from", WordType::NAME))
            return fail(this, "missing 'from' at start of import");
        parser.advance_word(1);
        parser.consume_blanks();

        this->source = {this};
        if (!parser.parse_token(&this->source, false))
            return fail(this, &this->source, "invalid source operand for import statement");

        parser.consume_blanks();

        if (!parser.get_keyword("import", WordType::NAME))
            return fail(this, "missing 'import' after source in import statement");
        parser.advance_word(1);
        parser.consume_blanks();

        bool res = parse_list(this, parser, [&](Parser&){
            IdentifierToken& sym_tok = this->symbols.emplace_back(this);
            if (!parser.parse_token(&sym_tok, true)) {
                this->symbols.pop_back();
                return false;
            }
            return true;
        }, Operator::COMMA);

        if (!res)
            return fail(this, "invalid list of symbols for import statement");

        return true;
    }

    bool SymbolStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

        if (!parser.get_keyword("sym", WordType::NAME))
            return fail(this, "missing 'sym' at start of symbol");
        parser.advance_word(1);
        parser.consume_blanks();

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, "missing the symbol's name after 'sym'");

        parser.consume_blanks();

        if (parser.get_keyword("export", WordType::NAME)) {
            this->properties = PROP_EXPORT;  // should be "&=", but somehow cpp doesn't allow bitwise on enums 
            parser.advance_word(1);
            parser.consume_blanks();
        }

        parser.consume_blanks();

        if (!parser.get_exact_operator(Operator::COLON))
            return fail(this, "missing ':' at end of symbol decl");

        parser.advance_operator(Operator::COLON);
        return true;
    }

    bool SectionStatementToken::parse(Parser& parser) {
        this->Token::parse(parser);

         if (!parser.get_keyword("section", WordType::NAME))
            return fail(this, "missing 'section' at start of section decl");
        parser.advance_word(1);
        parser.consume_blanks();

        this->has_dot = parser.get_exact_operator(Operator::DOT);
        if (this->has_dot)
            parser.advance_operator(Operator::DOT);

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, &this->name, "missing symbol name after 'sym'");

        parser.consume_blanks();

        IdentifierToken& acc_tok = this->access.emplace(this);
        if (!parser.parse_token(&acc_tok, true))
            this->access = std::nullopt;

        return true;
    }
}

