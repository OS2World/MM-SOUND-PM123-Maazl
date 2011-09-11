/* * Copyright 2006-2007 Marcel Mueller
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Utility factories to compile easy function adaptors in the data segment.
 * They require a platform with a IA-32 compatible instruction set and a flat
 * memory model to operate. Compiler or operating sytem is not significant.
 *
 * - Virtual .NET like delegates from function pointer.
 * - Replacer to replace objects in of member-function like functions.
 *
 * These implementations are very performant and can be altered at run time.
 * The VDELEGATE pattern cannot be solved in any other, more portable way.
 *
 * Type safe access to these function is possible through a C++ wrapper.
 * This is not part of this file.
 */

#ifndef VDELEGATE_H
#define VDELEGATE_H

// for DLLENTRYP
#include <config.h>

#define VDELEGATE_LEN 0x1B
#define VREPLACE1_LEN 0x0E
#ifdef __cplusplus
class VDELEGATE
{ unsigned char buffer[VDELEGATE_LEN];
  // non-copyable
  VDELEGATE(const VDELEGATE&);
  void operator=(const VDELEGATE&);
 public:
  VDELEGATE() {}
};
class VREPLACE1
{ unsigned char buffer[VREPLACE1_LEN];
  // non-copyable
  VREPLACE1(const VREPLACE1&);
  void operator=(const VREPLACE1&);
 public:
  VREPLACE1() {}
};

extern "C" {
#else
/* YOU MUST NOT COPY OBJECTS OF THIS TYPE. They are not POD like. */
typedef unsigned char VDELEGATE[VDELEGATE_LEN];
/* YOU MUST NOT COPY OBJECTS OF THIS TYPE. They are not POD like. */
typedef unsigned char VREPLACE1[VREPLACE1_LEN];
#endif

typedef int DLLENTRYP(V_FUNC)();

/** Generate proxy function which prepends an additional parameter at each function call.
 * @param dg    VDELEGATE structure. The lifetime of this structure must exceed the lifetime
 *              of the returned function pointer.
 * @param func  function to call. The function must have count +1 arguments.
 * @param count number of arguments of the returned function.
 * @param ptr   pointer to pass as first argument to func.
 * @return      generated function with count arguments.
 * @remarks Once created the returned function is thread-safe and reentrant as far as the called
 * function is. Updating an existing VDELEGATE object is generally not thread-safe.
 */
V_FUNC mkvdelegate(VDELEGATE* dg, V_FUNC func, int count, void* ptr);

/** Generate proxy function that replaces the first argument of the function call.
 * @param dg     VDELEGATE structure. The lifetime of this structure must exceed the lifetime
 *               of the returned function pointer.
 * @param func   function to call.
 * @param ptr    pointer to pass as first argument to func.
 * @return       generated function.
 * @remarks Once created the returned function is thread-safe and reentrant as far as the called
 * function is. Updating an existing VREPLACE1 object is generally not thread-safe.
 */
V_FUNC mkvreplace1(VREPLACE1* rp, V_FUNC func, void* ptr);

#ifdef __cplusplus
}
#endif
#endif

