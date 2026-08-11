#pragma once
#include <cstdint> 
#include <span>
#include <utility>
#include <string_view>
#include <algorithm>
#include <limits>
#include <vector>
#include <variant>
#include <sstream>
#include <concepts>
#include <new>
#include <exception>

#ifdef __GNUC__
#include <cxxabi.h>
#endif


namespace cpasm {
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    static constexpr size_t SIZE_INVALID = std::numeric_limits<size_t>::max();

    template<class T>
    using array_view = std::span<const T>;

    template<class T, size_t Len>
    static inline consteval size_t array_len(T(&)[Len]) {
        return Len;
    }

    class Unknown {};

    using ssize_t = std::make_signed_t<size_t>;


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


    template<class T, class TElem>
    concept is_forward_iterable_of = requires (T t) {
        { t.begin() } -> std::forward_iterator;
        { t.end() } -> std::forward_iterator;
        { *(t.begin()) } -> std::convertible_to<TElem>;
        { *(t.end()) } -> std::convertible_to<TElem>;
    };

    template<class T>
    concept is_forward_iterable = requires (T t) {
        { t.begin() } -> std::forward_iterator;
        { t.end() } -> std::forward_iterator;
    };


    template<class T>
    concept has_size = requires (T t) {
        { t.size() } -> std::convertible_to<size_t>;
    };

    template<class T, class TElem>
    concept is_sized_collection_of = is_forward_iterable_of<T, TElem> && has_size<T>;

    template<class T>
    concept is_sized_collection = is_forward_iterable<T> && has_size<T>;


    namespace __helpers {
        template<class T>
        struct _collection_info_t {};
        template<class T>
            requires is_sized_collection<T>
        struct _collection_info_t<T> {
            using element_value = typename T::value_type;
            using element_reference = typename T::reference;
            using const_element_reference = typename T::const_reference;
        };
    }

    template<class T>
    using collection_info = typename __helpers::template _collection_info_t<T>;

    /*
    Return true if item is part of collection.
    */
    template<class TColl, class TElem>
        requires is_forward_iterable_of<TColl, TElem>
    static inline bool coll_contains(const TColl& collection, TElem item) {
        auto it = std::find(collection.begin(), collection.end(), item);
        return it != collection.end();
    }

    /*
    Find the index of item within collection. If not found, return default_ instead.
    */
    template<class TColl, class TElem>
        requires is_forward_iterable_of<TColl, TElem>
    static inline size_t coll_index(const TColl& collection, TElem item, size_t default_ = SIZE_INVALID) {
        auto it = std::find(collection.begin(), collection.end(), item);
        if (it == collection.end())
            return default_;
        return it - collection.begin();
    }

    /*
    Access the last element of a sized collection.
    If the collection is empty, behaviour depends on container type.
    */
    template<class TColl>
        requires (is_sized_collection<TColl> && !std::is_const_v<TColl>)
    static inline collection_info<TColl>::element_reference coll_last(TColl& collection) {
        return collection[collection.size() - 1];
    }
    ///*
    //Access the last element of a sized collection.
    //If the collection is empty, behaviour depends on container type.
    //*/
    template<class TColl>
        requires is_sized_collection<TColl>
    static inline collection_info<TColl>::const_element_reference coll_last(const TColl& collection) {
        return collection[collection.size() - 1];
    }

    template<class T>
    static inline T& vec_last(const std::vector<T>& vec) {
        return vec[vec.size()-1];
    }

    static inline std::string_view vec2sview(std::vector<char> vec) {
        return { vec.data(), vec.size() };
    }

    static inline std::string_view sslice(std::string_view str, size_t offset, size_t count) {
        if (offset >= str.size())
            return {};
        size_t available = str.size() - offset;
        count = std::min(available, count);
        return { str.data() + offset, count };
    }

    static inline std::string_view sslice(const std::string& str, size_t offset, size_t count) {
        return sslice(std::string_view(str), offset, count);
    }   

    static inline std::string_view sslice(const std::vector<char>& str, size_t offset, size_t count) {
        return sslice(vec2sview(str), offset, count);
    }

    static inline std::string charvec_copy(std::vector<char> vec) {
        return { vec.data(), vec.size() };
    }
    
    template<class T>
    static inline array_view<T> vec2aview(std::vector<T> vec) {
        return { vec.data(), vec.size() };
    }


    template<class T>
    struct type_details {
        static inline const std::string_view mangled_name = typeid(T).name();

#ifdef __GNUC__
        static inline const std::string_view name = []() -> std::string_view {
            static std::string storage;

            int status = 0;

            char* ptr = abi::__cxa_demangle(mangled_name.data(), nullptr, nullptr, &status);

            if (!ptr) {
                return "<unknown>";
            }

            storage = ptr;
            std::free(ptr);
            return storage;
        }();
#else 
        static inline const std::string_view name = "<unknown>";
#endif
    };

    template<class TChild, class TBase>
    concept inherits = std::is_base_of_v<TBase, std::remove_cvref_t<TChild>>;

    template<class T>
    concept printable = requires(std::ostream& fs, T&& t) {
        { fs << std::forward<T>(t) } -> std::convertible_to<std::ostream&>;
    };

    template<class ...T>
        requires (printable<T> && ...)
    inline std::string sfmt(T&& ...args) {
        std::stringstream fs;

        (fs << ... << std::forward<T>(args));

        return fs.str();
    }

    template<class TEnum>
        requires std::is_enum_v<TEnum>
    inline std::underlying_type_t<TEnum> get_underlying(TEnum value) {
        return static_cast<std::underlying_type_t<TEnum>>(value);
    }

    namespace __helpers {
        template<class T>
        struct _forwardable_t {
            using type = T;
        };

        template<class T>
            requires (!std::is_reference_v<T>)
        struct _forwardable_t<T> {
            using type = const std::remove_const_t<T>&;
        };
    }

    template<class T>
    using forwardable = typename __helpers::_forwardable_t<T>::type;

    template<class TSrc, class TRes>
        requires (std::is_copy_constructible_v<TRes>)
    std::vector<TRes> vec_foreach(const std::vector<TSrc>& src, TRes (*action)(const TSrc&)) {
        std::vector<TRes> res = {};
        res.reserve(src.size());

        for (auto& item : src) {
            res.push_back(action(item));
        }

        return res;
    }

    template<class TSrc, class TRes>
        requires (std::is_move_constructible_v<TRes> && !std::is_copy_constructible_v<TRes>)
    std::vector<TRes> vec_foreach(const std::vector<TSrc>& src, TRes (*action)(const TSrc&)) {
        std::vector<TRes> res = {};
        res.reserve(src.size());

        for (auto& item : src) {
            res.emplace_back(std::move(action(item)));
        }

        return std::move(res);
    }


    /**
     * Index an array by enum value. If the enum value is out of range, return _default.
     */
    template<class T, class Enum, size_t Len>
        requires (std::is_enum_v<Enum> && !std::is_const_v<T>)
    inline constexpr T array_index_by_enum(T (&array)[Len], Enum idx, T _default) {
        auto under = get_underlying(idx);
        if (under < 0 || under >= Len)
            return _default;
        return array[under];
    }
    /**
     * Index an array by enum value. If the enum value is out of range, return _default.
     */
    template<class T, class Enum, size_t Len>
        requires (std::is_enum_v<Enum> && !std::is_const_v<T>)
    inline constexpr const T& array_index_by_enum(const T (&array)[Len], Enum idx, const T& _default) {
        auto under = get_underlying(idx);
        if (under < 0 || under >= Len)
            return _default;
        return array[under];
    }
}

