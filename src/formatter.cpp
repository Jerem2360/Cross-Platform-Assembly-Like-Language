#include "formatter.hpp"
#include <string_view>
#include "tokenizer/number_token.hpp"


namespace cpasm {
    static void _advance(std::string_view& text, size_t count) {
        text = sslice(text, count, SIZE_INVALID);
    }

    static constexpr u8 UPPER_LOWER_DIFF = 'a' - 'A';

    static constexpr char _to_lower(char c) {
        if ((c >= 'A') && (c <= 'Z'))
            return c + UPPER_LOWER_DIFF;
        return c;
    }

    static constexpr bool _get_digit(char c, u8* out) {
        if (c < '0' || c > '9')
            return false;

        *out = c - '0';
        return true;
    }

    static size_t _get_integer(std::string_view s, size_t* out) {
        size_t num = 0;

        size_t i = 0;
        for (i; i < s.size(); i++) {
            char c = s[i];

            u8 digit;

            if (!_get_digit(c, &digit)) {
                if (!i)
                    return 0;
                break;
            }

            // std::cout << "idx=" << i << "; digit=" << (int)digit << ".\n";

            num *= 10;
            num += digit;
        }
        
        *out = num;
        return i;
    }

    static constexpr size_t _NOT_PRESENT = SIZE_INVALID - 1;

    static size_t _parse_specifier(std::string_view& text) {
        if (!text.size())
            return _NOT_PRESENT;

        if (text[0] != '{') {
            return _NOT_PRESENT;
        }

        _advance(text, 1);
        if (!text.size())
            return SIZE_INVALID;

        size_t index;
        size_t n_digits = _get_integer(text, &index);
        // std::cout << "n_digits = " << n_digits << '\n';
        if (!n_digits) {
            return SIZE_INVALID;
        }

        _advance(text, n_digits);
        if (!text.size())
            return SIZE_INVALID;

        if (text[0] != '}')
            return SIZE_INVALID;

        _advance(text, 1);
        return index;
    }

    void _format_inner(
        std::string_view text, 
        std::function<void(char c)> on_char,
        std::function<void(size_t idx)> on_fmt
    ) {

        while (text.size()) {
            size_t idx = _parse_specifier(text);

            switch (idx) {
            case SIZE_INVALID:
                on_char('{');
                on_char('?');
                on_char('}');
                break;

            case _NOT_PRESENT:
                on_char(text[0]);
                _advance(text, 1);
                break;
            
            default:
                on_fmt(idx);
                break;
            }


        }
    }
}

