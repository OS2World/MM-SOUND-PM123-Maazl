/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
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

#ifndef FILEFIND_H
#define FILEFIND_H

#ifdef __cplusplus
extern "C" {
#endif

/* Attribute value that determines the file objects to be searched for. */
#define FIND_DIRECTORY MUST_HAVE_DIRECTORY | FILE_DIRECTORY
#define FIND_FILE      FILE_NORMAL
#define FIND_ALL       FILE_NORMAL | FILE_DIRECTORY

/* Finds the first file whose name match the specification. */
ULONG findfirst( HDIR* hdir, char* path, ULONG attributes, FILEFINDBUF3* buf );
/* Finds the next file whose name match the specification in a previous call
   to findfirst or findnext. */
ULONG findnext ( HDIR  hdir, FILEFINDBUF3* buf );
/* Closes the handle to a find request; that is, ends a search. */
ULONG findclose( HDIR  hdir );

#ifdef __cplusplus
}
#endif

#endif /* FILEFIND_H */
