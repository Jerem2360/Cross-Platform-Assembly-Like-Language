#pragma once
#include <vector>
#include <memory>
#include "../parser/parser.hpp"


namespace cpasm {
    class Tokenizer;
    class TokenStackTraceFrame;
    class TokenErrorGroup;


    class Token {
        friend class Tokenizer;
        friend class TokenErrorGroup;

        int _lineno = 0;
        Tokenizer* _tokenizer;  // the tokenizer knows about the filename
        std::unique_ptr<TokenStackTraceFrame> _frame = nullptr;

        bool _fail(std::vector<Token*> causes, std::string_view message, std::string_view class_name);
        bool _fail(std::vector<std::unique_ptr<TokenStackTraceFrame>> causes, std::string_view message, std::string_view class_name);

    public:
        Token(Token* parent = nullptr);

        /**
         * Parse a token using the provided parser.
         * This shall be overridden by subclasses with a function that
         * initializes the subclass' members using the parser.
         * This function never fails, and must be called by an overriding 
         * function at the beginning of its body.
         */
        bool parse(Parser& parser);

        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, std::string_view message) {
            return token->_fail(
                {},
                message,
                type_details<T>::name
            );
        }
        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, Token* cause, std::string_view message) {
            return token->_fail(
                std::vector{ cause },
                message,
                type_details<T>::name
            );
        }
        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, std::vector<std::unique_ptr<TokenStackTraceFrame>> causes, std::string_view message) {
            return token->_fail(
                std::move(causes),
                message,
                type_details<T>::name
            );
        }
    };  


    class TokenStackTraceFrame {
        std::vector<std::unique_ptr<TokenStackTraceFrame>> _children;
        std::string_view _message;
        std::string_view _tok_name;
        int _lineno;

    public:
        TokenStackTraceFrame(
            std::vector<std::unique_ptr<TokenStackTraceFrame>> causes,
            std::string_view msg,
            std::string_view tok_name,
            int lineno
        );
    };

    class TokenErrorGroup {
        std::vector<std::unique_ptr<TokenStackTraceFrame>> _causes;

    public:
        TokenErrorGroup();
        void add(Token* cause);
        template<class T>
            requires std::is_base_of_v<Token, T>
        inline bool fail(T* tok, std::string_view msg) {
            return Token::fail(tok, std::move(this->_causes), msg);
        }
    };
}

