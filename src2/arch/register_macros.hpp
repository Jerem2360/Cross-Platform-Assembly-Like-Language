#pragma once
#include "cpu.hpp"
#include "architecture.hpp"


namespace cpasm {
    constexpr std::nullptr_t _vREG_NONE = nullptr;

    #define REG_NONE REG_NONE


    namespace __helpers {
        inline consteval std::nullptr_t _resolve_reg(const std::nullptr_t* val) {
            return nullptr;
        }
        inline consteval const CPURegister* _resolve_reg(const CPURegister* reg) {
            return reg;
        }

        static consteval CPURegister _make_reg(std::string_view name, u8 size, CPURegisterFlags flags, const CPURegister* parent = nullptr, const CPURegister* low = nullptr, const CPURegister* high = nullptr) {
            return CPURegister{
                .name = name,
                .parent = parent,
                .low = low,
                .high = high,
                .flags = flags,
                .size = size,
            };
        }
        static RegisterRef _new_reg_ref() {
            static u16 idx = 0;
            return RegisterRef(idx++);
        }
    }

    
    #define _DECL_REG(NAME) static ::cpasm::CPURegister _v ## NAME; extern const ::cpasm::RegisterRef NAME = ::cpasm::__helpers::_new_reg_ref()
    
    #define _DEF_REG(NAME, SIZE, FLAGS, PARENT, LOW, HIGH) \
        static ::cpasm::CPURegister _v ## NAME = ::cpasm::__helpers::_make_reg( \
            #NAME, \
            SIZE, \
            FLAGS, \
            ::cpasm::__helpers::_resolve_reg(&_v ## PARENT), \
            ::cpasm::__helpers::_resolve_reg(&_v ## LOW), \
            ::cpasm::__helpers::_resolve_reg(&_v ## HIGH) \
        )

    #define _REG(NAME) &_v ## NAME
}

