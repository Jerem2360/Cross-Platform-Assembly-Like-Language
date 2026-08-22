#include "operand_tokens.hpp"


namespace cpasm {
    bool LabelUseToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        this->base = {this};
        if (!parser.parse_token(&this->base, false)) {
            return fail(this, &this->base, "Expected identifier.");
        }
        if (parser.get_exact_operator(Operator::DOT)) {
            parser.advance_operator(Operator::DOT);

            parser.consume_blanks();

            IdentifierToken& sub = this->sublabel.emplace(this);
            if (!parser.parse_token(&sub, true)) {
                return fail(this, &sub, "Expected identifier after '.'");
            }
        }
        return true;
    }

    bool ComptimeConstToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        if (!parser.get_exact_operator(Operator::CMP_TIME_VAR))
            return fail(this, "Missing '$' before compile time constant");

        this->name = {this};
        if (!parser.parse_token(&this->name, false))
            return fail(this, &this->name, "an identifier is required");

        return true;
    }

    bool DereferableOperandToken::parse(Parser& parser) {
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
            return g.fail(this, "missing dereferable token.");
        return true;
    }

    bool StaticOperandToken::parse(Parser& parser) {
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
            return g.fail(this, "missing static operand token.");
        return true;
    }

    static bool _parse_deref_base(Parser& parser, DereferenceToken* self) {
        self->base = {self};
        if (!parser.parse_token(&self->base, false)) {
            return Token::fail(self, &self->base, "Missing base component of dereference.");
        }
        parser.consume_blanks();
        return true;
    }

    static uint8_t _parse_deref_multicomponent(Parser& parser, DereferenceToken* self) {
        if (!parser.get_exact_operator(Operator::OPEN_BR)) {
            return 2;  // first element failed, might just be a sole base.
        }

        parser.consume_blanks();

        if (!_parse_deref_base(parser, self)) {
            return 0;  // hard failure, message is already stored into self
        }

        if (!parser.get_exact_operator(Operator::ADDR_ADD)) {
            return 1;  // we're done
        }
        parser.advance_operator(Operator::ADDR_ADD);
        parser.consume_blanks();

        auto& idx_tok = self->index.emplace(self);
        if (!parser.parse_token(&idx_tok, false)) {
            return Token::fail(self, &idx_tok, "Missing index component after '+' in dereference.");
        }

        parser.consume_blanks();

        if (!parser.get_exact_operator(Operator::ADDR_MUL)) {
            return 1;  // we're done
        }
        parser.advance_operator(Operator::ADDR_MUL);
        parser.consume_blanks();

        auto& scale_tok = self->scale.emplace(self);
        if (!parser.parse_token(&scale_tok, false)) {
            return Token::fail(self, &scale_tok, "Expected scale component after '+' in dereference.");
        }

        parser.consume_blanks();
        return 1;
    }

    static bool _parse_deref_type(Parser& parser, DereferenceToken* self) {
        if (!parser.get_exact_operator(Operator::OPEN_SQUARE))
            return Token::fail(self, "Missing '[' after dereferee in dereference.");
        parser.advance_operator(Operator::OPEN_SQUARE);
        parser.consume_blanks();
        
        self->type = {self};
        if (!parser.parse_token(&self->type, false)) {
            return Token::fail(self, &self->type, "Expected a type specification for a dereference.");
        }

        parser.consume_blanks();

        if (!parser.get_exact_operator(Operator::CLOSE_SQUARE))
            return Token::fail(self, "Missing ']' at end of dereference.");
        parser.advance_operator(Operator::CLOSE_SQUARE);

        return true;
    }

    bool DereferenceToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        auto st = parser.save();

        switch (_parse_deref_multicomponent(parser, this)) {
        case 0:  // hard failure
            return false;
        
        case 1: {  // success
            if (!parser.get_exact_operator(Operator::CLOSE_BR))
                return fail(this, "unclosed bracket in dereference.");
            parser.consume_blanks();
            break;
        }
        case 2: {   // might be something else
            st.restore();
            if (!_parse_deref_base(parser, this))
                return false;
            break;
        }
        }
        return _parse_deref_type(parser, this);
    }

    bool OperandToken::parse(Parser& parser) {
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
            return g.fail(this, "cannot resolve the operand type");
        return true;
    }

    bool ConditionToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        // not too sure what to do, that code could become an almost duplicate of SimpleInstructionToken::parse ...
        return fail(this, "not implemented"); 
    }
}

