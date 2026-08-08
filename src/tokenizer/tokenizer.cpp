#include "tokenizer.hpp"


namespace cpasm {
    Tokenizer::Tokenizer(Parser* parser) :
        _parser(parser)
    {}

    std::string_view Tokenizer::filename() const {
        return this->_parser->get_filename();
    }
}

