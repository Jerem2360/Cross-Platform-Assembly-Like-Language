#include "rules.hpp"
#include <iostream>


namespace cpasm {
    // Registry is a special linked list type that doesn't need runtime initialization, and thus does not
    // depend on runtime initialization order.

    static inline Registry<char> _storage;
    RegistryView<char> SymbolChars = {&_storage};

    static std::string_view _wordtype_names[] = {
        "NONE",
        "NAME",
        "NUMERIC",
        "SYMBOLS",
        "SPACES",
        "UNKNOWN",
    };

    std::string_view WordType_name(WordType value) {
        auto idx = get_underlying(value);
        if ((idx >= array_len(_wordtype_names)) || (idx < 0))
            return "??";

        return _wordtype_names[idx];
    }

    void ParsingRules::register_symbol_char(char c) {
        //std::cout << "Register |" << c << "|\n";
        if (!coll_contains(_storage, c))
            _storage.push(c);

        //std::cout << "New contents: (";
        //for (char c : _storage) {
        //    std::cout << " '" << c << "',";
        //}
        //std::cout << ")\n";        
    }

    array_view<ParsingRule> ParsingRules::rules() {
        return _rule_map;
    }
}

