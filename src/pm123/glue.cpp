/*
 * Copyright 2006-2011 Marcel Mueller
 * Copyright 2004-2006 Dmitry A.Steklenev <glass@ptv.ru>
 * Copyright 1997-2003 Samuel Audet  <guardia@step.polymtl.ca>
 *                     Taneli Lepp�  <rosmo@sektori.com>
 *
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

#define  INCL_PM
#include "glue.h"
#include "decoder.h"
#include "filter.h"
#include "output.h"
#include "visual.h"
#include "123_util.h"
#include "properties.h"
#include "pm123.h"
#include <fileutil.h>
#include <eautils.h>
#include <xio.h>
#include <os2.h>

#include <cpp/container/sorted_vector.h>

//#define DEBUG 1
#include <debuglog.h>


/****************************************************************************
*
* global events
*
****************************************************************************/

event<const dec_event_args>   dec_event;
event<const OUTEVENTTYPE>     out_event;


/****************************************************************************
*
* virtual output interface, including filter plug-ins
* This class logically connects the decoder and the output interface.
*
****************************************************************************/

class Glue
{private:
  static const dec_event_args ev_playstop;

  static BOOL              initialized;       // whether the following vars are true
  static OutputProcs       procs;             // entry points of the filter chain
  static OUTPUT_PARAMS2    params;            // parameters for output_command
  static DECODER_PARAMS2   dparams;           // parameters for decoder_command
  static xstring           url;               // current URL
  static PM123_TIME        stoptime;          // time index where to stop the current playback
  static volatile unsigned playstopsent;      // DECEVENT_PLAYSTOP has been sent. May be set to 0 to discard samples.
  static PM123_TIME        posoffset;         // Offset to posmarker parameter to make the output timeline monotonic
  static TechInfo          last_format;       // Format of last request_buffer
  static PM123_TIME        minpos;            // minimum sample position of a block from the decoder since the last dec_play
  static PM123_TIME        maxpos;            // maximum sample position of a block from the decoder since the last dec_play
  static int_ptr<Output>   output;            // currently initialized output plug-in.
  static PluginList        filters;           // List of initialized visual plug-ins.
  static int_ptr<Decoder>  decoder;           // currently active decoder plug-in.
  static delegate<void,const PluginEventArgs> plugin_deleg;

 private:
  /// Virtualize output procedures by invoking the filter plug-in no \a i.
  static void              virtualize         (int i);
  // setup the entire filter chain
  static ULONG             init();
  // uninitialize the filter chain
  static void              uninit();
  /// Select the currently active decoder
  /// @exception ModuleException Something went wrong.
  static void              dec_set_active     (const char* name);
  // Send command to the current decoder using the current content of dparams.
  static ULONG             dec_command        ( DECMSGTYPE msg );
  // Send command to the current output and filter chain using the current content of params.
  static ULONG             out_command        ( ULONG msg )
                                              { return (*procs.output_command)( procs.A, msg, &params ); }
  /// Handle plug-in changes while playing.
  static void              plugin_notification(void*, const PluginEventArgs& args);
 public:
  // output control interface (C style)
  friend ULONG             out_setup          ( const APlayable& song );
  friend ULONG             out_close          ();
  friend void              out_set_volume     ( double volume );
  friend ULONG             out_pause          ( BOOL pause );
  friend BOOL              out_flush          ();
  friend BOOL              out_trash          ();
  // decoder control interface (C style)
  friend ULONG             dec_play           ( const APlayable& song, PM123_TIME offset, PM123_TIME start, PM123_TIME stop );
  friend ULONG             dec_stop           ();
  friend ULONG             dec_fast           ( DECFASTMODE mode );
  friend ULONG             dec_jump           ( PM123_TIME pos );
  friend ULONG             dec_eq             ( const float* bandgain );
  friend ULONG             dec_save           ( const char* file );
  friend PM123_TIME        dec_minpos         ();
  friend PM123_TIME        dec_maxpos         ();
  friend ULONG DLLENTRY    dec_status         ();
  friend PM123_TIME DLLENTRY dec_length       ();
  // 4 visual interface (C style)
  friend PM123_TIME DLLENTRY out_playing_pos  ();
  friend BOOL   DLLENTRY   out_playing_data   ();
  friend ULONG  DLLENTRY   out_playing_samples( FORMAT_INFO* info, char* buf, int len );

 private: // glue
  PROXYFUNCDEF int DLLENTRY glue_request_buffer( void* a, const TECH_INFO* format, short** buf );
  PROXYFUNCDEF void DLLENTRY glue_commit_buffer( void* a, int len, PM123_TIME posmarker );
  // 4 callback interface
  PROXYFUNCDEF void DLLENTRY dec_event_handler( void* a, DECEVENTTYPE event, void* param );
  PROXYFUNCDEF void DLLENTRY out_event_handler( void* w, OUTEVENTTYPE event );
};

// statics
const dec_event_args Glue::ev_playstop = { DECEVENT_PLAYSTOP, NULL };

BOOL              Glue::initialized = false;
OutputProcs       Glue::procs;
OUTPUT_PARAMS2    Glue::params = {0};
DECODER_PARAMS2   Glue::dparams = {(const char*)NULL};
xstring           Glue::url;
PM123_TIME        Glue::stoptime;
volatile unsigned Glue::playstopsent;
PM123_TIME        Glue::posoffset;
TechInfo          Glue::last_format;
PM123_TIME        Glue::minpos;
PM123_TIME        Glue::maxpos;
int_ptr<Output>   Glue::output;
PluginList        Glue::filters(PLUGIN_FILTER);
int_ptr<Decoder>  Glue::decoder;
delegate<void,const PluginEventArgs> Glue::plugin_deleg(&Glue::plugin_notification);


void Glue::virtualize(int i)
{ DEBUGLOG(("Glue::virtualize(%i) - %p\n", i, params.Info));
  if (i < 0)
    return;

  Filter& fil = (Filter&)*filters[i];
  // virtualize procedures
  FILTER_PARAMS2 par;
  par.size                   = sizeof par;
  par.output_command         = procs.output_command;
  par.output_playing_samples = procs.output_playing_samples;
  par.output_request_buffer  = procs.output_request_buffer;
  par.output_commit_buffer   = procs.output_commit_buffer;
  par.output_playing_pos     = procs.output_playing_pos;
  par.output_playing_data    = procs.output_playing_data;
  par.a                      = procs.A;
  par.output_event           = params.OutEvent;
  par.w                      = params.W;
  if (!fil.Initialize(&par))
  { pm123_display_info(xstring::sprintf("The filter plug-in %s failed to initialize.", fil.ModRef.Key.cdata()));
    filters.erase(i);
    virtualize(i-1);
    return;
  }
  // store new entry points
  procs.output_command         = par.output_command;
  procs.output_playing_samples = par.output_playing_samples;
  procs.output_request_buffer  = par.output_request_buffer;
  procs.output_commit_buffer   = par.output_commit_buffer;
  procs.output_playing_pos     = par.output_playing_pos;
  procs.output_playing_data    = par.output_playing_data;
  procs.A                      = fil.GetProcs().F;
  void DLLENTRYP(last_output_event)(void* w, OUTEVENTTYPE event) = params.OutEvent;
  // next filter
  virtualize(i-1);
  // store new callback if virtualized by the plug-in.
  BOOL vcallback = par.output_event != last_output_event;
  last_output_event = par.output_event; // swap...
  par.output_event  = params.OutEvent;
  par.w             = params.W;
  if (vcallback)
  { // set params for next instance.
    params.OutEvent = last_output_event;
    params.W        = fil.GetProcs().F;
    DEBUGLOG(("Glue::virtualize: callback virtualized: %p %p\n", params.OutEvent, params.W));
  }
  if (par.output_event != last_output_event)
  { // now update the decoder event
    (*fil.GetProcs().filter_update)(fil.GetProcs().F, &par);
    DEBUGLOG(("Glue::virtualize: callback update: %p %p\n", par.output_event, par.w));
  }
}

// setup filter chain
ULONG Glue::init()
{ DEBUGLOG(("Glue::init\n"));

  Plugin::GetChangeEvent() += plugin_deleg;

  // setup callback handlers
  params.OutEvent  = &out_event_handler;
  //params.W         = NULL; // not required

  // setup callback handlers
  dparams.DecEvent = &dec_event_handler;
  //dparams.save_filename   = NULL;

  // Fetch current active output
  { PluginList outputs(PLUGIN_OUTPUT);
    Plugin::GetPlugins(outputs);
    if (outputs.size() == 0)
    { pm123_display_error("There is no active output plug-in.");
      return (ULONG)-1;  // ??
    }
    ASSERT(outputs.size() == 1);
    output = (Output*)outputs[0].get();
  }

  // initially only the output plugin
  output->InitPlugin();
  procs = output->GetProcs();
  // setup filters
  Plugin::GetPlugins(filters);
  virtualize(filters.size()-1);
  // setup output
  ULONG rc = out_command(OUTPUT_SETUP);
  if (rc == 0)
    initialized = TRUE;
  else
    uninit(); // deinit filters
  return rc;
}

void Glue::uninit()
{ DEBUGLOG(("Glue::uninit\n"));

  plugin_deleg.detach();

  initialized = FALSE;
  // uninitialize filter chain
  foreach (const int_ptr<Plugin>*, fpp, filters)
  { Filter& fil = (Filter&)**fpp;
    if (fil.IsInitialized())
      fil.UninitPlugin();
  }
  // Uninitialize output
  output->UninitPlugin();
  output.reset();
}

void Glue::dec_set_active(const char* name)
{ // Uninit current decoder if any
  if (decoder && decoder->IsInitialized())
  { int_ptr<Decoder> dp(decoder);
    decoder.reset();
    dp->UninitPlugin();
  }
  if (name == NULL)
    return;
  // Find new decoder
  int_ptr<Decoder> pd(Decoder::GetDecoder(name));
  pd->InitPlugin();
  decoder = pd;
}

ULONG Glue::dec_command(DECMSGTYPE msg)
{ DEBUGLOG(("dec_command(%d)\n", msg));
  if (decoder == NULL)
    return 3; // no decoder active
  ULONG ret = decoder->DecoderCommand(msg, &dparams);
  DEBUGLOG(("dec_command: %lu\n", ret));
  return ret;
}

/* invoke decoder to play an URL */
ULONG dec_play(const APlayable& song, PM123_TIME offset, PM123_TIME start, PM123_TIME stop)
{
  const xstring& url = song.GetPlayable().URL;
  DEBUGLOG(("dec_play(&%p{%s}, %f, %f,%g)\n", &song, url.cdata(), offset, start, stop));
  ASSERT((song.GetInfo().tech->attributes & TATTR_SONG));
  // set active decoder
  try
  { Glue::dec_set_active(xstring(song.GetInfo().tech->decoder));
  } catch (const ModuleException& ex)
  { pm123_display_error(ex.GetErrorText());
    return (ULONG)-2;
  }

  Glue::stoptime     = stop;
  Glue::playstopsent = FALSE;
  Glue::posoffset    = offset;
  Glue::minpos       = 1E99;
  Glue::maxpos       = 0;

  Glue::dparams.URL              = url;
  Glue::dparams.JumpTo           = start;
  Glue::dparams.OutRequestBuffer = &PROXYFUNCREF(Glue)glue_request_buffer;
  Glue::dparams.OutCommitBuffer  = &PROXYFUNCREF(Glue)glue_commit_buffer;
  Glue::dparams.A                = Glue::procs.A;

  ULONG rc = Glue::dec_command(DECODER_SETUP);
  if (rc != 0)
  { Glue::dec_set_active(NULL);
    return rc;
  }
  Glue::url = url;

  Glue::dec_command(DECODER_SAVEDATA);

  rc = Glue::dec_command(DECODER_PLAY);
  if (rc != 0)
    Glue::dec_set_active(NULL);
  else
  { // TODO: I hate this delay with a spinlock.
    int cnt = 0;
    while (Glue::decoder->DecoderStatus() == DECODER_STARTING)
    { DEBUGLOG(("dec_play - waiting for Spinlock\n"));
      DosSleep(++cnt > 10);
  } }
  return rc;
}

/* stop the current decoder immediately */
ULONG dec_stop()
{ Glue::url = NULL;
  ULONG rc = Glue::dec_command(DECODER_STOP);
  Glue::dec_set_active(NULL);
  return rc;
}

/* set fast forward/rewind mode */
ULONG dec_fast(DECFASTMODE mode)
{ Glue::dparams.Fast = mode;
  return Glue::dec_command(DECODER_FFWD);
}

/* jump to absolute position */
ULONG dec_jump(PM123_TIME location)
{ // Discard stop time if we seek beyond this point.
  if (location >= Glue::stoptime)
    Glue::stoptime = 1E99;
  Glue::dparams.JumpTo = location;
  ULONG rc = Glue::dec_command(DECODER_JUMPTO);
  if (rc == 0 && Glue::initialized)
  { Glue::params.PlayingPos = location;
    Glue::out_command(OUTPUT_TRASH_BUFFERS);
  }
  return rc;
}

/* set savefilename to save the raw stream data */
ULONG dec_save(const char* file)
{ if (file != NULL && *file == 0)
    file = NULL;
  Glue::dparams.SaveFilename = file;
  ULONG status = Glue::decoder ? Glue::decoder->DecoderStatus() : 0;
  return status == DECODER_PLAYING || status == DECODER_STARTING || status == DECODER_PAUSED
   ? Glue::dec_command( DECODER_SAVEDATA )
   : 0;
}

PM123_TIME dec_minpos()
{ return Glue::minpos;
}

PM123_TIME dec_maxpos()
{ return Glue::maxpos;
}


/* setup new output stage or change the properties of the current one */
ULONG out_setup( const APlayable& song )
{ const INFO_BUNDLE_CV& info = song.GetInfo();
  DEBUGLOG(("out_setup(&%p{%s,{%i,%i,%x...}})\n",
    &song, song.GetPlayable().URL.cdata(), info.tech->samplerate, info.tech->channels, info.tech->attributes));
  Glue::params.URL        = song.GetPlayable().URL;
  Glue::params.Info       = &info;
  // TODO: is this position correct?
  Glue::params.PlayingPos = Glue::posoffset;
  
  if (!Glue::initialized)
  { ULONG rc = Glue::init(); // here we initiate the setup of the filter chain
    if (rc != 0)
      return rc;
  }
  DEBUGLOG(("out_setup before out_command %p\n", Glue::params.Info));
  return Glue::out_command( OUTPUT_OPEN );
}

/* closes output device and uninitializes all filter and output plug-ins */
ULONG out_close()
{ if (!Glue::initialized)
    return (ULONG)-1;
  Glue::params.PlayingPos = (*Glue::procs.output_playing_pos)( Glue::procs.A );
  Glue::initialized = FALSE; // Disconnect decoder
  Glue::out_command( OUTPUT_TRASH_BUFFERS );
  ULONG rc = Glue::out_command( OUTPUT_CLOSE );
  Glue::uninit(); // Hmm, is it a good advise to do this in case of an error?
  return rc;
}

/* adjust output volume */
void out_set_volume( double volume )
{ if (!Glue::initialized)
    return; // can't help
  Glue::params.Volume = volume;
  Glue::params.Amplifier = 1.0;
  Glue::out_command( OUTPUT_VOLUME );
}

ULONG out_pause( BOOL pause )
{ if (!Glue::initialized)
    return (ULONG)-1; // error
  Glue::params.Pause = pause;
  return Glue::out_command( OUTPUT_PAUSE );
}

BOOL out_flush()
{ if (!Glue::initialized)
    return FALSE;
  (*Glue::procs.output_request_buffer)( Glue::procs.A, NULL, NULL );
  return TRUE;
}

BOOL out_trash()
{ if (!Glue::initialized)
    return FALSE;
  Glue::out_command( OUTPUT_TRASH_BUFFERS );
  return TRUE;
}

/* Returns 0 = success otherwize MMOS/2 error. */
ULONG DLLENTRY out_playing_samples( FORMAT_INFO* info, char* buf, int len )
{ if (!Glue::initialized)
    return (ULONG)-1; // N/A
  return (*Glue::procs.output_playing_samples)( Glue::procs.A, info, buf, len );
}

/* Returns time in ms. */
PM123_TIME DLLENTRY out_playing_pos( void )
{ if (!Glue::initialized)
    return 0; // ??
  return (*Glue::procs.output_playing_pos)( Glue::procs.A );
}

/* if the output is playing. */
BOOL DLLENTRY out_playing_data( void )
{ if (!Glue::initialized)
    return FALSE;
  return (*Glue::procs.output_playing_data)( Glue::procs.A );
}

PROXYFUNCIMP(int DLLENTRY, Glue)
glue_request_buffer( void* a, const TECH_INFO* format, short** buf )
{ if (!Glue::initialized)
    return 0;
  // do not pass flush, signal DECEVENT_PLAYSTOP instead.
  if (buf == NULL)
  { if (InterlockedXch(&Glue::playstopsent, TRUE) == FALSE)
      dec_event(Glue::ev_playstop);
    return 0;
  }
  // We are beyond the end?
  if (Glue::playstopsent)
    return 0;
  // pass request to output
  Glue::last_format = *format;
  return (*Glue::procs.output_request_buffer)(a, format, buf);
}

PROXYFUNCIMP(void DLLENTRY, Glue)
glue_commit_buffer( void* a, int len, PM123_TIME posmarker )
{ if (!Glue::initialized)
    return;
  bool send_playstop = false;
  PM123_TIME pos_e = posmarker + (PM123_TIME)len / Glue::last_format.samplerate;

  // check stop offset
  if (pos_e >= Glue::stoptime)
  { pos_e = Glue::stoptime;
    // calcutate fractional part
    len = posmarker >= Glue::stoptime
      ? 0 // begin is already beyond the limit => cancel request
      : (int)((Glue::stoptime - posmarker) * Glue::last_format.samplerate +.5);
    // Signal DECEVENT_PLAYSTOP if not yet sent.
    send_playstop = InterlockedXch(&Glue::playstopsent, TRUE) == FALSE;
  }

  // update min/max
  if (Glue::minpos > posmarker)
    Glue::minpos = posmarker;
  if (Glue::maxpos < pos_e)
    Glue::maxpos = pos_e;

  // pass to the output
  (*Glue::procs.output_commit_buffer)(a, len, posmarker + Glue::posoffset);

  // Signal DECEVENT_PLAYSTOP ?
  if (send_playstop)
    dec_event(Glue::ev_playstop);
}

PROXYFUNCIMP(void DLLENTRY, Glue)
dec_event_handler( void* a, DECEVENTTYPE event, void* param )
{ DEBUGLOG(("plugman:dec_event_handler(%p, %d, %p)\n", a, event, param));
  // We handle some event here immediately.
  switch (event)
  {case DECEVENT_CHANGEOBJ:
    /*if (Glue::url)
    { int_ptr<Playable> pp = Playable::FindByURL(Glue::url);
      if (pp)
        pp->SetTechInfo((TECH_INFO*)param);
    }*/
    break;

   case DECEVENT_CHANGEMETA:
    // TODO: Well, the backend does not support time dependant metadata.
    // From the playlist view, metadata changes should be immediately visible.
    // But during playback the change should be delayed until the apropriate buffer is really played.
    // The latter cannot be implemented with the current backend.
    if (Glue::url)
    { int_ptr<Playable> pp = Playable::FindByURL(Glue::url);
      if (pp)
        pp->SetMetaInfo((META_INFO*)param);
    }
    break;

   case DECEVENT_PLAYSTOP:
    Glue::stoptime = 0;
    // discard if already sent
    if (InterlockedXch(&Glue::playstopsent, TRUE))
      return;
   default: // avoid warnings
    break;
  }
  const dec_event_args args = { event, param };
  dec_event(args);
}

/* proxy, because the decoder is not interested in OUTEVENT_END_OF_DATA */
PROXYFUNCIMP(void DLLENTRY, Glue)
out_event_handler( void* w, OUTEVENTTYPE event )
{ DEBUGLOG(("plugman:out_event_handler(%p, %d)\n", w, event));
  out_event(event);
  // route high/low water events to the decoder (if any)
  switch (event)
  {case OUTEVENT_LOW_WATER:
   case OUTEVENT_HIGH_WATER:
    { if (decoder != NULL)
        decoder->DecoderEvent(event);
      break;
    }
   default: // avoid warnings
    break;
  }
}


/* Returns the decoder NAME that can play this file and returns 0
   if not returns error 200 = nothing can play that. */
ULONG DLLENTRY dec_fileinfo( const char* url, int* what, INFO_BUNDLE* info,
                             DECODER_INFO_ENUMERATION_CB cb, void* param )
{ DEBUGLOG(("dec_fileinfo(%s, *%x, %p{...}, %p, %p)\n", url, *what, info, cb, param));
  xio_clearerr(NULL);
  ASSERT(*what);
  PluginList decoders(PLUGIN_DECODER);
  Plugin::GetPlugins(decoders);
  bool* checked = (bool*)alloca(sizeof(bool) * decoders.size());
  Decoder* dp;
  int what2;
  const char* file = NULL;
  char* eadata = NULL;
  ULONG rc;
  size_t i;

  memset(checked, 0, sizeof *checked * decoders.size());

  int type_mask;
  if (is_cdda(url))
    type_mask = DECODER_TRACK;
  else if (strnicmp("file:", url, 5) == 0)
  { type_mask = DECODER_FILENAME;
    file = url + 5;
    if (file[2] == '/')
      file += 3;
    size_t size = 0;
    APIRET rc = eaget(file, ".type", &eadata, &size);
    if (rc != NO_ERROR)
    { // Errors of DosQueryPathInfo are considered to be permanent.
      char buf[256];
      os2_strerror(rc, buf, sizeof buf);
      info->tech->info = buf;
      // Even in case of an error the requested information is in fact available.
      *what = IF_Decoder|IF_Aggreg;
      info->phys->attributes = PATTR_INVALID;
      return PLUGIN_NO_READ;
    }
  } else if (strnicmp("http:", url, 5) == 0 || strnicmp("https:", url, 6) == 0 || strnicmp("ftp:", url, 4) == 0)
    type_mask = DECODER_URL;
  else
    type_mask = DECODER_OTHER;

  // First checks decoders supporting the specified type of files.
  for (i = 0; i < decoders.size(); i++)
  { dp = (Decoder*)decoders[i].get();
    DEBUGLOG(("dec_fileinfo: %s -> %u %x/%x\n", dp->ModRef.Key.cdata(),
      dp->GetEnabled(), dp->GetObjectTypes(), type_mask));
    if (dp->GetEnabled() && (dp->GetObjectTypes() & type_mask))
    { if (file && !dp->IsFileSupported(file, eadata))
        continue;
      what2 = *what;
      rc = dp->Fileinfo(url, &what2, info, cb, param);
      if (rc != PLUGIN_NO_PLAY)
        goto ok; // This also happens in case of a plug-in independent error
    }
    checked[i] = TRUE;
  }

  // Next checks the rest decoders.
  for (i = 0; i < decoders.size(); i++)
  { if (checked[i])
      continue;
    dp = (Decoder*)decoders[i].get();
    if (!dp->TryOthers)
      continue;
    what2 = *what;
    rc = dp->Fileinfo(url, &what2, info, cb, param);
    if (rc != PLUGIN_NO_PLAY)
      goto ok; // This also happens in case of a plug-in independent error
  }

  // No decoder found
  info->tech->info = "Cannot find a decoder that supports this item.";
  // Even in case of an error the requested information is in fact available.
  *what = IF_Decoder|IF_Aggreg;
  info->tech->attributes = TATTR_INVALID;
  return PLUGIN_NO_PLAY;
 ok:
  info->tech->decoder = dp->ModRef.Key;
  if (rc != 0)
  { info->phys->attributes |= PATTR_INVALID;
    if (info->tech->info == 0)
      info->tech->info = xstring::sprintf("Decoder error %i", rc);
  }
  DEBUGLOG(("dec_fileinfo: {PHYS{%.0f, %i, %x}, TECH{%i,%i, %x, %s, %s, %s}, OBJ{%.3f, %i, %i}, META{...} ATTR{%x, %s}, RPL{%d, %d, %d, %d}, DRPL{%f, %d, %.0f, %d}, ITEM{...}} -> %x\n",
    info->phys->filesize, info->phys->tstmp, info->phys->attributes,
    info->tech->samplerate, info->tech->channels, info->tech->attributes,
      info->tech->info.cdata(), info->tech->format.cdata(), info->tech->decoder.cdata(),
    info->obj->songlength, info->obj->bitrate, info->obj->num_items,
    info->attr->ploptions, info->attr->at.cdata(),
    info->rpl->songs, info->rpl->lists, info->rpl->lists, info->rpl->unknown,
    info->drpl->totallength, info->drpl->unk_length, info->drpl->totalsize, info->drpl->unk_size,
    what2));
  ASSERT((*what & ~what2) == 0); // The decoder must not reset bits.
  *what = what2 | *what; // do not reset bits
  return rc;
}


class FileTypesEnumerator : public IFileTypesEnumerator
{private:
  PluginList              Decoders;
  size_t                  DecNo;
  const vector<const DECODER_FILETYPE>* Current;
  size_t                  Item;
 public:
  FileTypesEnumerator();
  virtual const DECODER_FILETYPE* GetCurrent() const;
  virtual int_ptr<Decoder> GetDecoder() const;
  virtual bool            Next();
};

FileTypesEnumerator::FileTypesEnumerator()
: Decoders(PLUGIN_DECODER)
, Current(NULL)
{ Plugin::GetPlugins(Decoders);
}

const DECODER_FILETYPE* FileTypesEnumerator::GetCurrent() const
{ return (*Current)[Item];
}

int_ptr<Decoder> FileTypesEnumerator::GetDecoder() const
{ return (Decoder*)Decoders[DecNo];
}

bool FileTypesEnumerator::Next()
{ DEBUGLOG(("FileTypesEnumerator(%p)::Next() - %p\n", this, Current));
  if (!Current)
  { DecNo = 0;
   load_decoder:
    Current = &((const Decoder&)*Decoders[DecNo]).GetFileTypes();
    Item = 0;
  } else
    ++Item;
  if (Item < Current->size())
    return true;
  while (++DecNo < Decoders.size())
    if (Decoders[DecNo]->GetEnabled())
      goto load_decoder;
  Current = NULL;
  return false;
}

class FilteredFileTypesEnumerator : public FileTypesEnumerator
{private:
  const DECODER_TYPE      FlagsReq;
 public:
  FilteredFileTypesEnumerator(DECODER_TYPE flagsreq) : FlagsReq(flagsreq) {}
  virtual bool            Next();
};

bool FilteredFileTypesEnumerator::Next()
{ while (FileTypesEnumerator::Next())
  { if ((~GetCurrent()->flags & FlagsReq) == 0)
      return true;
  }
  return false;
}

IFileTypesEnumerator* dec_filetypes(DECODER_TYPE flagsreq)
{ DEBUGLOG(("dec_filetypes(%x)\n", flagsreq));
  return new FilteredFileTypesEnumerator(flagsreq);
}


ULONG dec_saveinfo(const char* url, const META_INFO* info, DECODERMETA haveinfo, const char* decoder_name, xstring& errortxt)
{ DEBUGLOG(("dec_saveinfo(%s, %p, %x, %s)\n", url, info, haveinfo, decoder_name));
  xio_clearerr(NULL);
  ULONG rc;
  try
  { int_ptr<Decoder> dp = Decoder::GetDecoder(decoder_name);
    rc = dp->SaveInfo(url, info, haveinfo, errortxt);
  } catch (const ModuleException& ex)
  { rc = PLUGIN_FAILED;
    errortxt = ex.GetErrorText();
  }
  DEBUGLOG(("dec_saveinfo: %d - %s\n", rc, errortxt.cdata()));
  return rc;
}

ULONG dec_editmeta(HWND owner, const char* url, const char* decoder_name)
{ DEBUGLOG(("dec_editmeta(%x, %s, %s)\n", owner, url, decoder_name));
  ULONG rc;
  xio_clearerr(NULL);
  try
  { int_ptr<Decoder> dp = Decoder::GetDecoder(decoder_name);
    rc = dp->EditMeta(owner, url);
  } catch (const ModuleException&)
  { rc = 100;
  }
  DEBUGLOG(("dec_editmeta: %d\n", rc));
  return rc;
}

struct save_cb_data
{ Playable&                 Parent;
  const bool                Relative;
  int_ptr<PlayableInstance> Current;
  InfoBundle                Info;
  save_cb_data(Playable& parent, bool relative)
  : Parent(parent), Relative(relative)
  { Info.Assign(parent.GetInfo()); }
};

static int DLLENTRY save_callback(void* param, xstring* url,
  const INFO_BUNDLE** info, int* valid, int* override)
{ save_cb_data& cbd = *(save_cb_data*)param;
  // TODO: This is a race condition, because the exact content that is saved is not
  // well defined if the list is currently manipulation. Normally a snapshot should be taken.
  cbd.Current = cbd.Parent.GetNext(cbd.Current);
  if (cbd.Current == NULL)
    return PLUGIN_FAILED;
  *url = cbd.Relative
    ? cbd.Current->GetPlayable().URL.makeRelative(cbd.Parent.URL)
    : cbd.Current->GetPlayable().URL;
  cbd.Info.Assign(cbd.Current->GetInfo());
  *info  = &cbd.Info;
  *valid = cbd.Current->GetValid();
  *override = cbd.Current->GetOverridden();
  return PLUGIN_OK;
}

ULONG dec_saveplaylist(const char* url, Playable& playlist, const char* decoder, const char* format, bool relative)
{ DEBUGLOG(("dec_saveplaylist(%s, {%s}, %s, %s, %u)\n", url, playlist.URL.cdata(), decoder, format, relative));
  ULONG rc;
  xio_clearerr(NULL);
  try
  { int_ptr<Decoder> dp = Decoder::GetDecoder(decoder);
    save_cb_data cbd(playlist, relative);
    int what = IF_Child;
    rc = dp->SaveFile(url, format, &what, &cbd.Info, &save_callback, &cbd);
  } catch (const ModuleException&)
  { rc = PLUGIN_FAILED;
  }
  DEBUGLOG(("dec_editmeta: %d\n", rc));
  return rc;
}

ULONG DLLENTRY dec_status()
{ // TODO: this copy is still not 100% thread safe
  int_ptr<Decoder> dp(Glue::decoder);
  return dp ? dp->DecoderStatus() : DECODER_ERROR;
}

PM123_TIME DLLENTRY dec_length()
{ // TODO: this copy is still not 100% thread safe
  int_ptr<Decoder> dp(Glue::decoder);
  return dp ? dp->DecoderLength() : -1;
}

static time_t nomsgtill = 0;

void Glue::plugin_notification(void*, const PluginEventArgs& args)
{ DEBUGLOG(("Glue::plugin_notification(, {&%p{%s}, %i})\n", &args.Plug, args.Plug.ModRef.Key.cdata(), args.Operation));
  switch (args.Operation)
  {case PluginEventArgs::Load:
   case PluginEventArgs::Unload:
    if (!args.Plug.GetEnabled())
      break;
   case PluginEventArgs::Enable:
   case PluginEventArgs::Disable:
    switch (args.Plug.PluginType)
    {case PLUGIN_DECODER:
     case PLUGIN_FILTER:
     case PLUGIN_OUTPUT:
      { time_t t = time(NULL);
        if (t > nomsgtill)
          pm123_display_info("Changes to decoder, filter or output plug-in will not take effect until playback stops.");
        nomsgtill = t + 15; // disable the message for a few seconds.
      }
     default:; // avoid warnings
    }
   default:; // avoid warnings
  }
}
