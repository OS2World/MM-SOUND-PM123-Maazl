/*
 * Copyright 2020 Marcel Mueller
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

#define USE_TAGGING

#include "mp4decoder.h"

#include <cpp/interlockedptr.h>
#include <strutils.h>
#include <charset.h>
#include <eautils.h>
#include <utilfct.h>
#include <process.h>
#include <fileutil.h>
#include <stddef.h>
#include <stdio.h>

#include <neaacdec.h>

/// Length of the chunks played during scan of a file.
const double seek_window = .2;

static uint32_t vio_read(void* w, void* ptr, uint32_t size)
{ DEBUGLOG2(("aac123:vio_read(%p, %p, %i)\n", w, ptr, size));
  return xio_fread(ptr, 1, size, (XFILE*)w);
}

static uint32_t vio_write(void* w, void* ptr, uint32_t size)
{ DEBUGLOG2(("aac123:vio_write(%p, %p, %i)\n", w, ptr, size));
  return xio_fwrite((const void*)ptr, 1, size, (XFILE*)w);
}

static uint32_t vio_seek(void* w, uint64_t offset)
{ DEBUGLOG2(("aac123:vio_seek(%p, %llu)\n", w, offset));
  if (xio_fseekl((XFILE*)w, offset, XIO_SEEK_SET) >= 0)
    return 0;
  else
    return ~0;
}

static uint32_t vio_truncate(void *w)
{ DEBUGLOG2(("aac123:vio_truncate(%p)\n", w));
  int64_t pos = xio_ftelll((XFILE*)w);
  if (pos == -1)
    return ~0;
  return xio_ftruncatel((XFILE*)w, pos);
}

Mp4Decoder::Mp4Decoder()
: MP4stream(NULL)
, Songlength(-1)
, Bitrate(-1)
{ Callbacks.read = &vio_read;
  Callbacks.write = &vio_write;
  Callbacks.seek = &vio_seek;
  Callbacks.truncate = &vio_truncate;
  Callbacks.user_data = NULL;
}

Mp4Decoder::~Mp4Decoder()
{ Close();
}

PROXYFUNCIMP(void DLLENTRY, Mp4Decoder) Observer(XIO_META type, const char* metabuff, long pos, void* arg)
{
  // TODO:
}

int Mp4Decoder::Init(mp4ff_t* mp4)
{ DEBUGLOG(("Mp4Decoder(%p)::Init(%p)\n", this, mp4));

  MP4stream = mp4;
  if (MP4stream == NULL)
    return -1;

  int num_tracks = mp4ff_total_tracks(MP4stream);
  for (Track = 0; ; ++Track)
  { if (Track >= num_tracks)
      return -1; // no audio track
    if (mp4ff_get_track_type(MP4stream, Track) == 1) // audio track?
      break;
  }

  Songlength = -1;
  FileTime = mp4ff_get_track_duration_use_offsets(MP4stream, Track);
  TimeScale = mp4ff_time_scale(MP4stream, Track);
  if (TimeScale < 0)
    return -1;
  if (FileTime != -1)
    Songlength = ((double)FileTime) / TimeScale + 0.5;

  Samplerate = -1;
  Channels = -1;
  unsigned char* decoder_info;
  unsigned decoder_info_size;
  if (!mp4ff_get_decoder_config(MP4stream, Track, &decoder_info, &decoder_info_size))
  { mp4AudioSpecificConfig mp4ASC;
    if (!NeAACDecAudioSpecificConfig(decoder_info, decoder_info_size, &mp4ASC))
    { Samplerate = (int)mp4ASC.samplingFrequency;
      Channels = mp4ASC.channelsConfiguration ? mp4ASC.channelsConfiguration : mp4ff_get_channel_count(MP4stream, Track);
      free(decoder_info);
  } }

  return 0;
}

xstring Mp4Decoder::ConvertString(const char* tag)
{
  if (!tag || !*tag)
    return xstring();
  char buffer[1024];
  return ch_convert(1208, tag, CH_CP_NONE, buffer, sizeof buffer, 0);
}

void Mp4Decoder::GetMeta(META_INFO& meta)
{
  for (const mp4ff_tag_t *tag, *tage = tag + mp4ff_meta_get(MP4stream, &tag); tag != tage; ++tag)
  { if (!stricmp(tag->item, "title"))
      meta.title = ConvertString(tag->value);
    else if(!stricmp(tag->item, "artist"))
      meta.artist = ConvertString(tag->value);
    else if(!stricmp(tag->item, "album"))
      meta.album = ConvertString(tag->value);
    else if(!stricmp(tag->item, "date"))
      meta.year = ConvertString(tag->value);
    else if(!stricmp(tag->item, "comment"))
      meta.comment = ConvertString(tag->value);
    else if(!stricmp(tag->item, "genre"))
      meta.genre = ConvertString(tag->value);
    else if(!stricmp(tag->item, "track"))
      meta.track = ConvertString(tag->value);
    else if(!stricmp(tag->item, "replaygain_track_gain"))
      sscanf(tag->value, "%f", &meta.track_gain);
    else if(!stricmp(tag->item, "replaygain_track_peak"))
      sscanf(tag->value, "%f", &meta.track_peak);
    else if(!stricmp(tag->item, "replaygain_album_gain"))
      sscanf(tag->value, "%f", &meta.album_gain);
    else if(!stricmp(tag->item, "replaygain_album_peak"))
      sscanf(tag->value, "%f", &meta.album_peak);
  }
}


vector<Mp4DecoderThread> Mp4DecoderThread::Instances;
Mutex Mp4DecoderThread::InstMtx;

Mp4DecoderThread::DECODER_STRUCT()
: DecoderTID(-1)
, Status(DECODER_STOPPED)
, SkipSecs(0)
, JumpToPos(-1)
, FAAD(NULL)
{
  Mutex::Lock lock(InstMtx);
  Instances.append() = this;
}

Mp4DecoderThread::~DECODER_STRUCT()
{
  DecoderCommand(DECODER_STOP, NULL);

  { Mutex::Lock lock(InstMtx);
    foreach (Mp4DecoderThread,*const*, dpp, Instances)
    { if (*dpp == this)
      { Instances.erase(dpp);
        break;
      }
    }
  }

  // Wait for decoder to complete
  int tid = DecoderTID;
  if (tid != -1)
    wait_thread(tid, 5000);

  NeAACDecClose(FAAD);
}

/* Decoding thread. */
void Mp4DecoderThread::DecoderThread()
{
  sco_arr<unsigned char> buffer;

  // Open stream
  { Mutex::Lock lock(Mtx);
    SetFile(xio_fopen(URL, "rbXU"));
    if (GetFile() == NULL)
    { xstring errortext;
      errortext.sprintf("Unable to open file %s\n%s", URL.cdata(), xio_strerror(xio_errno()));
      Ctx.plugin_api->message_display(MSG_ERROR, errortext);
      (*DecEvent)(A, DECEVENT_PLAYERROR, NULL);
      goto end;
    }

    int rc = Init(mp4ff_open_read(&Callbacks));
    if (rc != 0)
    { xstring errortext;
      errortext.sprintf("Unable to open file %s\nUnsupported file format", URL.cdata());
      Ctx.plugin_api->message_display(MSG_ERROR, errortext);
      (*DecEvent)(A, DECEVENT_PLAYERROR, NULL);
      goto end;
    }

    FAAD = NeAACDecOpen();

    NeAACDecConfigurationPtr config = NeAACDecGetCurrentConfiguration(FAAD);
    config->outputFormat = FAAD_FMT_FLOAT;
    //config->downMatrix = 1; should be optional by config dialog
  }

  NumFrames = mp4ff_num_samples(MP4stream, Track);
  CurrentFrame = 0;

  // After opening a new file we so are in its beginning.
  if (JumpToPos == 0)
    JumpToPos = -1;

  // Decoder frame buffer
  buffer.reset(mp4ff_get_max_samples_size(MP4stream, Track));

  for(;;)
  { Play.Wait();

    for (;; ++CurrentFrame)
    { Play.Reset();
      if (StopRq)
        goto end;
      Status = DECODER_PLAYING;

      double newpos = JumpToPos;
      if (SkipSecs && PlayedSecs >= NextSkip)
      { if (newpos < 0)
          newpos = PlayedSecs;
        newpos += SkipSecs;
        if (newpos < 0)
          break; // Begin of song
      }
      DiscardSamples = 0;
      if (newpos >= 0)
      { NextSkip = newpos + seek_window;
        { CurrentFrame = mp4ff_find_sample_use_offsets(MP4stream, Track, (int64_t)(JumpToPos * TimeScale), &DiscardSamples);
          NeAACDecPostSeekReset(FAAD, CurrentFrame);
          newpos = JumpToPos;
          JumpToPos = -1;
        }
        if (newpos >= 0)
          (*DecEvent)(A, DECEVENT_SEEKSTOP, NULL);
        if (CurrentFrame == -1)
          break; // Seek out of range => begin/end of song
      }

      int32_t frame_duration;
      if (CurrentFrame == 0)
        frame_duration = 0;
      else
      { frame_duration = mp4ff_get_sample_duration(MP4stream, Track, CurrentFrame);
        if (CurrentFrame > NumFrames)
          break; // end of song
      }
      frame_duration -= DiscardSamples;
      if (frame_duration <= 0) // empty frame
        continue;

      int32_t frame_size;
      { Mutex::Lock lock(Mtx);
        frame_size = mp4ff_read_sample_v2(MP4stream, Track, CurrentFrame, buffer.get());
      }
      if (frame_size <= 0)
      { (*DecEvent)(A, DECEVENT_PLAYERROR, NULL);
        goto end;
      }

      // AAC decode
      NeAACDecFrameInfo frame_info;
      float* source = (float*)NeAACDecDecode(FAAD, &frame_info, buffer.get(), frame_size);

      if (frame_info.error > 0)
      { (*DecEvent)(A, DECEVENT_PLAYERROR, faacDecGetErrorMessage(frame_info.error));
        goto end;
      }
      if (!source)
        continue; // error => skip this frame
      source += frame_info.channels * DiscardSamples; // skip some data?

      for (;;)
      { FORMAT_INFO2 output_format;
        output_format.channels = frame_info.channels;
        output_format.samplerate = frame_info.samplerate;
        float* target;
        int count = (*OutRequestBuffer)(A, &output_format, &target);
        if (count <= 0)
        { (*DecEvent)(A, DECEVENT_PLAYERROR, NULL);
          goto end;
        }

        if (count > frame_duration)
          count = frame_duration;
        memcpy(target, source, sizeof(float) * count * frame_info.channels);
        (*OutCommitBuffer)(A, count, PlayedSecs);
        PlayedSecs += count / GetSamplerate();

        if (frame_duration == count)
          break;
        frame_duration -= count;
        source += count * frame_info.channels;
      }
    }
    if (StopRq)
      goto end;
    DEBUGLOG(("Mp4DecoderThread::DecoderThread - playstop event - %.3f, %i\n", PlayedSecs, StopRq));
    (*DecEvent)(A, DECEVENT_PLAYSTOP, NULL);
  }

end:
  XFILE* file = InterlockedXch(&GetFile(), (XFILE*)NULL);
  if (file)
    xio_fclose(file);
  Status = DECODER_STOPPED;
  DecoderTID = -1;
}

PROXYFUNCIMP(void, Mp4DecoderThread) TFNENTRY DecoderThreadStub(void* arg)
{ ((Mp4DecoderThread*)arg)->DecoderThread();
}

ULONG Mp4DecoderThread::DecoderCommand(DECMSGTYPE msg, const DECODER_PARAMS2* params)
{ switch (msg)
  {
    case DECODER_SETUP:
      OutRequestBuffer = params->OutRequestBuffer;
      OutCommitBuffer  = params->OutCommitBuffer;
      DecEvent         = params->DecEvent;
      A                = params->A;
      break;

    case DECODER_PLAY:
    {
      if (DecoderTID != -1)
      { if (Status == DECODER_STOPPED)
          DecoderCommand(DECODER_STOP, NULL);
        else
          return PLUGIN_GO_ALREADY;
      }

      URL = params->URL;

      NextSkip  = JumpToPos = params->JumpTo;
      SkipSecs  = seek_window * params->SkipSpeed;
      Status    = DECODER_STARTING;
      StopRq    = false;
      DecoderTID = _beginthread(PROXYFUNCREF(Mp4DecoderThread)DecoderThreadStub, 0, 65535, this);
      if (DecoderTID == -1)
      { Status = DECODER_STOPPED;
        return PLUGIN_FAILED;
      }
      Play.Set(); // Go!
      break;
    }

    case DECODER_STOP:
    {
      if (DecoderTID == -1)
        return PLUGIN_GO_ALREADY;

      Status = DECODER_STOPPING;
      StopRq = true;

      Play.Set();
      break;
    }

    case DECODER_FFWD:
      /*if (params->Fast && File && xio_can_seek(File) != XIO_CAN_SEEK_FAST)
        return PLUGIN_UNSUPPORTED;*/
      SkipSecs = seek_window * params->SkipSpeed;
      NextSkip = PlayedSecs;
      Play.Set();
      break;

    case DECODER_JUMPTO:
      JumpToPos = params->JumpTo;
      NextSkip = JumpToPos;
      Play.Set();
      break;

    default:
      return PLUGIN_UNSUPPORTED;
   }

   return PLUGIN_OK;
}

int Mp4DecoderThread::ReplaceStream(const char* sourcename, const char* destname)
{
/*  // Suspend all active instances of the updated file.
  Mutex::Lock lock(InstMtx);

  foreach(Mp4DecoderThread,*const*, dpp, Instances)
  { Mp4DecoderThread& dec = **dpp;
    dec.Mtx.Request();

    if (dec.File && stricmp(dec.URL, destname) == 0)
    { DEBUGLOG(("oggplay: suspend currently used file: %s\n", destname));
      dec.ResumePcms = ov_pcm_tell(&dec.VrbFile);
      dec.OggClose();
      xio_fclose(dec.File);
    } else
      dec.ResumePcms = -1;
  }

  const char* srcfile = surl2file(sourcename);
  const char* dstfile = surl2file(destname);
  // Preserve EAs.
  eacopy(dstfile, srcfile);

  // Replace file.
  int rc = 0;
  if (remove(dstfile) != 0 || rename(srcfile, dstfile) != 0)
    rc = errno;

  // Resume all suspended instances of the updated file.
  foreach(Mp4DecoderThread,*const*, dpp, Instances)
  { Mp4DecoderThread& dec = **dpp;
    if (dec.ResumePcms != -1)
    { DEBUGLOG(("oggplay: resumes currently used file: %s\n", destname));
      dec.File = xio_fopen(destname, "rbXU");
      if (dec.File && dec.OggOpen() == PLUGIN_OK)
        ov_pcm_seek(&dec.VrbFile, dec.ResumePcms);
    }
    dec.Mtx.Release();
  }

  return rc;*/
  return -1;
}
