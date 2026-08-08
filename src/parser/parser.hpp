#pragma once
#include "../helpers.hpp"
#include "../wordizer/wordizer.hpp"
#include "../operators.hpp"


namespace cpasm {
    class Parser {
    public:
        class state {
            Parser* _parser;
            size_t _word_offset;
            size_t _char_offset;

        public:
            state(Parser* parser);
            void restore() const;
        };

    private:
        array_view<CharWord> _words;
        size_t _word_offset;
        size_t _char_offset;
        std::string _filename;

    public:
        Parser(array_view<CharWord> words, std::string_view filename);


        // low level helpers
        void advance_char(size_t n);
        void advance_word(size_t n);
        i16 get_char() const;
        std::string_view get_word() const;
        WordType get_word_type() const;
        int get_lineno() const;
        std::string_view get_filename() const;


        // higher level functions
        /**
         * If the parser text starts with a matching operator, return that operator; return nullptr instead.
         * Passing the NONE value for type or pos matches all possible values of type/pos. 
         */
        const Operator* get_operator(OperationType type, OperatorPositioning pos) const;
        /**
         * If the parser text starts with the provided operator, return the operator itself; nullptr otherwise.
         */
        const Operator* get_exact_operator(const Operator* op, bool strict = false) const;
        /*
        Same as get_exact_operator, except searches for op character by character until the end of the current word.
        Upon success, advances the parser up to the beginning of the operator.
        */
        const Operator* get_any_exact_operator(const Operator* op, size_t* offset) const;
        /**
         * If the parser text starts with the provided operator, consume the operator's characters. Meant to be passed
         * the result of a get_operator call.
         */
        void advance_operator(const Operator* op);
        /**
         * If the next word starts with the provided text, consume the characters in question 
         * and return true. Return false otherwise.
         */
        bool advance_word_prefix(std::string_view text);
        /**
         * If the next word matches exactly the provided text, consume the characters in question 
         * and return true. Return false otherwise.
         */
        bool advance_exact_word(std::string_view wrd);
        /**
         * Keep advancing within the current word until the predicate returns true.
         * The current word is passed as a string to the predicate at each iteration.
         * Upon success, consume until the predicate is true and return true.
         * Otherwise, consume until the end of the word, and return false.
         */
        bool advance_until(bool (*pred)(std::string_view));
        /**
         * Keep consuming characters until the first character is no longer a space character.
         * This checks characters word by word instead of char by char.
         */
        void consume_spaces();
        /**
         * If a comment begins right here, keep advancing until the end of the comment and return true.
         * Otherwise, do nothing and return false.
         */
        bool consume_comment();

        void consume_blanks();


        size_t remaining_words() const;
        bool exhausted() const;
        state save();

    };
}

