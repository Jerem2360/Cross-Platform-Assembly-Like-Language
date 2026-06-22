#pragma once
#include <string_view>

#include "../helpers.hpp"
#include "cpu.hpp"


namespace cpasm {

    // helper class that forwards to a register from the currently used instruction set.
    class RegisterRef {
        u16 _idx;

    public:
        explicit RegisterRef(u16 id);

        const CPURegister* operator->() const;
        // return false if the register is unavailable in the current instruction set
        bool available() const;
    };


    struct CPURegisterDef {
        const CPURegister* reg;
        bool keep_parent;
        bool keep_low;
        bool keep_high;
    };

    struct InstructionSet {
        array_view<CPURegisterDef> toplevel_regs;
        array_view<const CPUInstruction*> instructions;
    };

    struct Architecture {
        array_view<const InstructionSet*> instruction_sets;
    };
}

