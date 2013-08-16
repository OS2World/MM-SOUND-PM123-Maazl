/*
 * Copyright 2007-2012 M.Mueller
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


#define INCL_PM
#define INCL_BASE
#include "playable.h"
#include "../eventhandler.h"
#include "../engine/decoder.h"
#include "job.h"
#include "location.h"
#include "waitinfo.h"
#include <strutils.h>
#include <utilfct.h>
#include <fileutil.h> // is_cdda
#include <eautils.h>
#include <vdelegate.h>
#include <xio.h>
#include <cpp/algorithm.h>
#include <string.h>
#include <stdio.h>

#include <debuglog.h>

#ifdef DEBUG_LOG
#define RP_INITIAL_SIZE 5 // Errors come sooner...
#else
#define RP_INITIAL_SIZE 100
#endif


/****************************************************************************
*
*  class Playable
*
****************************************************************************/

Playable::Entry::Entry(Playable& parent, APlayable& refto, IDType::func_type ifn)
// The implementation always refers to the underlying playable object.
// But the overridable properties are copied.
: PlayableInstance(parent, refto.GetPlayable())
, InstDelegate(parent, ifn)
{ DEBUGLOG(("Playable::Entry(%p)::Entry(&%p, &%p, &%p)\n", this, &parent, &refto, &ifn));
  InfoFlags valid = ~refto.RequestInfo(IF_Meta|IF_Attr|IF_Item, PRI_None, REL_Cached);
  const INFO_BUNDLE_CV& ref_info = refto.GetInfo();
  const INFO_BUNDLE_CV& base_info = GetInfo();
  // Meta info valid and not instance equal.
  if ((valid & IF_Meta) && ref_info.meta != base_info.meta)
  { MetaInfo meta(*ref_info.meta);
    if (meta != MetaInfo(*base_info.meta))
      OverrideMeta(&meta);
  }
  // Attr info valid and not instance equal.
  if ((valid & IF_Attr) && ref_info.attr != base_info.attr)
  { AttrInfo attr(*ref_info.attr);
    if (attr != AttrInfo(*base_info.attr))
      OverrideAttr(&attr);
  }
  // Item info valid and not instance equal.
  if ((valid & IF_Attr) && ref_info.attr != base_info.attr)
  { ItemInfo item(*ref_info.item);
    if (!item.IsInitial())
      OverrideItem(&item);
  }
}

const ItemInfo Playable::MyInfo::Item;

Playable::MyInfo::MyInfo()
: CollectionInfo(PlayableSet::Empty, IF_Item|IF_Slice|IF_Display|IF_Usage)
{ phys = &Phys;
  tech = &Tech;
  obj  = &Obj;
  meta = &Meta;
  attr = &Attr;
  rpl  = &Rpl;
  drpl = &Drpl;
  item = &Item;
}

/*Playable::Lock::~Lock()
{ if (P.Mtx.GetStatus() == 1)
    P.OnReleaseMutex();
  P.Mtx.Release();
}*/


Playable::Playable(const url123& url)
: URL(url),
  Modified(false),
  LastAccess(clock())
{ DEBUGLOG(("Playable(%p)::Playable(%s)\n", this, url.cdata()));
}

Playable::~Playable()
{ DEBUGLOG(("Playable(%p{%s})::~Playable()\n", this, URL.cdata()));
  // Revoke child subscriptions.
  if (Playlist)
  { Entry* ep = NULL;
    while ((ep = Playlist->Items.next(ep)) != NULL)
      ep->Detach();
  }
  // Notify about dying
  CollectionChangeArgs args(*this);
  RaiseInfoChange(args);
  // No more events.
  GetInfoChange().reset();
  GetInfoChange().sync();
}

void Playable::SetAlias(const xstring& alias)
{ ItemInfo* ip = new ItemInfo();
  ip->alias = alias;
  Info.SetItem(ip);
}

const INFO_BUNDLE_CV& Playable::GetInfo() const
{ return Info;
}

xstring Playable::DoDebugName() const
{ return URL.getShortName();
}

void Playable::SetInUse(unsigned used)
{ DEBUGLOG(("Playable(%p{%s})::SetInUse(%u)\n", this, DebugName().cdata(), used));
  Mutex::Lock lock(Mtx);
  // TODO: keep origin in case of cascaded execution
  CollectionChangeArgs args(*this, this, IF_Usage, IF_None, IF_None);
  if (InUse != used)
  { InUse = used;
    args.Changed |= IF_Usage;
  }
  RaiseInfoChange(args);
}

void Playable::SetModified(bool modified, APlayable* origin)
{ DEBUGLOG(("Playable(%p{%s})::SetModified(%u)\n", this, DebugName().cdata(), modified));
  Mutex::Lock lock(Mtx);
  bool changed = Modified != modified;
  Modified = modified;
  CollectionChangeArgs args(*this, origin, IF_Usage, changed * IF_Usage, IF_None);
  RaiseInfoChange(args);
}

xstring Playable::GetDisplayName() const
{ xstring ret(Info.item->alias);
  if (!ret || !ret[0U])
  { ret = Info.meta->title;
    if (!ret || !ret[0U])
      ret = URL.getShortName();
  }
  return ret;
}

InfoFlags Playable::Invalidate(InfoFlags what, const Playable* source)
{ DEBUGLOG(("Playable(%p)::Invalidate(%x, %p)\n", this, what, source));
  InfoFlags inval = Info.InfoStat.Invalidate(what);
  if ((what & IF_Aggreg) && Playlist)
    // Invalidate CIC entries.
    inval |= Playlist->Invalidate(what, source);
  if (inval)
  { Mutex::Lock lock(Mtx);
    CollectionChangeArgs args(*this, this, IF_None, IF_None, inval);
    RaiseInfoChange(args);
  }
  return inval;
}

void Playable::PeekRequest(RequestState& req) const
{ DEBUGLOG(("Playable(%p)::PeekRequest({%x,%x, %x})\n", this, req.ReqLow, req.ReqHigh, req.InService));
  Info.InfoStat.PeekRequest(req);
  const CollectionInfoCache* cic = Playlist.get();
  if (cic)
    cic->PeekRequest(req);
  DEBUGLOG(("Playable::PeekRequest: {%x,%x, %x}\n", req.ReqLow, req.ReqHigh, req.InService));
}

void Playable::SetCachedInfo(const INFO_BUNDLE& info, InfoFlags cached, InfoFlags reliable)
{ DEBUGLOG(("Playable(%p)::SetCachedInfo(&%p, %x, %x)\n", this, &info, cached, reliable));
  reliable &= ~cached;
  cached &= ~reliable;
  if (!reliable)
    // Avoid unnecessary locks.
    cached &= Info.InfoStat.Check(cached, REL_Invalid);
  if (cached | reliable)
  { Mutex::Lock lock(Mtx);
    cached &= Info.InfoStat.Check(cached, REL_Invalid); // double check
    reliable = Info.InfoStat.BeginUpdate(reliable);
    InfoFlags changed = UpdateInfo(info, cached|reliable);
    changed &= Info.InfoStat.Cache(cached) | reliable;
    reliable = Info.InfoStat.EndUpdate(reliable);
    if (reliable|changed)
    { CollectionChangeArgs args(*this, reliable, changed);
      RaiseInfoChange(args);
    }
  }
}

int_ptr<PlayableInstance> Playable::GetPrev(const PlayableInstance* cur) const
{ DEBUGLOG(("Playable(%p)::GetPrev(%p)\n", this, cur));
  ASSERT(!((Playable*)this)->RequestInfo(IF_Child, PRI_None, REL_Invalid));
  ASSERT(cur == NULL || cur->HasParent(NULL) || cur->HasParent(this));
  if (!Playlist || (Info.Tech.attributes & TATTR_PLAYLIST) == 0)
    return NULL;
  return Playlist->Items.prev((Entry*)cur);
}

int_ptr<PlayableInstance> Playable::GetNext(const PlayableInstance* cur) const
{ DEBUGLOG(("Playable(%p)::GetNext(%p)\n", this, cur));
  ASSERT(!((Playable*)this)->RequestInfo(IF_Child, PRI_None, REL_Invalid));
  ASSERT(cur == NULL || cur->HasParent(NULL) || cur->HasParent(this));
  if (!Playlist || (Info.Tech.attributes & TATTR_PLAYLIST) == 0)
    return NULL;
  return Playlist->Items.next((Entry*)cur);
}


static inline void SetDependentInfo(InfoFlags& what)
{ if (what & IF_Meta)
    what |= IF_Display;
}

void Playable::RaiseInfoChange(CollectionChangeArgs& args)
{ SetDependentInfo(args.Loaded);
  SetDependentInfo(args.Changed);
  SetDependentInfo(args.Invalidated);
  APlayable::RaiseInfoChange(args);
}

InfoFlags Playable::UpdateInfo(const INFO_BUNDLE& info, InfoFlags what)
{ DEBUGLOG(("Playable(%p)::UpdateInfo(%p&, %x)\n", this, &info, what));
  ASSERT(Mtx.GetStatus() > 0);
  // set new values
  InfoFlags ret = IF_None;
  if (what & IF_Phys)
    ret |= IF_Phys * Info.Phys.CmpAssign(*info.phys);
  if (what & IF_Tech)
    ret |= IF_Tech * Info.Tech.CmpAssign(*info.tech);
  if (what & IF_Obj)
    ret |= IF_Obj  * Info.Obj .CmpAssign(*info.obj );
  if (what & IF_Meta)
    ret |= IF_Meta * Info.Meta.CmpAssign(*info.meta);
  if (what & IF_Attr)
    ret |= IF_Attr * Info.Attr.CmpAssign(*info.attr);
  if (what & IF_Rpl)
    ret |= IF_Rpl  * Info.Rpl. CmpAssign(*info.rpl );
  if (what & IF_Drpl)
    ret |= IF_Drpl * Info.Drpl.CmpAssign(*info.drpl);
  // deduplicate strings
  if (ret & (IF_Tech|IF_Meta))
  { xstring::deduplicator dedup;
    if (what & IF_Tech)
      Info.Tech.Deduplicate(dedup);
    if (what & IF_Meta)
      Info.Meta.Deduplicate(dedup);
  }
  return ret;
}

InfoFlags Playable::DoRequestInfo(InfoFlags& what, Priority pri, Reliability rel)
{ DEBUGLOG(("Playable(%p)::DoRequestInfo(%x&, %x, %u)\n", this, what, pri, rel));

  if (what & IF_Drpl)
    what |= IF_Phys|IF_Tech|IF_Obj|IF_Child; // required for DRPL_INFO aggregate
  else if (what & (IF_Rpl|IF_Child))
    what |= IF_Phys|IF_Tech|IF_Child; // required for RPL_INFO aggregate
  if (what & IF_Display)
    what |= IF_Item|IF_Meta; // required for GetDisplayName, but item info will be masked out below.

  // Mask info's that are always available
  what &= IF_Decoder|IF_Aggreg;

  what &= Info.InfoStat.Check(what, rel);
  InfoFlags op = pri == PRI_None || what == IF_None
    ? IF_None : Info.InfoStat.Request(what, pri);
  DEBUGLOG(("Playable::DoRequestInfo(%x): %x\n", what, op));
  return op;
}

/*AggregateInfo& Playable::DoAILookup(const PlayableSetBase& exclude)
{ DEBUGLOG(("Playable(%p)::DoAILookup({%u,})\n", this, exclude.size()));
  if ((Info.Tech.attributes & (TATTR_PLAYLIST|TATTR_SONG|TATTR_INVALID)) == TATTR_PLAYLIST)
  { // Playlist
    if (!Playlist) // Fastpath
    { Mutex::Lock lock(Mtx);
      EnsurePlaylist();
    }
    CollectionInfo* ci = Playlist->Lookup(exclude);
    if (ci)
      return *ci;
  }
  // noexclusion, invalid or unknown item
  return Info;
}*/

InfoFlags Playable::DoRequestAI(const PlayableSetBase& exclude, const volatile AggregateInfo*& ai, InfoFlags& what, Priority pri, Reliability rel)
{ DEBUGLOG(("Playable(%p)::DoRequestAI({%s},, %x&, %x, %d)\n", this, exclude.DebugDump(), what, pri, rel));
  ASSERT((what & ~IF_Aggreg) == 0);
  if ((Info.Tech.attributes & (TATTR_PLAYLIST|TATTR_SONG|TATTR_INVALID)) == TATTR_PLAYLIST)
  { // Playlist
    if (!Playlist) // Fastpath
    { Mutex::Lock lock(Mtx);
      EnsurePlaylist();
    }
    ai = Playlist->Lookup(exclude);
    if (!ai)
      goto definfo;
  } else
  {definfo:
    ai = &Info;
  }
  PASSERT(ai);

  if (!what)
    return IF_None; // fast path

  InfoFlags what2 = IF_None;
  if (what & IF_Drpl)
    what2 = IF_Phys|IF_Tech|IF_Obj|IF_Child; // required for DRPL_INFO aggregate
  else if (what & IF_Rpl)
    what2 = IF_Phys|IF_Tech|IF_Child; // required for RPL_INFO aggregate
  what2 = DoRequestInfo(what2, pri, rel);
  what2 |= ((CollectionInfo*)ai)->RequestAI(what, pri, rel);
  DEBUGLOG(("Playable::DoRequestAI(,%x&,) : %x\n", what, what2));
  return what2;
}

bool Playable::CalcRplInfo(AggregateInfo& cie, InfoState::Update& upd, PlayableChangeArgs& events, JobSet& job)
{ DEBUGLOG(("Playable(%p)::CalcRplInfo(, {%x}, , {%x,})\n", this, upd.GetWhat(), job.Pri));
  PASSERT(this);
  // The entire implementation of this function is basically lock-free.
  // This means that the result may not be valid after the function finished.
  // This is addressed by invalidating the rpl bits of cie.InfoStat by other threads.
  // When this happens the commit at the end of the function will not set the state to valid
  // and the function will return true.
  // The calculation is not done in place to avoid visibility of intermediate results.
  InfoFlags whatok = upd & IF_Aggreg;
  if (whatok == IF_None)
    return IF_None; // Nothing to do

  // Calculate exclusion list including this
  PlayableSet excluding(cie.Exclude.size() + 5); // Leave some space
  for (size_t i = 0; i < cie.Exclude.size(); ++i)
    excluding.append() = cie.Exclude[i]; // At this point the sort order is implied.
  excluding.add(*this);

  AggregateInfo ai(excluding);
  ai.Rpl.lists = 1; // At least one list: our own.

  // Iterate over children
  int_ptr<PlayableInstance> pi;
  while ((pi = GetNext(pi)) != NULL)
  { // Skip exclusion list entries to avoid recursion.
    if (excluding.contains(pi->GetPlayable()))
    { DEBUGLOG(("Playable::CalcRplInfo - recursive: %p->%p!\n", pi.get(), &pi->GetPlayable()));
      continue;
    }
    InfoFlags what2 = upd & IF_Aggreg;
    const volatile AggregateInfo& lai = job.RequestAggregateInfo(*pi, excluding, what2);
    DEBUGLOG(("Playable(%p)::CalcRplInfo - got sub info: %p %x\n", this, &ai, what2));
    whatok &= ~what2;
    // TODO: Increment unk_* counters instead of ignoring incomplete subitems?
    ai.Add(lai, whatok);
    // Alternation lists: stop at the first successful item.
    if ((Info.Attr.ploptions & PLO_ALTERNATION) && whatok && !(pi->GetInfo().tech->attributes & TATTR_INVALID))
      break;
  }
  bool delayed = job.Commit();
  DEBUGLOG(("Playable::CalcRplInfo: %x, RPL{%i, %i, %i, %i}, DRPL{%f, %i, %f, %i}\n", whatok,
    ai.Rpl.songs, ai.Rpl.lists, ai.Rpl.invalid, ai.Rpl.unknown, ai.Drpl.totallength, ai.Drpl.unk_length, ai.Drpl.totalsize, ai.Drpl.unk_size));
  // Update results

  upd.Rollback(~whatok & IF_Aggreg);
  InfoFlags whatdone = IF_None;
  { Mutex::Lock lock(Mtx);
    if (whatok & IF_Rpl)
      events.Changed |= IF_Rpl * cie.Rpl.CmpAssign(ai.Rpl);
    if (whatok & IF_Drpl)
      events.Changed |= IF_Drpl * cie.Drpl.CmpAssign(ai.Drpl);
    whatdone = upd.Commit(whatok);
  }
  events.Loaded |= whatdone;
  // Everything committed successfully?
  return (~whatdone & whatok) && !delayed;
}

struct deccbdata
{ Playable&                    Parent;
  vector_int<APlayable>        Children;
  deccbdata(Playable& parent) : Parent(parent) {}
  ~deccbdata() {}
};

void Playable::DoLoadInfo(JobSet& job)
{ DEBUGLOG(("Playable(%p{%s})::DoLoadInfo({%x,})\n", this, DebugName().cdata(), job.Pri));
  PASSERT(this);
  InfoState::Update upd(Info.InfoStat, job.Pri);
  DEBUGLOG(("Playable::DoLoadInfo: update %x\n", upd.GetWhat()));
  // There must not be outstanding requests on informations that cause a no-op.
  // DoRequestInfo should ensure this.
  ASSERT((upd & ~(IF_Decoder|IF_Aggreg)) == IF_None);

  // get information
  InfoBundle info;
  { CollectionChangeArgs args(*this);
   retry1:
    InfoFlags what2 = upd & IF_Decoder; // do not request RPL info from the decoder.
    if (what2)
    { // Keep some tech infos
      info.Tech.samplerate = Info.Tech.samplerate;
      info.Tech.channels   = Info.Tech.channels;
      info.Tech.format     = Info.Tech.format;
      info.Tech.decoder    = Info.Tech.decoder;
      deccbdata children(*this);
      int rc = DecoderFileInfo(what2, info, &children);
      upd.Extend(what2);
      DEBUGLOG(("Playable::DoLoadInfo - rc = %i, what2 = %x\n", rc, what2));

      // Update information, but only if still in service.
      Mutex::Lock lock(Mtx);
      args.Changed |= UpdateInfo(info, what2);
      // update children
      if ((upd & IF_Child) && (Info.Tech.attributes & TATTR_PLAYLIST))
      { EnsurePlaylist();
        if (UpdateCollection(children.Children))
        { args.Changed |= IF_Child;
          args.Type = PCT_All;
        }
        if (Modified)
        { // Well, the playlist content is currently the only thing that can be modified.
          // So reset the flag on reload.
          Modified = false;
          args.Changed |= IF_Usage;
        }
      }
      // Raise the first bunch of change events.
      args.Loaded |= upd.Commit(what2);
      if (~args.Loaded & what2)
      { upd.Reset(Info.InfoStat, job.Pri);
        goto retry1; // Information got invalidated during this step.
      }
      if (!args.IsInitial())
        RaiseInfoChange(args);
    }
    else if (!args.IsInitial())
    { Mutex::Lock lock(Mtx);
      RaiseInfoChange(args);
    }
  }

  // The required basic information to calculate aggregate infos might be on the way by another thread.
  // In this case we have either to give up and schedule a dependency to ourself.
  { InfoFlags what2 = upd & IF_Drpl ? IF_Phys|IF_Tech|IF_Obj|IF_Child : IF_Phys|IF_Tech|IF_Child;
    if (upd)
    { // Aggregate info explicitly requested
      if (job.RequestInfo(*this, what2))
      { job.Commit();
        return;
      }
    } else
    { // Aggregate information is not requested and should only be calculated if cheap.
      if (Info.InfoStat.Check(what2, REL_Cached))
        return;
    }
  }

  CollectionChangeArgs args(*this);
 retry2:
  if (Info.Tech.attributes & TATTR_INVALID)
  { // Always render aggregate info of invalid items, because this is cheap.
    info.Rpl.invalid = 1;
    /* Invalid items do not count
    if (Info.Phys.filesize > 0)
      info.Drpl.totalsize += Info.Phys.filesize;*/
    upd.Extend(IF_Aggreg);

  } else if ((Info.Tech.attributes & (TATTR_SONG|TATTR_PLAYLIST|TATTR_INVALID)) == TATTR_PLAYLIST)
  { // handle aggregate information of playlist items
    // For the event below
    if (upd)
      // Request for default recursive playlist information (without exclusions)
      // is to be completed.
      while (CalcRplInfo(Info, upd, args, job))
        upd.Reset(Info.InfoStat, job.Pri);
    // Request Infos with exclusion lists
    for (CollectionInfo* iep = NULL; Playlist->GetNextWorkItem(iep, job.Pri, upd);)
      // retrieve information
      while (CalcRplInfo(*iep, upd, args, job))
        upd.Reset(iep->InfoStat, job.Pri);

    // Raise event if any
    if (!args.IsInitial())
    { Mutex::Lock lock(Mtx);
      RaiseInfoChange(args);
    }
    // done
    return;

  } else
  { // Song items => work only if requested
    // Calculate RPL_INFO if not already done by dec_fileinfo.
    if ((Info.Tech.attributes & (TATTR_SONG|TATTR_PLAYLIST)) == TATTR_SONG)
    { // Always calculate RPL info of song items, because this is cheap.
      info.Rpl.songs = 1;
      // ... the same does not apply to DRPL info.
      if (!Info.InfoStat.Check(IF_Obj, REL_Invalid))
      { // IF_Obj is available
        upd.Extend(IF_Aggreg);
        info.Drpl.totallength = Info.Obj.songlength;
        if (info.Drpl.totallength < 0)
        { info.Drpl.totallength = 0;
          info.Drpl.unk_length  = 1;
        }
        info.Drpl.totalsize   = Info.Phys.filesize;
        if (info.Drpl.totallength < 0)
        { info.Drpl.totallength = 0;
          info.Drpl.unk_size    = 1;
        }
      } else
        // IF_Obj is not available
        upd.Extend(IF_Rpl);
    } else
    { // Cannot automatically calculate RPL info of hybrid items.
      info.Rpl.unknown = 1;
      info.Drpl.unk_length = 1;
      info.Drpl.unk_size = 1;
    }
    // nothing done
    if (!upd)
      return;
  }
  // update information
  Mutex::Lock lock(Mtx);
  InfoFlags what2 = upd;
  args.Changed = UpdateInfo(info, what2);
  args.Loaded = upd.Commit(IF_Aggreg);
  if (~args.Loaded & what2)
  { // Invalidation happened
    upd.Reset(Info.InfoStat, job.Pri);
    goto retry2;
  }
  // Raise event
  if (!args.IsInitial())
    RaiseInfoChange(args);
}

struct StreamHelper
{public:
  const char* const URL;
  XFILE* Handle;
  int    Error;
 public:
  StreamHelper(const char* url);
  ~StreamHelper() { if (Handle) xio_fclose(Handle); }
  XFILE* GetHandle() const { return Handle; }
  bool   Reset();
};

StreamHelper::StreamHelper(const char* url)
: URL(url)
, Handle(xio_fopen(url, "rbU"))
{ Error = Handle ? 0 : xio_errno();
}

bool StreamHelper::Reset()
{ if (Handle && xio_rewind(Handle))
  { // Try to recover from seek error.
    DEBUGLOG(("StreamHelper::Reset Buffer overrun for %s (at %li)\n", URL, xio_ftell(Handle)));
    xio_fclose(Handle);
    Handle = xio_fopen(URL, "rbU");
    Error = Handle ? 0 : xio_errno();
    if (Handle == NULL)
      return false;
  }
  return true;
}

ULONG Playable::DecoderFileInfo(InfoFlags& what, INFO_BUNDLE& info, void* param)
{ DEBUGLOG(("Playable(%p{%s})::DecoderFileInfo(%x&, {...}, %p)\n", this, DebugName().cdata(), what, param));
  ASSERT(what);
  int_ptr<PluginList> decoders(Plugin::GetPluginList(PLUGIN_DECODER));
  bool* checked = (bool*)alloca(sizeof(bool) * decoders->size());
  memset(checked, 0, sizeof *checked * decoders->size());
  PHYS_INFO& phys = *info.phys;
  TECH_INFO& tech = *info.tech;

  InfoFlags whatrq = what;

  // Try XIO123 first.
  StreamHelper stream(URL);
  XSTATL st;
  *st.type = 0;
  if (stream.Handle && xio_fstatl(stream.Handle, &st) == 0)
  { whatrq &= ~IF_Phys;
    what |= IF_Phys;
    phys.filesize = st.size;
    phys.tstmp = st.mtime;
    phys.attributes = PATTR_WRITABLE * !(st.attr & S_IAREAD);
  }
  DECODER_TYPE type_mask = Decoder::GetURLType(URL);

  ULONG rc;
  Decoder* dp = NULL;
  int whatdec;
  size_t i;

  // First check the decoder that matched last time.
  if (info.tech->decoder && *info.tech->decoder)
  { for (i = 0; i < decoders->size(); i++)
    { dp = (Decoder*)(*decoders)[i].get();
      if (dp->GetEnabled())
      { if (dp->ModRef->Key.compareToI(info.tech->decoder) != 0)
          continue;
        // Found
        if (!stream.Reset())
          goto err;
        whatdec = whatrq;
        rc = dp->Fileinfo(URL, stream.Handle, &whatdec, &info, &PROXYFUNCREF(Playable)Playable_DecoderEnumCb, param);
        if (rc != PLUGIN_NO_PLAY)
          goto ok; // This also happens in case of a plug-in independent error
        info.tech->info.reset();
      }
      checked[i] = true;
      break;
    }
  }

  // Next checks decoders supporting the specified type of files.
  for (i = 0; i < decoders->size(); i++)
  { dp = (Decoder*)(*decoders)[i].get();
    DEBUGLOG(("Playable::DecoderFileInfo: %s -> %u %x/%x\n", dp->ModRef->Key.cdata(),
      dp->GetEnabled(), dp->GetObjectTypes(), type_mask));
    if (!checked[i] && dp->GetEnabled() && (dp->GetObjectTypes() & type_mask))
    { if (type_mask == DECODER_FILENAME && !dp->IsFileSupported(URL, st.type))
        continue;
      if (!stream.Reset())
        goto err;
      whatdec = whatrq;
      rc = dp->Fileinfo(URL, stream.Handle, &whatdec, &info, &PROXYFUNCREF(Playable)Playable_DecoderEnumCb, param);
      if (rc != PLUGIN_NO_PLAY)
        goto ok; // This also happens in case of a plug-in independent error
      info.tech->info.reset();
    }
    checked[i] = true;
  }

  // At last checks the rest decoders with TryOthers set.
  for (i = 0; i < decoders->size(); i++)
  { if (checked[i])
      continue;
    dp = (Decoder*)(*decoders)[i].get();
    if (!dp->TryOthers)
      continue;
    if (!stream.Reset())
    { rc = PLUGIN_NO_READ;
      info.tech->info = xio_strerror(stream.Error);
      goto ok;
    }
    if (!stream.Reset())
      goto err;
    whatdec = whatrq;
    rc = dp->Fileinfo(URL, stream.Handle, &whatdec, &info, &PROXYFUNCREF(Playable)Playable_DecoderEnumCb, param);
    if (rc != PLUGIN_NO_PLAY)
      goto ok; // This also happens in case of a plug-in independent error
    info.tech->info.reset();
  }

  // No decoder found
  if (stream.Error)
  {err:
    tech.info = xio_strerror(stream.Error);
    whatdec = IF_Decoder;
    rc = PLUGIN_NO_READ;
    goto ok;
  }
  tech.info = "Cannot find a decoder that supports this item.";
  // Even in case of an error the requested information is in fact available.
  what = IF_Decoder;
  tech.attributes = TATTR_INVALID;
  return PLUGIN_NO_PLAY;

 ok:
  if (dp)
    tech.decoder = dp->ModRef->Key;
  if (rc != 0)
  { phys.attributes = PATTR_INVALID;
    tech.attributes = TATTR_INVALID;
    if (tech.info == NULL)
      tech.info.sprintf("Decoder error %i", rc);
  } else
  { if (info.obj->bitrate < 0 && info.obj->songlength > 0 && phys.filesize > 0)
      info.obj->bitrate = (int)(phys.filesize*8. / info.obj->songlength);
    // try streaming meta data if we do not have meta infos so far.
    if (stream.Handle)
    { META_INFO& meta = *info.meta;
      if (!meta.title)
        xio_get_metainfo(stream.Handle, XIO_META_TITLE, &meta.title);
      if (!meta.comment)
        xio_get_metainfo(stream.Handle, XIO_META_NAME, &meta.comment);
      if (!meta.genre)
        xio_get_metainfo(stream.Handle, XIO_META_GENRE, &meta.genre);
    }
  }
  DEBUGLOG(("Playable::DecoderFileInfo: {PHYS{%.0f, %i, %x}, TECH{%i,%i, %x, %s, %s, %s}, OBJ{%.3f, %i, %i}, META{...} ATTR{%x, %s}, RPL{%d, %d, %d, %d}, DRPL{%f, %d, %.0f, %d}, ITEM{...}} -> %x\n",
    phys.filesize, phys.tstmp, phys.attributes,
    tech.samplerate, tech.channels, tech.attributes,
      tech.info.cdata(), tech.format.cdata(), tech.decoder.cdata(),
    info.obj->songlength, info.obj->bitrate, info.obj->num_items,
    info.attr->ploptions, info.attr->at.cdata(),
    info.rpl->songs, info.rpl->lists, info.rpl->lists, info.rpl->unknown,
    info.drpl->totallength, info.drpl->unk_length, info.drpl->totalsize, info.drpl->unk_size,
    whatdec));
  ASSERT((whatrq & ~whatdec) == 0); // The decoder must not reset bits.
  what |= (InfoFlags)whatdec; // do not reset bits
  return rc;
}

PROXYFUNCIMP(void DLLENTRY, Playable)
Playable_DecoderEnumCb(void* param, const char* url, const INFO_BUNDLE* info, int cached, int reliable)
{ DEBUGLOG(("Playable::DecoderEnumCb(%p, %s, %p, %x, %x)\n", param, url, info, cached, reliable));
  deccbdata* cbdata = (deccbdata*)param;
  const url123& abs_url = cbdata->Parent.URL.makeAbsolute(url);
  if (!abs_url)
  { DEBUGLOG(("Playable::DecoderEnumCb: invalid URL.\n"));
    // TODO: error?
    return;
  }
  int_ptr<Playable> pp = Playable::GetByURL(abs_url);
  // Apply cached information to *pp.
  pp->SetCachedInfo(*info, (InfoFlags)cached, (InfoFlags)reliable);
  // Create reference
  InfoFlags override = (InfoFlags)(cached & reliable);
  if (override)
  { PlayableRef* ps = new PlayableRef(*pp);
    if (override & IF_Meta)
      ps->OverrideMeta(info->meta);
    if (override & IF_Attr)
      ps->OverrideAttr(info->attr);
    if (override & IF_Item)
      ps->OverrideItem(info->item);
    // Done
    cbdata->Children.append() = ps;
  } else
    // Fast path without PlayableRef
    cbdata->Children.append() = pp;
}

void Playable::ChildChangeNotification(const PlayableChangeArgs& args)
{ DEBUGLOG(("Playable(%p{%s})::ChildChangeNotification({%p{%s}, %p, %x,%x, %x})\n", this, DebugName().cdata(),
    &args.Instance, args.Instance.DebugName().cdata(), args.Origin, args.Loaded, args.Changed, args.Invalidated));

  InfoFlags aggreg = args.Changed & IF_Attr
    ? IF_Aggreg // because of PLO_ALTERNATION
    : (args.Changed|args.Invalidated) & IF_Aggreg;
  if (aggreg)
    Invalidate(aggreg, &args.Instance.GetPlayable());

  if ((args.Changed & (IF_Item|IF_Attr)) && args.Origin == &args.Instance) // Only if the change is originated from the child itself.
    SetModified(true, args.Origin);
}

void Playable::SetMetaInfo(const META_INFO* meta)
{ DEBUGLOG(("Playable(%p)::SetMetaInfo({...})\n", this));
  Mutex::Lock lock(Mtx);
  CollectionChangeArgs change(*this);
  if (Info.InfoStat.BeginUpdate(IF_Meta))
  { if (meta == NULL)
      meta = &MetaInfo::Empty;
    change.Loaded = IF_Meta;
    change.Changed = IF_Meta * Info.Meta.CmpAssign(*meta);
    // This kind of update never changes the state of the information,
    // because it does not set all fields.
    Info.InfoStat.EndUpdate(IF_Meta);
  } else
    // cannot lock meta info => invalidate instead
    change.Invalidated = Info.InfoStat.Invalidate(IF_Meta);
  // Fire change event if any
  if (!change.IsInitial())
    RaiseInfoChange(change);
}

void Playable::EnsurePlaylist()
{ ASSERT(!(Info.Tech.attributes & TATTR_SONG));
  if (!Playlist)
    Playlist = new PlaylistData(*this);
}

Playable::Entry* Playable::CreateEntry(APlayable& refto)
{ DEBUGLOG(("Playable(%p)::CreateEntry(&%p)\n", this, &refto));
  return new Entry(*this, refto, &Playable::ChildChangeNotification);
}

void Playable::InsertEntry(Entry* entry, Entry* before)
{ DEBUGLOG(("Playable(%p{%s})::InsertEntry(%p{%s}, %p{%s})\n", this, DebugName().cdata(),
    entry, entry->DebugName().cdata(), before, before->DebugName().cdata()));
  // insert new item at the desired location
  if (&entry->RefTo->GetPlayable() != this) // Do not track changes of ourself.
    entry->Attach();
  Playlist->Items.insert(entry, before);
  Playlist->Invalidate(IF_Aggreg, &entry->GetPlayable());
}

bool Playable::MoveEntry(Entry* entry, Entry* before)
{ DEBUGLOG(("Playable(%p{%s})::MoveEntry(%p{%s}, %p{%s})\n", this, DebugName().cdata(),
    entry, entry->DebugName().cdata(), before, before->DebugName().cdata()));
  ASSERT(entry->HasParent(this));
  return Playlist->Items.move(entry, before);
}

void Playable::RemoveEntry(Entry* entry)
{ DEBUGLOG(("Playable(%p{%s})::RemoveEntry(%p{%s})\n", this, DebugName().cdata(),
    entry, entry->DebugName().cdata()));
  ASSERT(entry->HasParent(this));
  entry->Detach();
  Playlist->Items.remove(entry);
}

void Playable::RenumberEntries(Entry* from, const Entry* to, unsigned index)
{ while (from != to)
  { ASSERT(from); // passed NULL or [from,to[ is no valid range
    from->SetIndex(index++);
    from = Playlist->Items.next(from);
  }
}

bool Playable::UpdateCollection(const vector<APlayable>& newcontent)
{ DEBUGLOG(("Playable(%p)::UpdateCollection({%u,...})\n", this, newcontent.size()));
  bool ret = false;
  PlayableRef* first_new = NULL;
  int index = 0;
  // TODO: RefreshActive = true;
  // Place new entries, try to recycle existing ones.
  foreach (APlayable,*const*, npp, newcontent)
  { APlayable& cur_new = **npp;

    // Priority 1: prefer an exactly matching one over a reference only to the same Playable.
    // Priority 2: prefer the first match over subsequent matches.
    Entry* match = NULL;
    Entry* cur_search = NULL;
    for(;;)
    { cur_search = Playlist->Items.next(cur_search);
      // End of list of old items?
      if (cur_search == NULL || cur_search == first_new)
        // No matching entry found, however, we may already have an inexact match.
        break;
      // Is matching item?
      if (cur_search == &cur_new)
      { // exactly the same object?
        match = cur_search;
        goto exactmatch;
      } else if (cur_search->GetPlayable() == cur_new.GetPlayable())
      { DEBUGLOG(("Playable::UpdateCollection potential match: %p\n", cur_search));
        // only the first inexact match counts
        match = cur_search;
      }
    }
    // Has match?
    if (match)
    { // Match! => Swap properties
      // If the slice changes this way an ChildInstChange event is fired here that invalidates the CollectionInfoCache.
      if (match != &cur_new && &match->GetPlayable() != &cur_new)
        match->AssignInstanceProperties((PlayableRef&)cur_new);
     exactmatch:
      // If it happened to be the first new entry we have to update first_new too.
      if (&cur_new == first_new)
        first_new = match;
      // move entry to the new location
      ret |= MoveEntry(match, NULL);
    } else
    { // No match => create new entry.
      match = CreateEntry(cur_new.GetPlayable());
      if (match != &cur_new && &match->GetPlayable() != &cur_new)
        match->AssignInstanceProperties((PlayableRef&)cur_new);
      InsertEntry(match, NULL);
      ret = true;
    }
    // Update index
    match->SetIndex(++index);
    // Keep the first element for stop conditions below
    if (first_new == NULL)
      first_new = match;
  }
  // Remove remaining old entries not recycled so far.
  // We loop here until we reach first_new. This /must/ be reached sooner or later.
  // first_new may be null if there are no new entries.
  Entry* cur = Playlist->Items.next(NULL);
  if (cur != first_new)
  { do
    { RemoveEntry(cur);
      cur = Playlist->Items.next(NULL);
    } while (cur != first_new);
    ret = true;
  }
  if (ret)
  { // TODO ValidateInfo(IF_Child, true, true);
    //UpdateModified(true);
  }

  // RefteshActive = false;
  return ret;
}

int_ptr<PlayableInstance> Playable::InsertItem(APlayable& item, PlayableInstance* before)
{ DEBUGLOG(("Playable(%p{%s})::InsertItem(%s, %p{%s})\n", this, DebugName().cdata(),
    item.DebugName().cdata(), before, before->DebugName().cdata()));
  Mutex::Lock lock(Mtx);
  // Check object type. Can't add to a song.
  if (Info.Tech.attributes & TATTR_SONG)
    return NULL;
  // Check whether the parameter before is still valid
  if (before && !before->HasParent(this))
    return NULL;
  InfoState::Update upd(Info.InfoStat);
  upd.Extend(IF_Child|IF_Obj|(IF_Tech * !(Info.Tech.attributes & TATTR_PLAYLIST)));
  if (!(upd & IF_Child))
    // TODO: Warning: Object locked by pending loadinfo
    return NULL;

  // point of no return...
  EnsurePlaylist();
  Entry* ep = CreateEntry(item);
  InsertEntry(ep, (Entry*)before);

  if (upd & IF_Tech)
  { // Make playlist from invalid item.
    Info.Tech.samplerate = -1;
    Info.Tech.channels   = -1;
    Info.Tech.attributes = (Info.Tech.attributes & ~TATTR_INVALID) | TATTR_PLAYLIST;
    Info.Tech.info.reset();
    Info.Obj.num_items = 1;
  } else
    ++Info.Obj.num_items;
  RenumberEntries(ep, NULL, before ? before->GetIndex() : Info.Obj.num_items);

  InfoFlags what = upd;
  upd.Commit();

  if (!Modified)
  { what |= IF_Usage;
    Modified = true;
  }

  CollectionChangeArgs args(*this, PCT_Insert, ep, what, what);
  args.Invalidated = Info.InfoStat.Invalidate(IF_Aggreg)
                   | Playlist->Invalidate(IF_Aggreg, &ep->GetPlayable());
  RaiseInfoChange(args);
  return ep;
}

bool Playable::MoveItem(PlayableInstance& item, PlayableInstance* before)
{ DEBUGLOG(("Playable(%p{%s})::InsertItem(&%p{%s}, %p{%s}) - %u\n", this, DebugName().cdata(),
    &item, item.DebugName().cdata(), before->DebugName().cdata()));
  Mutex::Lock lock(Mtx);
  // Check whether the parameter before is still valid
  if (!item.HasParent(this) || (before && !before->HasParent(this)))
    return false;
  InfoFlags what = Info.InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    // TODO: Warning: Object locked by pending loadinfo
    return false;

  // Now move the entry.
  Entry* const next = Playlist->Items.next(&(Entry&)item);
  if (MoveEntry(&(Entry&)item, (Entry*)before))
  { if (before == NULL || item.GetIndex() < before->GetIndex())
      RenumberEntries(next, (Entry*)before, item.GetIndex());
    else
      RenumberEntries(&(Entry&)item, next, before->GetIndex());
    if (!Modified)
    { what |= IF_Usage;
      Modified = true;
    }
  } else
    // No change
    what &= ~IF_Child;
  // done!
  Info.InfoStat.EndUpdate(IF_Child);
  CollectionChangeArgs args(*this, PCT_Move, &item, what|IF_Child, what);
  RaiseInfoChange(args);
  return true;
}

int_ptr<PlayableInstance> Playable::RemoveItem(PlayableInstance& item)
{ DEBUGLOG(("Playable(%p{%s})::RemoveItem(&%p{%s})\n", this, DebugName().cdata(),
    &item, item.DebugName().cdata()));
  Mutex::Lock lock(Mtx);
  // Check whether the item is still valid
  if (!item.HasParent(this))
  { DEBUGLOG(("Playable::RemoveItem: Bad item or bad parent.\n"));
    return int_ptr<PlayableInstance>();
  }
  InfoState::Update upd(Info.InfoStat);
  upd.Extend(IF_Child|IF_Obj);
  if (!(upd & IF_Child))
    // TODO: Hmm, normally we should wait
    return int_ptr<PlayableInstance>();

  // now detach the item from the container
  Entry* next = Playlist->Items.next(&(const Entry&)item);
  RenumberEntries(next, NULL, item.GetIndex());
  RemoveEntry(&(Entry&)item);
  --Info.Obj.num_items;

  InfoFlags what = upd;
  upd.Commit();
  if (!Modified)
  { what |= IF_Usage;
    Modified = true;
  }

  DEBUGLOG(("Playable::RemoveItem: before change event\n"));
  CollectionChangeArgs args(*this, PCT_Delete, &item, what, what);
  args.Invalidated = Info.InfoStat.Invalidate(IF_Aggreg)
                   | Playlist->Invalidate(IF_Aggreg, &item.GetPlayable());
  RaiseInfoChange(args);
  return next;
}

bool Playable::Clear()
{ DEBUGLOG(("Playable(%p{%s})::Clear()\n", this, DebugName().cdata()));
  Mutex::Lock lock(Mtx);
  InfoState::Update upd(Info.InfoStat);
  upd.Extend(IF_Child|IF_Obj|IF_Rpl|IF_Drpl);
  if (!(upd & IF_Child))
    // TODO: Hmm, normally we should wait
    return false;
  InfoFlags what = upd;
  // now detach all items from the container
  if (Playlist != NULL && !Playlist->Items.is_empty())
  { for (;;)
    { Entry* ep = Playlist->Items.next(NULL);
      DEBUGLOG(("PlayableCollection::Clear - %p\n", ep));
      if (ep == NULL)
        break;
      RemoveEntry(ep);
    }
    if (!Modified)
    { what |= IF_Usage;
      Modified = true;
    }
  } else
    // already empty
    what &= ~IF_Child;
  Info.Obj.num_items = 0;
  // TODO: no event if RPL/DRPL is not modified.
  Info.Rpl.Reset();
  Info.Drpl.Reset();
  Info.Rpl.lists = 1;

  upd.Commit();
  // TODO: join invalidate events
  CollectionChangeArgs args(*this, PCT_All, this, what|IF_Child, what);
  RaiseInfoChange(args);
  return true;
}

bool Playable::Sort(ItemComparer comp)
{ DEBUGLOG(("Playable(%p)::Sort(%p)\n", this, comp));
  Mutex::Lock lock(Mtx);
  if (!Playlist || Playlist->Items.prev(NULL) == Playlist->Items.next(NULL))
    return true; // Empty or one element lists are always sorted.
  InfoFlags what = Info.InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    // TODO: Hmm, normally we should wait
    return false;

  // Create index array
  vector<PlayableInstance> index(Info.Obj.num_items);
  Entry* ep = NULL;
  while ((ep = Playlist->Items.next(ep)) != NULL)
    index.append() = ep;
  // Sort index array
  merge_sort(index.begin(), index.end(), comp);
  // Adjust item sequence
  vector<APlayable> newcontent(index.size());
  for (PlayableInstance** cur = index.begin(); cur != index.end(); ++cur)
    newcontent.append() = *cur;
  bool changed = UpdateCollection(newcontent);
  ASSERT(index.size() == 0); // All items should have been reused
  // done
  Info.InfoStat.EndUpdate(IF_Child);
  CollectionChangeArgs args(*this, IF_Child, IF_Child*changed);
  RaiseInfoChange(args);
  return true;
}

bool Playable::Shuffle()
{ DEBUGLOG(("Playable(%p)::Shuffle\n", this));
  Mutex::Lock lock(Mtx);
  if (!Playlist || Playlist->Items.prev(NULL) == Playlist->Items.next(NULL))
    return true; // Empty or one element lists are always sorted.
  InfoFlags what = Info.InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    // TODO: Hmm, normally we should wait
    return false;

  // Create index array
  vector<APlayable> newcontent(Info.Obj.num_items);
  Entry* ep = NULL;
  while ((ep = Playlist->Items.next(ep)) != NULL)
    newcontent.insert(rand()%(newcontent.size()+1)) = ep;
  bool changed = UpdateCollection(newcontent);
  ASSERT(newcontent.size() == 0); // All items should have been reused
  // done
  Info.InfoStat.EndUpdate(IF_Child);
  CollectionChangeArgs args(*this, IF_Child, IF_Child*changed);
  RaiseInfoChange(args);
  return true;
}

class SaveCallbackData
{private:
  /// Playlist to save
  Playable&                 Parent;
  /// URL to save the playlist, not necessarily the same than \c Parent.URL
  const url123&             Dest;
  /// Use relative paths
  const bool                Relative;
  /// Current item saved at the last callback, initially NULL
  int_ptr<PlayableInstance> Current;
  /// Deep, non volatile copy of Current->GetInfo(), initially Parent.GetInfo()
  InfoBundle                Info;
 public:
  /// Constructor
  SaveCallbackData(Playable& parent, const url123& dest, bool relative)
  : Parent(parent), Dest(dest), Relative(relative)
  { Info.Assign(parent.GetInfo()); }
  /// Access Info
  const InfoBundle&         GetInfo() const { return Info; }
  /// Callback to be invoked by the decoder
  PROXYFUNCDEF int DLLENTRY SaveCallbackFunc(void* param, xstring* url, const INFO_BUNDLE** info, int* cached, int* reliable);
};

PROXYFUNCIMP(int DLLENTRY, SaveCallbackData)
SaveCallbackFunc (void* param, xstring* url, const INFO_BUNDLE** info, int* cached, int* reliable)
{ SaveCallbackData& cbd = *(SaveCallbackData*)param;
  // TODO: This is a race condition, because the exact content that is saved is not
  // well defined if the list is currently manipulation. Normally a snapshot should be taken.
  cbd.Current = cbd.Parent.GetNext(cbd.Current);
  if (cbd.Current == NULL)
    return PLUGIN_FAILED;
  *url = cbd.Relative
    ? cbd.Current->GetPlayable().URL.makeRelative(cbd.Dest)
    : cbd.Current->GetPlayable().URL;
  cbd.Info.Assign(cbd.Current->GetInfo());
  *info  = &cbd.Info;
  *reliable = ~cbd.Current->RequestInfo(~IF_None, PRI_None);
  *cached = cbd.Current->RequestInfo(~IF_None, PRI_None, REL_Cached) & ~*reliable;
  InfoFlags override = cbd.Current->GetOverridden();
  *reliable |= override;
  *cached   |= override;
  return PLUGIN_OK;
}

bool Playable::Save(const url123& dest, const char* decoder, const char* format, bool relative)
{ DEBUGLOG(("Playable::Save(%s, %s, %s, %u)\n", dest.cdata(), decoder, format, relative));
  ULONG rc;
  try
  { int_ptr<Decoder> dp = Decoder::GetDecoder(decoder);
    SaveCallbackData cbd(*this, dest, relative);
    int what = IF_Child;
    rc = dp->SaveFile(dest, format, &what, &cbd.GetInfo(), &PROXYFUNCREF(SaveCallbackData)SaveCallbackFunc, &cbd);
  } catch (const ModuleException& ex)
  { EventHandler::PostFormat(MSG_ERROR, "Failed to save playlist %s:\n%s", dest.cdata(), ex.GetErrorText().cdata());
    return false;
  }

  if (dest == URL)
  { // Save in place
    SetModified(false, this);
  } else
  { // Save copy as => Mark the copy as modified since it's content has changed
    // in the file system.
    int_ptr<Playable> pp = FindByURL(dest);
    if (pp)
      pp->SetModified(true, this);
  }
  return dest;
}

int Playable::SaveMetaInfo(const META_INFO& meta, DECODERMETA haveinfo)
{ DEBUGLOG(("Playable(%p)::SaveMetaInfo({...}, %x,)\n", this, haveinfo));
  // Check state
  if (!Info.Tech.decoder || (Info.Tech.attributes & TATTR_INVALID))
    return PLUGIN_NO_SAVE;
  // Write meta Info by plugin
  try
  { int_ptr<Decoder> dp = Decoder::GetDecoder(Info.Tech.decoder);
    ULONG rc = dp->SaveInfo(URL, &meta, haveinfo);
    if (rc != 0)
      return rc;
  } catch (const ModuleException& ex)
  { EventHandler::Post(MSG_ERROR, ex.GetErrorText());
    return PLUGIN_FAILED;
  }

  // adjust local meta info too.
  Mutex::Lock lock(Mtx);
  if (Info.InfoStat.Check(IF_Meta, REL_Cached))
    return 0;
  CollectionChangeArgs change(*this);
  if (!Info.InfoStat.BeginUpdate(IF_Meta))
  { // cannot lock meta info => invalidate instead
    change.Invalidated = Info.InfoStat.Invalidate(IF_Meta);
  } else
  { MetaInfo new_info(Info.Meta); // copy
    if (haveinfo & DECODER_HAVE_TITLE)
      new_info.title = meta.title;
    if (haveinfo & DECODER_HAVE_ARTIST)
      new_info.artist = meta.artist;
    if (haveinfo & DECODER_HAVE_ALBUM)
      new_info.album = meta.album;
    if (haveinfo & DECODER_HAVE_TRACK)
      new_info.track = meta.track;
    if (haveinfo & DECODER_HAVE_YEAR)
      new_info.year = meta.year;
    if (haveinfo & DECODER_HAVE_GENRE)
      new_info.genre = meta.genre;
    if (haveinfo & DECODER_HAVE_COMMENT)
      new_info.comment = meta.comment;
    if (haveinfo & DECODER_HAVE_COPYRIGHT)
      new_info.copyright = meta.copyright;
    change.Changed = IF_Meta * Info.Meta.CmpAssign(new_info);
    // This kind of update never changes the state of the information,
    // because it does not set all fields.
    Info.InfoStat.CancelUpdate(IF_Meta);
  }
  // Fire change event if any
  if (!change.IsInitial())
    RaiseInfoChange(change);
  return 0;
}

/*void Playable::SetTechInfo(const TECH_INFO* tech)
{ Lock lock(*this);
  InfoFlags what = BeginUpdate(IF_Tech);
  if (what & IF_Tech)
    UpdateTech(tech);
  // TODO: We cannot simply ignore update requests in case of concurrency issues.
  EndUpdate(what);
}*/

const Playable& Playable::DoGetPlayable() const
{ return *this; }

int_ptr<Location> Playable::GetStartLoc() const
{ return int_ptr<Location>(); }

int_ptr<Location> Playable::GetStopLoc() const
{ return int_ptr<Location>(); }

/****************************************************************************
*
*  class Playable - URL repository
*
****************************************************************************/

int Playable::Comparer(const xstring& key, const Playable& elem)
{ return key.compareToI(elem.URL);
}

Playable::RepositoryType Playable::RPIndex;
Mutex Playable::RPMtx;
clock_t Playable::LastCleanup = 0;

#ifdef DEBUG_LOG
void Playable::RPDebugDump()
{ RepositoryAccess rp;
  for (RepositoryType::iterator pos(rp->begin()); !pos.isend(); ++pos)
    DEBUGLOG(("Playable::RPDump: %p{%s}\n", *pos, (*pos)->URL.cdata()));
}
#endif

int_ptr<Playable> Playable::FindByURL(const xstring& url)
{ DEBUGLOG(("Playable::FindByURL(%s)\n", url.cdata()));
  int_ptr<Playable> ret;
  { Mutex::Lock lock(RPMtx);
    ret = RPIndex.find(url);
  }
  if (ret)
    ret->LastAccess = clock();
  return ret;
}

int_ptr<Playable> Playable::GetByURL(const url123& url)
{ DEBUGLOG(("Playable::GetByURL(%s)\n", url.cdata()));
  Playable* ret;
  // Repository lookup
  { Mutex::Lock lock(RPMtx);
    Playable*& pp = RPIndex.get(url);
    if (!pp)
    { int_ptr<Playable> ppf = new Playable(url);
      // keep reference count alive
      // The opposite function is at Cleanup().
      pp = ppf.toCptr();
    }
    ret = pp;
  }
  ret->LastAccess = clock();
  return ret;
}

void Playable::Cleanup()
{ DEBUGLOG(("Playable::Cleanup() - %u\n", LastCleanup));
  // Keep destructor calls out of the mutex
  vector<Playable> todelete;
  #ifdef DEBUG_LOG
  unsigned keep = 0;
  #endif
  // search for unused items, remove them from the repository and put them into todelete.
  // All of that in an atomic operation.
  { Mutex::Lock lock(RPMtx);
    RepositoryType::iterator pos(RPIndex.begin());
    while (!pos.isend())
    { Playable& p = **pos;
      if (p.RefCountIsUnique() && !p.Modified && (long)(p.LastAccess - LastCleanup) <= 0)
      { todelete.append() = &p;
        RPIndex.erase(pos);
      } else
      { ++pos;
        #ifdef DEBUG_LOG
        ++keep;
        #endif
      }
    }
  }
  // Destroy items
  { int_ptr<Playable> killer;
    foreach (Playable, *const*, ppp, todelete)
    { Playable& p = **ppp;
      DEBUGLOG(("Playable::Cleanup - detaching %p{%s}\n", &p, p.URL.cdata()));
      killer.fromCptr(&p);
    }
  }
  DEBUGLOG(("Playable::Cleanup: destroyed %u items, keep %u items.\n", todelete.size(), keep));
  // prepare next run
  LastCleanup = clock();
}

void Playable::Uninit()
{ DEBUGLOG(("Playable::Uninit()\n"));
  APlayable::Uninit();
  LastCleanup = clock();
  // The following clean-up could be left to the operating system as well.
  // However, for debug builds it is helpful if there ore not thousands
  // of memory objects left.
  #ifdef DEBUG
  //vector<Playable> todelete;
  int_ptr<Playable> killer;
  { Mutex::Lock lock(RPMtx);
    RepositoryType::iterator pos(RPIndex.begin());
    while (!pos.isend())
      killer.fromCptr(RPIndex.erase(pos));
  }
  /*int_ptr<Playable> killer;
  foreach (Playable, *const*, ppp, todelete)
    killer.fromCptr(*ppp);*/
  #endif
}
