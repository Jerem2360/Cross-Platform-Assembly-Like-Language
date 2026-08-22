#include "ident_token.hpp"


namespace cpasm {
    bool IdentifierToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        if (parser.get_word_type() != WordType::NAME)
            return fail(this, "Invalid identifier.");
        
        this->value = parser.get_word();
        parser.advance_word(1);
        return true;
    }

    bool SymbolNameToken::parse(Parser& parser) {
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
            return g.fail(this, "invalid name");
        return true;
    };

    bool LabelUseToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        this->base = {this};
        if (!parser.parse_token(&this->base, false)) {
            return fail(this, &this->base, "Expected identifier as base of label");
        }
        if (parser.get_exact_operator(Operator::DOT)) {
            this->nested = {this};
            if (!parser.parse_token(&this->nested, false)) {
                return fail(this, &this->nested, "Label: expected identifier after '.'");
            }
        }
        return true;
    }
}

