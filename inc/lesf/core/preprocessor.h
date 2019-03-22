// This black magic fuckery implements a very nice map() function for macros
// Courtesy of http://jhnet.co.uk/articles/cpp_magic

#define LESF_CORE_PREPROCESSOR_FIRST(a, ...) a
#define LESF_CORE_PREPROCESSOR_SECOND(a, b, ...) b

#define LESF_CORE_PREPROCESSOR_EMPTY()

#define LESF_CORE_PREPROCESSOR_EVAL(...) LESF_CORE_PREPROCESSOR_EVAL8192(__VA_ARGS__)
#define LESF_CORE_PREPROCESSOR_EVAL8192(...) LESF_CORE_PREPROCESSOR_EVAL4096(LESF_CORE_PREPROCESSOR_EVAL4096(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL4096(...) LESF_CORE_PREPROCESSOR_EVAL2048(LESF_CORE_PREPROCESSOR_EVAL2048(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL2048(...) LESF_CORE_PREPROCESSOR_EVAL1024(LESF_CORE_PREPROCESSOR_EVAL1024(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL1024(...) LESF_CORE_PREPROCESSOR_EVAL512(LESF_CORE_PREPROCESSOR_EVAL512(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL512(...) LESF_CORE_PREPROCESSOR_EVAL256(LESF_CORE_PREPROCESSOR_EVAL256(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL256(...) LESF_CORE_PREPROCESSOR_EVAL128(LESF_CORE_PREPROCESSOR_EVAL128(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL128(...) LESF_CORE_PREPROCESSOR_EVAL64(LESF_CORE_PREPROCESSOR_EVAL64(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL64(...) LESF_CORE_PREPROCESSOR_EVAL32(LESF_CORE_PREPROCESSOR_EVAL32(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL32(...) LESF_CORE_PREPROCESSOR_EVAL16(LESF_CORE_PREPROCESSOR_EVAL16(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL16(...) LESF_CORE_PREPROCESSOR_EVAL8(LESF_CORE_PREPROCESSOR_EVAL8(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL8(...) LESF_CORE_PREPROCESSOR_EVAL4(LESF_CORE_PREPROCESSOR_EVAL4(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL4(...) LESF_CORE_PREPROCESSOR_EVAL2(LESF_CORE_PREPROCESSOR_EVAL2(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL2(...) LESF_CORE_PREPROCESSOR_EVAL1(LESF_CORE_PREPROCESSOR_EVAL1(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR_EVAL1(...) __VA_ARGS__

#define LESF_CORE_PREPROCESSOR_DEFER1(m) m LESF_CORE_PREPROCESSOR_EMPTY()
#define LESF_CORE_PREPROCESSOR_DEFER2(m) m LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY()()
#define LESF_CORE_PREPROCESSOR_DEFER3(m) m LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY()()()
#define LESF_CORE_PREPROCESSOR_DEFER4(m) m LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY LESF_CORE_PREPROCESSOR_EMPTY()()()()

#define LESF_CORE_PREPROCESSOR_IS_PROBE(...) LESF_CORE_PREPROCESSOR_SECOND(__VA_ARGS__, 0)
#define LESF_CORE_PREPROCESSOR_PROBE() ~, 1

#define LESF_CORE_PREPROCESSOR_CAT(a,b) a ## b

#define LESF_CORE_PREPROCESSOR_NOT(x) LESF_CORE_PREPROCESSOR_IS_PROBE(LESF_CORE_PREPROCESSOR_CAT(LESF_CORE_PREPROCESSOR__NOT_, x))
#define LESF_CORE_PREPROCESSOR__NOT_0 LESF_CORE_PREPROCESSOR_PROBE()

#define LESF_CORE_PREPROCESSOR_BOOL(x) LESF_CORE_PREPROCESSOR_NOT(LESF_CORE_PREPROCESSOR_NOT(x))

#define LESF_CORE_PREPROCESSOR_IF_ELSE(condition) LESF_CORE_PREPROCESSOR__IF_ELSE(LESF_CORE_PREPROCESSOR_BOOL(condition))
#define LESF_CORE_PREPROCESSOR__IF_ELSE(condition) LESF_CORE_PREPROCESSOR_CAT(LESF_CORE_PREPROCESSOR__IF_, condition)

#define LESF_CORE_PREPROCESSOR__IF_1(...) __VA_ARGS__ LESF_CORE_PREPROCESSOR__IF_1_ELSE
#define LESF_CORE_PREPROCESSOR__IF_0(...)             LESF_CORE_PREPROCESSOR__IF_0_ELSE

#define LESF_CORE_PREPROCESSOR__IF_1_ELSE(...)
#define LESF_CORE_PREPROCESSOR__IF_0_ELSE(...) __VA_ARGS__

#define LESF_CORE_PREPROCESSOR_HAS_ARGS(...) LESF_CORE_PREPROCESSOR_BOOL(LESF_CORE_PREPROCESSOR_FIRST(LESF_CORE_PREPROCESSOR__END_OF_ARGUMENTS_ __VA_ARGS__)())
#define LESF_CORE_PREPROCESSOR__END_OF_ARGUMENTS_() 0

#define LESF_CORE_PREPROCESSOR_MAP(m, first, ...)           \
  m(first)                           \
  LESF_CORE_PREPROCESSOR_IF_ELSE(LESF_CORE_PREPROCESSOR_HAS_ARGS(__VA_ARGS__))(    \
    LESF_CORE_PREPROCESSOR_DEFER2(LESF_CORE_PREPROCESSOR__MAP)()(m, __VA_ARGS__)   \
  )(                                 \
    /* Do nothing, just terminate */ \
  )
#define LESF_CORE_PREPROCESSOR__MAP() LESF_CORE_PREPROCESSOR_MAP

/* Same macros, but with _PREPROCESSOR2_ prefix. This allows nested MAP()s up
 * to a single level of recursion. Ugly trick, but it works. */

#define LESF_CORE_PREPROCESSOR2_FIRST(a, ...) a
#define LESF_CORE_PREPROCESSOR2_SECOND(a, b, ...) b

#define LESF_CORE_PREPROCESSOR2_EMPTY()

#define LESF_CORE_PREPROCESSOR2_EVAL(...) LESF_CORE_PREPROCESSOR2_EVAL8192(__VA_ARGS__)
#define LESF_CORE_PREPROCESSOR2_EVAL8192(...) LESF_CORE_PREPROCESSOR2_EVAL4096(LESF_CORE_PREPROCESSOR2_EVAL4096(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL4096(...) LESF_CORE_PREPROCESSOR2_EVAL2048(LESF_CORE_PREPROCESSOR2_EVAL2048(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL2048(...) LESF_CORE_PREPROCESSOR2_EVAL1024(LESF_CORE_PREPROCESSOR2_EVAL1024(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL1024(...) LESF_CORE_PREPROCESSOR2_EVAL512(LESF_CORE_PREPROCESSOR2_EVAL512(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL512(...) LESF_CORE_PREPROCESSOR2_EVAL256(LESF_CORE_PREPROCESSOR2_EVAL256(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL256(...) LESF_CORE_PREPROCESSOR2_EVAL128(LESF_CORE_PREPROCESSOR2_EVAL128(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL128(...) LESF_CORE_PREPROCESSOR2_EVAL64(LESF_CORE_PREPROCESSOR2_EVAL64(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL64(...) LESF_CORE_PREPROCESSOR2_EVAL32(LESF_CORE_PREPROCESSOR2_EVAL32(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL32(...) LESF_CORE_PREPROCESSOR2_EVAL16(LESF_CORE_PREPROCESSOR2_EVAL16(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL16(...) LESF_CORE_PREPROCESSOR2_EVAL8(LESF_CORE_PREPROCESSOR2_EVAL8(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL8(...) LESF_CORE_PREPROCESSOR2_EVAL4(LESF_CORE_PREPROCESSOR2_EVAL4(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL4(...) LESF_CORE_PREPROCESSOR2_EVAL2(LESF_CORE_PREPROCESSOR2_EVAL2(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL2(...) LESF_CORE_PREPROCESSOR2_EVAL1(LESF_CORE_PREPROCESSOR2_EVAL1(__VA_ARGS__))
#define LESF_CORE_PREPROCESSOR2_EVAL1(...) __VA_ARGS__

#define LESF_CORE_PREPROCESSOR2_DEFER1(m) m LESF_CORE_PREPROCESSOR2_EMPTY()
#define LESF_CORE_PREPROCESSOR2_DEFER2(m) m LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY()()
#define LESF_CORE_PREPROCESSOR2_DEFER3(m) m LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY()()()
#define LESF_CORE_PREPROCESSOR2_DEFER4(m) m LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY LESF_CORE_PREPROCESSOR2_EMPTY()()()()

#define LESF_CORE_PREPROCESSOR2_IS_PROBE(...) LESF_CORE_PREPROCESSOR2_SECOND(__VA_ARGS__, 0)
#define LESF_CORE_PREPROCESSOR2_PROBE() ~, 1

#define LESF_CORE_PREPROCESSOR2_CAT(a,b) a ## b

#define LESF_CORE_PREPROCESSOR2_NOT(x) LESF_CORE_PREPROCESSOR2_IS_PROBE(LESF_CORE_PREPROCESSOR2_CAT(LESF_CORE_PREPROCESSOR2__NOT_, x))
#define LESF_CORE_PREPROCESSOR2__NOT_0 LESF_CORE_PREPROCESSOR2_PROBE()

#define LESF_CORE_PREPROCESSOR2_BOOL(x) LESF_CORE_PREPROCESSOR2_NOT(LESF_CORE_PREPROCESSOR2_NOT(x))

#define LESF_CORE_PREPROCESSOR2_IF_ELSE(condition) LESF_CORE_PREPROCESSOR2__IF_ELSE(LESF_CORE_PREPROCESSOR2_BOOL(condition))
#define LESF_CORE_PREPROCESSOR2__IF_ELSE(condition) LESF_CORE_PREPROCESSOR2_CAT(LESF_CORE_PREPROCESSOR2__IF_, condition)

#define LESF_CORE_PREPROCESSOR2__IF_1(...) __VA_ARGS__ LESF_CORE_PREPROCESSOR2__IF_1_ELSE
#define LESF_CORE_PREPROCESSOR2__IF_0(...)             LESF_CORE_PREPROCESSOR2__IF_0_ELSE

#define LESF_CORE_PREPROCESSOR2__IF_1_ELSE(...)
#define LESF_CORE_PREPROCESSOR2__IF_0_ELSE(...) __VA_ARGS__

#define LESF_CORE_PREPROCESSOR2_HAS_ARGS(...) LESF_CORE_PREPROCESSOR2_BOOL(LESF_CORE_PREPROCESSOR2_FIRST(LESF_CORE_PREPROCESSOR2__END_OF_ARGUMENTS_ __VA_ARGS__)())
#define LESF_CORE_PREPROCESSOR2__END_OF_ARGUMENTS_() 0

#define LESF_CORE_PREPROCESSOR2_MAP(m, first, ...)           \
  m(first)                           \
  LESF_CORE_PREPROCESSOR2_IF_ELSE(LESF_CORE_PREPROCESSOR2_HAS_ARGS(__VA_ARGS__))(    \
    LESF_CORE_PREPROCESSOR2_DEFER2(LESF_CORE_PREPROCESSOR2__MAP)()(m, __VA_ARGS__)   \
  )(                                 \
    /* Do nothing, just terminate */ \
  )
#define LESF_CORE_PREPROCESSOR2__MAP() LESF_CORE_PREPROCESSOR2_MAP
