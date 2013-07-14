/*
 * Copyright 2010-2011 M.Mueller
 * Copyright 2008 Dmitry A.Steklenev
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

#ifndef EAUTILS_H
#define EAUTILS_H

#define INCL_BASE
#include <stdlib.h>
//#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _FEA2;
/** FILEFINDBUF3 for DosFindFirst/DosFindNext with FIL_QUERYEASFROMLIST */
typedef struct _FILEFINDBUF3FEA2LIST
{ // FILEFINDBUF3
  ULONG   oNextEntryOffset;
  FDATE   fdateCreation;
  FTIME   ftimeCreation;
  FDATE   fdateLastAccess;
  FTIME   ftimeLastAccess;
  FDATE   fdateLastWrite;
  FTIME   ftimeLastWrite;
  ULONG   cbFile;
  ULONG   cbFileAlloc;
  ULONG   attrFile;
  // FEA2LIST
  ULONG   cbList;
  FEA2    list[];
} FILEFINDBUF3FEA2LIST;

/** Copies extended attributes from one file or directory to another.
 * Attributes are added to a target file or replaced.
 */
APIRET eacopy(const char* source, const char* target);
/* Copies extended attributes from one file or directory to another.
 * Attributes are added to a target file or replaced.
 * The source file must be opened with read access and deny write access.
 */
//APIRET eacopy2( HFILE source, const char* target );

/** Reads the value of the extended attribute eaname from file and returns it in eadata.
 * You must free eadata when you don't need it anymore.
 * The length of eadata is returned in easize. 
 */
APIRET eaget(const char* file, const char* eaname, char** eadata, size_t* easize);

/** Decodes EAT_ASCII, EAT_MVST or EAT_MVMT into a tab separated list of strings.
 * Other EA types are ignored.
 * @param dst target string, appended!
 * @param len target string length including terminating \0
 * @param eadata EA data
 * @return true: everything OK, false: *dst too small, EAs truncated.
 * @remarks The function will not return partial type strings if *dst is too small.
 */
BOOL eadecode(char* dst, size_t len, const struct _FEA2* eadata);

#ifdef __cplusplus
}
#endif
#endif
