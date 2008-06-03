/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 *
 * Copyright 2006 Dmitry A.Steklenev <glass@ptv.ru>
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

#include "strutils.h"
#include <string.h>

size_t memcmpcpy( void* dst, const void* src, size_t len )
{ size_t ret = len;
  // work in units of 4 Bytes
  while (len >= sizeof(long))
  { if (*(long*)dst != *(long*)src)
    { // determine exact location of the difference
      if (((char*)dst)[0] == ((char*)src)[0])
      { ++ret;
        if (((char*)dst)[1] == ((char*)src)[1])
        { ++ret;
          if (((char*)dst)[2] == ((char*)src)[2])
            ++ret;
        }
      }
      memcpy(dst, src, len);
      return ret - len;
    }
    len -= sizeof(long);
    // These lines give warnings in gcc, but otherwise we can't increment the pointers correctly.
    ++*(long**)&src;
    ++*(long**)&dst;
  }
  // remaining part
  while (len)
  { if (*(char*)dst != *(char*)src)
    { ret -= len;
      do
      { *(*(char**)&dst)++ = *(*(char**)&src)++;
      } while (--len);
      return ret;
    }
    --len;
    // These lines give warnings in gcc, but otherwise we can't increment the pointers correctly.
    ++*(char**)&src;
    ++*(char**)&dst;
  }
  return ~0;
}
