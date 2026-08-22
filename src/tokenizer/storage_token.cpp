#include "storage_token.hpp"


namespace cpasm {
    bool BoxToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        if (!parser.get_exact_operator(Operator::BOX)) {
            return fail(this, "missing '$' at beginning of token");
        }
        this->name = IdentifierToken{this};
        if (!this->name.parse(parser)) {
            return fail(this, &this->name, "missing identifier after '$'");
        }
        return true;
    }

    bool RegisterToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        if (!parser.get_exact_operator(Operator::REG)) {
            return fail(this, "missing '@' at beginning of token");
        }
        this->name = IdentifierToken{this};
        if (!this->name.parse(parser)) {
            return fail(this, &this->name, "missing identifier after '@'");
        }
        return true;
    }

    bool StorageToken::parse(Parser& parser) {
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
            return g.fail(this, "missing storage token.");

        return true;
    }
}

