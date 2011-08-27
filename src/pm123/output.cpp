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

/* This is the core interface to the plug-ins. It loads the plug-ins and
 * virtualizes them if required to refect always the most recent interface
 * level to the application.
 */

#define  INCL_PM
#define  INCL_DOS
#define  INCL_ERRORS

#include "output.h"
#include "123_util.h"
#include "pm123.h" // for hab

#include <debuglog.h>


/****************************************************************************
*
* output interface
*
****************************************************************************/

delegate<void, const PluginEventArgs> Output::PluginDeleg(&Output::PluginNotification);

void Output::PluginNotification(void*, const PluginEventArgs& args)
{ if (args.Plug.PluginType == PLUGIN_OUTPUT)
  { switch(args.Operation)
    {case PluginEventArgs::Load:
      if (!args.Plug.GetEnabled())
        break;
     case PluginEventArgs::Enable:
      // Output plug-ins are like a radio button. Only one plug-in is enabled at a time.
      foreach (const int_ptr<Plugin>*, ppp, Outputs)
        if (*ppp != &args.Plug)
          (*ppp)->SetEnabled(false);
     default:;
    }
  }
}

/* Assigns the addresses of the output plug-in procedures. */
void Output::LoadPlugin()
{ DEBUGLOG(("Output(%p{%s})::LoadPlugin()\n", this, ModRef.Key.cdata()));
  A = NULL;
  const Module& mod = ModRef;
  mod.LoadMandatoryFunction(&output_init,            "output_init");
  mod.LoadMandatoryFunction(&output_uninit,          "output_uninit");
  mod.LoadMandatoryFunction(&output_playing_samples, "output_playing_samples");
  mod.LoadMandatoryFunction(&output_playing_pos,     "output_playing_pos");
  mod.LoadMandatoryFunction(&output_playing_data,    "output_playing_data");
  mod.LoadMandatoryFunction(&output_command,         "output_command");
  mod.LoadMandatoryFunction(&output_request_buffer,  "output_request_buffer");
  mod.LoadMandatoryFunction(&output_commit_buffer,   "output_commit_buffer");
}

bool Output::InitPlugin()
{ DEBUGLOG(("Output(%p{%s})::InitPlugin()\n", this, ModRef.Key.cdata()));

  if ((*output_init)(&A) != 0)
  { A = NULL;
    return false;
  }
  RaisePluginChange(PluginEventArgs::Init);
  return true;
}

bool Output::UninitPlugin()
{ DEBUGLOG(("Output(%p{%s})::UninitPlugin()\n", this, ModRef.Key.cdata()));

  if (IsInitialized())
  { (*output_command)(A, OUTPUT_CLOSE, NULL);
    (*output_uninit)(A);
    A = NULL;
    RaisePluginChange(PluginEventArgs::Uninit);
  }
  return true;
}


// Proxy for loading level 1 plug-ins
class OutputProxy1 : public Output, protected ProxyHelper
{private:
  int         DLLENTRYP(voutput_command     )(void* a, ULONG msg, OUTPUT_PARAMS* info);
  int         DLLENTRYP(voutput_play_samples)(void* a, const FORMAT_INFO* format, const char* buf, int len, int posmarker);
  ULONG       DLLENTRYP(voutput_playing_pos )(void* a);
  short       voutput_buffer[BUFSIZE/2];
  int         voutput_buffer_level;             // current level of voutput_buffer
  BOOL        voutput_trash_buffer;
  BOOL        voutput_flush_request;            // flush-request received, generate OUTEVENT_END_OF_DATA from WM_OUTPUT_OUTOFDATA
  HWND        voutput_hwnd;                     // Window handle for catching event messages
  double      voutput_posmarker;
  FORMAT_INFO voutput_format;
  int         voutput_bufsamples;
  BOOL        voutput_always_hungry;
  void        DLLENTRYP(voutput_event)(void* w, OUTEVENTTYPE event);
  void*       voutput_w;
  VDELEGATE   vd_output_command, vd_output_request_buffer, vd_output_commit_buffer, vd_output_playing_pos;

 private:
  PROXYFUNCDEF ULONG  DLLENTRY proxy_1_output_command       (OutputProxy1* op, void* a, ULONG msg, OUTPUT_PARAMS2* info);
  PROXYFUNCDEF int    DLLENTRY proxy_1_output_request_buffer(OutputProxy1* op, void* a, const TECH_INFO* format, short** buf);
  PROXYFUNCDEF void   DLLENTRY proxy_1_output_commit_buffer (OutputProxy1* op, void* a, int len, double posmarker);
  PROXYFUNCDEF double DLLENTRY proxy_1_output_playing_pos   (OutputProxy1* op, void* a);
  friend MRESULT EXPENTRY proxy_1_output_winfn(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
 public:
  OutputProxy1(Module& mod) : Output(mod), voutput_hwnd(NULLHANDLE), voutput_posmarker(0) {}
  virtual ~OutputProxy1();
  virtual void LoadPlugin();
};

OutputProxy1::~OutputProxy1()
{ if (voutput_hwnd != NULLHANDLE)
    WinDestroyWindow(voutput_hwnd);
}

/* Assigns the addresses of the out7put plug-in procedures. */
void OutputProxy1::LoadPlugin()
{ DEBUGLOG(("OutputProxy1(%p{%s})::LoadPlugin()\n", this, ModRef.Key.cdata()));
  A = NULL;
  const Module& mod = ModRef;
  mod.LoadMandatoryFunction(&output_init,            "output_init");
  mod.LoadMandatoryFunction(&output_uninit,          "output_uninit");
  mod.LoadMandatoryFunction(&output_playing_samples, "output_playing_samples");
  mod.LoadMandatoryFunction(&voutput_playing_pos,    "output_playing_pos");
  mod.LoadMandatoryFunction(&output_playing_data,    "output_playing_data");
  mod.LoadMandatoryFunction(&voutput_command,        "output_command");
  mod.LoadMandatoryFunction(&voutput_play_samples,   "output_play_samples");

  output_command        = vdelegate(&vd_output_command,        &proxy_1_output_command,        this);
  output_request_buffer = vdelegate(&vd_output_request_buffer, &proxy_1_output_request_buffer, this);
  output_commit_buffer  = vdelegate(&vd_output_commit_buffer,  &proxy_1_output_commit_buffer,  this);
  output_playing_pos    = vdelegate(&vd_output_playing_pos,    &proxy_1_output_playing_pos,    this);
}

/* virtualization of level 1 output plug-ins */
PROXYFUNCIMP(ULONG DLLENTRY, OutputProxy1)
proxy_1_output_command(OutputProxy1* op, void* a, ULONG msg, OUTPUT_PARAMS2* info)
{ DEBUGLOG(("proxy_1_output_command(%p {%s}, %p, %d, %p)\n", op, op->ModRef.Key.cdata(), a, msg, info));

  if (info == NULL) // sometimes info is NULL
    return (*op->voutput_command)(a, msg, NULL);

  OUTPUT_PARAMS params = { sizeof params };
  DECODER_INFO  dinfo  = { sizeof dinfo };

  // preprocessing
  switch (msg)
  {case OUTPUT_TRASH_BUFFERS:
    op->voutput_trash_buffer   = TRUE;
    break;

   case OUTPUT_SETUP:
    op->voutput_hwnd = OutputProxy1::CreateProxyWindow("OutputProxy1", op);
   case OUTPUT_OPEN:
    { // convert DECODER_INFO2 to DECODER_INFO
      ProxyHelper::ConvertINFO_BUNDLE(&dinfo, info->Info);
      params.formatinfo        = dinfo.format;
      params.info              = &dinfo;
      params.hwnd              = op->voutput_hwnd;
      break;
    }
  }
  params.buffersize            = BUFSIZE;
  params.boostclass            = DECODER_HIGH_PRIORITY_CLASS;
  params.normalclass           = DECODER_LOW_PRIORITY_CLASS;
  params.boostdelta            = DECODER_HIGH_PRIORITY_DELTA;
  params.normaldelta           = DECODER_LOW_PRIORITY_DELTA;
  params.nobuffermode          = FALSE;
  params.error_display         = &pm123_display_error;
  params.info_display          = &pm123_display_info;
  params.volume                = (char)(info->Volume*100+.5);
  params.amplifier             = info->Amplifier;
  params.pause                 = info->Pause;
  params.temp_playingpos       = TstmpF2I(info->PlayingPos);

  if (info->URL != NULL && strnicmp(info->URL, "file:", 5) == 0)
  { char* fname = (char*)alloca(strlen(info->URL)+1);
    strcpy(fname, info->URL);
    params.filename            = ConvertUrl2File(fname);
  } else
    params.filename            = info->URL;

  // call plug-in
  int r = (*op->voutput_command)(a, msg, &params);

  // postprocessing
  switch (msg)
  {case OUTPUT_SETUP:
    op->voutput_buffer_level   = 0;
    op->voutput_trash_buffer   = FALSE;
    op->voutput_flush_request  = FALSE;
    op->voutput_always_hungry  = params.always_hungry;
    op->voutput_event          = info->OutEvent;
    op->voutput_w              = info->W;
    op->voutput_format.bits    = 16;
    op->voutput_format.format  = WAVE_FORMAT_PCM;
    break;

   case OUTPUT_CLOSE:
    OutputProxy1::DestroyProxyWindow(op->voutput_hwnd);
    op->voutput_hwnd = NULLHANDLE;
  }
  DEBUGLOG(("proxy_1_output_command: %d\n", r));
  return r;
}

PROXYFUNCIMP(int DLLENTRY, OutputProxy1)
proxy_1_output_request_buffer( OutputProxy1* op, void* a, const TECH_INFO* format, short** buf )
{
  #ifdef DEBUG_LOG
  if (format != NULL)
    DEBUGLOG(("proxy_1_output_request_buffer(%p, %p, {%i,%i,%x...}, %p) - %d\n",
      op, a, format->samplerate, format->channels, format->attributes, buf, op->voutput_buffer_level));
   else
    DEBUGLOG(("proxy_1_output_request_buffer(%p, %p, %p, %p) - %d\n", op, a, format, buf, op->voutput_buffer_level));
  #endif

  if (op->voutput_trash_buffer)
  { op->voutput_buffer_level = 0;
    op->voutput_trash_buffer = FALSE;
  }

  if ( op->voutput_buffer_level != 0
    && ( buf == 0
      || (op->voutput_format.samplerate != format->samplerate || op->voutput_format.channels != format->channels) ))
  { // flush
    (*op->voutput_play_samples)(a, &op->voutput_format, (char*)op->voutput_buffer, op->voutput_buffer_level * op->voutput_format.channels * sizeof(short), ProxyHelper::TstmpF2I(op->voutput_posmarker));
    op->voutput_buffer_level = 0;
  }
  if (buf == 0)
  { if (op->voutput_always_hungry)
      (*op->voutput_event)(op->voutput_w, OUTEVENT_END_OF_DATA);
     else
      op->voutput_flush_request = TRUE; // wait for WM_OUTPUT_OUTOFDATA
    return 0;
  }

  *buf = op->voutput_buffer + op->voutput_buffer_level * format->channels;
  op->voutput_format.samplerate = format->samplerate;
  op->voutput_format.channels   = format->channels;
  op->voutput_bufsamples        = sizeof op->voutput_buffer / sizeof *op->voutput_buffer / format->channels;
  DEBUGLOG(("proxy_1_output_request_buffer: %d\n", op->voutput_bufsamples - op->voutput_buffer_level));
  return op->voutput_bufsamples - op->voutput_buffer_level;
}

PROXYFUNCIMP(void DLLENTRY, OutputProxy1)
proxy_1_output_commit_buffer( OutputProxy1* op, void* a, int len, double posmarker )
{ DEBUGLOG(("proxy_1_output_commit_buffer(%p {%s}, %p, %i, %g) - %d\n",
    op, op->ModRef.Key.cdata(), a, len, posmarker, op->voutput_buffer_level));

  if (op->voutput_buffer_level == 0)
    op->voutput_posmarker = posmarker;

  op->voutput_buffer_level += len;
  if (op->voutput_buffer_level == op->voutput_bufsamples)
  { (*op->voutput_play_samples)(a, &op->voutput_format, (char*)op->voutput_buffer, op->voutput_buffer_level * op->voutput_format.channels * sizeof(short), ProxyHelper::TstmpF2I(op->voutput_posmarker));
    op->voutput_buffer_level = 0;
  }
}

PROXYFUNCIMP(double DLLENTRY, OutputProxy1)
proxy_1_output_playing_pos( OutputProxy1* op, void* a )
{ DEBUGLOG(("proxy_1_output_playing_pos(%p {%s}, %p)\n", op, op->ModRef.Key.cdata(), a));
  return ProxyHelper::TstmpI2F((*op->voutput_playing_pos)(a), op->voutput_posmarker);
}

MRESULT EXPENTRY proxy_1_output_winfn(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ OutputProxy1* op = (OutputProxy1*)WinQueryWindowPtr(hwnd, QWL_USER);
  DEBUGLOG2(("proxy_1_output_winfn(%p, %u, %p, %p) - %p {%s}\n", hwnd, msg, mp1, mp2, op, op == NULL ? NULL : op->ModuleName.cdata()));
  switch (msg)
  {case WM_PLAYERROR:
    (*op->voutput_event)(op->voutput_w, OUTEVENT_PLAY_ERROR);
    return 0;
   case WM_OUTPUT_OUTOFDATA:
    if (op->voutput_flush_request) // don't care unless we have a flush_request condition
    { op->voutput_flush_request = FALSE;
      (*op->voutput_event)(op->A, OUTEVENT_END_OF_DATA);
    }
    return 0;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


int_ptr<Output> Output::FindInstance(const Module& module)
{ Mutex::Lock lock(Module::Mtx);
  Output* out = module.Out;
  return out && !out->RefCountIsUnmanaged() ? out : NULL;
}

int_ptr<Output> Output::GetInstance(Module& module)
{ if ((module.GetParams().type & PLUGIN_OUTPUT) == 0)
    throw ModuleException("Cannot load plug-in %s as output plug-in.", module.Key.cdata());
  Mutex::Lock lock(Module::Mtx);
  Output* out = module.Out;
  if (out && !out->RefCountIsUnmanaged())
    return out;
  out = module.GetParams().interface <= 1 ? new OutputProxy1(module) : new Output(module);
  try
  { out->LoadPlugin();
  } catch (...)
  { delete out;
    throw;
  }
  return module.Out = out;
}


void Output::Init()
{ PMRASSERT(WinRegisterClass(amp_player_hab, "OutputProxy1", &proxy_1_output_winfn, 0, sizeof(OutputProxy1*)));
  GetChangeEvent() += PluginDeleg;
}
