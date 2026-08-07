#pragma once
#include <vector>

#include "CPU.hpp"
#include "AsmWriter.hpp"
#include "Operand.hpp"


namespace cpasm {
    class FunctionCaller {
        AssemblyWriter* _writer;
        const CallingConvention* _callconv;
        array_view<Operand> _args;
        Operand _ret_loc;
        Operand _ret_loc_resolved;
        const Code* _owner;
        bool _never_returns;
        std::vector<Operand> _pushed_locals;
        size_t _args_offset;
        std::vector<size_t> _int_argreg_offsets;
        std::vector<size_t> _fp_argreg_offsets;
        size_t _int_used_argregs;
        size_t _fp_used_argregs;
        size_t _stk_args_sz;
        size_t _stk_align_amount;

        void _comment(const std::string& text) const;
        void _match_argreg(const CPURegister* reg);
        const CPURegister* _next_arg_reg(DataType type, size_t* fp_arg_counter);
        size_t _argreg_already_assigned(const Operand& arg, std::vector<const CPURegister*>& assigned);

    public:
        /*
        Owner should not be null, but calling convention can.
        */
        FunctionCaller(AssemblyWriter& writer, const CallingConvention* callconv, const Code* owner);
        void push_locals();
        void pass_stack_args();
        void pass_reg_args();
        void align_stack();
        void unalign_stack();
        void push_shadow();
        void pop_shadow();
        void pop_stk_args();
        void pop_locals();
        void fetch_result();

        void args(array_view<Operand> args);
        void return_location(const Operand& location);
        void noreturn();
    };
}

