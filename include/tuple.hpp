#ifndef RUST_ENUM_TUPLE__
#define RUST_ENUM_TUPLE__

#include "xtraits.hpp"

namespace rust_enum {
    namespace detail {
        template <std::size_t N, typename T>
        struct tuple_leaf {
            static constexpr std::size_t value = N;
            using type = T;
            T data;

            constexpr T& at_index(const index_constant<N>&) {
                return data;
            }

            static constexpr type_wrapper<T> at_index_type(const index_constant<N>&) {
                return {};
            }

            constexpr std::size_t at_type(const type_wrapper<T>&) const {
                return N;
            }
        };

        template <typename /* sequence */, typename...> struct tuple_elements;

        template <std::size_t...NN, typename...TT>
        struct tuple_elements<std::index_sequence<NN...>, TT...> : tuple_leaf<NN, TT>... {
            using tuple_leaf<NN, TT>::at_index...;
            using tuple_leaf<NN, TT>::at_index_type...;
            using tuple_leaf<NN, TT>::at_type...;

            template <std::size_t N>
            constexpr decltype(auto) get_member(const index_constant<N>& i) {
                return this->at_index(i);
            }

            template <std::size_t N>
            constexpr decltype(auto) get_member_type(const index_constant<N>& i) {
                return this->at_index_type(i);
            }

            template <typename T>
            constexpr std::size_t get_index(const type_wrapper<T>& t) const {
                return this->at_type(t);
            }
        };

        template <typename...TT>
        tuple_elements(TT&&...tt) -> tuple_elements<seq_t<sizeof...(TT)>, decltype(static_cast<TT>(tt))...>;
    }

    template <typename...TT>
    struct tuple {
        using self_type = detail::tuple_elements< seq_t<sizeof...(TT)>, TT... >;
        self_type data;

        static constexpr std::size_t count = sizeof...(TT);

        template <std::size_t N>
        constexpr decltype(auto) get_value(index_constant<N> i) requires (N < count) {
            static_assert(N < count, "Index out of range!");
            return data.get_member(i);
        }

        template <std::size_t N>
        constexpr decltype(auto) get_type(index_constant<N> i) requires (N < count) {
            static_assert(N < count, "Index out of range!");
            return data.get_member_type(i);
        }

        template <typename T>
        constexpr std::size_t get_value(type_wrapper<T> t) const {
            return data.get_index(t);
        }

        template <std::size_t N>
        constexpr decltype(auto) operator[](index_constant<N> i) requires (N < count) {
            static_assert(N < count, "Index out of range!");
            return data.get_member(i);
        }

        template <typename T>
        constexpr std::size_t operator[](type_wrapper<T> t) const {
            return data.get_index(t);
        }
    };

    template <typename...TT>
    tuple(TT&&...tt) -> tuple<decltype(static_cast<TT>(tt))...>;


    template <typename> struct is_tuple : std::false_type {};

    template <typename...TT>
    struct is_tuple<tuple<TT...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;

    template <typename...TT>
    constexpr auto tie(TT&&...tt) {
        return tuple<TT...>{ std::forward<TT>(tt)... };
    }
}

namespace std {
    template <typename...TT>
    struct tuple_size<rust_enum::tuple<TT...>> 
        : integral_constant<::size_t, sizeof...(TT)> {};

    template <::size_t N, typename...TT>
    struct tuple_element<N, rust_enum::tuple<TT...>> {
        using type = typename decltype(
            declval<rust_enum::tuple<TT...>>().get_type(I<N>)
        )::type;
    };

    template <::size_t N, typename...TT, typename Tup = rust_enum::tuple<TT...>>
    tuple_element_t<N, Tup>& get(Tup& tup) {
        return tup[I<N>];
    }

    template <::size_t N, typename...TT, typename Tup = rust_enum::tuple<TT...>>
    const tuple_element_t<N, Tup>& get(const Tup& tup) {
        return tup[I<N>];
    }

    template <::size_t N, typename...TT, typename Tup = rust_enum::tuple<TT...>>
    tuple_element_t<N, Tup> get(Tup&& tup) {
        return tup[I<N>];
    }
}

#endif // RUST_ENUM_TUPLE__
