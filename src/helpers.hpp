#pragma once
#include <span>
#include <type_traits>
#include <string>
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <cstdint>


#ifndef EOF
#define _CPASM_HELPERS_NOEOF
#include <cstdio>
#endif

namespace cpasm {
	namespace __helpers{
		static constexpr auto __end_of_file = EOF;
	}

#ifdef _CPASM_HELPERS_NOEOF
#undef _CPASM_HELPERS_NOEOF
#endif
#undef EOF


	static constexpr auto EOF = __helpers::__end_of_file;

	using ssize_t = std::make_signed_t<size_t>;


	template<class T, size_t Len>
	static inline constexpr size_t array_len(T(&)[Len]) {
		return Len;
	}

	/*
	Readonly view on a C-style array of type T. Supports iteration and indexing.
	*/
	template<class T, size_t Len = std::dynamic_extent>
	using array_view = std::span<const std::remove_const_t<T>, Len>;

	/*
	Cleanly take a string_view pointing to the provided string object.
	*/
	static inline constexpr std::string_view sview(const std::string& str) {
		return { str };
	}
	/*
	Clean explicit conversion of a string literal into a string view.
	Useful for ambiguous function signatures.
	*/
	template<size_t Len>
	static inline constexpr std::string_view sview(const char(&str)[Len]) {
		return { str };
	}

	/*
	Create a string view of a portion of the provided string.
	*/
	static inline constexpr std::string_view ssubview(std::string_view str, size_t offset, size_t length) {
		return { str.data() + offset, length };
	}

	/*
	Create a string view of a portion of the provided string.
	*/
	static inline constexpr std::string_view ssubview(const std::string& str, size_t offset, size_t length) {
		return ssubview(sview(str), offset, length);
	}

	/*
	Clean way of creating a std::string from a string view
	*/
	static inline constexpr std::string scopy(std::string_view str) {
		return { str.data(), str.size() };
	}

	static inline constexpr std::string_view vec2str(const std::vector<char>& vec) {
		return { vec.data(), vec.size() };
	}

	template<class T>
	static inline constexpr array_view<T> vec2aview(const std::vector<T>& vec) {
		return { vec.data(), vec.size() };
	}

	template<class T>
	static inline constexpr array_view<T> obj2aview(const T* pobj) {
		return { pobj, 1 };
	}

	template<class T>
	static inline void vec_extend(std::vector<T>& vec, const std::vector<T>& extra) {
		for (auto& val : extra) {
			vec.push_back(val);
		}
	}

	inline bool sstartwith(std::string_view target, std::string_view prefix) {
		if (target.size() < prefix.size())
			return false;
		return ssubview(target, 0, prefix.size()) == prefix;
	}
	inline bool sstartwith(const std::string& target, std::string_view prefix) {
		return sstartwith(sview(target), prefix);
	}

	inline std::string_view srmprefix(std::string_view target, std::string_view prefix) {
		if (!sstartwith(target, prefix))
			return target;
		return ssubview(target, prefix.size(), target.size() - prefix.size());
	}
	inline std::string_view srmprefix(const std::string& target, std::string_view prefix) {
		return srmprefix(sview(target), prefix);
	}

	inline bool sendwith(std::string_view target, std::string_view suffix) {
		if (target.size() < suffix.size())
			return false;
		return ssubview(target, target.size() - suffix.size(), suffix.size()) == suffix;
	}
	inline bool sendwith(const std::string& target, std::string_view suffix) {
		return sendwith(sview(target), suffix);
	}

	inline std::string_view srmsuffix(std::string_view target, std::string_view suffix) {
		if (!sendwith(target, suffix))
			return target;
		return ssubview(target, 0, target.size() - suffix.size());
	}
	inline std::string_view srmsuffix(const std::string& target, std::string_view suffix) {
		return srmsuffix(sview(target), suffix);
	}

	template<class T>
	static inline std::vector<T> aview2vec(array_view<T> view) {
		std::vector<T> res;
		res.reserve(view.size());

		for (auto& elem : view) {
			res.push_back(elem);
		}
		return res;
	}

	template<class T>
	static inline array_view<T> vec_slice(const std::vector<T>& vec, size_t start, size_t size) {
		if ((start + size) > vec.size())
			return {};
		return { vec.data() + start, size };
	}

	template<class T>
	inline constexpr T& vec_last(std::vector<T>& src) {
		return src[src.size() - 1];
	}
	template<class T>
	inline constexpr const T& vec_last(const std::vector<T>& src) {
		return src[src.size() - 1];
	}
	template<class T>
	inline constexpr const T& aview_last(array_view<T> src) {
		return src[src.size() - 1];
	}

	struct char_condition {

		bool (*const _check) (char c);


		consteval char_condition(bool (*check_fn)(char)) :
			_check(check_fn)
		{ }
		constexpr bool check(char c) const {
			return _check(c);
		}
		constexpr bool check(std::string_view s) const {
			for (char c : s) {
				if (!check(c))
					return false;
			}
			return true;
		}
	};

	template<char min, char max>
	constexpr char_condition char_range = { [](char c) { return (c >= min) && (c <= max); } };

	template<char ...targets>
	constexpr char_condition char_specific = { [](char c) { return ((c == targets) || ...); } };

	template<char_condition ...conds>
	constexpr char_condition char_intersect = { [](char c) { return (conds.check(c) && ...); } };

	template<char_condition ...conds>
	constexpr char_condition char_union = { [](char c) { return (conds.check(c) || ...); } };

	constexpr char_condition char_all = { [](char c) { return true; } };

	constexpr char_condition char_none = { [](char c) { return false; } };

	struct times {
		char c;
		int count;
	};

	inline std::ostream& operator <<(std::ostream& fs, times t) {
		for (int i = 0; i < t.count; i++) {
			fs << t.c;
		}
		return fs;
	}

	/*
	Struct used to keep track of the ordering of elements of different types.
	Mostly used inside a vector, where TSelect is an enum telling the code the type of each item
	and index gives an index into the associated collection.
	*/
	template<class TSelect>
	struct Selector {
		TSelect selector;
		int index;
		int lineno = 1;

		inline Selector(const TSelect& ty, int index, int lineno) :
			selector(ty), index(index), lineno(lineno)
		{ }
	};

	/*
	Convenient map indexing with default value.
	If the key is not found, returns _default.
	*/
	template<class TK, class TV>
	inline const TV& map_get(const std::map<TK, TV>& map, const TK& key, const TV& _default) {
		auto it = map.find(key);
		if (it == map.end())
			return _default;
		return it->second;
	}
	/*
	Convenient map indexing with default value.
	If the key is not found, returns _default.
	*/
	template<class TK, class TV>
	inline const TV& map_get(const std::unordered_map<TK, TV>& map, const TK& key, const TV& _default) {
		auto it = map.find(key);
		if (it == map.end())
			return _default;
		return *it;
	}

	template<class T>
		requires std::is_enum_v<T>
	inline constexpr std::underlying_type_t<T> get_underlying(T value) {
		return static_cast<std::underlying_type_t<T>>(value);
	}

	template<class ...T>
	inline std::string sfmt(T&& ...args) {
		std::stringstream fs;
		(fs << ... << std::forward<T>(args));
		return fs.str();
	}

	template<class T>
		requires std::constructible_from<bool, T>
	inline bool vec_all(const std::vector<T>& vec) {
		for (auto& val : vec) {
			if (!val)
				return false;
		}
		return true;
	}

	template<class T>
		requires std::constructible_from<bool, T>
	inline bool vec_any(const std::vector<T>& vec) {
		for (auto& val : vec) {
			if (val)
				return true;
		}
		return false;
	}

	template<class... Ts>
	struct overloaded : Ts... {
		using Ts::operator()...;
	};

	template<class... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;


	class ConditionElement {
		uint8_t _data;


		static constexpr uint8_t _CUR_COND_MASK = 1;
		static constexpr uint8_t _ALREADY_MET_MASK = 1 << 1;

	public:
		inline constexpr ConditionElement(bool val) :
			_data(val ? _CUR_COND_MASK | _ALREADY_MET_MASK : 0)
		{ }

		inline constexpr ConditionElement() : ConditionElement(false) {}

		inline void set(bool val) {
			if (val)
				this->_data |= (_CUR_COND_MASK | _ALREADY_MET_MASK);
			else
				this->_data &= ~_CUR_COND_MASK;
		}
		inline constexpr bool already_met() const {
			return this->_data & _ALREADY_MET_MASK;
		}
		inline constexpr bool get() const {
			return this->_data & _CUR_COND_MASK;
		}
		inline constexpr operator bool() const {
			return this->get();
		}
	};


	namespace __helpers {
		template<class ...TArgs>
		struct h_min {};
		template<class T>
		struct h_min<T> {
			static inline constexpr T func(T t) {
				return t;
			}
		};
		template<class TFirst, class ...TArgs>
		struct h_min<TFirst, TArgs...> {
			static inline constexpr TFirst func(TFirst&& first, TArgs&& ...rest) {
				return std::min(std::forward<TFirst>(first), h_min<TArgs...>::func(std::forward<TArgs>(rest)...));
			}
		};

		template<class ...TArgs>
		struct h_max {};
		template<class T>
		struct h_max<T> {
			static inline constexpr T func(T t) {
				return t;
			}
		};
		template<class TFirst, class ...TArgs>
		struct h_max<TFirst, TArgs...> {
			static inline constexpr TFirst func(TFirst&& first, TArgs&& ...rest) {
				return std::max(std::forward<TFirst>(first), h_max<TArgs...>::func(std::forward<TArgs>(rest)...));
			}
		};
	}

	template<class T>
	concept is_numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

	template<class ...TArgs>
		requires (is_numeric<TArgs> && ...)
	inline constexpr auto min(TArgs&& ...args) {
		return __helpers::h_min<TArgs...>::func(std::forward<TArgs>(args)...);
	}

	template<class ...TArgs>
		requires (is_numeric<TArgs> && ...)
	inline constexpr auto max(TArgs&& ...args) {
		return __helpers::h_max<TArgs...>::func(std::forward<TArgs>(args)...);
	}

	inline constexpr size_t remaining_for_align(size_t val, size_t align) {
		size_t extra = val % align;
		if (!extra)
			return 0;
		return align - extra;
	}


	namespace __helpers {
		template<class T>
		struct h_vartype {
			using type = T;
		};
		template<class TM, class TO>
		struct h_vartype<TM TO::*> {
			using type = TM;
		};
		template<class TR, class TO, class ...TA>
		struct h_vartype<TR(TO::*)(TA...)> {
			using type = TR(*)(TO*, TA...);
		};
	}
	template<class T>
	using vartype = typename __helpers::h_vartype<T>::type;
	template<auto X>
	using type_of = vartype<decltype(X)>;
}

