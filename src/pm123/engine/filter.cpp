/*
 * Copyright 2006-2011 M.Mueller
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

#include "filter.h"
#include "../core/infobundle.h"
#include "../eventhandler.h"
#include "proxyhelper.h"
#include <vdelegate.h>

#include <debuglog.h>


/****************************************************************************
*
* filter interface
*
****************************************************************************/

Filter::~Filter()
{ ModRef->Fil = NULL;
}

/* Assigns the addresses of the filter plug-in procedures. */
void Filter::LoadPlugin()
{ const Module& mod = *ModRef;
  DEBUGLOG(("Filter(%p{%s})::LoadPlugin\n", this, mod.Key.cdata()));
  F = NULL;
  mod.LoadMandatoryFunction(&filter_init,   "filter_init");
  mod.LoadMandatoryFunction(&filter_update, "filter_update");
  mod.LoadMandatoryFunction(&filter_uninit, "filter_uninit");
}

bool Filter::InitPlugin()
{ return true; // filters are not initialized unless they are used
}

bool Filter::UninitPlugin()
{ DEBUGLOG(("Filter(%p{%s})::UninitPlugin\n", this, ModRef->Key.cdata()));

  if (IsInitialized())
  { (*filter_uninit)(F);
    F = NULL;
    RaisePluginChange(PluginEventArgs::Uninit);
  }
  return true;
}

bool Filter::Initialize(FILTER_PARAMS2& params)
{ DEBUGLOG(("Filter(%p{%s})::Initialize(&%p)\n", this, ModRef->Key.cdata(), &params));

  Params = params;
  if (IsInitialized() || (*filter_init)(&F, &params) != 0)
    return false;

  if (F == NULL)
  { // plug-in does not require local structures
    // => pass the pointer of the next stage and skip virtualization of untouched function
    F = Params.a;
  } else
  { // virtualize untouched functions
    if (Params.output_command          == params.output_command)
      params.output_command         = VRStubs[0].assign(Params.output_command, Params.a);
    if (Params.output_playing_samples  == params.output_playing_samples)
      params.output_playing_samples = VRStubs[1].assign(Params.output_playing_samples, Params.a);
    if (Params.output_request_buffer   == params.output_request_buffer)
      params.output_request_buffer  = VRStubs[2].assign(Params.output_request_buffer, Params.a);
    if (Params.output_commit_buffer    == params.output_commit_buffer)
      params.output_commit_buffer   = VRStubs[3].assign(Params.output_commit_buffer, Params.a);
    if (Params.output_playing_pos      == params.output_playing_pos)
      params.output_playing_pos     = VRStubs[4].assign(Params.output_playing_pos, Params.a);
    if (Params.output_playing_data     == params.output_playing_data)
      params.output_playing_data    = VRStubs[5].assign(Params.output_playing_data, Params.a);
  }
  RaisePluginChange(PluginEventArgs::Init);
  return true;
}

void Filter::UpdateEvent(const FILTER_PARAMS2& params)
{ Params.w = params.w;
  Params.output_event = params.output_event;
  (*filter_update)(F, &Params);
}


// proxy for level 1 filters
class FilterProxy1 : public Filter, protected ProxyHelper
{private:
  int   DLLENTRYP(vfilter_init         )( void** f, FILTER_PARAMS* params );
  BOOL  DLLENTRYP(vfilter_uninit       )( void*  f );
  int   DLLENTRYP(vfilter_play_samples )( void*  f, const FORMAT_INFO* format, const char *buf, int len, int posmarker );
  void* vf;
  int   DLLENTRYP(output_request_buffer)( struct FILTER_STRUCT* a, const FORMAT_INFO2* format, float** buf );
  void  DLLENTRYP(output_commit_buffer )( struct FILTER_STRUCT* a, int len, PM123_TIME posmarker );
  struct FILTER_STRUCT* a;
  FORMAT_INFO vformat;                      // format of the samples (old style)
  FORMAT_INFO2 vformat2;                    // format of the samples (new style)
  union
  { float     fbuf[BUFSIZE/2];              // buffer to store incoming samples
    short     sbuf[BUFSIZE/2];              // buffer to store 16 bit converted samples
  }           vbuffer;
  int         vbufsamples;                  // size of vbuffer in samples
  int         vbuflevel;                    // current filled to vbuflevel
  PM123_TIME  vposmarker;                   // starting point of the current buffer
  BOOL        trash_buffer;                 // TRUE: signal to discard any buffer content
  VDELEGATE   vd_filter_init;
  VREPLACE1   vr_filter_update;
  VREPLACE1   vr_filter_uninit;

 private:
  PROXYFUNCDEF ULONG DLLENTRY proxy_1_filter_init          ( FilterProxy1* pp, struct FILTER_STRUCT** f, FILTER_PARAMS2* params );
  PROXYFUNCDEF void  DLLENTRY proxy_1_filter_update        ( FilterProxy1* pp, const FILTER_PARAMS2* params );
  PROXYFUNCDEF BOOL  DLLENTRY proxy_1_filter_uninit        ( struct FILTER_STRUCT* f ); // empty stub
  PROXYFUNCDEF int   DLLENTRY proxy_1_filter_request_buffer( FilterProxy1* f, const FORMAT_INFO2* format, float** buf );
  PROXYFUNCDEF void  DLLENTRY proxy_1_filter_commit_buffer ( FilterProxy1* f, int len, PM123_TIME posmarker );
  PROXYFUNCDEF int   DLLENTRY proxy_1_filter_play_samples  ( FilterProxy1* f, const FORMAT_INFO* format, const char *buf, int len, int posmarker );
  void         SendSamples();
 public:
  FilterProxy1(Module& mod) : Filter(mod) {}
  virtual void LoadPlugin();
};

void FilterProxy1::LoadPlugin()
{ const Module& mod = *ModRef;
  DEBUGLOG(("FilterProxy1(%p{%s})::LoadPlugin()\n", this, mod.Key.cdata()));
  F = NULL;
  mod.LoadMandatoryFunction(&vfilter_init,         "filter_init");
  mod.LoadMandatoryFunction(&vfilter_uninit,       "filter_uninit");
  mod.LoadMandatoryFunction(&vfilter_play_samples, "filter_play_samples");

  filter_init   = vd_filter_init.assign(&proxy_1_filter_init,   this);
  filter_update = (void DLLENTRYPF()(struct FILTER_STRUCT*, const FILTER_PARAMS2*)) // type of parameter is replaced too
                  vr_filter_update.assign(&proxy_1_filter_update, this);
  // filter_uninit is initialized at the filter_init call to a non-no-op function
  // However, the returned pointer will stay the same.
  filter_uninit = vr_filter_uninit.assign(&proxy_1_filter_uninit, (struct FILTER_STRUCT*)NULL);
}

inline void FilterProxy1::SendSamples()
{ DEBUGLOG(("FilterProxy1(%p)::SendSamples() - full: %d\n", this, vbuflevel));
  // convert samples to 16 bit short
  Float2Short(vbuffer.sbuf, vbuffer.fbuf, vbuflevel * vformat.channels);
  // Send samples to filter
  (*vfilter_play_samples)(vf, &vformat, (char*)vbuffer.sbuf, vbuflevel * vformat.channels * sizeof(short), TstmpF2I(vposmarker));
  vbuflevel = 0;
}

PROXYFUNCIMP(ULONG DLLENTRY, FilterProxy1)
proxy_1_filter_init(FilterProxy1* pp, struct FILTER_STRUCT** f, FILTER_PARAMS2* params)
{ DEBUGLOG(("proxy_1_filter_init(%p{%s}, %p, %p{a=%p})\n", pp, pp->ModRef->Key.cdata(), f, params, params->a));

  FILTER_PARAMS par;
  par.size                = sizeof par;
  par.output_play_samples = (int DLLENTRYPF()(void*, const FORMAT_INFO*, const char*, int, int))
                            &PROXYFUNCREF(FilterProxy1)proxy_1_filter_play_samples;
  par.a                   = pp;
  par.audio_buffersize    = BUFSIZE;
  par.error_display       = &PROXYFUNCREF(ProxyHelper)PluginDisplayError;
  par.info_display        = &PROXYFUNCREF(ProxyHelper)PluginDisplayInfo;
  par.pm123_getstring     = &PROXYFUNCREF(ProxyHelper)PluginGetString;
  par.pm123_control       = &PROXYFUNCREF(ProxyHelper)PluginControl;
  int r = (*pp->vfilter_init)(&pp->vf, &par);
  if (r != 0)
    return r;
  // save some values
  pp->output_request_buffer = params->output_request_buffer;
  pp->output_commit_buffer  = params->output_commit_buffer;
  pp->a                     = params->a;
  // setup internals
  pp->vbuflevel             = 0;
  pp->trash_buffer          = FALSE;
  pp->vformat.bits          = 16;
  pp->vformat.format        = WAVE_FORMAT_PCM;
  // replace the unload function
  pp->vr_filter_uninit.assign(pp->vfilter_uninit, pp->vf);
  // now return some values
  *f = (struct FILTER_STRUCT*)pp;
  params->output_request_buffer = (int  DLLENTRYPF()(struct FILTER_STRUCT*, const FORMAT_INFO2*, float**))
                                  &PROXYFUNCREF(FilterProxy1)proxy_1_filter_request_buffer;
  params->output_commit_buffer  = (void DLLENTRYPF()(struct FILTER_STRUCT*, int, PM123_TIME))
                                  &PROXYFUNCREF(FilterProxy1)proxy_1_filter_commit_buffer;
  return 0;
}

PROXYFUNCIMP(void DLLENTRY, FilterProxy1)
proxy_1_filter_update(FilterProxy1* pp, const FILTER_PARAMS2* params)
{ DEBUGLOG(("proxy_1_filter_update(%p{%s}, %p)\n", pp, pp->ModRef->Key.cdata(), params));

  CritSect cs;
  // replace function pointers
  pp->output_request_buffer = params->output_request_buffer;
  pp->output_commit_buffer  = params->output_commit_buffer;
  pp->a                     = params->a;
}

PROXYFUNCIMP(BOOL DLLENTRY, FilterProxy1)
proxy_1_filter_uninit(struct FILTER_STRUCT*)
{ return TRUE;
}

PROXYFUNCIMP(int DLLENTRY, FilterProxy1)
proxy_1_filter_request_buffer(FilterProxy1* pp, const FORMAT_INFO2* format, float** buf)
{ DEBUGLOG(("proxy_1_filter_request_buffer(%p, %p, %p)\n", pp, format, buf));

  if ( pp->trash_buffer )
  { pp->vbuflevel = 0;
    pp->trash_buffer = FALSE;
  }

  if ( buf == 0
    || ( pp->vbuflevel != 0 &&
         (pp->vformat.samplerate != format->samplerate || pp->vformat.channels != format->channels) ))
    // local flush
    pp->SendSamples();
  if ( buf == 0 )
  { return (*pp->output_request_buffer)( pp->a, format, NULL );
  }
  pp->vformat.samplerate = format->samplerate;
  pp->vformat.channels   = format->channels;
  pp->vbufsamples   = sizeof pp->vbuffer.fbuf / sizeof *pp->vbuffer.fbuf / format->channels;

  DEBUGLOG(("proxy_1_filter_request_buffer: %d\n", pp->vbufsamples - pp->vbuflevel));
  *buf = pp->vbuffer.fbuf + pp->vbuflevel * format->channels;
  return pp->vbufsamples - pp->vbuflevel;
}

PROXYFUNCIMP(void DLLENTRY, FilterProxy1)
proxy_1_filter_commit_buffer( FilterProxy1* pp, int len, PM123_TIME posmarker )
{ DEBUGLOG(("proxy_1_filter_commit_buffer(%p, %d, %g)\n", pp, len, posmarker));

  if (len == 0)
    return;

  if (pp->vbuflevel == 0)
    pp->vposmarker = posmarker;

  pp->vbuflevel += len;
  if (pp->vbuflevel == pp->vbufsamples)
    // buffer full
    pp->SendSamples();
}

PROXYFUNCIMP(int DLLENTRY, FilterProxy1)
proxy_1_filter_play_samples(FilterProxy1* pp, const FORMAT_INFO* format, const char *buf, int len, int posmarker_i)
{ DEBUGLOG(("proxy_1_filter_play_samples(%p, %p{%d,%d,%d,%d}, %p, %d, %d)\n",
    pp, format, format->samplerate, format->channels, format->bits, format->format, buf, len, posmarker_i));

  if (format->format != WAVE_FORMAT_PCM || format->bits != 16)
  { EventHandler::Post(MSG_ERROR, "The proxy for old style filter plug-ins can only handle 16 bit raw PCM data.");
    return 0;
  }
  PM123_TIME posmarker = TstmpI2F(posmarker_i, pp->vposmarker);
  len /= pp->vformat.channels * sizeof(short);
  int rem = len;
  while (rem != 0)
  { // request new buffer
    float* dest;
    pp->vformat2.samplerate = format->samplerate;
    pp->vformat2.channels   = format->channels;
    int dlen = (*pp->output_request_buffer)( pp->a, &pp->vformat2, &dest );
    DEBUGLOG(("proxy_1_filter_play_samples: now at %p %d, %p, %d\n", buf, rem, dest, dlen));
    if (dlen <= 0)
      return 0; // error
    if (dlen > rem)
      dlen = rem;
    // store data
    ProxyHelper::Short2Float(dest, (short*)buf, dlen * pp->vformat.channels);
    // commit destination
    (*pp->output_commit_buffer)( pp->a, dlen, posmarker + (PM123_TIME)(len-rem)/format->samplerate );
    buf += dlen * pp->vformat.channels * sizeof(short);
    rem -= dlen;
  }
  return len * pp->vformat.channels * sizeof(short);
}


int_ptr<Filter> Filter::FindInstance(const Module& module)
{ Mutex::Lock lock(Module::Mtx);
  Filter* fil = module.Fil;
  return fil && !fil->RefCountIsUnmanaged() ? fil : NULL;
}

int_ptr<Filter> Filter::GetInstance(Module& module)
{ ASSERT(getTID() == 1);
  if ((module.GetParams().type & PLUGIN_FILTER) == 0)
    throw ModuleException("Cannot load plug-in %s as filter.", module.Key.cdata());
  Mutex::Lock lock(Module::Mtx);
  Filter* fil = module.Fil;
  if (fil && !fil->RefCountIsUnmanaged())
    return fil;
  if (module.GetParams().interface == 2)
    throw ModuleException("The filter plug-in %s is not supported. It is intended for PM123 1.40.", module.Key.cdata());
  fil = module.GetParams().interface <= 1 ? new FilterProxy1(module) : new Filter(module);
  try
  { fil->LoadPlugin();
  } catch (...)
  { delete fil;
    throw;
  }
  return module.Fil = fil;
}
