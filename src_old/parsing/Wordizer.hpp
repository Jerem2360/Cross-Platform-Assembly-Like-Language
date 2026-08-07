#pragma once
#include <vector>
#include "CharGrouper.hpp"
#include "../Operator.hpp"


namespace cpasm {
	class Wordizer {
		CharGrouper _initial_groups;
		std::vector<CharGroup> _distinguished_operators;
		std::vector<CharGroup> _commentsRemoved;

		bool _distinguish_operators(const CharGroup&);

	public:
		Wordizer(std::istream&);
		bool parse();
		const std::vector<CharGroup>& result() const;

		inline void print_initial(std::ostream& fs) const {
			this->_initial_groups.print(fs);
		}
		inline void print_with_operators(std::ostream& fs) const {
			for (auto& grp : this->_distinguished_operators) {
				fs << "data=\"" << grp.chars << "\" type=" << grp.type;
				if (grp.extra)
					fs << " extra=\"" << reinterpret_cast<const Operator*>(grp.extra)->name << "\"";
				fs << '\n';
			}
		}
		inline void print_without_comments(std::ostream& fs) const {
			for (auto& grp : this->_commentsRemoved) {
				fs << "data=\"" << grp.chars << "\" type=" << grp.type;
				if (grp.extra)
					fs << " extra=\"" << reinterpret_cast<const Operator*>(grp.extra)->name << "\"";
				fs << '\n';
			}
		}
	};
}

