#pragma once
#include <string_view>
#include "helpers.hpp"


namespace cpasm {
	class CPURegisterSupport {
		uint8_t _val;

	public:
		constexpr CPURegisterSupport() : _val(0) {}

		constexpr CPURegisterSupport(bool supportsIntVar, bool supportsFloatVar, bool holdsStackBase, bool holdsStackTop, bool isUserland) {
			uint8_t _supInt = (uint8_t)supportsIntVar;
			uint8_t _supFloat = (uint8_t)supportsFloatVar;
			uint8_t _holdSB = (uint8_t)holdsStackBase;
			uint8_t _holdST = (uint8_t)holdsStackTop;
			uint8_t _isUsr = (uint8_t)isUserland;

			this->_val = _supInt |
				(_supFloat << 1) |
				(_holdSB << 2) |
				(_holdST << 3) |
				(_isUsr << 4);
		}

		/*
		Whether the register is suitable for storing integer variables of a user program.
		*/
		constexpr bool supportsIntVar() const {
			return this->_val & 0b00001;
		}
		/*
		Whether the register is suitable for storing floating-point variables of a user program.
		*/
		constexpr bool supportsFloatVar() const {
			return this->_val & 0b00010;
		}
		/*
		Whether the register holds the pointer to the base of the stack.
		*/
		constexpr bool holdsStackBase() const {
			return this->_val & 0b00100;
		}
		/*
		Whether the register holds the pointer to the top of the stack.
		*/
		constexpr bool holdsStackTop() const {
			return this->_val & 0b01000;
		}
		/*
		Whether non-kernel code is allowed to access the register.
		*/
		constexpr bool isUserLand() const {
			return this->_val & 0b10000;
		}
	};


	struct RegConventionFlags {
		static constexpr uint8_t MASK_CALLEE_SV = 0b01;
		static constexpr uint8_t MASK_CALLER_SV = 0b10;

		uint8_t data;

		inline constexpr RegConventionFlags(bool callee_saved, bool caller_saved) :
			data((MASK_CALLEE_SV* callee_saved) | (MASK_CALLER_SV * caller_saved))
		{
		}
		inline constexpr RegConventionFlags() : RegConventionFlags(0, 0) {}

		inline constexpr bool caller_saved() const {
			return data & MASK_CALLER_SV;
		}
		inline constexpr bool callee_saved() const {
			return data & MASK_CALLEE_SV;
		}

		static constexpr RegConventionFlags CALLEE_SAVED() {
			return { true, false };
		}
		static constexpr RegConventionFlags CALLER_SAVED() {
			return { false, true };
		}
	};

	struct CPURegister {
		std::string_view name;
		const CPURegister* parent = nullptr;
		const CPURegister* low = nullptr;
		const CPURegister* high = nullptr;
		CPURegisterSupport support;
		uint8_t size;
		RegConventionFlags conv_flags = {};

		inline const CPURegister* smallest_to_fit(uint8_t size) const {
			if (size > this->size)
				return nullptr;
			const CPURegister* res = nullptr;
			if (this->low) res = this->low->smallest_to_fit(size);
			if (res)
				return res;
			if (this->high) res = this->high->smallest_to_fit(size);
			if (res)
				return res;
			return this;
		}

		inline bool owns(const CPURegister* other) const {
			if (!other)
				return false;
			if (this->low == other)
				return true;
			if (this->high == other)
				return true;
			if (this->low && this->low->owns(other))
				return true;
			if (this->high && this->high->owns(other))
				return true;
			return false;
		}
		inline bool related(const CPURegister* other) const {
			if (!other)
				return false;
			return (this == other) || this->owns(other) || other->owns(this);
		}
		inline const CPURegister* toplevel() const {
			if (!this->parent)
				return this;
			return this->parent->toplevel();
		}
	};

	using CPUInstruction = uint16_t;


	struct TemporaryRules {
		array_view<const CPURegister*> tmp_registers;
		array_view<const CPURegister*> fp_tmp_registers;
	};

	struct CallingConvention {
		const CPURegister* return_reg;
		const CPURegister* fp_return_reg;
		array_view<const CPURegister*> arg_registers;
		array_view<const CPURegister*> fp_arg_registers;
		const TemporaryRules* temp_rules;
		uint8_t stk_args_align;
		array_view<RegConventionFlags> reg_convflags;
		uint8_t shadow_space;
		uint8_t stk_ptr_align;
		std::string_view name;
		uint8_t funcentry_stack_align;
	};

	struct OpFlags {
		enum _op_status : uint8_t {
			NONE = 0,
			LEFT = 0b01,
			RIGHT = 0b10,
			ALL = LEFT | RIGHT,
		};

		uint8_t size;  // size of the operation; uses the size of the smallest non-zero operand
		_op_status float_st;  // whether each of the operands is a floating-point register
		_op_status memory_st;  // whether each of the operands is a memory operand
		// if an operand is neither float nor memory, it is a general purpose register

		_op_status signed_st;  // whether each of the operands is signed or not (non-integers are considered unsigned)
	};
}

