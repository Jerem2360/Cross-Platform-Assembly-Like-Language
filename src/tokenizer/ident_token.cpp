#include "ident_token.hpp"


namespace cpasm {
    bool IdentifierToken::parse(Parser& parser) {
        if (parser.get_word_type() != WordType::NAME)
            return fail(this, "Invalid identifier.");
        
        this->value = parser.get_word();
        parser.advance_word(1);
        return true;
    }

    bool LabelUseToken::parse(Parser& parser) {
        this->base = IdentifierToken{this};
        if (!this->base.parse(parser)) {
            return fail(this, &this->base, "Expected identifier as base of label");
        }
        if (parser.get_exact_operator(Operator::DOT)) {
            this->nested = {this};
            if (!this->nested.parse(parser)) {
                return fail(this, &this->nested, "Label: expected identifier after '.'");
            }
        }
        return true;
    }
}

