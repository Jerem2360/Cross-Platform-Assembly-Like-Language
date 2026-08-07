#include "FuncCall.hpp"

#include <limits>
#include "Program.hpp"


namespace cpasm {
    static constexpr size_t OFFSET_NONE = std::numeric_limits<size_t>::max();

    FunctionCaller::FunctionCaller(AssemblyWriter& writer, const CallingConvention* callconv, const Code* owner) :
        _writer(&writer), 
        _callconv(callconv), 
        _owner(owner), 
        _args(), 
        _ret_loc(), 
        _ret_loc_resolved(), 
        _never_returns(false), 
        _pushed_locals(), 
        _args_offset(0),
        _int_argreg_offsets(),
        _fp_argreg_offsets(),
        _int_used_argregs(0),
        _fp_used_argregs(0),
        _stk_args_sz(0),
        _stk_align_amount(0)
    {
        if (callconv) {
            vec_push_times(this->_int_argreg_offsets, OFFSET_NONE, callconv->arg_registers.size());
            vec_push_times(this->_fp_argreg_offsets, OFFSET_NONE, callconv->fp_arg_registers.size());
        }
    }
    void FunctionCaller::_comment(const std::string& text) const {
        this->_writer->comment(sview(text));
    }
    /*
    If the provided register is an argument register by the current callconv, maps the current stack
    offset to that register for later access.
    */
    void FunctionCaller::_match_argreg(const CPURegister* reg) {
        if (!this->_callconv)
            return;
        if (!reg)
            return;

        reg = reg->toplevel();
        
        this->_comment(sfmt("    Searching for ", reg->name, " in callconv args"));
        auto it = std::find(this->_callconv->arg_registers.begin(), this->_callconv->arg_registers.end(), reg);
        if (it != this->_callconv->arg_registers.end()) {
            ssize_t idx = it - this->_callconv->arg_registers.begin();
            this->_comment(sfmt("    Found int reg with index ", idx, ". Stack offset is ", this->_writer->stack_offset()));
            this->_int_argreg_offsets[idx] = this->_writer->stack_offset();
            return;
        }
        this->_comment("    Did not find int reg.");
        it = std::find(this->_callconv->fp_arg_registers.begin(), this->_callconv->fp_arg_registers.end(), reg);
        if (it != this->_callconv->fp_arg_registers.end()) {
            ssize_t idx = it - this->_callconv->fp_arg_registers.begin();
            this->_comment(sfmt("    Found float reg with index ", idx, ". Stack offset is ", this->_writer->stack_offset()));
            this->_fp_argreg_offsets[idx] = this->_writer->stack_offset();
            return;
        }
        this->_comment("    Did not find float reg.");
    }
    const CPURegister* FunctionCaller::_next_arg_reg(DataType type, size_t* fp_arg_counter) {
        const CPURegister* toplevel;
        if (type.type == DataType::FLOAT) {

            if (this->_fp_used_argregs >= this->_callconv->fp_arg_registers.size())
                return nullptr;
            toplevel = this->_callconv->fp_arg_registers[this->_fp_used_argregs];

            this->_fp_used_argregs++;
            if (this->_callconv->args_method == CallArgsMethod::MAPPED)
                this->_int_used_argregs++;

            if (fp_arg_counter)
                (*fp_arg_counter)++;

        } else {

            if (this->_int_used_argregs >= this->_callconv->arg_registers.size())
                return nullptr;
            toplevel = this->_callconv->arg_registers[this->_int_used_argregs];
            
            this->_int_used_argregs++;
            if (this->_callconv->args_method == CallArgsMethod::MAPPED)
                this->_fp_used_argregs++;
        }

        if (type.size)
            return toplevel->smallest_to_fit(type.size);
        return toplevel;
    }
    
    size_t FunctionCaller::_argreg_already_assigned(const Operand& arg, std::vector<const CPURegister*>& assigned) {
        const CPURegister* reg = nullptr;

        this->_comment("    Checking if the argument was already assigned by a previous argument push");
        for (auto& argreg : assigned) {
            if (arg.is_related_register(argreg)) {
                reg = argreg;
                break;
            }
        }
        if (!reg)
            return OFFSET_NONE;

        this->_comment(sfmt("    Found previous assignment to: ", reg->name));

        auto it = std::find(this->_callconv->arg_registers.begin(), this->_callconv->arg_registers.end(), reg);
        if (it != this->_callconv->arg_registers.end()) {
            size_t idx = it - this->_callconv->arg_registers.begin();
            this->_comment(sfmt("    Found offset=", this->_int_argreg_offsets[idx], ". Current offset is ", this->_writer->stack_offset()));
            return this->_int_argreg_offsets[idx];
        }
        this->_comment("    Did not find it in callconv int arg regsisters.");
        it = std::find(this->_callconv->fp_arg_registers.begin(), this->_callconv->fp_arg_registers.end(), reg);
        if (it != this->_callconv->fp_arg_registers.end()) {
            size_t idx = it - this->_callconv->fp_arg_registers.begin();
            this->_comment(sfmt("    Found offset=", this->_fp_argreg_offsets[idx], ". Current offset is ", this->_writer->stack_offset()));
            return this->_fp_argreg_offsets[idx];
        }
        this->_comment("    Did not find it in callconv float arg regsisters.");
        return OFFSET_NONE;
    }
    void FunctionCaller::push_locals() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;

        this->_comment("  {Pushing locals to the stack}");
        for (auto& [name, box] : this->_owner->boxes) {
            if (!CurrentImpl.register_callconv(box.reg, this->_callconv).caller_saved()) {
                this->_comment(sfmt("  - ", name, ": ignored because register ", box.reg->name, " is callee saved."));
                continue;
            }
            
            if (this->_ret_loc_resolved.is_related_register(box.reg)) {
                this->_comment(sfmt("  - ", name, ": ignored because register ", box.reg->name, " is the return location."));
                continue;
            }
            
            this->_comment(sfmt("  - ", name, " (", box.reg->name, ") needs to be pushed to the stack."));
            auto& op = this->_pushed_locals.emplace_back(Operand::from_register(box.reg));
			this->_writer->push(op);
            this->_match_argreg(box.reg);
        }
    }
    void FunctionCaller::pass_stack_args() {
        if (!this->_callconv)
            return;

        this->_comment("  {Pushing remaining args to the stack}");
        size_t prev_stack_offset = this->_writer->stack_offset();
        for (size_t i = this->_args.size(); i-- > this->_args_offset;) {  // arguments need to be pushed in reverse order

            size_t padding = remaining_for_align(this->_writer->stack_offset(), this->_callconv->stk_args_align);
            this->_comment(sfmt("  - arg ", i, " needs ", padding, " bytes of padding and has size ", (int)this->_args[i].type().size, '.'));
            this->_writer->push_amount(padding);
            this->_writer->push(this->_args[i]);
        }
        // we do this because a push operation is allowed to push more than what it is told to when pushing an operand.
        this->_stk_args_sz += (this->_writer->stack_offset() - prev_stack_offset);
    }
    void FunctionCaller::pass_reg_args() {
        if (!this->_callconv)
            return;

        this->_comment("  {Writing register arguments}");
        std::vector<const CPURegister*> assigned_argregs = {};

        size_t i = 0;

        size_t n_fp_args = 0;

        for (i; i < this->_args.size(); i++) {
            Operand arg = this->_args[this->_args_offset + i];

            auto arg_reg = this->_next_arg_reg(arg.type(), &n_fp_args);
            // if we run out of arg registers, stop here
            if (!arg_reg)
                break;

            // if the argument source was already assigned when passing a previous argument, use the stack copy of it instead
            // this works assuming that all arg registers are considered caller-saved.
            size_t offset = this->_argreg_already_assigned(arg.resolve(), assigned_argregs);
            if (offset != OFFSET_NONE) {
                offset -= this->_writer->stack_offset();  // obtain the offset relative to the current stack pointer

                this->_comment(sfmt("  - arg ", i, " (", arg_reg->name, ") will use stack offset ", offset, " because source register was overridden by previous arg."));
                arg = Operand(
                    SimpleOperand::from_register(CurrentImpl.stack_pointer(), nullptr, 0), 
                    arg.type(), 
                    SimpleOperand::from_const_int(offset, 0)
                );
            } else {
                this->_comment(sfmt("  - arg ", i, " (", arg_reg->name, ") passed normally."));
            }
            
            // if the argument is already in the right register, skip it
            if (arg.resolve().is_related_register(arg_reg)) {
                this->_comment("    argument is already in the right place, skipping it.");
                continue;
            }

            this->_writer->move(Operand::from_register(arg_reg),arg);
            assigned_argregs.push_back(arg_reg->toplevel());
        }
        if (this->_callconv->argcount_reg) {
            this->_comment(sfmt("  - writing into ", this->_callconv->argcount_reg->name, " the number of float registers used."));
            this->_writer->move(Operand::from_register(this->_callconv->argcount_reg), Operand::from_const_int(n_fp_args));
        }

        this->_args_offset += i;  // here i should be the number of args passed through registers
    }
    void FunctionCaller::align_stack() {
        if (!this->_callconv)
            return;
        this->_comment(sfmt("  {aligning the stack (", this->_writer->stack_offset(), " => ", (int)this->_callconv->stk_ptr_align, ") before call}"));
        size_t align_missing = remaining_for_align(this->_writer->stack_offset(), this->_callconv->stk_ptr_align);

        this->_writer->push_amount(align_missing);
        // we do this because a push operation is allowed to push more than what it is told to when pushing an operand.
        this->_stk_align_amount += align_missing;
    }
    void FunctionCaller::unalign_stack() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;
        this->_comment(sfmt("  {popping ", this->_stk_align_amount, " bytes of stack alignment padding}"));
        this->_writer->pop_amount(this->_stk_align_amount);
        this->_stk_align_amount = 0;
    }
    void FunctionCaller::push_shadow() {
        if (!this->_callconv)
            return;
        this->_comment(sfmt("  {pushing ", (int)this->_callconv->shadow_space, " bytes of shadow space}"));
        this->_writer->push_amount(this->_callconv->shadow_space);
    }
    void FunctionCaller::pop_shadow() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;
        this->_comment(sfmt("  {popping ", (int)this->_callconv->shadow_space, " bytes of shadow space}"));
        this->_writer->pop_amount(this->_callconv->shadow_space);
    }
    void FunctionCaller::pop_stk_args() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;
        this->_comment(sfmt("  {popping ", this->_stk_args_sz, " bytes worth of stack arguments}"));
        this->_writer->pop_amount(this->_stk_args_sz);
        this->_stk_args_sz = 0;
    }
    void FunctionCaller::pop_locals() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;
        
            this->_comment(sfmt("  {popping ", this->_pushed_locals.size(), " locals from the stack}"));
        for (size_t i = this->_pushed_locals.size(); i-- > 0;) {
            this->_writer->pop(this->_pushed_locals[i]);
        }
        this->_pushed_locals.clear();
    }
    void FunctionCaller::fetch_result() {
        if (!this->_callconv)
            return;
        if (this->_never_returns)
            return;
        if (this->_ret_loc.is_empty())
            return;

        const CPURegister* ret_reg;
        if (this->_ret_loc.type().type == DataType::Type::FLOAT)
            ret_reg = this->_callconv->fp_return_reg;
        else
            ret_reg = this->_callconv->return_reg;

        this->_comment(sfmt("  {return value needs to be fetched from the ", ret_reg->name, " register}"));

        if (this->_ret_loc_resolved.is_related_register(ret_reg)) {
            this->_comment("  - skipping because the result is already in the right register");
            return;  // result is already in the return location
        }

        this->_writer->move(this->_ret_loc_resolved, Operand::from_register(ret_reg));
    }

    void FunctionCaller::args(array_view<Operand> args) {
        this->_args = args;
    }
    void FunctionCaller::return_location(const Operand& location) {
        this->_ret_loc = location;
        this->_ret_loc_resolved = location.resolve();
    }
    void FunctionCaller::noreturn() {
        this->_never_returns = true;
    }
}

