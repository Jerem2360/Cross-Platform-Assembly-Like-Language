#pragma once
#include <iostream>
#include "helpers.hpp"


namespace cpasm {
    namespace __helpers {
        template<class T>
        struct _RegistryNode {
            _RegistryNode<T>* next = nullptr;
            alignas(T) unsigned char _storage[sizeof(T)];

            // NOTE: do not dereference this until std::construct_at has been called.
            inline T* data() {
                return std::launder(reinterpret_cast<T*>(this->_storage));
            }
            inline const T* data() const  {
                return std::launder(reinterpret_cast<const T*>(this->_storage));
            }

            // storage is intentionally left uninitialized because std::construct_at will be called on it later
            inline _RegistryNode() : next(nullptr) {}
        };
    }
    
    class frozen_registry_error : public std::exception {
        using std::exception::exception;
    };

    /**
     * Registry optimized for iteration that uses a linked list internally.
     * Supports constant initialization so no runtime initialization is needed.
     * Entries are stored in reverse order of insertion.
     */
    template<class T>
    class Registry {
    public:
        class iterator : public std::forward_iterator_tag {
            friend class Registry;

            __helpers::_RegistryNode<T>* _node;

            inline iterator(__helpers::_RegistryNode<T>* src) : _node(src) {}

        public:
            inline iterator() : iterator(nullptr) {}
            
            using difference_type = std::ptrdiff_t;
            using value_type = T; 

            inline T& operator *() const {
                return *this->_node->data();
            }

            inline iterator& operator++() {
                this->_node = this->_node->next;
                return *this;
            }

            inline iterator operator++(int) {
                auto res = *this;
                this->_node = this->_node->next;
                return res;
            }

            inline bool operator ==(const iterator& other) const {
                return this->_node == other._node;
            }

        };

    private:
        __helpers::_RegistryNode<T>* _first = nullptr;
        bool _frozen = false;

        static inline void _destroy(__helpers::_RegistryNode<T>* node) {
            while (node) {
                auto next = node->next;
                std::destroy_at(node->data());
                delete node;
                node = next;
            }
        }

    public:

        constexpr Registry() = default;
        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) noexcept = delete;

        inline void push(const T& value) {
            if (this->_frozen)
                throw frozen_registry_error();

            __helpers::_RegistryNode<T>* node = new __helpers::_RegistryNode<T>();
            try {
                std::construct_at(node->data(), value);
            } catch (...) {
                delete node;
                throw;
            }
            node->next = this->_first;
            this->_first = node;
        }
        
        template<class ...Args>
            //requires std::constructible_from<T, std::remove_cvref_t<Args>...>
        inline T& emplace(Args&& ...args) {
            if (this->_frozen)
                throw frozen_registry_error();

            __helpers::_RegistryNode<T>* node = new __helpers::_RegistryNode<T>();
            try {
                std::construct_at(node->data(), std::forward<Args>(args)...);
            } catch (...) {
                delete node;
                throw;
            }
            node->next = this->_first;
            this->_first = node;

            return *node->data();
        }

        inline void freeze(std::string_view debug_name = "") {
            if (debug_name.size()) {
                size_t cnt = 0;
                for (auto& elem : *this) {
                    cnt += 1;
                }
                std::cout << "[Registry] Freezing " << cnt << " entries of type \"" << debug_name << "\".\n";
            }

            this->_frozen = true;
        }

        iterator begin() const {
            return iterator(this->_first);
        }

        iterator end() const {
            return iterator(nullptr);
        }

        ~Registry() {
            if (!this->_first)
                return;
        
            _destroy(this->_first);
        }
    };

    /**
     * Readonly proxy to a Registry object.
     */
    template<class  T>
    class RegistryView {
    public:
        class iterator {
            Registry<T>::iterator _underlying;

        public:
            inline iterator() : _underlying() {}
            inline iterator(Registry<T>::iterator&& val) :
                _underlying(val)
            {}
            
            using difference_type = std::ptrdiff_t;
            using value_type = T; 

            inline const T& operator *() const {
                return *this->_underlying;
            }

            inline iterator& operator++() {
                this->_underlying++;
                return *this;
            }

            inline iterator operator++(int) {
                auto res = *this;
                this->_underlying++;
                return res;
            }

            inline bool operator ==(const iterator& other) const {
                return this->_underlying == other._underlying;
            }
        };
        
    private:
        Registry<T>* _owner;

    public:
        inline RegistryView(Registry<T>* owner) :
            _owner(owner)
        {}
        
        inline iterator begin() const {
            return iterator(this->_owner->begin());
        }
        inline iterator end() const {
            return iterator(this->_owner->end());
        }

        // does it make sense to keep that ?
        inline void freeze(std::string_view debug_name = "") const {
            this->_owner->freeze(debug_name);
        }
        
    };
}

