/* This file is part of libesf.
 * 
 * Copyright (c) 2019, Alexandre Monti
 * 
 * libesf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libesf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with libesf.  If not, see <http://www.gnu.org/licenses/>.
 */

// This black magic fuckery implements a very nice map() function for macros
// Courtesy of http://jhnet.co.uk/articles/cpp_magic

#define LESF_IPC_MAGIC_FIRST(a, ...) a
#define LESF_IPC_MAGIC_SECOND(a, b, ...) b

#define LESF_IPC_MAGIC_EMPTY()

#define LESF_IPC_MAGIC_EVAL(...) LESF_IPC_MAGIC_EVAL1024(__VA_ARGS__)
#define LESF_IPC_MAGIC_EVAL1024(...) LESF_IPC_MAGIC_EVAL512(LESF_IPC_MAGIC_EVAL512(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL512(...) LESF_IPC_MAGIC_EVAL256(LESF_IPC_MAGIC_EVAL256(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL256(...) LESF_IPC_MAGIC_EVAL128(LESF_IPC_MAGIC_EVAL128(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL128(...) LESF_IPC_MAGIC_EVAL64(LESF_IPC_MAGIC_EVAL64(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL64(...) LESF_IPC_MAGIC_EVAL32(LESF_IPC_MAGIC_EVAL32(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL32(...) LESF_IPC_MAGIC_EVAL16(LESF_IPC_MAGIC_EVAL16(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL16(...) LESF_IPC_MAGIC_EVAL8(LESF_IPC_MAGIC_EVAL8(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL8(...) LESF_IPC_MAGIC_EVAL4(LESF_IPC_MAGIC_EVAL4(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL4(...) LESF_IPC_MAGIC_EVAL2(LESF_IPC_MAGIC_EVAL2(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL2(...) LESF_IPC_MAGIC_EVAL1(LESF_IPC_MAGIC_EVAL1(__VA_ARGS__))
#define LESF_IPC_MAGIC_EVAL1(...) __VA_ARGS__

#define LESF_IPC_MAGIC_DEFER1(m) m LESF_IPC_MAGIC_EMPTY()
#define LESF_IPC_MAGIC_DEFER2(m) m LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY()()
#define LESF_IPC_MAGIC_DEFER3(m) m LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY()()()
#define LESF_IPC_MAGIC_DEFER4(m) m LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY LESF_IPC_MAGIC_EMPTY()()()()

#define LESF_IPC_MAGIC_IS_PROBE(...) LESF_IPC_MAGIC_SECOND(__VA_ARGS__, 0)
#define LESF_IPC_MAGIC_PROBE() ~, 1

#define LESF_IPC_MAGIC_CAT(a,b) a ## b

#define LESF_IPC_MAGIC_NOT(x) LESF_IPC_MAGIC_IS_PROBE(LESF_IPC_MAGIC_CAT(LESF_IPC_MAGIC__NOT_, x))
#define LESF_IPC_MAGIC__NOT_0 LESF_IPC_MAGIC_PROBE()

#define LESF_IPC_MAGIC_BOOL(x) LESF_IPC_MAGIC_NOT(LESF_IPC_MAGIC_NOT(x))

#define LESF_IPC_MAGIC_IF_ELSE(condition) LESF_IPC_MAGIC__IF_ELSE(LESF_IPC_MAGIC_BOOL(condition))
#define LESF_IPC_MAGIC__IF_ELSE(condition) LESF_IPC_MAGIC_CAT(LESF_IPC_MAGIC__IF_, condition)

#define LESF_IPC_MAGIC__IF_1(...) __VA_ARGS__ LESF_IPC_MAGIC__IF_1_ELSE
#define LESF_IPC_MAGIC__IF_0(...)             LESF_IPC_MAGIC__IF_0_ELSE

#define LESF_IPC_MAGIC__IF_1_ELSE(...)
#define LESF_IPC_MAGIC__IF_0_ELSE(...) __VA_ARGS__

#define LESF_IPC_MAGIC_HAS_ARGS(...) LESF_IPC_MAGIC_BOOL(LESF_IPC_MAGIC_FIRST(LESF_IPC_MAGIC__END_OF_ARGUMENTS_ __VA_ARGS__)())
#define LESF_IPC_MAGIC__END_OF_ARGUMENTS_() 0

#define LESF_IPC_MAGIC_MAP(m, first, ...)           \
  m(first)                           \
  LESF_IPC_MAGIC_IF_ELSE(LESF_IPC_MAGIC_HAS_ARGS(__VA_ARGS__))(    \
    LESF_IPC_MAGIC_DEFER2(LESF_IPC_MAGIC__MAP)()(m, __VA_ARGS__)   \
  )(                                 \
    /* Do nothing, just terminate */ \
  )
#define LESF_IPC_MAGIC__MAP() LESF_IPC_MAGIC_MAP
  