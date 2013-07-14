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

#ifndef DIRSCAN_H
#define DIRSCAN_H

#define PLUGIN_INTERFACE_LEVEL 3
#define INCL_BASE
#include <decoder_plug.h>
#include <cpp/xstring.h>
#include <cpp/container/vector.h>
#include <time.h>
#include <strutils.h>
#include <os2.h>
#include <stdlib.h>

class DirScan
{private:
  struct Entry
  { /// URL of the entry.
          char       Name[_MAX_FNAME]; // mutable, because lately initialized in the constructor.
    /// File Attributes
    const ULONG      Attributes;
    /// File size
    const PM123_SIZE Size;
    /// File modification time
    const int        Timestamp;
    /// Create Entry
    Entry(const char* name, size_t len, ULONG attr, PM123_SIZE size, int tstmp)
    : Attributes(attr), Size(size), Timestamp(tstmp) { ASSERT(len < _MAX_PATH); memcpy(Name, name, len); Name[len] = 0; }
  };
 private:
  // Parameters
  bool    Recursive;    ///< Scan sub directories recursively
  bool    AllFiles;     ///< Include all files regardless of \c Ctx.plugin_api->obj_supported
  bool    Hidden;       ///< Include hidden files
  xstring Pattern;      ///< Search pattern, for file manes only
  bool    FoldersFirst; ///< Return folders first
  // Working set
  /// Path to current item without any URL parameters.
  /// Points to the Folder at first, the sub items later.
  xstringbuilder Path;
  /// Pointer into URL starting a '?' or '\0' if no parameters.
  const   char* Params;
  /// Pointer into Path that skips "file://".
  char*   DosPath;
  /// Offset in Path after the trailing slash of the base folder.
  size_t  BasePathLen;
  /// Item container
  vector_own<Entry> Items;

 private:
  static time_t ConvertOS2FTime(FDATE date, FTIME time);
  /// Return true on "." or "..".
  static bool IsDot(const char* name, size_t len);
  static int FoldersFirstComparer(const DirScan::Entry* l, const DirScan::Entry* r);
 public:
  /// Step 1: initialize a new directory scanner
  DirScan() {}
  /// Step 2: use the following URL as base path
  /// @return PLUGIN_OK if the operation succeeds.
  PLUGIN_RC InitUrl(const char* url);
  /// Step 3: is also filled in case of an error.
  /// @return PLUGIN_OK if the operation succeeds.
  PLUGIN_RC GetInfo(const INFO_BUNDLE* info);
  /// Step 4: start the directory scan.
  void      Scan();
  /// Step 5: forward the results.
  int       SendResult(DECODER_INFO_ENUMERATION_CB cb, void* param);
};

#endif
