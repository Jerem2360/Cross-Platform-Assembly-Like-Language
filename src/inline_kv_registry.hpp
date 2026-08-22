#pragma once
#include <vector>
#include <cstdint>
#include <forward_list>


namespace cpasm {
    /**
     * Hashmap registry where the keys are directly contained within their associated values.
     */
    template<class TKey, class TValue, TKey TValue::*KeyMember>
        requires (std::is_copy_constructible_v<TValue> && std::is_copy_assignable_v<TValue>)
    class InlineKeyRegistry {

        struct _entry {
            TValue value;

            inline TKey& key() {
                return this->value.*KeyMember;
            }

            inline const TKey& key() const {
                return this->value.*KeyMember;
            }
        };


        struct _bucket {
            std::forward_list<_entry> entries = {};

            inline bool push(const TValue& value, TValue** pout) {
                const TKey& key = value.*KeyMember;

                bool found = false;
                for (auto iter = this->entries.begin(); iter != this->entries.end(); ++iter) {
                    if (key == iter->key()) {
                        iter->value = value;
                        *pout = &iter->value;
                        return false;
                    }
                }
                *pout = &this->entries.emplace_front(value).value;
                return true;
            }

            inline TValue* find(const TKey& key) {
                for (_entry& e : this->entries) {
                    if (e.key() == key) {
                        return &e.value;
                    }
                }
                return nullptr;
            }
            
            inline const TValue* find(const TKey& key) const {
                for (const _entry& e : this->entries) {
                    if (e.key() == key) {
                        return &e.value;
                    }
                }
                return nullptr;
            }
        };

        std::vector<_bucket> _buckets = {};
        size_t _size = 0;
        bool _frozen = false;

        inline size_t _get_index(const TKey& key) {
            return std::hash<TKey>()(key) % this->_buckets.size();
        }

        inline TValue* _add(const TValue& value) {
            size_t idx = this->_get_index(value.*KeyMember);
            TValue* res;
            this->_size += this->_buckets.at(idx).push(value, &res);
            return res;
        }

        inline void _resize(size_t new_size) {
            if (new_size == this->_buckets.size())
                return;
            std::vector<_bucket> old_buckets = std::vector<_bucket>(new_size);
            std::swap(old_buckets, this->_buckets);
            this->_size = 0;

            if (!new_size)  // if new_size is 0, we can't insert anything
                return;

            for (const _bucket& b : old_buckets) {
                for (const _entry& e : b.entries) {
                    this->_add(e.value);
                }
            }
        }

    public:
        constexpr InlineKeyRegistry() = default;

        /**
         * Make the registry immutable and optimize its memory consumption.
         * May invalidate pointers to elements.
         */
        inline void freeze() {
            this->_resize(this->_size);
            this->_frozen = true;
        }

        /**
         * Insert an element into the registry.
         * If the value is already present, overwrites it.
         * May invalidate pointers to elements.
         * Returns a pointer to the newly inserted element, or nullptr if the registry is frozen.
         */
        inline TValue* push(const TValue& value) {
            if (this->_frozen)
                return nullptr;

            if (this->_buckets.size() <= (2 * this->_size)) {
                this->_resize(this->_buckets.size() ? this->_buckets.size() * 2 : 8);
            }
            
            return this->_add(value);
        }
        /**
         * Obtain a pointer to an element of the registry given its key.
         * Return nullptr if not found. 
         */
        inline TValue* get(const TKey& key) {
            if (!this->_size)
                return nullptr;
            size_t idx = this->_get_index(key);
            return this->_buckets.at(idx).find(key);
        }
        /**
         * Obtain a pointer to an element of the registry given its key.
         * Return nullptr if not found. 
         */
        inline const TValue* get(const TKey& key) const {
            if (!this->_size)
                return nullptr;
            size_t idx = this->_get_index(key);
            return this->_buckets.at(idx).find(key);
        }
        /**
         * The number of entries currently stored in the registry.
         */
        inline size_t size() const {
            return this->_size;
        }

        /**
         * Free all dynamic memory used by the registry.
         * Discards all existing entries.
         */
        inline bool clear() {
            if (this->_frozen)
                return false;
            this->_resize(0);
            return true;
        }
    };

    template<class TKey, class TValue, TKey TValue::* KeyMember>
        requires (std::is_copy_constructible_v<TValue> && std::is_copy_assignable_v<TValue>)
    class InlineKeyRegistryView {
        const InlineKeyRegistry<TKey, TValue, KeyMember>* _src;

    public:
        inline InlineKeyRegistryView(const InlineKeyRegistry<TKey, TValue, KeyMember>* src) :
            _src(src)
        {}
        inline const TValue* get(const TKey& key) const {
            return this->_src->get(key);
        }
        inline size_t size() const {
            return this->_src->size();
        }
    };
}

