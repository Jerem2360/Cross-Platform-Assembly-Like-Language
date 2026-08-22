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
}

