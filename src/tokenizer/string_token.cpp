#include "string_token.hpp"
#include "number_token.hpp"
#include "../formatter.hpp"


namespace cpasm {


    enum class StringParseStatus {
        NONE,
        BAD_ESCAPE,
        NUMBER_ESCAPE_OVERFLOW,
        NUMBER_ESCAPE_INVALID,
        NO_TERM,
    };


    static std::string_view _string_error_msgs[] = {
        "Success",
        "Invalid escape sequence '\\{0}' at position {1}",
        "Numeric escape sequence '\\{0}' too big at position {1}",
        "Numeric escape sequence '\\{0}' invalid at postiion {1}: {2}",
        "Missing string terminator at position {1}",
    };

    struct StringParseError {
        StringParseStatus status;
        std::string_view esc_sequence = "";
        u16 position = 0;
        std::string suberror = "";

        std::string format() const {
            // auto idx = get_underlying(this->status);
            // if (idx < 0 || idx >= array_len(_string_error_msgs))
            //     return "Unknown error";

            std::string_view text = array_index_by_enum<std::string_view>(_string_error_msgs, this->status, "Unknown error"); //_string_error_msgs[idx];
            // the following is safe because our text is not user input, but one of the hard-coded messages defined by our program
            return cpasm::format(text, this->esc_sequence, this->position, this->suberror);
        }
        operator bool() const {
            return this->status == StringParseStatus::NONE;
        }

        static StringParseError success() {
            return {
                .status = StringParseStatus::NONE,
            };
        }

        static StringParseError BAD_ESCAPE(std::string_view esc_sequence, u16 pos) {
            return {
                .status = StringParseStatus::BAD_ESCAPE,
                .esc_sequence = esc_sequence,
                .position = pos,
            };
        }
        static StringParseError NUMBER_ESCAPE_OVERFLOW(std::string_view esc_sequence, u16 pos) {
            return {
                .status = StringParseStatus::NUMBER_ESCAPE_OVERFLOW,
                .esc_sequence = esc_sequence,
                .position = pos,
            };
        }   
        static StringParseError NUMBER_ESCAPE_INVALID(std::string_view esc_sequence, u16 pos, std::string_view suberr) {
             return {
                .status = StringParseStatus::NUMBER_ESCAPE_INVALID,
                .esc_sequence = esc_sequence,
                .position = pos,
                .suberror = (std::string)suberr,
            };
        }
        static StringParseError NO_TERM(u16 pos) {  
            return {
                .status = StringParseStatus::BAD_ESCAPE,
                .position = pos,
            };
        }
    };


    static StringParseError _parse_numeric_escape(Parser& parser, int base, std::vector<char>* out, bool start_next) {
        if (start_next) parser.advance_char(1);
        size_t num;

        IntParseError err;
        if (!(err = parse_digits(parser, &num, base))) {
            return StringParseError::NUMBER_ESCAPE_INVALID("??", 0, err.format());  // invalid escape sequence: bad integer literal
        }
        if (num > std::numeric_limits<u8>::max()) {
            return StringParseError::NUMBER_ESCAPE_OVERFLOW("??", 0);  // invalid escape sequence: bad integer literal
        }
        out->push_back(static_cast<char>(num));

        return StringParseError::success();
    }

    template<class T>
    static bool _parse_string(Parser& parser, const Operator* delimiter, std::vector<char>* out, T* tok) {
        *out = {};

        auto quote_tok = parser.get_exact_operator(delimiter);
        if (!quote_tok)
            return Token::fail(tok, "Missing quote at beginning of string.");
        
        auto st = parser.save();
        parser.advance_operator(quote_tok);

        bool in_escape = false;

        for (u16 pos = 0;; pos++) {
            i16 c_i = parser.get_char();
            if (c_i < 0) {  // missing string terminator
                st.restore();
                return Token::fail(tok, "Unterminated string.");  
            }

            char c = (char)c_i;
            bool advanced = false;

            StringParseError err;
            if (in_escape) {
                i16 i;
                switch (c) {
                    case 'a':
                        out->push_back('\a');
                        break;
                    case 'b':
                        out->push_back('\b');
                        break;
                    case 't':
                        out->push_back('\t');
                        break;
                    case 'n':
                        out->push_back('\n');
                        break;
                    case 'v':
                        out->push_back('\v');
                        break;
                    case 'f':
                        out->push_back('\f');
                        break;
                    case 'r':
                        out->push_back('\r');
                        break;
                    case '"':
                        out->push_back('"');
                        break;
                    case '\'':
                        out->push_back('\'');
                        break;
                    case '\\':
                        out->push_back('\\');
                        break;
                    case '`':
                        out->push_back('`');
                        break;
                    case 'B':
                        advanced = true;
                        if (!(err = _parse_numeric_escape(parser, 2, out, true))) {
                            st.restore();
                            err.position = pos;
                            return Token::fail(tok, err.format());
                        }
                        break;
                    case 'o':
                        advanced = true;
                        if (!(err = _parse_numeric_escape(parser, 8, out, true))) {
                            st.restore();
                            err.position = pos;
                            return Token::fail(tok, err.format());
                        }
                        break;
                    case 'd':
                        advanced = true;
                        if (!(err = _parse_numeric_escape(parser, 10, out, true))) {
                            st.restore();
                            err.position = pos;
                            return Token::fail(tok, err.format());
                        }
                        break;
                    case 'x':
                        advanced = true;
                        if (!(err = _parse_numeric_escape(parser, 16, out, true))) {
                            st.restore();
                            err.position = pos;
                            return Token::fail(tok, err.format());
                        }
                        break;
                    default:
                        if (ParsingRules::decimal_digits.check(c)) {
                            if (!(err = _parse_numeric_escape(parser, 10, out, false))) {
                                st.restore();
                                err.position = pos;
                                return Token::fail(tok, err.format());
                            }
                            break;
                        }
                        st.restore();
                        return Token::fail(tok, sfmt("invalid escape sequence '\\", c, "[...]' in string literal"));
                }

            } else {
                bool done = false;

                switch (c) {
                    case '\n':
                    case '\r':
                        st.restore();
                        return Token::fail(tok, "Unterminated string.");  
                    
                    case '\\':
                        in_escape = true;
                        break;

                    default:
                        if (std::string_view(&c, 1) == delimiter->name) {
                            done = true;
                            break;
                        }
                        out->push_back(c);
                        break;
                }

                if (done) {
                    if (!advanced)
                        parser.advance_char(1);
                    break;
                }
            }

            if (!advanced)
                parser.advance_char(1);
        }

        return true;
    }


    bool StringLiteralToken::parse(Parser& parser) {
        this->Token::parse(parser);

        std::vector<char> out;

        bool res = _parse_string(
            parser, Operator::DQUOTE, &out, this
        );

        if (res)
            this->value = (std::string)vec2sview(out);
        return res;
    }

    bool IdentStringToken::parse(Parser& parser) {
        this->Token::parse(parser);

        std::vector<char> out;

        bool res = _parse_string(
            parser, Operator::BTICK, &out, this
        );

        if (res)
            this->value = (std::string)vec2sview(out);
        return res;
    }
}

