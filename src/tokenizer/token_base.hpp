#pragma once
#include <vector>
#include <memory>
#include "../parser/parser.hpp"


namespace cpasm {
    class Tokenizer;
    class TokenStackTraceFrame;
    class TokenErrorGroup;

    
    enum class TokenErrorCode : u8 {
        NONE = 0,   // success
        UNSPECIFIED_ERROR,
        INT_OVERFLOW,
    };

    enum SupportType : u8 {
        SUPPORTS_NONE = 0,
        SUPPORTS_RUNTIME =  0b01,
        SUPPORTS_COMPTIME = 0b10,
        SUPPORTS_BOTH =     0b11,
    };


    /**
     * Base class for all token types.
     * Child classes should "override" the parse method by hiding the base class version.
     * When parsing fails, the 'fail' static method must be called.
     */
    class Token {
        // The tokenizer needs to access the _frame member of the root token in order to retrieve a potential stacktrace.
        friend class Tokenizer;
        // The TokenErrorGroup class needs to access the _frame member so it can properly propagate exceptions.
        friend class TokenErrorGroup;

        int _lineno = 0;
        Tokenizer* _tokenizer;  // the tokenizer knows about the filename
        std::unique_ptr<TokenStackTraceFrame> _frame = nullptr;

        bool _fail(std::vector<Token*> causes, std::string_view message, std::string_view class_name, TokenErrorCode code);
        bool _fail(std::vector<std::unique_ptr<TokenStackTraceFrame>> causes, std::string_view message, std::string_view class_name, TokenErrorCode code);

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

        /**
         * Called when parsing fails, with the current token instance as the first argument.
         * Raises an error onto the token stacktrace.
         */
        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, std::string_view message, TokenErrorCode code = TokenErrorCode::UNSPECIFIED_ERROR) {
            return token->_fail(
                std::vector<Token*>{},
                message,
                type_details<T>::name,
                code
            );
        }
        /**
         * Called when parsing fails, with the current token instance as the first argument.
         * Propagates an error (the second argument) onto the token stacktrace.
         */
        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, Token* cause, std::string_view message, TokenErrorCode code = TokenErrorCode::UNSPECIFIED_ERROR) {
            return token->_fail(
                std::vector{ cause },
                message,
                type_details<T>::name,
                code
            );
        }
        /**
         * Called when parsing fails, with the current token instance as the first argument.
         * Propagates a group of errors (the second argument) onto the token stacktrace.
         * This makes the resulting stacktrace look like a tree, which branches with the provided error message.
         */
        template<class T>
            requires std::is_base_of_v<Token, T>
        static bool fail(T* token, std::vector<std::unique_ptr<TokenStackTraceFrame>> causes, std::string_view message, TokenErrorCode code = TokenErrorCode::UNSPECIFIED_ERROR) {
            return token->_fail(
                std::move(causes),
                message,
                type_details<T>::name,
                code
            );
        }
    };  


    /**
     * A token stacktrace is a tree-like structure that describes why parsing failed at a given location
     * in the user code.
     * Each node corresponds to a token type failing to parse the program, and explains the reason.
     * Each non-leaf node contains references to one or more nodes describing the failures of child tokens.
     */
    class TokenStackTraceFrame {
        std::vector<std::unique_ptr<TokenStackTraceFrame>> _children;
        std::string_view _message;  // error message describing the failure
        std::string_view _tok_name;  // name of the token (extracted from c++ rtti)
        int _lineno;  // line at which the token was expected
        TokenErrorCode _errcode;   // optional detailed code describing the cause of the error.

    public:
        TokenStackTraceFrame(
            std::vector<std::unique_ptr<TokenStackTraceFrame>> causes,
            std::string_view msg,
            std::string_view tok_name,
            int lineno,
            TokenErrorCode code
        );

        TokenErrorCode code() const;
    };

    class TokenErrorGroup {
        std::vector<std::unique_ptr<TokenStackTraceFrame>> _causes;

    public:
        TokenErrorGroup();
        void add(Token* cause);
        template<class T>
            requires std::is_base_of_v<Token, T>
        inline bool fail(T* tok, std::string_view msg, TokenErrorCode code = TokenErrorCode::UNSPECIFIED_ERROR) {
            return Token::fail(tok, std::move(this->_causes), msg, code);
        }
    };

    template<class T>
    concept _has_support = requires() {
        { T::support() } -> std::same_as<SupportType>;
    };

    /**
     * Return the SupportType of a std::variant of tokens.
     * The SupportType of the currently active token is used.
     */
    template<class ...T>
        requires (std::is_base_of_v<Token, T> && ...)
    SupportType token_variant_supports(const std::variant<T...>& value) {
        SupportType result = SUPPORTS_NONE;

        ((
            (std::holds_alternative<T>(value) && _has_support<T>) ? 
                (void)(result = T::support()) :
                void()
        ), ...);
        return result;
    }
}

