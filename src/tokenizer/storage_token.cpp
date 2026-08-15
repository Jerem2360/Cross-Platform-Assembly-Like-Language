#include "storage_token.hpp"


namespace cpasm {
    bool BoxToken::parse(Parser& parser) {
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
        if (!parser.get_exact_operator(Operator::REG)) {
            return fail(this, "missing '@' at beginning of token");
        }
        this->name = IdentifierToken{this};
        if (!this->name.parse(parser)) {
            return fail(this, &this->name, "missing identifier after '@'");
        }
        return true;
    }
}

