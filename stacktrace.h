
#pragma once

#include "shared/preprocessor.h"

/* Runtime assertions */
#define AssertBreak (*((int*)0) = 0)
#define AssertCheck(Predicate) \
  Statement( if (!(Predicate)) { AssertBreak; } )

#if defined(_MSC_VER)
/* NOTE (sammynilla): Disables 'conditional expression is constant' warning 
 * This warning only seems to pop up when compiling as C code and is only
 * flagging as a warning because of AssertCheck's boolean predicate.
 */
# define Assert(Predicate) \
  __pragma(warning(push)) \
  __pragma(warning(disable:4127)) \
  AssertCheck(Predicate) \
  __pragma(warning(pop))
#else
# define Assert(Predicate) AssertCheck(Predicate)
#endif
