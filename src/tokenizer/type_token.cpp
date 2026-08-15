#include "type_token.hpp"


namespace cpasm {
    bool DataTypeToken::parse(Parser& parser) {
        this->name = IdentifierToken{this};
        if (!this->name.parse(parser)) {
            return fail(this, &this->name, "data type: expected a name to start with");
        }
        if (parser.get_exact_operator(Operator::DOT)) {
            this->bits = IntLiteralToken{this};
            if (!this->bits.parse(parser)) {
                return fail(this, &this->bits, "data type: size in bits is required after '.'");
            }
        }
        return true;
    }
}

