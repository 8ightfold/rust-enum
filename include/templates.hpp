#ifndef RUST_ENUM_TEMPLATES__
#define RUST_ENUM_TEMPLATES__

#include <new>
#include <type_traits>
#include <utility>
#include "xtraits.hpp"

namespace rust_enum {
    namespace meta {
        template <typename T, typename...TT>
        struct enum_data {
            static constexpr std::size_t size = 
                (sizeof(T) > enum_data<TT...>::size) ? sizeof(T) : enum_data<TT...>::size;
            static constexpr std::size_t align = 
                (alignof(T) > enum_data<TT...>::align) ? alignof(T) : enum_data<TT...>::align;
        };

        template <typename T>
        struct enum_data<T> {
            static constexpr std::size_t size = sizeof(T);
            static constexpr std::size_t align = alignof(T);
        };
    
        template <typename...TT>
        struct enum_bytes {
            constexpr enum_bytes() = default;

            template <typename T, typename...UU>
            constexpr enum_bytes(type_wrapper<T>, UU&&... uu) {
                new(data) T{ std::forward<UU>(uu)... };
            }

            template <typename T>
            T* get_as() noexcept {
                return std::launder( reinterpret_cast<T*>(data) );
            }

            template <typename T>
            const T* get_as() const noexcept {
                return std::launder( reinterpret_cast<const T*>(data) );
            }

            template <typename T>
            const T copy_as() const noexcept {
                T t_copy { *get_as<T>() };
                return t_copy;
            }

            template <typename T>
            const T move_as() noexcept {
                return std::move(*get_as<T>());
            }

            template <typename T, typename...UU>
            constexpr void create_as(UU&&...uu) noexcept {
                new(data) T{ std::forward<UU>(uu)... };
            }

            template <typename T>
            constexpr void destroy_as() noexcept {
                if constexpr(std::is_destructible_v<T>) {
                    get_as<T>()->~T();
                }
            }

            alignas(enum_data<TT...>::align) unsigned char data[enum_data<TT...>::size] {};
        };
    }

    template <typename...TT>
    struct enum_base {
        constexpr enum_base() = default;

        template <typename T, typename...UU>
        constexpr enum_base(type_wrapper<T> t, UU&&... uu) : data(t, std::forward<UU>(uu)...) {}

        constexpr enum_base(enum_base&& b) : data(b.data) {}

        constexpr enum_base& operator=(enum_base&& b) {
            this->data = b.data;
            return *this;
        }

        template <typename T>
        T* get_as() noexcept {
            return data.template get_as<T>();
        }

        template <typename T>
        const T* get_as() const noexcept {
            return data.template get_as<T>();
        }

        template <typename T, typename Old, typename...UU>
        constexpr void create_as(UU&&...uu) noexcept {
            data.template destroy_as<Old>();
            data.template create_as<T>(std::forward<UU>(uu)...);
        }

        template <typename T>
        constexpr enum_base copy_as() const noexcept {
            return { type_wrapper<T>{}, data.template copy_as<T>() };
        }

        template <typename T>
        constexpr void destroy_as() noexcept {
            data.template destroy_as<T>();
        }

        meta::enum_bytes<TT...> data;
    };

    struct enum_tag {};


    template <typename T, std::size_t N>
    struct type_node {
        static constexpr type_wrapper<T> type_internal(const index_constant<N>) noexcept {
            return {};
        }
    };

    template <typename, typename...TT> struct node_funnel;

    template <std::size_t...NN, typename...TT> 
    struct node_funnel<std::index_sequence<NN...>, TT...> : type_node<TT, NN>... {
        using type_node<TT, NN>::type_internal...;
    };

    template <typename...TT>
    struct node_pack : node_funnel<seq_t<sizeof...(TT)>, TT...> {
        using node_funnel<seq_t<sizeof...(TT)>, TT...>::type_internal;

        template <std::size_t N>
        static constexpr auto get(const index_constant<N> i) noexcept {
            return type_internal(i);
        }
    };
}

#endif // RUST_ENUM_TEMPLATES__
