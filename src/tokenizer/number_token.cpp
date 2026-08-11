#include "number_token.hpp"
#include <cmath>
#include "../formatter.hpp"


namespace cpasm {
    // static std::string _fmt_error(std::string_view fmt, char character, u16 pos, u8 base) {
    //     std::stringstream res;

    //     bool in_fmt = false;

    //     for (std::string_view rest = fmt; rest.size(); rest = sslice(rest, 1, SIZE_INVALID)) {
    //         if (rest.starts_with('{') && !in_fmt) {
    //             in_fmt = true;
    //             continue;
    //         }
    //         if (in_fmt) {
    //             if (!rest.size()) {
    //                 in_fmt = false;
    //                 res << "{}";
    //                 continue;
    //             }
    //             if (rest[0] >= '0' && rest[0] <= '9') {
    //                 size_t idx = rest[0] - '0';
    //                 switch (idx) {
    //                 case 0:
    //                     res << character;
    //                     break;
    //                 case 1:
    //                     res << pos;
    //                     break;
    //                 case 2:
    //                     res << (int)base;
    //                     break;
    //                 default:
    //                     res << "{";
    //                     break;
    //                 }
    //                 continue;
    //             }
    //             if (rest[0] == '}') {
    //                 in_fmt = false;
    //                 continue;
    //             }

                
    //             res << "{}";
    //             in_fmt = false;
    //             continue;
    //         }
    //         res << rest[0];
    //     }

    //     return res.str();
    // }

    static std::string_view _int_error_messages[] {
        "Success",
        "Invalid digit '{0}' at position {1} for number with base {2}",
        "Invalid prefix for numeric literal",
        "Invalid syntax for numeric literal at position {1}",
        "Integer overflow"
    };


    std::string IntParseError::format() {
        // auto idx = get_underlying(this->status);
        // if (idx >= array_len(_int_error_messages) || idx < 0)
        //     return "Unknown error";

        std::string_view text = array_index_by_enum<std::string_view>(_int_error_messages, this->status, "Unknown error"); //_int_error_messages[idx];
        // the following is safe because our text is not user input, but one of the hard-coded messages defined by our program
        return cpasm::format(text, this->c, this->position, (int)this->base);
    }

    IntParseError::operator bool() {
        return this->status == IntParseStatus::NONE;
    }

    static constexpr u8 UPPER_LOWER_DIFF = 'a' - 'A';

    static constexpr char _to_lower(char c) {
        if ((c >= 'A') && (c <= 'Z'))
            return c + UPPER_LOWER_DIFF;
        return c;
    }

    static constexpr IntParseError _get_digit(char c, int base, u8* out) {
        if (c < '0')
            return {
                .status = IntParseStatus::BAD_SYNTAX,
                .c = c,
            };

        u8 diff0 = c - '0';
        if (diff0 >= base)
            return {
                .status = IntParseStatus::BAD_DIGIT,
                .c = c,
                .base = (u8)base,
            };

        if (diff0 <= 9) {
            *out = diff0;
            return IntParseError::success();
        }

        c = _to_lower(c);

        if (c < 'a' || c > 'z')
            return {
                .status = IntParseStatus::BAD_SYNTAX,
                .c = c,
            };

        u8 diff1 = c - 'a' + 10;
        if (diff1 >= base)
            return {
                .status = IntParseStatus::BAD_DIGIT,
                .c = c,
                .base = (u8)base,
            };
        
        *out = diff1;
        return IntParseError::success();
    }

    static IntParseError _parse_prefix(Parser& parser, int* base, i8* sign) {
        *base = 10;
        *sign = 1;

        // potential '-' character in front of number
        auto sign_op = parser.get_exact_operator(Operator::ADDR_SUB);
        if (sign_op) {
            parser.advance_operator(sign_op);
            *sign = -1;
            parser.consume_spaces();
        }

        // next word must be a dot or numeric
        if (parser.get_word_type() != WordType::NUMERIC) {
            if (parser.get_exact_operator(Operator::DOT, true))
                return IntParseError::success();
            return {
                .status = IntParseStatus::BAD_PREFIX,
            };
        }

        if (parser.advance_word_prefix("0b"))
            *base = 2;
        else if (parser.advance_word_prefix("0o"))
            *base = 8;
        else if (parser.advance_word_prefix("0d"))
            *base = 10;
        else if (parser.advance_word_prefix("0x"))
            *base = 16;
        
        return IntParseError::success();
    }

    IntParseError parse_digits(Parser& parser, size_t* out, int base) {
        size_t res = 0;


        if (parser.get_word_type() != WordType::NUMERIC)
            return {
                .status = IntParseStatus::BAD_SYNTAX,
                .c = parser.get_word()[0],
                .position = 0,
                .base = (u8)base,
            };

        for (size_t i = 0; i < parser.get_word().size(); i++) {
            char c = parser.get_word()[i];
            u8 digit;
            auto err = _get_digit(c, base, &digit);

            if (!err) {
                err.position = i;
                return err;
            }

            res *= base;
            res += digit;
        }

        parser.advance_word(1);

        return IntParseError::success();
    }


    bool IntLiteralToken::parse(Parser& parser) {
        this->Token::parse(parser);

        auto st = parser.save();

        int base;
        i8 sign;

        IntParseError err;
        if (!(err = _parse_prefix(parser, &base, &sign))) {
            st.restore();
            return fail(this, sfmt("Failed to parse integer literal: ", err.format()));
        }

        size_t result;
        if (!(err = parse_digits(parser, &result, base))) {
            st.restore();
            return fail(
                this, 
                sfmt("Failed to parse integer literal: ", err.format()),
                err.status == IntParseStatus::OVERFLOW ? TokenErrorCode::INT_OVERFLOW : TokenErrorCode::UNSPECIFIED_ERROR
            );
        }

        this->value = result;
        this->sign = sign;
        return true;
    }

    bool FloatLiteralToken::parse(Parser& parser) {
        this->Token::parse(parser);
        
        auto st = parser.save();

        int base;
        i8 sign;

        IntParseError err;
        if (!(err = _parse_prefix(parser, &base, &sign))) {
            st.restore();
            return fail(this, sfmt("Failed to parse floating-point literal: ", err.format()));
        }

        size_t int_part = 0;
        size_t dec_part = 0;

        if (parser.get_word_type() == WordType::NUMERIC) {
            if (!(err = parse_digits(parser, &int_part, base))) {
                st.restore();
                return fail(this, sfmt("Failed to parse floating-point literal: ", err.format()));
            }
        }

        if (!parser.get_exact_operator(Operator::DOT, true)) {
            this->value = static_cast<double>(int_part) * sign;

            return true;
        }

        parser.advance_operator(Operator::DOT);

        size_t dec_dig_cnt = 0;
        if (parser.get_word_type() == WordType::NUMERIC) {
            dec_dig_cnt = parser.get_word().size();
            if (!(err = parse_digits(parser, &dec_part, base))) {
                st.restore();
                return fail(this, sfmt("Failed to parse floating-point literal: ", err.format()));
            }
        }

        double factor = std::pow(base, dec_dig_cnt);  // room for improvement: this can be made faster when base is a power of 2

        this->value = (double)int_part + ((double)dec_part / factor);
        this->value *= sign;
        
        return true;
    }

    bool NumberLiteralToken::parse(Parser& parser) {

        // idiom for when dealing with multiple possibilities that might all fail
        this->Token::parse(parser);

        TokenErrorGroup g;

        auto& itok = this->value.emplace<IntLiteralToken>(this);
        if (itok.parse(parser)) {
            return true;
        } else g.add(&itok);
        
        auto& ftok = this->value.emplace<FloatLiteralToken>(this);
        if (ftok.parse(parser)) {
            return true;
        } else g.add(&ftok);

        return g.fail(this, "Hello");
    }
}

