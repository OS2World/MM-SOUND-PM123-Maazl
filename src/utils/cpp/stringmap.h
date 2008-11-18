/*
 * Copyright 2008-2008 M.Mueller
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

#ifndef STRMAP_H
#define STRMAP_H

#include <cpp/xstring.h>
#include <cpp/container/sorted_vector.h>


/* Element class to provide a string repository with a sorted_vector<> as storage container.
 * This class should be extended to provide a mapping to a target type.
 */
struct strkey : public IComparableTo<xstring>
{ const xstring Key;
  strkey(const xstring& key) : Key(key) {}
  virtual int compareTo(const xstring& key) const;
};

typedef sorted_vector<strkey, xstring> stringset;

class stringset_own : public stringset
{public:
  stringset_own(size_t capacity) : stringset(capacity) {}
  ~stringset_own();
};

template <class V>
struct strmapentry : public strkey
{ V Value;
  strmapentry(const xstring& key, const V& value) : strkey(key), Value(value) {}
};

typedef strmapentry<xstring> stringmapentry;
typedef sorted_vector<stringmapentry, xstring> stringmap;

class stringmap_own : public stringmap
{public:
  stringmap_own(size_t capacity) : stringmap(capacity) {}
  ~stringmap_own();
};

#endif

