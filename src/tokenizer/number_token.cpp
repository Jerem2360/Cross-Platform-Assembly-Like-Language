#include "number_token.hpp"


namespace cpasm {
    bool IntToken::parse(Parser& parser) {
        this->Token::parse(parser);
        return false;
    }

    bool FloatToken::parse(Parser& parser) {
        this->Token::parse(parser);
        return false;
    }

    bool NumberToken::parse(Parser& parser) {

        // idiom for when dealing with multiple possibilities that might all fail
        this->Token::parse(parser);

        TokenErrorGroup g;

        auto& itok = this->value.emplace<IntToken>(this);
        if (itok.parse(parser)) {
            return true;
        } else g.add(&itok);
        
        auto& ftok = this->value.emplace<FloatToken>(this);
        if (ftok.parse(parser)) {
            return true;
        } else g.add(&ftok);

        return g.fail(this, "Hello");
    }
}

