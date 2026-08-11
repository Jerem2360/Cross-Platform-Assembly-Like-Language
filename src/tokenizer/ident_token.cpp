#include "ident_token.hpp"


namespace cpasm {
    bool IdentifierToken::parse(Parser& parser) {
        if (parser.get_word_type() != WordType::NAME)
            return fail(this, "Expected identifier.");
        
        this->value = parser.get_word();
        parser.advance_word(1);
        return true;
    }
}

