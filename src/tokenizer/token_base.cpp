#include "token_base.hpp"


namespace cpasm {
    Token::Token(Token* parent) :
        _tokenizer(parent ? parent->_tokenizer : nullptr)
    {}

    bool Token::parse(Parser& parser) {
        this->_lineno = parser.get_lineno();
        return true;
    }

    bool Token::_fail(std::vector<Token*> causes, std::string_view message, std::string_view class_name) {
        this->_frame = std::make_unique<TokenStackTraceFrame>(
            vec_foreach<Token*, std::unique_ptr<TokenStackTraceFrame>>(causes, 
                [](Token* const& t) -> std::unique_ptr<TokenStackTraceFrame> {
                    if (t)
                        return std::move(t->_frame);
                    return nullptr;
                }
            ),
            message,
            class_name,
            this->_lineno
        );
        return false;
    }
    bool Token::_fail(std::vector<std::unique_ptr<TokenStackTraceFrame>> causes, std::string_view message, std::string_view class_name) {
        this->_frame = std::make_unique<TokenStackTraceFrame>(
            std::move(causes),
            message,
            class_name,
            this->_lineno
        );
        return false;
    }



    TokenStackTraceFrame::TokenStackTraceFrame(
        std::vector<std::unique_ptr<TokenStackTraceFrame>> causes,
        std::string_view msg,
        std::string_view tok_name,
        int lineno
    ) :
        _children(std::move(causes)),
        _message(msg),
        _tok_name(tok_name),
        _lineno(lineno)
    {}

    TokenErrorGroup::TokenErrorGroup() :
        _causes()
    {}
    void TokenErrorGroup::add(Token* cause) {
        this->_causes.emplace_back(std::move(cause->_frame));
    }
}

