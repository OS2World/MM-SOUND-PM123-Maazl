/*
 * Copyright 2007-2010 M.Mueller
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
#include "properties.h"
#include "dstring.h"
#include "glue.h"
#include <strutils.h>
#include <utilfct.h>
#include <vdelegate.h>
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
*  class PlayableInstance
*
****************************************************************************/

PlayableInstance::PlayableInstance(const Playable& parent, APlayable& refto)
: PlayableRef(refto),
  Parent(&parent),
  Index(0)
{ DEBUGLOG(("PlayableInstance(%p)::PlayableInstance(&%p{%s}, &%p{%s})\n", this,
    &parent, parent.URL.cdata(), &refto, refto.GetPlayable().URL.cdata()));
}

/*void PlayableInstance::Swap(PlayableRef& r)
{ DEBUGLOG(("PlayableInstance(%p)::Swap(&%p)\n", this, &r));
  ASSERT(Parent == ((PlayableInstance&)r).Parent);
  // Analyze changes
  StatusFlags events = SF_None;
  if (GetAlias() != r.GetAlias())
    events |= SF_Alias;
  if ( CompareSliceBorder(GetStart(), r.GetStart(), SB_Start)
    || CompareSliceBorder(GetStop(),  r.GetStop(),  SB_Stop) )
    events |= SF_Slice;
  // Execute changes
  if (events)
  { PlayableRef::Swap(r);
    StatusChange(change_args(*this, events));
  }
}*/

/*bool operator==(const PlayableInstance& l, const PlayableInstance& r)
{ return l.Parent == r.Parent
      && l.RefTo  == r.RefTo  // Instance equality is sufficient in case of the Playable class.
      && l.GetAlias()    == r.GetAlias();
//      && (const Slice&)l == (const Slice&)r;
}*/

int PlayableInstance::CompareTo(const PlayableInstance& r) const
{ if (this == &r)
    return 0;
  int li = Index;
  int ri = r.Index;
  // Removed items or different collection.
  if (li == 0 || ri == 0 || Parent != r.Parent)
    return INT_MIN;
  // Attension: nothing is locked here except for the life-time of the parent playlists.
  // So the item l or r may still be removed from the collection by another thread.
  return li - ri;
}


/****************************************************************************
*
*  class Playable
*
****************************************************************************/

Playable::Entry::Entry(Playable& parent, APlayable& refto, IDType::func_type ifn)
// The implementation always refers to the underlying playable object.
// But the overrideable properties are copied.
: PlayableInstance(parent, refto.GetPlayable()),
  InstDelegate(parent, ifn)
{ DEBUGLOG(("Playable::Entry(%p)::Entry(&&p, &%p, &%p)\n", this, &parent, &refto, &ifn));
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
: CollectionInfo(PlayableSet::Empty)
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

Mutex Playable::CollectionInfoMutex;


Playable::Playable(const url123& url)
: URL(url),
  Modified(false),
  InUse(false),
  LastAccess(clock())
{ DEBUGLOG(("Playable(%p)::Playable(%s)\n", this, url.cdata()));
}

Playable::~Playable()
{ DEBUGLOG(("Playable(%p{%s})::~Playable()\n", this, URL.cdata()));
  // Notify about dieing
  InfoChange(PlayableChangeArgs(*this));
  // No more events.
  InfoChange.reset();
}

bool Playable::IsInUse() const
{ return InUse;
}

const INFO_BUNDLE_CV& Playable::GetInfo() const
{ return Info;
}

void Playable::SetInUse(bool used)
{ DEBUGLOG(("Playable(%p{%s})::SetInUse(%u)\n", this, URL.cdata(), used));
  Mutex::Lock lock(Mtx);
  bool changed = InUse != used;
  InUse = used;
  // TODO: keep origin in case of cascaded execution
  InfoChange(PlayableChangeArgs(*this, this, IF_Usage, changed * IF_Usage, IF_None));
}

xstring Playable::GetDisplayName() const
{ xstring ret(Playable::GetInfo().meta->title);
  return ret && ret[0U] ? ret : URL.getShortName();
}

InfoFlags Playable::GetOverridden() const
{ return IF_None;
}

void Playable::SetCachedInfo(const INFO_BUNDLE& info, InfoFlags what)
{ DEBUGLOG(("Playable(%p)::SetCachedInfo(&%p, %x)\n", this, &info, what));
  // double check
  what &= InfoStat.Check(what, REL_Invalid);
  if (what)
  { Mutex::Lock lock(Mtx);
    what &= InfoStat.Check(what, REL_Invalid);
    InfoFlags changed = UpdateInfo(info, what);
    changed &= InfoStat.Cache(what);
    RaiseInfoChange(IF_None, changed);
  }
}

int_ptr<PlayableInstance> Playable::GetPrev(const PlayableInstance* cur) const
{ DEBUGLOG(("Playable(%p)::GetPrev(%p)\n", this, cur));
  ASSERT(!((Playable*)this)->RequestInfo(IF_Child, PRI_None, REL_Invalid));
  ASSERT(cur == NULL || cur->HasParent(NULL) || cur->HasParent(this));
  return List.prev((Entry*)cur);
}

int_ptr<PlayableInstance> Playable::GetNext(const PlayableInstance* cur) const
{ DEBUGLOG(("Playable(%p)::GetNext(%p)\n", this, cur));
  ASSERT(!((Playable*)this)->RequestInfo(IF_Child, PRI_None, REL_Invalid));
  ASSERT(cur == NULL || cur->HasParent(NULL) || cur->HasParent(this));
  return List.next((Entry*)cur);
}

xstring Playable::SerializeItem(const PlayableInstance* item, SerializationOptions opt) const
{ DEBUGLOG(("PlayableCollection(%p{%s})::SerializeItem(%p, %u)\n", this, URL.getShortName().cdata(), item, opt));
  // check whether we are unique
  size_t count = 1;
  { int_ptr<PlayableInstance> pi = GetPrev(item);
    while (pi != NULL)
    { if (opt & SO_IndexOnly || pi->GetPlayable() == item->GetPlayable())
        ++count;
      pi = GetPrev(pi);
    }
  }
  // Index only?
  if (opt & SO_IndexOnly)
    return xstring::sprintf("[%u]", count);
  xstring ret;
  // fetch relative or absolute URL
  if (opt & SO_RelativePath)
    ret = item->GetPlayable().URL.makeRelative(URL, !!(opt & SO_UseUpdir));
  else
    ret = item->GetPlayable().URL;
  // append count?
  return xstring::sprintf(count > 1 ? "\"%s\"[%u]" : "\"%s\"", ret.cdata(), count);
}


/*void Playable::InvalidateInfo(InfoFlags what)
{ DEBUGLOG(("Playable::InvalidateInfo(%x)\n", what));
  InfoSvc.EndUpdate(what);
  what = InfoRel.Invalidate(what);
  InfoChange(PlayableChangeArgs(*this, IF_None, IF_None, what));
}

InfoFlags Playable::InvalidateInfoSync(InfoFlags what)
{ DEBUGLOG(("Playable::InvalidateInfoSync(%x)\n", what));
  what &= ~InfoSvc.IsInService();
  what = InfoRel.Invalidate(what);
  InfoChange(PlayableChangeArgs(*this, IF_None, IF_None, what));
  return what;
}*/

InfoFlags Playable::UpdateInfo(const INFO_BUNDLE& info, InfoFlags what)
{ DEBUGLOG(("Playable::UpdateInfo(%p&, %x)\n", &info, what));
  ASSERT(Mtx.GetStatus() > 0);
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
  return ret;
}

void Playable::RaiseInfoChange(InfoFlags loaded, InfoFlags changed)
{ if (loaded|changed)
    InfoChange(PlayableChangeArgs(*this, this, loaded, changed, IF_None));
}

InfoFlags Playable::DoRequestInfo(InfoFlags& what, Priority pri, Reliability rel)
{ DEBUGLOG(("Playable(%p)::DoRequestInfo(%x&, %u, %u)\n", this, what, pri, rel));
  if (what & IF_Drpl)
    what |= IF_Phys|IF_Tech|IF_Obj|IF_Child; // required for DRPL_INFO aggregate
  else if (what & IF_Rpl)
    what |= IF_Tech|IF_Child; // required for RPL_INFO aggregate

  what &= InfoStat.Check(what, rel);
  if (pri == PRI_None || what == IF_None)
    return IF_None;

  InfoFlags rq = what;
  InfoFlags what2 = what & IF_Aggreg;
  if (what2)
  { // In case of a Playlist place the Request in the CIC cache too.
    rq &= DoRequestAI(Info, what2, pri, rel) | ~IF_Aggreg;
    what &= what2 | ~IF_Aggreg;
  }

  return InfoStat.Request(rq, pri);
}

Playable::CollectionInfo& Playable::CICLookup(const PlayableSetBase& exclude)
{ DEBUGLOG(("Playable::CICLookup({%u,})\n", exclude.size()));
  // Fastpath: no cache entry for the default object.
  size_t size = exclude.size();
  if (size == 0 || (size == 1 && exclude[0] == this))
    return Info;

  // Remove current Playable from the exclude list.
  sco_ptr<PlayableSet> more;
  const PlayableSetBase* current = &exclude;
  if (exclude.contains(*this))
  { more = new PlayableSet(size - 1);
    for (size_t i = 0; i < size; ++i)
    { Playable* pp = exclude[i];
      if (pp != this)
        more->append() = pp;
    }
    current = more.get();
  }

  Mutex::Lock lock(CollectionInfoMutex);
  CollectionInfoEntry*& cic = CollectionInfoCache.get(*current);
  if (cic == NULL)
    cic = more != NULL ? new CollectionInfoEntry(*more) : new CollectionInfoEntry(exclude);
  return *cic;
}

AggregateInfo& Playable::DoAILookup(const PlayableSetBase& exclude)
{ 
  if (Info.Tech.attributes & (TATTR_PLAYLIST|TATTR_SONG|TATTR_INVALID) == TATTR_PLAYLIST)
  { // Playlist => Delegate to PlayableCollection
    return CICLookup(exclude);
  } else
  { // Invalid or unknown item
    return Info;
  }
}

InfoFlags Playable::DoRequestAI(AggregateInfo& ai, InfoFlags& what, Priority pri, Reliability rel)
{ DEBUGLOG(("Playable(%p)::RequestCollectionInfo(&%p, %x, %d, %d)\n", this, &ai, what, pri, rel));
  ASSERT((what & ~IF_Aggreg) == 0);
  CollectionInfo& ci = (CollectionInfo&)ai;
  what = ci.InfoStat.Check(what, rel);
  if (pri == PRI_None || (what & IF_Aggreg) == IF_None)
    return IF_None;
  InfoFlags rq = ci.InfoStat.Request(what & IF_Aggreg, pri);
  // But we have to place an ordinary request too.
  return DoRequestInfo(rq, pri, rel);
}

InfoFlags Playable::GetNextCIWorkItem(CollectionInfo*& item, Priority pri)
{ DEBUGLOG(("Playable(%p)::GetNextCIWorkItem(%p, %u)\n", this, item, pri));
  if (!item)
  { InfoFlags req = Info.InfoStat.GetRequest(pri);
    if (req)
    { item = &Info;
      return req;
    }
  }
  if (CollectionInfoCache.size()) // Fastpath without mutex
  { Mutex::Lock lck(CollectionInfoMutex);
    CollectionInfoEntry** cipp = CollectionInfoCache.begin();
    if (item && item != &Info)
    { size_t pos;
      RASSERT(CollectionInfoCache.binary_search(item->Exclude, pos));
      cipp += pos +1;
    }
    CollectionInfoEntry** cepp = CollectionInfoCache.end();
    for (; cipp != cepp; ++ cipp)
    { InfoFlags req = (*cipp)->InfoStat.GetRequest(pri);
      if (req)
      { item = *cipp;
        return req;
      }
    }
  }
  item = NULL;
  return IF_None;
}

struct deccbdata
{ Playable&                    Parent;
  vector_own<PlayableRef>      Children;
  deccbdata(Playable& parent) : Parent(parent) {}
  ~deccbdata() {}
};

bool Playable::DoLoadInfo(Priority pri)
{ DEBUGLOG(("Playable(%p{%s})::DoLoadInfo(%u)\n", this, URL.getShortName().cdata(), pri));
  InfoState::Update upd(InfoStat, InfoStat.GetRequest(pri));
  DEBUGLOG(("Playable::DoLoadInfo: update %x\n", upd.GetWhat()));
  if (!upd)
    return false;
  InfoBundle info;
  // get information
  int what2 = upd & IF_Decoder; // incompatible types and do not request RPL info from the decoder.
  if (what2)
  { deccbdata children(*this);
    int rc = dec_fileinfo(URL, &what2, &info, &PROXYFUNCREF(Playable)Playable_DecoderEnumCb, &children);
    upd.Extend((InfoFlags)what2);
    DEBUGLOG(("Playable::DoLoadInfo - rc = %i\n", rc));
    
    // Update information, but only if still in service.
    Mutex::Lock lock(Mtx);
    InfoFlags changed = UpdateInfo(info, upd);
    // update children
    if (upd & IF_Child)
      changed |= IF_Child * UpdateCollection(children.Children);
    // Raise the first bunch of change events.
    if (upd & IF_Aggreg)
      Info.InfoStat.EndUpdate(upd & ~IF_Aggreg);
    RaiseInfoChange(upd.Commit((InfoFlags)what2 | ~IF_Aggreg), changed);
    // Everything already done?
    if (!upd)
      return false;
  }
  // Calculate RPL_INFO if not already done by dec_fileinfo.
  if ((Info.Tech.attributes & (TATTR_SONG|TATTR_PLAYLIST|TATTR_INVALID)) == TATTR_PLAYLIST)
  { // For the event below
    PlayableChangeArgs args(*this);
    InfoFlags reschedule = IF_None;
    CollectionInfo* iep = NULL;
    for(;;)
    { InfoFlags req = GetNextCIWorkItem(iep, pri);
      if (!req)
        break;
      // retrieve information
      InfoFlags done = CalcRplInfo(*iep, req, pri, args.Changed);
      reschedule |= req & ~done;
      args.Loaded |= done;
    }
    upd.Rollback(reschedule);
    Mutex::Lock lock(Mtx);
    upd.Commit();
    // Raise event if any    
    if (!args.IsInitial())
      InfoChange(args);
    return !!reschedule;
  } else
  { if (Info.Tech.attributes & TATTR_INVALID)
    { info.Rpl.num_invalid  = 1;
    } else if (Info.Tech.attributes & TATTR_SONG)
    { info.Rpl.totalsongs   = 1;
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
    }
    // update information
    Mutex::Lock lock(Mtx);
    InfoFlags changed = UpdateInfo(info, upd);
    // Raise event if any
    RaiseInfoChange(upd.Commit(), changed);
    return false;
  }
}

PROXYFUNCIMP(void DLLENTRY, Playable)
Playable_DecoderEnumCb( void* param, const char* url, const INFO_BUNDLE* info, int cached, int override )
{ DEBUGLOG(("Playable::DecoderEnumCb(%p, %s, %p, %x, %x)\n", param, url, info, cached, override));
  ASSERT((cached & override) == 0);
  deccbdata* cbdata = (deccbdata*)param;
  const url123& abs_url = cbdata->Parent.URL.makeAbsolute(url);
  if (!abs_url)
  { DEBUGLOG(("Playable::DecoderEnumCb: invalid URL.\n"));
    // TODO: error?
    return;
  }
  int_ptr<Playable> pp = Playable::GetByURL(abs_url);
  // Apply cached information to *pp.
  cached &= ~pp->InfoStat.Check((InfoFlags)cached, REL_Invalid);
  // double check
  if (cached)
  { Mutex::Lock lock(pp->Mtx);
    cached &= ~pp->InfoStat.Check((InfoFlags)cached, REL_Invalid);
    pp->RaiseInfoChange(IF_None, pp->InfoStat.Cache(pp->UpdateInfo(*info, (InfoFlags)cached)));
  }
  PlayableRef* ps = new PlayableRef(*pp);
  if (override & IF_Meta)
    ps->OverrideMeta(info->meta);
  if (override & IF_Attr)
    ps->OverrideAttr(info->attr);
  // Done
  cbdata->Children.append() = ps;
}

InfoFlags Playable::CalcRplInfo(CollectionInfo& cie, InfoFlags what, Priority pri, InfoFlags& changed)
{ DEBUGLOG(("Playable(%p)::CalcRplInfo({}, %x, %u, %x)\n", this, what, pri, changed));
  // The entire implementation of this function is basically lock-free.
  // This means that the result may not be valid after the function finished.
  // This is addressed by invalidating the rpl bits of cie.InfoStat by other threads.
  // When this happens the commit after the function completes will not set the state to valid
  // and the function will be called again when the rpl information is requested again.
  // The calculation must not be done in place to avoid visibility of intermediate results.
  InfoFlags whatok = what;
  RplInfo rpl;
  DrplInfo drpl;
  // Iterate over children
  int_ptr<PlayableInstance> pi;
  while ((pi = GetNext(pi)) != NULL)
  { // Skip exclusion list entries to avoid recursion.
    if (&pi->GetPlayable() == this || cie.Exclude.contains(pi->GetPlayable()))
    { DEBUGLOG(("PlayableCollection::GetCollectionInfo - recursive: %p->%p!\n", pi.get(), &pi->GetPlayable()));
      continue;
    }
    InfoFlags what2 = what;
    const volatile AggregateInfo& ai = pi->RequestAggregateInfo(cie.Exclude, what2, pri, REL_Cached);
    whatok &= what2;
    if (whatok & IF_Rpl)
      rpl += ai.Rpl;
    if (whatok & IF_Drpl)
      drpl += ai.Drpl;
  }
  // Update results
  Mutex::Lock lock(Mtx);
  if (whatok & IF_Rpl)
    changed |= IF_Rpl * cie.Rpl.CmpAssign(rpl);
  if (whatok & IF_Drpl)
    changed |= IF_Drpl * cie.Drpl.CmpAssign(drpl);
  return whatok;
}

void Playable::ChildChangeNotification(const PlayableChangeArgs& args)
{ DEBUGLOG(("Playable(%p{%s})::ChildChangeNotification({%p{%s}, %p, %x,%x, %x})", this, URL.getShortName().cdata(),
    &args.Instance, args.Instance.GetPlayable().URL.getShortName().cdata(), args.Origin, args.Loaded, args.Changed, args.Invalidated));
  /*  InfoFlags f = args.Changed & (IF_Tech|IF_Rpl);
    if (f)
    { // Invalidate dependant info and reaload if already known
      InvalidateInfo(f, true);
      // Invalidate CollectionInfoCache entries.
      InvalidateCIC(f, args.Instance);
    }
  }

    if (args.Flags & PlayableInstance::SF_Slice)
      // Update dependant tech info, but only if already available
      // Same as TechInfoChange => emulate another event
      ChildInfoChange(Playable::change_args(*args.Instance.GetPlayable(), IF_Tech, IF_Tech));
  }*/
}

Playable::Entry* Playable::CreateEntry(APlayable& refto)
{ DEBUGLOG(("Playable(%p)::CreateEntry(&%p)\n", this, &refto));
  return new Entry(*this, refto, &Playable::ChildChangeNotification);
}

void Playable::InsertEntry(Entry* entry, Entry* before)
{ DEBUGLOG(("Playable(%p{%s})::InsertEntry(%p{%s,%p,%p}, %p{%s})\n", this, URL.getShortName().cdata(),
    entry, entry->GetPlayable().URL.getShortName().cdata(), List.prev(entry), List.next(entry),
    before, before ? before->GetPlayable().URL.cdata() : ""));
  // insert new item at the desired location
  entry->Attach();
  List.insert(entry, before);
  DEBUGLOG(("Playable::InsertEntry - before event\n"));
  //InvalidateCIC(IF_Aggreg, entry->GetPlayable());
  //CollectionChange(CollectionChangeArgs(*this, *entry, PCT_Insert));
  DEBUGLOG(("Playable::InsertEntry - after event\n"));
}

bool Playable::MoveEntry(Entry* entry, Entry* before)
{ DEBUGLOG(("Playable(%p{%s})::MoveEntry(%p{%s,%p,%p}, %p{%s})\n", this, URL.getShortName().cdata(),
    entry, entry->GetPlayable().URL.getShortName().cdata(), List.prev(entry), List.next(entry),
    before, (before ? before->GetPlayable().URL : url123::EmptyURL).getShortName().cdata()));
  if (!List.move(entry, before))
    return false;
  // raise event
  DEBUGLOG(("Playable::MoveEntry - before event\n"));
  //CollectionChange(CollectionChangeArgs(*this, *entry, PCT_Move));
  DEBUGLOG(("Playable::MoveEntry - after event\n"));
  return true;
}

void Playable::RemoveEntry(Entry* entry)
{ DEBUGLOG(("Playable(%p{%s})::RemoveEntry(%p{%s,%p,%p})\n", this, URL.getShortName().cdata(),
    entry, entry->GetPlayable().URL.getShortName().cdata(), List.prev(entry), List.next(entry)));
  //InvalidateCIC(IF_Aggreg, entry->GetPlayable());
  //CollectionChange(CollectionChangeArgs(*this, *entry, PCT_Delete));
  DEBUGLOG(("Playable::RemoveEntry - after event\n"));
  ASSERT(entry->HasParent(this));
  entry->Detach();
  List.remove(entry);
}

bool Playable::UpdateCollection(const vector<PlayableRef>& newcontent)
{ DEBUGLOG(("Playable(%p)::UpdateCollection({%u,...})\n", this, newcontent.size()));
  bool ret = false;
  PlayableRef* first_new = NULL;
  // TODO: RefreshActive = true;
  // Place new entries, try to recycle existing ones.
  for (PlayableRef*const* npp = newcontent.begin(); npp != newcontent.end(); ++npp)
  { PlayableRef* cur_new = *npp;

    // Priority 1: prefer an exactly matching one over a reference only to the same Playable.
    // Priority 2: prefer the first match over subsequent matches.
    Entry* match = NULL;
    Entry* cur_search = NULL;
    for(;;)
    { cur_search = List.next(cur_search);
      // End of list of old items?
      if (cur_search == NULL || cur_search == first_new)
        // No matching entry found, however, we may already have an inexact match.
        break;
      // Is matching item?
      if (cur_search == cur_new)
      { // exactly the same object?
        match = cur_search;
        goto exactmatch;
      } else if (cur_search->RefTo == cur_new->RefTo)
      { DEBUGLOG(("Playable::UpdateCollection potential match: %p\n", cur_search));
        // Check for exact match
        // TODO: race condition
        //if ( *cur_search->GetInfo().item == *cur_new->GetInfo().item
        //  && *cur_search->GetInfo().attr == *cur_new->GetInfo().attr
        //  && *cur_search->GetInfo().meta == *cur_new->GetInfo().meta )
        //{ // exact match => take this one
        //  match = cur_search;
        //  goto exactmatch;
        //} else if (match == NULL)
          // only the first inexact match counts
          match = cur_search;
      }
    }
    // Has match?
    if (match)
    { // Match! => Swap properties
      // If the slice changes this way an ChildInstChange event is fired here that invalidates the CollectionInfoCache.
      match->AssignInstanceProperties(*cur_new);
     exactmatch:
      // If it happened to be the first new entry we have to update first_new too.
      if (cur_new == first_new)
        first_new = match;
      // move entry to the new location
      ret |= MoveEntry(match, NULL);
    } else
    { // No match => create new entry.
      match = CreateEntry(cur_new->GetPlayable());
      match->AssignInstanceProperties(*cur_new);
      InsertEntry(match, NULL);
      ret = true;
    }
    // Keep the first element for stop conditions below
    if (first_new == NULL)
      first_new = match;
  }
  // Remove remaining old entries not recycled so far.
  // We loop here until we reach first_new. This /must/ be reached sooner or later.
  // first_new may be null if there are no new entries.
  Entry* cur = List.next(NULL);
  if (cur != first_new)
  { do
    { RemoveEntry(cur);
      cur = List.next(NULL);
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
{ DEBUGLOG(("Playable(%p{%s})::InsertItem(%s, %p{%s})\n", this, URL.getShortName().cdata(),
    item.GetPlayable().URL.getShortName().cdata(), before, before ? before->GetPlayable().URL.cdata() : ""));
  Mutex::Lock lock(Mtx);
  // Check object type. Can't add to a song.
  if (Info.Tech.attributes & TATTR_SONG)
    return NULL;
  // Check whether the parameter before is still valid
  if (before && !before->HasParent(this))
    return NULL;
  InfoState::Update upd(InfoStat, IF_Child|IF_Obj|(IF_Tech * !(Info.Tech.attributes & TATTR_PLAYLIST)));
  if (!(upd & IF_Child))
    // TODO: Warning: Object locked by pending loadinfo
    return NULL;

  // point of no return...
  Entry* ep = CreateEntry(item);

  /*ep->SetAlias(item.GetAlias());
  if (item.GetStart())
    ep->SetStart(new SongIterator(*item.GetStart()));
  if (item.GetStop())
    ep->SetStop(new SongIterator(*item.GetStop()));*/

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

  InfoFlags what = upd;
  upd.Commit();

  if (!Modified)
  { what |= IF_Usage;
    Modified = true;
  }
  //InvalidateInfo(IF_Tech|IF_Rpl, true);
  // TODO: join invalidate events
  RaiseInfoChange(what, what);
  return ep;
}

bool Playable::MoveItem(PlayableInstance* item, PlayableInstance* before)
{ DEBUGLOG(("Playable(%p{%s})::InsertItem(%p{%s}, %p{%s}) - %u\n", this, URL.getShortName().cdata(),
    item, item->GetPlayable().URL.cdata(), before ? before->GetPlayable().URL.cdata() : ""));
  Mutex::Lock lock(Mtx);
  // Check whether the parameter before is still valid
  if (!item->HasParent(this) || (before && !before->HasParent(this)))
    return false;
  InfoFlags what = InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    // TODO: Warning: Object locked by pending loadinfo
    return false;

  // Now move the entry.
  if (MoveEntry((Entry*)item, (Entry*)before))
  { if (!Modified)
    { what |= IF_Usage;
      Modified = true;
    }
  } else
    // No change
    what &= ~IF_Child;
  // done!
  InfoStat.EndUpdate(IF_Child);
  RaiseInfoChange(what|IF_Child, what);
  return true;
}

bool Playable::RemoveItem(PlayableInstance* item)
{ DEBUGLOG(("Playable(%p{%s})::RemoveItem(%p{%s})\n", this, URL.getShortName().cdata(),
    item, item->GetPlayable().URL.cdata()));
  Mutex::Lock lock(Mtx);
  // Check whether the item is still valid
  if (item && !item->HasParent(this))
    return false;
  InfoState::Update upd(InfoStat, IF_Child|IF_Obj);
  if (!(upd & IF_Child))
    return false;

  // now detach the item from the container
  RemoveEntry((Entry*)item);
  --Info.Obj.num_items;

  InfoFlags what = upd;
  upd.Commit();
  if (!Modified)
  { what |= IF_Usage;
    Modified = true;
  }
  // TODO InvalidateInfo(IF_Tech|IF_Rpl, true);
  // TODO: join invalidate events
  RaiseInfoChange(what, what);
  return true;
}

bool Playable::Clear()
{ DEBUGLOG(("Playable(%p{%s})::Clear()\n", this, URL.getShortName().cdata()));
  Mutex::Lock lock(Mtx);
  InfoState::Update upd(InfoStat, IF_Child|IF_Obj|IF_Rpl|IF_Drpl);
  if (!(upd & IF_Child))
    return false;
  InfoFlags what = upd;
  // now detach all items from the container
  if (!List.is_empty())
  { for (;;)
    { Entry* ep = List.next(NULL);
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
  Info.Rpl.Clear();
  Info.Drpl.Clear();

  upd.Commit();
  // TODO: join invalidate events
  RaiseInfoChange(what|IF_Child, what);
  return true;
}

bool Playable::Sort(ItemComparer comp)
{ DEBUGLOG(("Playable(%p)::Sort(%p)\n", this, comp));
  Mutex::Lock lock(Mtx);
  if (List.prev(NULL) == List.next(NULL))
    return true; // Empty or one element lists are always sorted.
  InfoFlags what = InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    return false;

  // Create index array
  vector<PlayableInstance> index(Info.Obj.num_items);
  Entry* ep = NULL;
  while ((ep = List.next(ep)) != NULL)
    index.append() = ep;
  // Sort index array
  merge_sort(index.begin(), index.end(), comp);
  // Adjust item sequence
  vector<PlayableRef> newcontent(index.size());
  for (PlayableInstance** cur = index.begin(); cur != index.end(); ++cur)
    newcontent.append() = *cur;
  bool changed = UpdateCollection(newcontent);
  ASSERT(index.size() == 0); // All items should have been reused
  // done
  InfoStat.EndUpdate(IF_Child);
  RaiseInfoChange(IF_Child, IF_Child*changed);
  return true;
}

bool Playable::Shuffle()
{ DEBUGLOG(("Playable(%p)::Shuffle\n", this));
  Mutex::Lock lock(Mtx);
  if (List.prev(NULL) == List.next(NULL))
    return true; // Empty or one element lists are always sorted.
  InfoFlags what = InfoStat.BeginUpdate(IF_Child);
  if (what == IF_None)
    return false;

  // Create index array
  vector<PlayableRef> newcontent(Info.Obj.num_items);
  Entry* ep = NULL;
  while ((ep = List.next(ep)) != NULL)
    newcontent.insert(rand()%(newcontent.size()+1)) = ep;
  bool changed = UpdateCollection(newcontent);
  ASSERT(newcontent.size() == 0); // All items should have been reused
  // done
  InfoStat.EndUpdate(IF_Child);
  RaiseInfoChange(IF_Child, IF_Child*changed);
  return true;
}


void Playable::SetMetaInfo(const META_INFO* meta)
{ DEBUGLOG(("Playable(%p)::SetMetaInfo({...})\n", this));
  /* TODO: Write meta Info by plugin
  Lock lock(*this);
  InfoFlags what = BeginUpdate(IF_Meta);
  if (what & IF_Meta)
    UpdateMeta(meta);
  // TODO: We cannot simply ignore update requests in case of concurrency issues.
  EndUpdate(what);*/
}

/*void Playable::SetTechInfo(const TECH_INFO* tech)
{ Lock lock(*this);
  InfoFlags what = BeginUpdate(IF_Tech);
  if (what & IF_Tech)
    UpdateTech(tech);
  // TODO: We cannot simply ignore update requests in case of concurrency issues.
  EndUpdate(what);
}*/

/*void Playable::CalcRplInfo()
{ if (!Collection)
  { // It's just a song
    RPL_INFO rpl =
    { sizeof(RPL_INFO),         // size
      1,                        // totalsongs
      0,                        // totallists
      Info.TechInfo.songlength, // totallength
      Info.PhysInfo.filesize,   // totalsize
      false                     // recursive
    };
    UpdateRpl(&rpl, true);
    return;
  }
  // It's a PlayableCollection
  CollectionInfo ci;
  int_ptr<PlayableInstance> pi = Collection->GetNext(NULL);
  while (pi)
  { ci.Add(Collection->GetCollectionInfo());
    pi = Collection->GetNext(pi);
  }
  UpdateRpl(&ci, true);
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

sorted_vector<Playable, const char*> Playable::RPInst(RP_INITIAL_SIZE);
Mutex   Playable::RPMutex;
clock_t Playable::LastCleanup = 0;

int Playable::compareTo(const char*const& str) const
{ DEBUGLOG2(("Playable(%p{%s})::compareTo(%s)\n", this, URL.cdata(), str));
  return stricmp(URL, str);
}

#ifdef DEBUG_LOG
void Playable::RPDebugDump()
{ for (Playable*const* ppp = RPInst.begin(); ppp != RPInst.end(); ++ppp)
    DEBUGLOG(("Playable::RPDump: %p{%s}\n", *ppp, (*ppp)->URL.cdata()));
}
#endif

int_ptr<Playable> Playable::FindByURL(const char* url)
{ DEBUGLOG(("Playable::FindByURL(%s)\n", url));
  Mutex::Lock lock(RPMutex);
  return RPInst.find(url);
}

int_ptr<Playable> Playable::GetByURL(const url123& URL)
{ DEBUGLOG(("Playable::GetByURL(%s)\n", URL.cdata()));
  // Repository lookup
  Mutex::Lock lock(RPMutex);
  Playable*& ppn = RPInst.get(URL);
  if (ppn == NULL)
  { // no match => factory
    int_ptr<Playable> ppf = new Playable(URL);
    ppn = ppf.toCptr(); // keep reference count alive
                        // The opposite function is at Cleanup().
    DEBUGLOG(("Playable::GetByURL: factory &%p{%p}\n", &ppn, ppn));
    return ppn;
  }
  // else match
  ppn->LastAccess = clock();
  return ppn;
  
  /*if (ca)
  { InfoFlags what = (bool)(ca->format != NULL) * IF_Format
                   | (bool)(ca->tech   != NULL) * IF_Tech
                   | (bool)(ca->meta   != NULL) * IF_Meta
                   | (bool)(ca->phys   != NULL) * IF_Phys
                   | (bool)(ca->rpl    != NULL) * IF_Rpl;
    DEBUGLOG(("Playable::GetByURL: merge {%p, %p, %p, %p, %p} %x %x\n",
      ca->format, ca->tech, ca->meta, ca->phys, ca->rpl, pp->InfoValid, what));
    what &= ~pp->InfoValid;
    // Double check to avoid unneccessary mutex delays.
    if ( what )
    { // Merge meta info
      Lock lock(*pp);
      what = pp->BeginUpdate(what & ~pp->InfoValid);
      if (what & IF_Format)
        pp->UpdateFormat(ca->format, false);
      if (what & IF_Tech)
        pp->UpdateTech(ca->tech, false);
      if (what & IF_Meta)
        pp->UpdateMeta(ca->meta, false);
      if (what & IF_Phys)
        pp->UpdatePhys(ca->phys, false);
      if (what & IF_Rpl)
        pp->UpdateRpl(ca->rpl, false);
      pp->EndUpdate(what);
    }
  }*/
}

void Playable::DetachObjects(const vector<Playable>& list)
{ DEBUGLOG(("Playable::DetachObjects({%u,})\n", list.size()));
  int_ptr<Playable> killer;
  for (Playable*const* ppp = list.begin(); ppp != list.end(); ++ppp)
  { DEBUGLOG(("Playable::DetachObjects - detaching %p{%s}\n", *ppp, (*ppp)->URL.cdata()));
    killer.fromCptr(*ppp);
  }
}

void Playable::Cleanup()
{ DEBUGLOG(("Playable::Cleanup() - %u\n", LastCleanup));
  // Keep destructor calls out of the mutex
  vector<Playable> todelete(32);
  // serach for unused items
  { Mutex::Lock lock(RPMutex);
    for (Playable*const* ppp = RPInst.end(); --ppp != RPInst.begin(); )
    { if ((*ppp)->RefCountIsUnique() && (long)((*ppp)->LastAccess - LastCleanup) <= 0)
        todelete.append() = RPInst.erase(ppp);
    }
  }
  // Destroy items
  DetachObjects(todelete);
  // prepare next run
  LastCleanup = clock();
}

void Playable::Uninit()
{ DEBUGLOG(("Playable::Uninit()\n"));
  APlayable::Uninit();
  LastCleanup = clock();
  // Keep destructor calls out of the mutex
  static sorted_vector<Playable, const char*> rp;
  { // detach all items
    Mutex::Lock lock(RPMutex);
    rp.swap(RPInst);
  }
  // Detach items
  DetachObjects(rp);
  DEBUGLOG(("Playable::Uninit - complete - %u\n", RPInst.size()));
}


/****************************************************************************
*
*  class Song
*
****************************************************************************/

/*Song::Song(const url123& URL, const INFO_BUNDLE* ca)
: Playable(URL, ca)
{ DEBUGLOG(("Song(%p)::Song(%s, %p)\n", this, URL.cdata(), ca));
  // Always overwrite RplInfo
  static const RPL_INFO defrpl = { sizeof(RPL_INFO), 1, 0 };
  UpdateRpl(&defrpl, true);
}

Playable::InfoFlags Song::DoLoadInfo(InfoFlags what)
{ DEBUGLOG(("Song(%p)::DoLoadInfo(%x) - %s\n", this, what, GetDecoder()));
  what |= BeginUpdate(IF_Other); // IF_Other always inclusive when we call dec_fileinfo
  // get information
  sco_ptr<DecoderInfo> info(new DecoderInfo()); // a bit much for the stack
  INFOTYPE what2 = (INFOTYPE)((int)what & INFO_ALL); // inclompatible types
  DecoderName decoder = "";
  int rc = dec_fileinfo(GetURL(), &what2, info.get(), decoder, sizeof decoder);
  InfoFlags done = (InfoFlags)what2 | IF_All;
  what |= BeginUpdate(done);
  DEBUGLOG(("Song::LoadInfo - rc = %i\n", rc));
  // update information
  Lock lock(*this);
  bool valid = rc == 0;
  if (!valid)
  { if (*info->tech->info == 0)
      sprintf(info->tech->info, "Decoder error %i", rc);
  }
  UpdateInfo(valid ? STA_Valid : STA_Invalid, info.get(), decoder, what, valid); // Reset fields
  return what;
}

ULONG Song::SaveMetaInfo(const META_INFO& info, int haveinfo)
{ DEBUGLOG(("Song(%p{%s})::SaveMetaInfo(, %x)\n", this, GetURL().cdata(), haveinfo));
  haveinfo &= DECODER_HAVE_TITLE|DECODER_HAVE_ARTIST|DECODER_HAVE_ALBUM  |DECODER_HAVE_TRACK
             |DECODER_HAVE_YEAR |DECODER_HAVE_GENRE |DECODER_HAVE_COMMENT|DECODER_HAVE_COPYRIGHT;
  EnsureInfo(Playable::IF_Other);
  ULONG rc = dec_saveinfo(GetURL(), &info, haveinfo, GetDecoder());
  if (rc == 0)
  { Lock lock(*this);
    if (BeginUpdate(IF_Meta))
    { META_INFO new_info = *GetInfo().meta; // copy
      if (haveinfo & DECODER_HAVE_TITLE)
        strlcpy(new_info.title,    info.title,      sizeof new_info.title);
      if (haveinfo & DECODER_HAVE_ARTIST)
        strlcpy(new_info.artist,   info.artist,     sizeof new_info.artist);
      if (haveinfo & DECODER_HAVE_ALBUM)
        strlcpy(new_info.album,    info.album,      sizeof new_info.album);
      if (haveinfo & DECODER_HAVE_TRACK)
        new_info.track = info.track;
      if (haveinfo & DECODER_HAVE_YEAR)
        strlcpy(new_info.year,      info.year,      sizeof new_info.year);
      if (haveinfo & DECODER_HAVE_GENRE)
        strlcpy(new_info.genre,     info.genre,     sizeof new_info.genre);
      if (haveinfo & DECODER_HAVE_COMMENT)
        strlcpy(new_info.comment,   info.comment,   sizeof new_info.comment);
      if (haveinfo & DECODER_HAVE_COPYRIGHT)
        strlcpy(new_info.copyright, info.copyright, sizeof new_info.copyright);
      UpdateMeta(&new_info);
      EndUpdate(IF_Meta);
    }
  }
  return rc;
}*/


