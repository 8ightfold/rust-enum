#ifndef RUST_ENUM_PREPROCESSOR__
#define RUST_ENUM_PREPROCESSOR__

#include <boost/preprocessor.hpp>
#include "meta/match.hpp"

// Exposed expressions
#define $enum(name, ...) struct name $REIN_CREATE(name, $REIN_ARGS_TO_SEQ(__VA_ARGS__))
#define $match(value, ...)  $REIN_MATCH(value,  $REIN_ARGS_TO_SEQ(__VA_ARGS__))
#define $fmatch(value, ...) $REIN_FMATCH(value, $REIN_ARGS_TO_SEQ(__VA_ARGS__))
#define $vmatch(value, ...) $REIN_VMATCH(value, $REIN_ARGS_TO_SEQ(__VA_ARGS__))

// Utils
#define $REIN_EAT(...)
#define $REIN_EXP(...) __VA_ARGS__
#define $REIN_EXP_I(expr) $REIN_EXP expr
#define $REIN_EATP(expr, ...) expr
#define $REIN_EAT_NEXT(...) (__VA_ARGS__) $REIN_EAT
// ---------------------------
#define $REIN_XCAT(x, y) x ## y
#define $REIN_CAT(x, y) $REIN_XCAT(x, y)
#define $REIN_LOCAL(fn) $REIN_CAT(fn, __LINE__)
// ---------------------------
#define $REIN_APPLY(fn, ...) BOOST_PP_SEQ_FOR_EACH(                                         \
    $REIN_APPLY_I, fn, $REIN_ARGS_TO_SEQ(__VA_ARGS__))
#define $REIN_APPLY_I(r, fn, elem) fn(elem);
/* Apply first element to second if true: $REIN_EXPAND_IF( cond, (tex, (...)), (fex, (...)) ) */
#define $REIN_EXPAND_IF_I(expr, args) expr args
#define $REIN_EXPAND_IF(cond, t, f)                                                         \
    BOOST_PP_IF(cond, $REIN_EXPAND_IF_I, $REIN_EAT) t                                       \
    BOOST_PP_IF(cond, $REIN_EAT, $REIN_EXPAND_IF_I) f

/* Sequences */
#define $REIN_SIZEOF(seq) BOOST_PP_SEQ_SIZE(seq)
// ---------------------------
#define $REIN_MSIZEOF(seq) BOOST_PP_DEC($REIN_SIZEOF(seq))
#define $REIN_SEQ_BACK(seq) BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq))
#define $REIN_IS_EMPTY(cond, t, f) BOOST_PP_IF(BOOST_PP_CHECK_EMPTY(cond), t, f)
/* Create argument list with final element removed if empty */
#define $REIN_ARGS_TO_SEQ(...) $REIN_ARGS_TO_SEQ_I(BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__)))
#define $REIN_ARGS_TO_SEQ_I(seq)                                                            \
    $REIN_IS_EMPTY($REIN_SEQ_BACK(seq), BOOST_PP_SEQ_POP_BACK(seq), seq)
// ---------------------------
#define $REIN_CMP_SEQ_SIZE(seq, sz, t, f)                                                   \
    $REIN_EXPAND_IF(BOOST_PP_GREATER_EQUAL($REIN_SIZEOF(seq), sz), t, f)
/* Get user type or name */
#define $REIN_EXP_NAME(elem) BOOST_PP_SEQ_ELEM(0, elem)
#define $REIN_EXP_TYPE(elem)                                                                \
    $REIN_CMP_SEQ_SIZE(elem, 2, ($REIN_EXP_TYPE_I, (elem)), (,__internal_type))
// ---------------------------
#define $REIN_EXP_TYPE_I(elem) $REIN_EXP_I($REIN_EXP_TYPE_II(elem))
#define $REIN_EXP_TYPE_II(elem)                                                             \
    BOOST_PP_IF($REIN_EXP_TYPE_IS_TUPLE(elem),                                              \
    (rust_enum::tuple<$REIN_EXP BOOST_PP_SEQ_ELEM(1, elem)>),                               \
    (BOOST_PP_SEQ_ELEM(1, elem)))
#define $REIN_EXP_TYPE_IS_TUPLE(elem)                                                       \
    BOOST_PP_IS_BEGIN_PARENS(BOOST_PP_SEQ_ELEM(1, elem))


// Enum internals
/* Create internal enum */
#define $REIN_CREATE_ENUM(ls)                                                               \
    enum class __internal { BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_ENUM_FE,, ls) };
#define $REIN_CREATE_ENUM_FE(r,d, elem) BOOST_PP_SEQ_HEAD(elem),

/* Create private values */
#define $REIN_CREATE_PRIVATE(name, ls)                                                      \
    using __internal_type = std::underlying_type_t<__internal>;                             \
    using __value_type = rust_enum::enum_base<$REIN_CREATE_PRIVATE_I(ls)>;                  \
    [[no_unique_address]] rust_enum::node_pack<$REIN_CREATE_PRIVATE_I(ls)> __nodes {};      \
    __internal __type; __value_type __value;                                                \
    $REIN_CREATE_PRIVATE_CONSTRUCTOR(name)
// ---------------------------
#define $REIN_CREATE_PRIVATE_IFE(r, sz, i, elem)                                            \
    $REIN_EXP_TYPE(elem)                                                                    \
    BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, sz))
// ---------------------------
#define $REIN_CREATE_PRIVATE_I(ls)                                                          \
    BOOST_PP_SEQ_FOR_EACH_I($REIN_CREATE_PRIVATE_IFE, $REIN_MSIZEOF(ls), ls)
// ---------------------------
#define $REIN_CREATE_PRIVATE_CONSTRUCTOR(name)                                              \
    template <typename __T_v, typename... __TT_v>                                           \
    name(__internal i, rust_enum::type_wrapper<__T_v>, __TT_v&&... __tt_v)                  \
    : __type(i), __value(rust_enum::type_wrapper<__T_v>{},                                  \
        std::forward<__TT_v>(__tt_v)...) {}

/* Create generator functions */
#define $REIN_CREATE_CONSTRUCTORS(name, ls)                                                 \
    BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_CONSTRUCTORS_FE, name, ls)
// ---------------------------
#define $REIN_CREATE_CONSTRUCTORS_FE(r, name, elem)                                         \
    template <typename...__TT_v>                                                            \
    static constexpr auto $REIN_EXP_NAME(elem)(__TT_v&&... __tt_v)                          \
    { return name { __internal::$REIN_EXP_NAME(elem),                                       \
    rust_enum::type_wrapper<$REIN_EXP_TYPE(elem)>{}, std::forward<__TT_v>(__tt_v)... }; }

/* Create destructor function */
#define $REIN_CREATE_DESTRUCTOR(name, ls)                                                   \
    constexpr void __destroy() noexcept                                                     \
    { switch(__type) { BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_DESTRUCTOR_FE, name, ls) } }      \
    ~name() { __destroy(); }

// ---------------------------
#define $REIN_CREATE_DESTRUCTOR_FE(r, name, elem)                                           \
    case __internal::$REIN_EXP_NAME(elem):                                                  \
    { using __tmp_type =                                                                    \
    typename decltype(get_type<__internal::$REIN_EXP_NAME(elem)>())::type;                  \
    __value.template destroy_as<std::remove_cvref_t<__tmp_type>>(); break; }

/* Create copy constructor */
#define $REIN_CREATE_COPY(name, ls)                                                         \
    constexpr name(const name& lhs) : __type(lhs.__type)                                    \
    { switch(__type) { BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_COPY_FE, name, ls) } }
// ---------------------------
#define $REIN_CREATE_COPY_FE(r, name, elem)                                                 \
    case __internal::$REIN_EXP_NAME(elem):                                                  \
    { using __tmp_type =                                                                    \
        std::remove_cvref_t<                                                                \
        typename decltype(get_type<__internal::$REIN_EXP_NAME(elem)>())::type>;             \
    __value = __value_type{ rust_enum::type_wrapper<__tmp_type>{},                          \
        *lhs.__value.template get_as<__tmp_type>() };                                       \
    break; }

/* Create copy assignment */
#define $REIN_CREATE_COPY_ASSIGN(name, ls)                                                  \
    constexpr name& operator=(const name& lhs)                                              \
    { __destroy(); __type = lhs.__type;                                                     \
    switch(__type) { BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_COPY_FE, name, ls) } return *this; }

/* Create accessors */
#define $REIN_ACCESS_ENUM()                                                                 \
    constexpr __internal get_value() const noexcept { return __type; }                      \
    template <__internal __T_v> constexpr auto get_type() const noexcept                    \
    { return __nodes.get(rust_enum::I<static_cast<std::size_t>(__T_v)>); }                  \
    template <__internal __T_v> constexpr decltype(auto) get_with() const noexcept          \
    { return *__value.template get_as<typename decltype(get_type<__T_v>())::type>(); }      \
    template <__internal __T_v> constexpr auto get_type() noexcept                          \
    { return __nodes.get(rust_enum::I<static_cast<std::size_t>(__T_v)>); }                  \
    template <__internal __T_v> constexpr decltype(auto) get_with() noexcept                \
    { return *__value.template get_as<typename decltype(get_type<__T_v>())::type>(); }

/* Create all */
#define $REIN_CREATE(name, ls) : rust_enum::enum_tag                                        \
    { $REIN_CREATE_BODY(name, $REIN_CREATE_I(ls)) }
#define $REIN_CREATE_I(ls) BOOST_PP_SEQ_FOR_EACH($REIN_CREATE_IFE,, ls)
#define $REIN_CREATE_IFE(r,d, elem)                                                         \
    ((BOOST_PP_SEQ_HEAD(elem)) $REIN_CREATE_IFE_II(elem)($REIN_EAT elem))
#define $REIN_CREATE_IFE_I(elem_back)                                                       \
    BOOST_PP_IF(BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(elem_back), 1),                        \
    (elem_back), elem_back)
#define $REIN_CREATE_IFE_II(elem)                                                           \
    BOOST_PP_IF(BOOST_PP_CHECK_EMPTY ($REIN_EAT elem), $REIN_EAT, $REIN_CREATE_IFE_I)
// ---------------------------
#define $REIN_CREATE_BODY(name, ls)                                                         \
    $REIN_CREATE_ENUM(ls)                                                                   \
    $REIN_ACCESS_ENUM()                                                                     \
private:                                                                                    \
    $REIN_CREATE_PRIVATE(name, ls)                                                          \
public:                                                                                     \
    $REIN_CREATE_CONSTRUCTORS(name, ls)                                                     \
    $REIN_CREATE_DESTRUCTOR(name, ls)                                                       \
    $REIN_CREATE_COPY(name, ls)                                                             \
    $REIN_CREATE_COPY_ASSIGN(name, ls)

#endif // RUST_ENUM_PREPROCESSOR__
