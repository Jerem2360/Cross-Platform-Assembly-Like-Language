/**
 * This file contains an interface to simplify adding support for execution environments.
 * In this case, execution environment refers to everything assembly user code runs on top of,
 * including for embedded environments, except for:
 * - Hardware, both physical and virtual
 * - Build system
 * 
 * the compiler option for selecting an execution environment is '-env <name>'.
 */


#pragma once
#include <string_view>
#include <map>
#include "../arch/cpu.hpp"
#include "../helpers.hpp"


namespace cpasm {
    enum class RegSavingRules : u8 {
        NONE = 0,
        CALLER_SV,
        CALLEE_SV,
    };

    enum class ArgPassingPolicy : u8 {
        NONE = 0,
        MAPPED,
        NEXT_AVAILABLE,
    };

    struct CallingConvention {
        std::string_view name;
        bool (*available)(Unknown ctx);  // return true if this callconv is available in the provided context
        array_view<const CPURegister*> tmp_regs;  // three or more registers in which internal temporaries can be stored by the compiler
        array_view<const CPURegister*> return_regs;  // one or more registers in which return values are stored when a function returns
        std::map<const CPURegister*, RegSavingRules> reg_saving;  // maybe change this to std::unordered_map ?
        const CPURegister* argcount_reg;  // register that needs to hold the number of registers used for arguments upon call; ignored if nullptr
        array_view<const CPURegister*> argument_regs;  // list of registers through which arguments can be passed to functions, extra arguments are passed on the stack
        ArgPassingPolicy arg_passing;  // policy determining how register arguments are passed 
        u8 stack_args_align;  // memory alignment of each stack argument
        u8 shadow_space;  // amount of shadow space; ignored if 0
        u8 funcentry_stack_align;  // alignment of the stack as soon as an assembly function is jumped to
        u8 stack_precall_align;  // alignment of the stack needed before calling a function
    };

    struct ExecutionEnvironment {
        std::string_view name;
        array_view<const CallingConvention*> callconvs;
        std::string_view (*default_callconv) (Unknown ctx);
    };
}

