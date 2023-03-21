#ifndef RUST_ENUM_TEMPLATES__
#define RUST_ENUM_TEMPLATES__

#include <new>
#include <type_traits>
#include <utility>

namespace rust_enum {
    template <typename T>
    struct type_wrapper {
        using type = T;
    };

    template <std::size_t N>
    using index_constant = std::integral_constant<std::size_t, N>;

    template <std::size_t N>
    inline constexpr std::integral_constant<std::size_t, N> I{};

    template <std::size_t N>
    using seq_t = decltype(std::make_index_sequence<N>{});

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
            template <typename T, typename...UU>
            constexpr enum_bytes(type_wrapper<T>, UU&&... uu) {
                if constexpr(requires { new(data) T{ std::forward<UU>(uu)... }; }) {
                    new(data) T{ std::forward<UU>(uu)... };
                }
                else new(data) T(std::forward<UU>(uu)...);
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
        template <typename T, typename...UU>
        constexpr enum_base(type_wrapper<T> t, UU&&... uu) : data(t, std::forward<UU>(uu)...) {}

        template <typename T>
        T* get_as() noexcept {
            return data.template get_as<T>();
        }

        template <typename T>
        const T* get_as() const noexcept {
            return data.template get_as<T>();
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
