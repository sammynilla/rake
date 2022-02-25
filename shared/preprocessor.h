
#pragma once

#define Statement(s) do { s } while (0)

#define glue_(a,b) a##b
#define glue(a,b) glue_(a,b)
#define stringify_(x) #x
#define stringify(x) stringify_(x)

/* NOTE (sammynilla): Only works with arrays declared on the stack. */
#define ArrayCount(a) (sizeof(a)/sizeof((a)[0]))
