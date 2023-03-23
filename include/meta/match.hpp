#ifndef RUST_ENUM_MATCH__
#define RUST_ENUM_MATCH__

/* --------------------------- MATCH --------------------------- */
#define $REIN_MATCH(value, ls)                                                              \
    $REIN_MATCH_FUNCTIONS(value, ls)                                                        \
    $REIN_MATCH_SELECTORS(value, ls)
/* Create functors */
#define $REIN_MATCH_FUNCTIONS(value, ls)                                                    \
    $REIN_MATCH_CREATE_FUNCTION(__fmatch, value, ls, $REIN_FMATCH)                          \
    $REIN_MATCH_CREATE_FUNCTION(__vmatch, value, ls, $REIN_VMATCH)
// ---------------------------
#define $REIN_MATCH_CREATE_FUNCTION(name, value, ls, type)                                  \
    rust_enum::invoke_or $REIN_LOCAL(name) { [](auto&& value) { type(value, ls) } };
/* Pick match type */
#define $REIN_MATCH_SELECTORS(value, ls)                                                    \
    $REIN_MATCH_CREATE_SELECTOR(__fmatch, value, rust_enum::is_rust_enum)                   \
    $REIN_MATCH_CREATE_SELECTOR(__vmatch, value, rust_enum::is_virtual)
#define $REIN_MATCH_CREATE_SELECTOR(name, value, expr)                                      \
    if constexpr(constexpr bool __inline_b = expr<decltype(value)>)                         \
    { $REIN_LOCAL(name).template invoke<__inline_b>(value); }

/* Generic cases */
#define $REIN_SWITCH_CASE_BODY(elem)                                                        \
    $REIN_CMP_SEQ_SIZE(elem, 3, (;BOOST_PP_SEQ_ELEM, (2, elem)), (BOOST_PP_SEQ_ELEM, (1, elem)))
/* Default case */
#define $REIN_DEFAULT(value, elem) default: { $REIN_SEQ_BACK(elem) }
/* Test for _ */
#define $REIN_CHECK_COND(name)                                                              \
    BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(($REIN_CAT($REIN_CHECK_, name)())), 1)
#define $REIN_CHECK__() 1, 2

/* --------------------------- FMATCH --------------------------- */
#ifndef USE_REFERENCE_ENUM_CASE
#  define $REIN_FMATCH_SWITCH_CASE_T auto
#else
#  define $REIN_FMATCH_SWITCH_CASE_T auto&&
#endif

/* Switch case body */
#define $REIN_FMATCH_SWITCH_CASE(value, elem)                                               \
    $REIN_FMATCH_SWITCH_CASE_FN(elem)(value, elem) $REIN_SWITCH_CASE_BODY(elem)
// ---------------------------
#define $REIN_FMATCH_SWITCH_CASE_FN(elem)                                                   \
    BOOST_PP_IF(BOOST_PP_EQUAL($REIN_SIZEOF(elem), 3), $REIN_FMATCH_SWITCH_CASE_I, $REIN_EAT)
// ---------------------------
#define $REIN_FMATCH_SWITCH_CASE_BODY(elem)                                                 \
    $REIN_CMP_SEQ_SIZE(elem, 3, (;BOOST_PP_SEQ_ELEM, (2, elem)), (BOOST_PP_SEQ_ELEM, (1, elem)))
// ---------------------------
#define $REIN_FMATCH_SWITCH_CASE_I(value, elem)                                             \
    $REIN_FMATCH_SWITCH_CASE_T $REIN_FMATCH_SWITCH_CASE_II(elem) =                          \
    value.template get_with<$REIN_FMATCH_CASE_V(value, elem)>()
#define $REIN_FMATCH_SWITCH_CASE_II(elem) $REIN_EXP_I($REIN_FMATCH_SWITCH_CASE_III(elem))
#define $REIN_FMATCH_SWITCH_CASE_III(elem)                                                  \
    BOOST_PP_IF(BOOST_PP_IS_BEGIN_PARENS(BOOST_PP_SEQ_ELEM(1, elem)),                       \
    $REIN_FMATCH_SWITCH_CASE_IV(elem),                                                      \
    (BOOST_PP_SEQ_ELEM(1, elem)))
#define $REIN_FMATCH_SWITCH_CASE_IV(elem) ([$REIN_EXP_I(BOOST_PP_SEQ_ELEM(1, elem))])

/* Switch case base */
#define $REIN_FMATCH_BODY(value, ls) BOOST_PP_SEQ_FOR_EACH($REIN_FMATCH_BODY_FE, value, ls)
#define $REIN_FMATCH_BODY_FE(r, value, elem) $REIN_FMATCH_GET_FN(elem)(value, elem)
#define $REIN_FMATCH_EXPLICIT(value, elem)                                                  \
    case $REIN_FMATCH_CASE_V(value, elem):                                                  \
    { $REIN_FMATCH_SWITCH_CASE(value, elem); break; }
// ---------------------------
#define $REIN_FMATCH_CASE_V(value, elem)                                                    \
    std::remove_cvref_t<decltype(value)>::__internal::BOOST_PP_SEQ_HEAD(elem)
// ---------------------------
/* Match _ */
#define $REIN_FMATCH_GET_FN(elem) $REIN_FMATCH_CHECK(BOOST_PP_SEQ_HEAD(elem))
#define $REIN_FMATCH_CHECK(name)                                                            \
    BOOST_PP_IF($REIN_CHECK_COND(name), $REIN_FMATCH_EXPLICIT, $REIN_DEFAULT)

/* Match all */
#define $REIN_FMATCH(value, ls)                                                             \
    switch(value.get_value()) { $REIN_FMATCH_BODY(value, $REIN_FMATCH_I(ls)) }
// ---------------------------
#define $REIN_FMATCH_I(ls) BOOST_PP_SEQ_FOR_EACH($REIN_FMATCH_IFE,, ls)
#define $REIN_FMATCH_IFE(r,d, elem)                                                         \
    ((BOOST_PP_SEQ_HEAD(elem)) $REIN_FMATCH_GROUP_TUPLE_ARGS(BOOST_PP_SEQ_TAIL(elem)))
/* Double parenthesize tuple arguments */
#define $REIN_FMATCH_GROUP_TUPLE_ARGS(elem_tail)                                            \
    BOOST_PP_IF(BOOST_PP_CHECK_EMPTY($REIN_EAT elem_tail),,                                 \
    $REIN_FMATCH_GROUP_TUPLE_ARGS_I) elem_tail
#define $REIN_FMATCH_GROUP_TUPLE_ARGS_I(...)                                                \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), 1),                      \
    (__VA_ARGS__), ((__VA_ARGS__)))


/* --------------------------- VMATCH --------------------------- */
#ifndef USE_NO_REFERENCE_VIRTUAL_CASE
#  define $REIN_VMATCH_SWITCH_CASE_T auto&&
#else
#  define $REIN_VMATCH_SWITCH_CASE_T auto
#endif

/* Switch case body */
#define $REIN_VMATCH_SWITCH_CASE(value, elem)                                               \
    $REIN_VMATCH_SWITCH_CASE_FN(elem)(value, elem) $REIN_SWITCH_CASE_BODY(elem)
// ---------------------------
#define $REIN_VMATCH_SWITCH_CASE_FN(elem)                                                   \
    BOOST_PP_IF(BOOST_PP_EQUAL($REIN_SIZEOF(elem), 3), $REIN_VMATCH_SWITCH_CASE_I, $REIN_EAT)
// ---------------------------
#define $REIN_VMATCH_SWITCH_CASE_I(value, elem)                                             \
    $REIN_VMATCH_SWITCH_CASE_T $REIN_EXP_I($REIN_FMATCH_SWITCH_CASE_III(elem)) =            \
    rust_enum::dispatch<BOOST_PP_SEQ_HEAD(elem)>(value)

/* Switch case base */
#define $REIN_VMATCH_BODY(value, ls) BOOST_PP_SEQ_FOR_EACH($REIN_VMATCH_BODY_FE, value, ls)
#define $REIN_VMATCH_BODY_FE(r, value, elem) $REIN_VMATCH_GET_FN(elem)(value, elem)
#define $REIN_VMATCH_EXPLICIT(value, elem)                                                  \
    BOOST_PP_IF(BOOST_PP_EQUAL($REIN_SIZEOF(elem), 0),                                      \
    $REIN_EAT, $REIN_VMATCH_EXPLICIT_I)(value, elem)
#define $REIN_VMATCH_EXPLICIT_I(value, elem)                                                \
    case $REIN_VMATCH_CASE_V(elem):                                                         \
    { $REIN_VMATCH_SWITCH_CASE(value, elem); break; }
// ---------------------------
/* Match _ */
#define $REIN_VMATCH_GET_FN(elem) $REIN_VMATCH_CHECK_CASE(BOOST_PP_SEQ_HEAD(elem))
#define $REIN_VMATCH_CHECK_CASE(name)                                                       \
    BOOST_PP_IF($REIN_CHECK_COND(name), $REIN_VMATCH_EXPLICIT, $REIN_DEFAULT)
// ---------------------------
#define $REIN_VMATCH_CASE_V(elem) rust_enum::type_hash_v<BOOST_PP_SEQ_HEAD(elem)>

/* Switch case functions */
#define $REIN_VMATCH_DISPATCH(value) $REIN_VMATCH_DISPATCHER_F()(value)
#define $REIN_VMATCH_GEN_DISPATCH(value, ls)                                                \
    using $REIN_VMATCH_DISPATCHER_T() = std::uint64_t(*)($REIN_VMATCH_GET_ARG_T(value));    \
    $REIN_VMATCH_FORWARD_DECL(ls)                                                           \
    $REIN_VMATCH_DISPATCHER_T() $REIN_VMATCH_DISPATCHER_F() =                               \
    $REIN_VMATCH_GEN_DISPATCH_I(value, ls);
// ---------------------------
#define $REIN_VMATCH_GEN_DISPATCH_I(value, ls) $REIN_VMATCH_GET_TYPE(value,                 \
    BOOST_PP_SEQ_FOR_EACH($REIN_VMATCH_GEN_DISPATCH_IFE,, ls) void)
#define $REIN_VMATCH_GEN_DISPATCH_IFE(r,d, elem) $REIN_VMATCH_CHECK(BOOST_PP_SEQ_HEAD(elem))
#define $REIN_VMATCH_CHECK(name)                                                            \
    $REIN_EXP(BOOST_PP_IF($REIN_CHECK_COND(name), $REIN_EXP, $REIN_EAT)(name,))
// ---------------------------
#define $REIN_VMATCH_DISPATCHER_T() $REIN_CAT(__RE_vmatch_, __LINE__)
#define $REIN_VMATCH_DISPATCHER_F() $REIN_CAT(__RE_inline_function_call_, __LINE__)
// ---------------------------
#define $REIN_VMATCH_GET_TYPE(value, ...)                                                   \
    &rust_enum::vmatch_cases<                                                               \
    $REIN_VMATCH_GET_VALUE_T(value) __VA_OPT__(,) __VA_ARGS__>
#define $REIN_VMATCH_GET_VALUE_T(value)                                                     \
    std::remove_pointer_t<std::remove_cvref_t<decltype(value)>>
#define $REIN_VMATCH_GET_ARG_T(value) const $REIN_VMATCH_GET_VALUE_T(value)*
// ---------------------------
#define $REIN_VMATCH_FORWARD_DECL(ls)                                                       \
    $REIN_VMATCH_FORWARD_DECL_I(BOOST_PP_SEQ_FOR_EACH($REIN_VMATCH_GEN_DISPATCH_IFE,, ls) void)
#define $REIN_VMATCH_FORWARD_DECL_I(...)                                                    \
    BOOST_PP_SEQ_FOR_EACH($REIN_VMATCH_FORWARD_DECL_IFE,,                                   \
    BOOST_PP_SEQ_POP_BACK($REIN_ARGS_TO_SEQ(__VA_ARGS__)))
#define $REIN_VMATCH_FORWARD_DECL_IFE(r,d, name)                                            \
    using $REIN_LOCAL($REIN_CAT(__, name)) = struct name;

/* Match all */
#define $REIN_VMATCH(value, ls)                                                             \
    $REIN_VMATCH_GEN_DISPATCH(value, ls)                                                    \
    switch($REIN_VMATCH_DISPATCH(value)) { $REIN_VMATCH_BODY(value, $REIN_VMATCH_I(ls)) }
// ---------------------------
#define $REIN_VMATCH_I(ls) BOOST_PP_SEQ_FOR_EACH($REIN_FMATCH_IFE,, ls)

#endif // RUST_ENUM_MATCH__
