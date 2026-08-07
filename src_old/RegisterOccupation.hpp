#pragma once

#include <vector>
#include <memory>
#include <type_traits>

#include "helpers.hpp"
#include "CPU.hpp"


namespace cpasm {

	/*
	Keeps track of which registers among the provided ones are occupied.
	Only toplevel registers must be provided.
	*/
	class RegisterOccupation {
		struct _node {
			enum usage_t : uint8_t { NODE_EMPTY = 0, NODE_PARTIAL_USE, NODE_FULL } used;
			std::unique_ptr<_node> low;
			std::unique_ptr<_node> high;

			enum free_res_t : uint8_t { FREE_NONE = 0, FREE_PARTIAL, FREE_ALL };

			_node(const CPURegister* base);
			const CPURegister* alloc(uint8_t req_sz, const CPURegister* base);
			free_res_t free(const CPURegister* to_free, const CPURegister* base);
			const CPURegister* reserve(const CPURegister* reg, const CPURegister* base);
			void clear();

		private:
			void _update_usage();
			usage_t _get_subusage();
		};

		std::vector<const CPURegister*> _toplevel_registers;
		std::vector<_node> _occupation;

	public:
		RegisterOccupation(array_view<const CPURegister*> sources);
		RegisterOccupation(RegisterOccupation&&) noexcept = default;
		RegisterOccupation(const RegisterOccupation&) = delete;
		RegisterOccupation();

		RegisterOccupation& operator =(RegisterOccupation&&) noexcept = default;
		RegisterOccupation& operator =(const RegisterOccupation&) = delete;
		
		const CPURegister* alloc(uint8_t size);
		bool free(const CPURegister*);
		const CPURegister* reserve(const CPURegister* reg);
		void get_occupied(std::vector<const CPURegister*>*) const;
		void release_all();
	};
}

