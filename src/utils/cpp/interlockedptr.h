/*
 * Copyright 2011-2011 M.Mueller
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


#ifndef CPP_INTERLOCKEDPTR_H
#define CPP_INTERLOCKEDPTR_H

#include <interlocked.h>


/*****************************************************************************
*
*  Primitive functions to do atomic pointer operations.
*
*****************************************************************************/

/// Exchange pointer. Replaces *dst by src and return the old value of *dst.
template <class T>
inline T* InterlockedXch(T*volatile* dst, T* src)
{ return (T*)InterlockedXch((volatile unsigned*)dst, (unsigned)src);
}
/// Interlocked compare exchange. Replace *dst by new if *dst == old and
/// return the old value.
template <class T>
inline T* InterlockedCxc(T*volatile* dst, T* oldval, T* newval)
{ return (T*)InterlockedCxc((volatile unsigned*)dst, (unsigned)oldval, (unsigned)newval);
}
#endif
