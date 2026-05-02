#pragma once
#include <ostream>
#include <string_view>
#include <string>
#include <vector>

#include "Operand.hpp"
#include "Operator.hpp"
#include "CPU.hpp"
#include "assembler/Assembler.hpp"
#include "architecture/Architecture.hpp"
#include "environment/Environment.hpp"
#include "RegisterOccupation.hpp"
#include "helpers.hpp"


namespace cpasm {

	class AssemblyWriter;


	class TmpRegFlags {
		uint8_t _value;


		/*
		0 => backed register is general purpose
		1 => backed register is floating point
		*/
		static constexpr uint8_t MASK_BACKREGTYPE = 1;
		/*
		0 => back constants into a register
		1 => don't back constants into a register
		*/
		static constexpr uint8_t MASK_SKIPCONST = 1 << 1;
		/*
		0 => back memory operands by a register
		1 => don't back memory operands by a register
		*/
		static constexpr uint8_t MASK_SKIPMEM = 1 << 2;

		/*
		0 => back register operands of a different type than the backing register
		1 => don't back register operands of a different type than the backing register
		*/
		static constexpr uint8_t MASK_SKIPDIFFREGS = 1 << 3;

		inline constexpr TmpRegFlags(uint8_t _val) :
			_value(_val)
		{ }

	public:
		static inline constexpr TmpRegFlags GP_backed(bool back_constants = true, bool back_memory = true, bool back_diffregs = true) {
			uint8_t flags = (MASK_SKIPCONST * !back_constants) |
				(MASK_SKIPMEM * !back_memory) |
				(MASK_SKIPDIFFREGS * !back_diffregs);

			return TmpRegFlags{
				flags
			};
		}
		static inline constexpr TmpRegFlags FP_backed(bool back_constants = true, bool back_memory = true, bool back_diffregs = true) {
			uint8_t flags = MASK_BACKREGTYPE |
				(MASK_SKIPCONST * !back_constants) |
				(MASK_SKIPMEM * !back_memory) |
				(MASK_SKIPDIFFREGS * !back_diffregs);
			
			return TmpRegFlags{
				flags
			};
		}
		inline constexpr bool is_FP_backed() const {
			return _value & MASK_BACKREGTYPE;
		}
		inline constexpr bool backs_consts() const {
			return !(_value & MASK_SKIPCONST);
		}
		inline constexpr bool backs_memory() const {
			return !(_value & MASK_SKIPMEM);
		}
		inline constexpr bool backs_diffreg() const {
			return !(_value & MASK_SKIPDIFFREGS);
		}
	};


	class TmpRegWrapper {
		const CPURegister* _reg;
		const CPURegister* _back;
		const Operand* _src;
		RegisterOccupation* _occupation;
		AssemblyWriter* _aw;
		enum : uint8_t {
			ST_EXISTING_OP,
			ST_EXISTING_REG,
			ST_OWNED_TMP_REG,
		} _status;
		bool _pull;
		void (*_destroy_cb) () = nullptr;

	public:
		explicit TmpRegWrapper(
			AssemblyWriter& writer,
			RegisterOccupation& occ, 
			const Operand& src, 
			const CPURegister* backing,  // may be null
			bool copy, 
			bool push, 
			bool pull,
			uint8_t size
		);
		TmpRegWrapper();
		TmpRegWrapper(const TmpRegWrapper&) = delete;
		TmpRegWrapper(TmpRegWrapper&&) noexcept;
		TmpRegWrapper& operator =(const TmpRegWrapper&) = delete;
		TmpRegWrapper& operator =(TmpRegWrapper&&) noexcept;
		Operand get() const;
		const CPURegister* get_tmp() const;
		TmpRegWrapper& with_callback(decltype(_destroy_cb) callback);
		~TmpRegWrapper();
	};


	class AssemblyWriter {
		std::ostream& _output;
		const AssemblerFuncs* _asm_funcs;
		const ArchitectureFuncs* _arch_funcs;
		const EnvironmentFuncs* _env_funcs;
		RegisterOccupation _tmp_regs;
		RegisterOccupation _fp_tmp_regs;
		const CallingConvention* _callconv;
		unsigned int _stack_offset;
		std::vector<const CPURegister*> _tmp_usage;

	public:
		AssemblyWriter(std::ostream& out);
		void enter_code_block(const CallingConvention* callconv);
		TmpRegWrapper wrap_tmp(const Operand& src, bool push, bool pull, TmpRegFlags accept, uint8_t size = 0);
		TmpRegWrapper wrap_given_reg(const Operand& src, const CPURegister* back, bool push, bool pull);
		void leave_code_block();
		const CallingConvention* current_callconv() const;
		unsigned int stack_offset() const;
		void stack_offset(unsigned int to_add);
		void get_temp_usage(std::vector<const CPURegister*>* out_regs) const;

		// assembler functionality:
		bool cpu_instruction(CPUInstruction instr, std::vector<Operand> operands, std::string_view comment = "") const;
		bool custom_instruction(std::string_view name, std::vector<Operand> operands, std::string_view comment = "") const;
		bool define_bytes(uint8_t size, const SimpleOperand& value) const;
		bool reserve_bytes(uint8_t size) const;
		bool set_bitness(uint8_t bits) const;
		bool def_global_label(std::string_view name) const;
		bool def_local_label(std::string_view name) const;
		bool use_global_label(std::string_view name) const;
		bool use_local_label(std::string_view name) const;
		bool include_file(std::string_view filename) const;
		bool import_symbol(std::string_view name) const;
		bool export_symbol(std::string_view name) const;
		bool const_int(size_t value) const;
		bool const_float(double value) const;
		bool const_char(char value) const;
		bool const_str(std::string_view value) const;
		bool deref(const SimpleOperand& base, const SimpleOperand& index, const SimpleOperand& scale, uint8_t size) const;
		bool register_(const CPURegister* reg) const;
		bool comment(std::string_view text) const;
		std::string random_identifier(std::string_view base = "") const;
		bool section(std::string_view name) const;
		bool enable_relative_addr() const;

		// cpu functionality
		bool push(const Operand& operand);
		bool pop(const Operand& operand);
		bool push_amount(size_t cnt, bool nested = false);
		bool pop_amount(size_t cnt, bool nested = false);
		bool jump(const Operand& target);
		bool jump_if(const Operand& target, const Operator* op, const Operand& lhs, const Operand& rhs);
		bool call(const Operand& target);
		bool return_();
		bool add(const Operand& target, const Operand& op);
		bool sub(const Operand& target, const Operand& op);
		bool mul(const Operand& target, const Operand& op);
		bool div(const Operand& target, const Operand& op);
		bool move(const Operand& target, const Operand& src);
		bool inc(const Operand& target);
		bool dec(const Operand& target);
		bool bxor(const Operand& target, const Operand& op);
		bool band(const Operand& target, const Operand& op);
		bool bor(const Operand& target, const Operand& op);
		bool mod(const Operand& target, const Operand& op);

		// environment-specific instructions
		bool exit(const Code*, const Operand& code);
	};

	class AssemblyGenerator {
		struct InstructionItem {
			CPUInstruction instr;
			std::vector<Operand> operands;
			std::string_view comment;
		};
		struct WrapTmp {
			const Operand& src; 
			bool push; 
			bool pull; 
			TmpRegFlags accept; 
			uint8_t size = 0;
		};

		enum _ElemType : uint8_t {
			ELEM_INSTR,
			ELEM_LBL,
		};

		std::stringstream _ss;
		AssemblyWriter _writer;
		std::vector<InstructionItem> _instr_stack;
		std::vector<std::string> _labels;
		std::vector<Selector<_ElemType>> _order;

	public:
		AssemblyGenerator();

		bool cpu_instruction(CPUInstruction instr, std::vector<Operand> operands, std::string_view comment = "");
		TmpRegWrapper wrap_tmp(const Operand& src, bool push, bool pull, TmpRegFlags accept, uint8_t size = 0);
		TmpRegWrapper wrap_given_reg(const Operand& src, const CPURegister* back, bool push, bool pull);
		bool def_local_label(std::string_view name);

		void push();
	};
}

