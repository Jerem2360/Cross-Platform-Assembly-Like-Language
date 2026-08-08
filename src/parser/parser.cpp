#include "parser.hpp"


namespace cpasm {
    Parser::state::state(Parser* parser) :
        _parser(parser), 
        _word_offset(parser ? parser->_word_offset : 0),
        _char_offset(parser ? parser->_char_offset : 0)
    {}
    void Parser::state::restore() const {
        if (!this->_parser)
            return;

        this->_parser->_word_offset = this->_word_offset;
        this->_parser->_char_offset = this->_char_offset;
    }


    Parser::Parser(array_view<CharWord> words, std::string_view filename) :
        _words(words), _word_offset(0), _char_offset(0), _filename(filename)
    {}


    void Parser::advance_char(size_t n) {
        size_t remaining = n;

        while (true) {
            if (!this->remaining_words())
                return;

            size_t word_remain = this->_words[this->_word_offset].chars.size() - this->_char_offset;

            if (word_remain > remaining) {
                this->_char_offset += remaining;
                return;
            }

            this->advance_word(1);
            remaining -= word_remain;
        }
    }
    void Parser::advance_word(size_t n) {
        size_t amount = std::min(this->remaining_words(), n);

        this->_word_offset += amount;
        if (amount)
            this->_char_offset = 0;
    }
    i16 Parser::get_char() const {
        if (this->exhausted())
            return -1;
        return this->_words[this->_word_offset].chars[this->_char_offset];
    }
    std::string_view Parser::get_word() const {
        if (!this->remaining_words())
            return {};
        return sslice(
            this->_words[this->_word_offset].chars, this->_char_offset, SIZE_INVALID        
        );
    }
    WordType Parser::get_word_type() const {
        if (!this->remaining_words())
            return WordType::NONE;
        return this->_words[this->_word_offset].type;
    }
    int Parser::get_lineno() const {
        if (!this->remaining_words()) {
            if (!this->_words.size())
                return 0;
            return coll_last(this->_words).lineno;
        }
        return this->_words[this->_word_offset].lineno;
    }
    std::string_view Parser::get_filename() const {
        return this->_filename;
    }


    const Operator* Parser::get_operator(OperationType type, OperatorPositioning pos) const {        
        auto wtype = this->get_word_type();
        if (wtype != WordType::SYMBOLS)  // when exhausted word type is NONE so that case is taken care of
            return nullptr;

        std::string_view wrd = this->get_word();
        
        // first operator that is present and matches requirements.
        // The list method returns the operators in order of decreasing name length, which
        // prevents small operators from masking long operators.
        for (const Operator& op : Operators) {
            if ((type != OperationType::NONE) && (op.type != type))
                continue;
            if ((pos != OperatorPositioning::NONE) && (op.position != pos))
                continue;
            if (wrd.starts_with(op.name)) {
                return &op;
            }
        }
        return nullptr;
    }

    const Operator* Parser::get_exact_operator(const Operator* op, bool strict) const {
        if (!op)
            return nullptr;
        if (this->get_word_type() != WordType::SYMBOLS)  // when exhausted word type is NONE so that case is taken care of
            return nullptr;
        if (!this->get_word().starts_with(op->name))
            return nullptr;
        if (strict && this->get_word().size() > op->name.size())
            return nullptr;
        return op;
    }

    const Operator* Parser::get_any_exact_operator(const Operator* op, size_t* poffset) const {
        if (!op)
            return nullptr;
        if (this->get_word_type() != WordType::SYMBOLS)  // when exhausted word type is NONE so that case is taken care of
            return nullptr;

        std::string_view text = this->get_word();

        for (size_t i = 0; i < text.size(); i++) {   // if the next word's size is 0, the loop never runs and we return nullptr immediately
            if (text.size() < op->name.size())
                return nullptr;
                
            if (text.starts_with(op->name)) {
                *poffset = i;
                return op;
            }

            text = sslice(text, 1, text.size() - 1);
        }

        return nullptr;
    }

    void Parser::advance_operator(const Operator* op) {
        if (!op)
            return;
        if (this->get_word_type() != WordType::SYMBOLS)  // when exhausted word type is NONE so that case is taken care of
            return;
        if (!this->get_word().starts_with(op->name))
            return;
        this->advance_char(op->name.size());
    }

    bool Parser::advance_word_prefix(std::string_view text) {
        if (this->exhausted())
            return false;
        
        if (this->get_word().starts_with(text)) {
            this->advance_char(text.size());
            return true;
        }
        return false;
    }

    bool Parser::advance_exact_word(std::string_view text) {
        if (this->exhausted())
            return false;
        
        if (this->get_word() == text) {
            this->advance_word(1);
            return true;
        }
        return false;
    }

    bool Parser::advance_until(bool (*pred)(std::string_view)) {
        for (size_t i = 0; i < this->get_word().size(); i++) {
            if (pred(this->get_word()))
                return true;
            this->advance_char(1);
        }
        return false;
    }

    void Parser::consume_spaces() {
        while (this->get_word_type() == WordType::SPACES) {   // when exhausted word type is NONE so that case is taken care of
            this->advance_word(1);
        }
    }

    bool Parser::consume_comment() {
        const Operator* op = this->get_exact_operator(Operator::OPEN_CMT);
        if (op) {
            while (!this->exhausted()) {
                size_t offset;
                op = this->get_any_exact_operator(Operator::CLOSE_CMT, &offset);
                if (!op) {
                    this->advance_word(1);
                    continue;
                }

                this->advance_char(offset + Operator::CLOSE_CMT->name.size());
                return true;
            }
            return true;
        }

        op = this->get_exact_operator(Operator::LINE_CMT);
        if (!op)
            return false;

        while (!this->exhausted()) {
            if (this->get_word_type() != WordType::SPACES) {
                this->advance_word(1);
                continue;
            }

            if (this->advance_until(
                [](std::string_view word) {
                    return word.starts_with("\r\n") || word.starts_with('\r') || word.starts_with('\n');
                }
            )) {
                int cnt = 1;
                if (this->get_word().starts_with("\r\n"))   // TODO: still not perfect: maybe we can improve this further ?
                    cnt++;
                this->advance_char(cnt);
                return true;
            }

        }

        return true;
    }

    void Parser::consume_blanks() {
        while (!this->exhausted()) {
            this->consume_spaces();
            if (!this->consume_comment())
                break;
        }
    }

    size_t Parser::remaining_words() const {
        return this->_words.size() - this->_word_offset;
    }
    bool Parser::exhausted() const {
        if (this->remaining_words() != 1)
            return !this->remaining_words();
        
        return this->_char_offset > this->_words[this->_word_offset].chars.size();
    }

    Parser::state Parser::save() {
        return state(this);
    }

}

