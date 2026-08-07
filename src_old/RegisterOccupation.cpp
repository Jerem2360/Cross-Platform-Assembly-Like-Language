#include "RegisterOccupation.hpp"


namespace cpasm {
	static const CPURegister* _get_toplevel(const CPURegister* reg) {
		if (!reg)
			return nullptr;

		while (reg->parent)
			reg = reg->parent;

		return reg;
	}

	RegisterOccupation::_node::_node(const CPURegister* base) :
		used(NODE_EMPTY)
	{
		this->low = base->low ? std::make_unique<_node>(base->low) : nullptr;
		this->high = base->high ? std::make_unique<_node>(base->high) : nullptr;
	}

	RegisterOccupation::_node::usage_t RegisterOccupation::_node::_get_subusage() {
		if (low && high) {
			if ((this->low->used == NODE_FULL) && (this->high->used == NODE_FULL))
				return NODE_FULL;
			if ((this->low->used == NODE_EMPTY) && (this->high->used == NODE_EMPTY))
				return NODE_EMPTY;
			return NODE_PARTIAL_USE;
		}
		if (low)
			return low->used;
		if (high)
			return high->used;
		return NODE_EMPTY;
	}

	void RegisterOccupation::_node::_update_usage() {
		auto subusage = this->_get_subusage();
		if (this->used < subusage)
			this->used = subusage;
	}

	const CPURegister* RegisterOccupation::_node::alloc(uint8_t req_sz, const CPURegister* base) {
		if (this->used == NODE_FULL)
			return nullptr;

		if (req_sz > base->size)
			return nullptr;

		// try to allocate children
		const CPURegister* res = nullptr;

		if (this->low) res = this->low->alloc(req_sz, base->low);
		if (this->high && !res) res = this->high->alloc(req_sz, base->high);

		if (res) {
			if (this->low && this->high)
				this->used = static_cast<usage_t>(get_underlying(this->used) + 1);
			else
				this->used = NODE_FULL;
			return res;
		}

		// if that fails, try to allocate ourself

		if (this->used != NODE_EMPTY)
			return nullptr;
		
		this->used = NODE_FULL;
		return base;

		/*if (req_sz == base->size) {
			if (this->used)
				return nullptr;
			this->used = NODE_FULL;
			return base;
		}

		if (req_sz < base->size) {
			if (this->used == NODE_FULL)
				return nullptr;

			const CPURegister* res = nullptr;
			if (this->low) {
				res = this->low->alloc(req_sz, base->low);
			}
			if (!res && this->high) {
				res = this->high->alloc(req_sz, base->high);
			}
			if (!res) {
				if (this->used == NODE_PARTIAL_USE)
					return nullptr;
				this->used = NODE_FULL;
				return base;
			}
				//return nullptr;

			this->_update_usage();

			return res;
		}

		return nullptr;*/
	}

	RegisterOccupation::_node::free_res_t RegisterOccupation::_node::free(const CPURegister* to_free, const CPURegister* base) {
		if (this->used == NODE_EMPTY)
			return FREE_NONE;

		// try to free children
		free_res_t res = FREE_NONE;

		if (this->low) res = this->low->free(to_free, base->low);
		if (this->high && !res) res = this->high->free(to_free, base->high);

		// if that fully frees a child, our usage must be updated
		if (res == FREE_ALL) {
			if (this->low && this->high)
				this->used = static_cast<usage_t>(get_underlying(this->used) - 1);
			else
				this->used = NODE_EMPTY;

			return this->used ? FREE_PARTIAL : FREE_ALL;
		}
		// if it partially frees a child, then our usage remains the same
		if (res == FREE_PARTIAL)
			return FREE_PARTIAL;

		// if that doesn't free anything, try to free ourself
		if (this->used != NODE_FULL)
			return FREE_NONE;

		if (to_free != base)
			return FREE_NONE;

		this->used = NODE_EMPTY;
		return FREE_ALL;

		/*if (to_free == base) {
			if (this->used != NODE_FULL)
				return false;
			this->used = NODE_EMPTY;
			return true;
		}

		if (to_free->size < base->size) {
			if (!this->used)
				return false;

			bool res = false;
			if (this->low)
				res = this->low->free(to_free, base->low);
			if (!res && this->high)
				res = this->high->free(to_free, base->high);

			if (!res)
				return false;
			this->_update_usage();
			return true;
		}
		return false;*/
	}

	const CPURegister* RegisterOccupation::_node::reserve(const CPURegister* reg, const CPURegister* base) {
		if (this->used == NODE_FULL)
			return nullptr;

		if (reg->size > base->size)
			return nullptr;

		const CPURegister* res = nullptr;
		if (this->low) res = this->low->reserve(reg, base->low);
		if (this->high && !res) res = this->high->reserve(reg, base->high);

		if (res) {
			if (this->low && this->high)
				this->used = static_cast<usage_t>(get_underlying(this->used) + 1);
			else
				this->used = NODE_FULL;
			return res;
		}

		if (this->used != NODE_EMPTY)
			return nullptr;
		if (reg != base)
			return nullptr;

		this->used = NODE_FULL;
		return base;

		/*if (this->used == NODE_FULL)
			return nullptr;
		if (reg != base) {
			if (this->low && this->low->reserve(reg, base->low)) {
				this->_update_usage();
				return reg;
			}
			if (this->high && this->high->reserve(reg, base->high)) {
				this->_update_usage();
				return reg;
			}
			return nullptr;
		}
		if (this->used == NODE_PARTIAL_USE)
			return nullptr;
		this->used = NODE_FULL;
		return reg;*/
	}

	void RegisterOccupation::_node::clear() {
		this->used = NODE_EMPTY;
		if (this->low)
			this->low->clear();
		if (this->high)
			this->high->clear();
	}

	RegisterOccupation::RegisterOccupation(array_view<const CPURegister*> sources) :
		_toplevel_registers()
	{
		this->_toplevel_registers.reserve(sources.size());
		this->_occupation.reserve(sources.size());

		for (auto reg : sources) {
			this->_toplevel_registers.push_back(reg);
			this->_occupation.emplace_back(reg);
		}
	}

	RegisterOccupation::RegisterOccupation() : RegisterOccupation(array_view<const CPURegister*>{})
	{ }

	const CPURegister* RegisterOccupation::alloc(uint8_t req_sz) {
		for (int i = 0; i < this->_toplevel_registers.size(); i++) {
			auto reg = this->_occupation[i].alloc(req_sz, this->_toplevel_registers[i]);
			if (reg)
				return reg;
		}
		return nullptr;
	}

	bool RegisterOccupation::free(const CPURegister* reg) {
		const CPURegister* toplevel = _get_toplevel(reg);

		for (int i = 0; i < this->_toplevel_registers.size(); i++) {
			if (toplevel != this->_toplevel_registers[i])
				continue;
			if (this->_occupation[i].free(reg, this->_toplevel_registers[i]))
				return true;
		}
		return false;
	}

	const CPURegister* RegisterOccupation::reserve(const CPURegister* reg) {
		for (int i = 0; i < this->_toplevel_registers.size(); i++) {
			if (this->_occupation[i].reserve(reg, this->_toplevel_registers[i]))
				return reg;
		}
		return nullptr;
	}
	void RegisterOccupation::get_occupied(std::vector<const CPURegister*>* out) const {

		for (int i = 0; i < this->_toplevel_registers.size(); i++) {
			if (this->_occupation[i].used != _node::NODE_EMPTY)
				out->push_back(this->_toplevel_registers[i]);
		}
	}
	void RegisterOccupation::release_all() {
		for (auto& node : this->_occupation) {
			node.clear();
		}
	}
}

