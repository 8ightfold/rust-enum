#ifndef RUST_ENUM_TEMPLATES__
#define RUST_ENUM_TEMPLATES__

#include <new>
#include <type_traits>
#include <utility>
#include "xtraits.hpp"

/* $enum internals */
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


#include <map>
#include <typeinfo>
#include "meta/cttypeid.hpp"

/* $vmatch internals */
namespace rust_enum {
    namespace meta {
        constexpr std::uint64_t fnv1a_64(const std::string_view s, std::uint64_t count) {
            constexpr std::uint64_t fnv_offset = 14695981039346656037llu;
            constexpr std::uint64_t fnv_prime = 1099511628211llu;
            return ((count ? fnv1a_64(s, count - 1) : fnv_offset) ^ s[count]) * fnv_prime;
        }
    }

    template <typename T>
    constexpr std::uint64_t type_hash() noexcept {
        using U = std::remove_cvref_t<T>;
        constexpr std::string_view name = cttypeid<U>.name();
        return meta::fnv1a_64(name, name.size() - 1);
    }

    template <>
    constexpr std::uint64_t type_hash<void>() noexcept { return 0; }

    template <typename T>
    inline constexpr std::uint64_t type_hash_v = type_hash<T>();


    template <typename T, typename...UU>
    [[nodiscard]] std::uint64_t vmatch_cases(const T* t) noexcept {
        static const std::map<std::uint64_t, std::uint64_t> runtime_to_compile_time {
            { typeid(UU).hash_code(), type_hash_v<UU> }...
        };
        std::uint64_t runtime_hash = typeid(*t).hash_code();

        if(runtime_to_compile_time.contains(runtime_hash)) {
            return runtime_to_compile_time.at(runtime_hash);
        }
        else return type_hash_v<void>;
    }

    template <typename T, typename...UU>
    [[nodiscard]] std::uint64_t vmatch_cases(const T& t) noexcept {
        return vmatch_cases<T, UU...>(&t);
    }

    template <typename T, typename U>
    T& dispatch(U& u) noexcept { return dynamic_cast<T&>(u); }

    template <typename T, typename U>
    T* dispatch(U* u) noexcept { return dynamic_cast<T*>(u); }

    template <typename T, typename U>
    const T& dispatch(const U& u) noexcept { return dynamic_cast<const T&>(u); }

    template <typename T, typename U>
    const T* dispatch(const U* u) noexcept { return dynamic_cast<const T*>(u); }
}


/* generic internals */
namespace rust_enum {
    template <typename T>
    concept is_rust_enum = std::is_base_of_v<enum_tag, std::remove_cvref_t<T>>;

    template <typename T>
    concept is_virtual = std::has_virtual_destructor_v<std::remove_cvref_t<std::remove_pointer_t<T>>>;

    template <typename T>
    struct invoke_or {
        T expression;

        template <bool B, typename...TT>
        constexpr auto invoke(TT&&...tt) requires B {
            return expression(std::forward<TT>(tt)...);
        }

        template <bool, typename...TT>
        constexpr auto invoke(TT&&...) {}
    };
}

#endif // RUST_ENUM_TEMPLATES__
