#pragma once
#include <string_view>
#include <bitset>
#include "../helpers.hpp"


namespace cpasm {
    class CPURegisterFlags {
        u8 _bits;

    public:
        constexpr CPURegisterFlags(bool supportsInt, bool supportsFloat, bool isUserLand) :
            _bits(supportsInt * 0b1 | supportsFloat * 0b10 | isUserLand * 0b100 )
        {}

        bool supportsInt() const;
        bool supportsFloat() const;
        bool isUserLand() const;
    };

    struct CPURegister;

    struct CPURegister {
        std::string_view name;
        const CPURegister* parent;
        const CPURegister* low;
        const CPURegister* high;
        CPURegisterFlags flags;
        u8 size;
    };

    struct CPUInstruction {
        std::string_view name;
    };
}

