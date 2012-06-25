/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 *
 * Copyright 2007-2008 Dmitry A.Steklenev <glass@ptv.ru>
 * Copyright 2007-2012 Marcel Mueller
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

#define  INCL_OS2MM
#define  INCL_PM
#define  INCL_DOS
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <utilfct.h>
#include <format.h>
#include <output_plug.h>
#include <decoder_plug.h>
#include <plugin.h>
#include <debuglog.h>

#include "os2audio.h"
#define   INFO( mb ) ((OS2AUDIO*)(mb)->ulUserParm)

#if 0
#define  DosRequestMutexSem( mutex, wait )                                                 \
         DEBUGLOG(( "os2audio: request mutex %08X at line %04d\n", mutex, __LINE__ ));     \
         DEBUGLOG(( "os2audio: request mutex %08X at line %04d is completed, rc = %08X\n", \
                               mutex, __LINE__, DosRequestMutexSem( mutex, wait )))

#define  DosReleaseMutexSem( mutex )                                                       \
         DEBUGLOG(( "os2audio: release mutex %08X at line %04d\n", mutex, __LINE__ ));     \
         DosReleaseMutexSem( mutex )
#endif


/* Scalefactor for logarithmic volume emulation. Should not exceed sqrt(10) */
#define scalefactor 3.16227766


static  const PLUGIN_CONTEXT* context;

#define load_prf_value(var) \
  context->plugin_api->profile_query(#var, &var, sizeof var)

#define save_prf_value(var) \
  context->plugin_api->profile_write(#var, &var, sizeof var)


static int   device       = 0;
static int   numbuffers   = 64;
static int   lockdevice   = 0;
static int   logvolume    = 0;
static int   kludge48as44 = 0;
static int   noprecpos    = 0;

static void
output_error( OS2AUDIO* a, ULONG ulError )
{
  unsigned char message[1536];
  unsigned char buffer [1024];

  if( mciGetErrorString( ulError, buffer, sizeof( buffer )) == MCIERR_SUCCESS ) {
    sprintf( message, "MCI Error %lu: %s\n", ulError, buffer );
  } else {
    sprintf( message, "MCI Error %lu: Cannot query error message.\n", ulError );
  }

  a->original_info.error_display( message );
  WinPostMsg( a->original_info.hwnd, WM_PLAYERROR, 0, 0 );
}

/* Returns the current position of the audio device in milliseconds. */
static APIRET
output_position( OS2AUDIO* a, ULONG* position )
{
  MCI_STATUS_PARMS mstatp = { 0 };
  ULONG rc;

  mstatp.ulItem = MCI_STATUS_POSITION;
  rc = LOUSHORT(mciSendCommand( a->mci_device_id, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, &mstatp, 0 ));

  DEBUGLOG(( "os2audio: output_position: %lu, %lu\n", rc, mstatp.ulReturn ));

  if( rc == MCIERR_SUCCESS ) {
    *position = mstatp.ulReturn;
  }

  return rc;
}

/* Boosts a priority of the driver thread. */
static void
output_boost_priority( OS2AUDIO* a )
{
  if( a->drivethread && !a->boosted ) {
    APIRET rc = DosSetPriority( PRTYS_THREAD, a->original_info.boostclass,
                                  a->original_info.boostdelta, a->drivethread );
    a->boosted = TRUE;
    DEBUGLOG(( "os2audio: boosts priority of the driver thread %d to %d/%d: %u.\n", a->drivethread, a->original_info.boostclass, a->original_info.boostdelta, rc ));
  }
}

/* Normalizes a priority of the driver thread. */
static void
output_normal_priority( OS2AUDIO* a )
{
  if( a->drivethread && a->boosted ) {
    APIRET rc = DosSetPriority( PRTYS_THREAD, a->original_info.normalclass,
                                  a->original_info.normaldelta, a->drivethread );
    a->boosted = FALSE;
    DEBUGLOG(( "os2audio: normalizes priority of the driver thread %d to %d/%d: %u.\n", a->drivethread, a->original_info.normalclass, a->original_info.normaldelta, rc ));
  }
}

/* When the mixer has finished writing a buffer it will
   call this function. */
static LONG APIENTRY
dart_event( ULONG status, MCI_MIX_BUFFER* buffer, ULONG flags )
{
  DEBUGLOG(( "os2audio: receive DART event, status=%d, flags=%08X, time=%lu\n", status, flags, buffer->ulTime ));

  if( flags & MIX_WRITE_COMPLETE )
  {
    OS2AUDIO* a = INFO(buffer);
    const int current = buffer - a->mci_buffers;
    int filled;
    
    // Now the next buffer is playing => update time index, set next buffer.
    // Since the DART thread has very high priority this is implicitely atomic
    // with respect to other PM123 threads.
    a->buf_is_play = ( current +1 ) % a->mci_buf_parms.ulNumBuffers;
    a->mci_time    = buffer->ulTime; // buffer->ulTime is the end of the played buffer = start of next buffer
    
    // Number of currently filled buffers.
    filled = ( a->buf_to_fill - a->buf_is_play ) % a->mci_buf_parms.ulNumBuffers;
    DEBUGLOG(("os2audio:dart_event: filled=%i time=%lu\n", filled, buffer->ulTime));
    // If we're about to be short of decoder's data, let's boost its priority!
    if( filled <= a->low_water_mark && !a->nomoredata ) {
      output_boost_priority(a);
    // If we're out of the water, let's reduce the driver thread priority.
    } else if( filled >= a->high_water_mark ) {
      output_normal_priority( a );
    }

    // Just too bad, the decoder fell behind...
    if( filled == 0 )
    { // Implicitely atomic with the above condition unless we do the DEBUGLOG.
      a->buf_to_fill = ( a->buf_is_play + 1 ) % a->mci_buf_parms.ulNumBuffers;
      a->nomoredata  = TRUE;
      DEBUGLOG(( "os2audio: output is hungry.\n" ));
      WinPostMsg( a->original_info.hwnd, WM_OUTPUT_OUTOFDATA, 0, 0 );
    }

    // Clear the played buffer and to place it to the end of the queue.
    // By the moment of playing of this buffer, it already must contain a new data.
    memset( buffer->pBuffer, 0, a->mci_buf_parms.ulBufferSize );
    // We set the playing position of the new buffer to value of the last buffer.
    // If the decoder falls behind this position is used.
    a->buf_positions[current] = a->buf_positions[(current-1)%a->mci_buf_parms.ulNumBuffers];
    a->mci_mix.pmixWrite( a->mci_mix.ulMixHandle, buffer, 1 );

    DosPostEventSem( a->dataplayed );
  }
  return TRUE;
}

/* Changes the volume of an output device. */
static ULONG
output_set_volume( OS2AUDIO* a, unsigned char setvolume, float setamplifier )
{
  a->volume    = min( setvolume, 100 );
  a->amplifier = setamplifier;

  // Useful when device is closed and reopened.
  if( a->status == DEVICE_OPENED )
  {
    MCI_SET_PARMS msp = { 0 };
    msp.ulAudio = MCI_SET_AUDIO_ALL;
    if (a->logvolume)
    { double vol = a->volume / 100.;
      vol /= scalefactor + 1. - scalefactor * vol;
      msp.ulLevel = (int)(vol * 100. +.5);
    } else
      msp.ulLevel = a->volume;

    mciSendCommand( a->mci_device_id, MCI_SET, MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME, &msp, 0 );
  }
  return 0;
}

/* Pauses or resumes the playback. */
static ULONG DLLENTRY
output_pause( OS2AUDIO* a, BOOL pause )
{
  MCI_GENERIC_PARMS mgp = { 0 };
  ULONG rc = 0;

  if( a->status == DEVICE_OPENED )
  {
    rc = LOUSHORT(mciSendCommand( a->mci_device_id, pause ? MCI_PAUSE : MCI_RESUME, MCI_WAIT, &mgp, 0 ));

    if( rc != MCIERR_SUCCESS ) {
      output_error( a, rc );
    }
  }
  return rc;
}

/* Closes the output audio device. */
ULONG DLLENTRY
output_close( OS2AUDIO* a )
{
  ULONG rc = 0;

  MCI_GENERIC_PARMS mgp = { 0 };

  DEBUGLOG(( "os2audio: closing the output device.\n" ));
  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );

  if( a->status != DEVICE_OPENED && a->status != DEVICE_FAILED ) {
    DEBUGLOG(( "os2audio: unable to close a not opened device.\n" ));
    DosReleaseMutexSem( a->mutex );
    return 0;
  }

  a->status = DEVICE_CLOSING;
  // Release a waiting decoder thread if any.
  DosPostEventSem( a->dataplayed );
  DosReleaseMutexSem( a->mutex );

  rc = mciSendCommand( a->mci_device_id, MCI_STOP,  MCI_WAIT, &mgp, 0 );

  if( LOUSHORT(rc) != MCIERR_SUCCESS ) {
    output_error( a, rc );
  }

  if( a->mci_buf_parms.ulNumBuffers ) {
    rc = mciSendCommand( a->mci_device_id, MCI_BUFFER,
                         MCI_WAIT | MCI_DEALLOCATE_MEMORY, &a->mci_buf_parms, 0 );
    if( LOUSHORT(rc) != MCIERR_SUCCESS ) {
      output_error( a, rc );
    }
  }

  rc = mciSendCommand( a->mci_device_id, MCI_MIXSETUP,
                       MCI_WAIT | MCI_MIXSETUP_DEINIT, &a->mci_mix, 0 );

  if( LOUSHORT(rc) != MCIERR_SUCCESS ) {
    output_error( a, rc );
  }

  rc = mciSendCommand( a->mci_device_id, MCI_CLOSE, MCI_WAIT, &mgp, 0 );
  if( LOUSHORT(rc) != MCIERR_SUCCESS ) {
    output_error( a, rc );
  }

  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );

  free( a->buf_positions );
  free( a->mci_buffers   );

  a->buf_positions = NULL;
  a->mci_buffers   = NULL;
  a->mci_device_id = 0;
  a->status        = DEVICE_CLOSED;
  a->nomoredata    = TRUE;

  DEBUGLOG(( "os2audio: output device is successfully closed.\n" ));
  DosReleaseMutexSem( a->mutex );
  return rc;
}

/* Opens the output audio device. */
static ULONG
output_open( OS2AUDIO* a )
{
  OUTPUT_PARAMS* info = &a->original_info;
  ULONG  openflags = 0;
  ULONG  i;
  ULONG  rc;

  MCI_AMP_OPEN_PARMS mci_aop = {0};

  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );
  DEBUGLOG(( "os2audio: opening the output device.\n" ));

  if( a->status != DEVICE_CLOSED ) {
    DEBUGLOG(( "os2audio: unable to open a not closed device.\n" ));
    DosReleaseMutexSem( a->mutex );
    return 0;
  }

  a->status          = DEVICE_OPENING;
  a->device          = device;
  a->lockdevice      = lockdevice;
  a->logvolume       = logvolume;
  a->numbuffers      = numbuffers;
  a->kludge48as44    = kludge48as44;
  a->noprecpos       = noprecpos;
  a->low_water_mark  = 4; /* Hard coded so far... */
  a->high_water_mark = 7; /* Hard coded so far... */ 
  a->mci_device_id   = 0;
  a->drivethread     = 0;
  a->boosted         = FALSE;
  a->nomoredata      = TRUE;

/* Uh, well, we don't want to mask bugs here ...
  if( info->formatinfo.format     <= 0 ) { info->formatinfo.format     = WAVE_FORMAT_PCM; }
  if( info->formatinfo.samplerate <= 0 ) { info->formatinfo.samplerate = 44100; }
  if( info->formatinfo.channels   <= 0 ) { info->formatinfo.channels   = 2;     }
  if( info->formatinfo.bits       <= 0 ) { info->formatinfo.bits       = 16;    }
*/

  DEBUGLOG(("os2audio:output_open - {%i, %i, %i, %i}\n",
    info->formatinfo.samplerate, info->formatinfo.channels, info->formatinfo.bits, info->formatinfo.format ));
  // There can be the audio device supports other formats, but
  // whether can interpret other plug-ins them correctly?
  if( info->formatinfo.format != WAVE_FORMAT_PCM ||
      info->formatinfo.samplerate <= 0 ||
      info->formatinfo.bits != 16 ||
      info->formatinfo.channels <= 0 || info->formatinfo.channels > 2 )
  { rc = MCIERR_UNSUPP_FORMAT_MODE;
    goto end;
  }

  memset( &a->mci_mix, 0, sizeof( a->mci_mix ));
  memset( &a->mci_buf_parms, 0, sizeof( a->mci_buf_parms ));

  // Open the mixer device.
  mci_aop.pszDeviceType = (PSZ)MAKEULONG( MCI_DEVTYPE_AUDIO_AMPMIX, a->device );

  if( !a->lockdevice ) {
    openflags = MCI_OPEN_SHAREABLE;
  }

  rc = LOUSHORT(mciSendCommand( 0, MCI_OPEN, MCI_WAIT | MCI_OPEN_TYPE_ID | openflags, &mci_aop, 0 ));
  if( rc != MCIERR_SUCCESS ) {
    DEBUGLOG(( "os2audio:output_open:  unable to open a mixer: %u.\n", rc ));
    goto end;
  }

  a->mci_device_id = mci_aop.usDeviceID;

  // Set the MCI_MIXSETUP_PARMS data structure to match the audio stream.
  a->mci_mix.ulBitsPerSample = info->formatinfo.bits;

  if( a->kludge48as44 && info->formatinfo.samplerate == 48000 ) {
    a->mci_mix.ulSamplesPerSec = 44100;
  } else {
    a->mci_mix.ulSamplesPerSec = info->formatinfo.samplerate;
  }

  a->mci_mix.ulChannels   = info->formatinfo.channels;
  a->mci_mix.ulFormatTag  = MCI_WAVE_FORMAT_PCM;

  // Setup the mixer for playback of wave data.
  a->mci_mix.ulFormatMode = MCI_PLAY;
  a->mci_mix.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
  a->mci_mix.pmixEvent    = dart_event;

  rc = LOUSHORT(mciSendCommand( a->mci_device_id, MCI_MIXSETUP, MCI_WAIT | MCI_MIXSETUP_INIT, &a->mci_mix, 0 ));
  if( rc != MCIERR_SUCCESS ) {
    DEBUGLOG(( "os2audio:output_open: MCI_MIXSETUP failed: %u.\n", rc ));
    goto end;
  }

  // Set up the MCI_BUFFER_PARMS data structure and allocate
  // device buffers from the mixer.
  a->numbuffers    = limit2( a->numbuffers, 16, 200 );
  a->mci_buffers   = calloc( a->numbuffers, sizeof( *a->mci_buffers   ));
  a->buf_positions = calloc( a->numbuffers, sizeof( *a->buf_positions ));

  if( !a->mci_buffers || !a->buf_positions ) {
    DEBUGLOG(( "os2audio: not enough memory.\n" ));
    rc = MCIERR_OUT_OF_MEMORY;
    goto end;
  }

  a->buffersize = info->buffersize;

  a->mci_buf_parms.ulNumBuffers = a->numbuffers;
  a->mci_buf_parms.ulBufferSize = a->buffersize;
  a->mci_buf_parms.pBufList     = a->mci_buffers;

  rc = LOUSHORT(mciSendCommand( a->mci_device_id, MCI_BUFFER,
                                MCI_WAIT | MCI_ALLOCATE_MEMORY, (PVOID)&a->mci_buf_parms, 0 ));
  if( rc != MCIERR_SUCCESS ) {
    DEBUGLOG(( "os2audio:output_open: MCI_BUFFER failed: %u.\n", rc ));
    goto end;
  }

  DEBUGLOG(( "os2audio: suggested buffers is %d, allocated %d\n",
                        a->numbuffers, a->mci_buf_parms.ulNumBuffers ));
  DEBUGLOG(( "os2audio: suggested buffer size is %d, allocated %d\n",
                        a->buffersize, a->mci_buf_parms.ulBufferSize ));

  for( i = 0; i < a->mci_buf_parms.ulNumBuffers; i++ )
  {
    a->mci_buffers[i].ulFlags        = 0;
    a->mci_buffers[i].ulBufferLength = a->mci_buf_parms.ulBufferSize;
    a->mci_buffers[i].ulUserParm     = (ULONG)a;

    memset( a->mci_buffers[i].pBuffer, 0, a->mci_buf_parms.ulBufferSize );
  }

  a->buf_is_play = 0;
  a->buf_to_fill = 2;

  // Write buffers to kick off the amp mixer.
  rc = LOUSHORT(a->mci_mix.pmixWrite( a->mci_mix.ulMixHandle, a->mci_buffers, a->mci_buf_parms.ulNumBuffers ));
  if( rc != MCIERR_SUCCESS ) {
    DEBUGLOG(( "os2audio:output_open: pmixWrite failed: %u.\n", rc ));
    goto end;
  }

  // Current time of the device is placed at end of
  // playing the buffer, but we require this already now.
  output_position( a, &a->mci_time );

end:

  if( rc != MCIERR_SUCCESS )
  {
    output_error( a, rc );
    DEBUGLOG(( "os2audio: output device opening is failed: %u.\n", rc ));

    if( a->mci_device_id ) {
      a->status = DEVICE_FAILED;
      output_close( a );
    } else {
      a->status = DEVICE_CLOSED;
    }
  } else {
    a->status = DEVICE_OPENED;
    output_set_volume( a, a->volume, a->amplifier );
    DEBUGLOG(( "os2audio: output device is successfully opened.\n" ));
  }

  DosReleaseMutexSem( a->mutex );
  return rc;
}

/* This function is called by the decoder or last in chain filter plug-in
   to play samples. */
int DLLENTRY
output_play_samples( OS2AUDIO* a, FORMAT_INFO* format, char* buf, int len, int posmarker )
{
  DEBUGLOG(("output_play_samples({%i,%i,%i,%i,%x}, %p, %i, %i)\n",
      format->size, format->samplerate, format->channels, format->bits, format->format, buf, len, posmarker));

  // Update TID always because the decoder thread may change while playing gapless.
  { TID tid = getTID();
    if (a->drivethread != tid)
    { a->drivethread = tid;
      // If we were already boosted we also have to boost the new thread
      if (a->boosted)
        DosSetPriority( PRTYS_THREAD, a->original_info.boostclass,
                                      a->original_info.boostdelta, a->drivethread );
    }
  }

  // Set the new format structure before re-opening.
  if( memcmp( format, &a->original_info.formatinfo, sizeof( FORMAT_INFO )) != 0 || a->status == DEVICE_CLOSED )
  {
    DEBUGLOG(( "os2audio: old format: size %d, sample: %d, channels: %d, bits: %d, id: %d\n",
        a->original_info.formatinfo.size,
        a->original_info.formatinfo.samplerate,
        a->original_info.formatinfo.channels,
        a->original_info.formatinfo.bits,
        a->original_info.formatinfo.format ));

    DEBUGLOG(( "os2audio: new format: size %d, sample: %d, channels: %d, bits: %d, id: %d\n",
        format->size,
        format->samplerate,
        format->channels,
        format->bits,
        format->format ));

    if( output_close(a) != 0 ) {
      return 0;
    }
    a->original_info.formatinfo = *format;
    if( output_open (a) != 0 ) {
      return 0;
    }
  }

  if( a->status != DEVICE_OPENED ) {
    return 0;
  }

  if( len > 0 ) {
    int current = a->buf_to_fill;
    unsigned char* out;

    // set next buffer
    a->buf_to_fill = ( current + 1 ) % a->mci_buf_parms.ulNumBuffers;
  
    // If we're too quick, let's wait.
    while( a->buf_to_fill == a->buf_is_play ) {
      ULONG resetcount;
      DEBUGLOG(("output_play_samples: wait for dataplayed (at %i)\n", a->buf_is_play));
      DosWaitEventSem ( a->dataplayed, SEM_INDEFINITE_WAIT );
      DosResetEventSem( a->dataplayed, &resetcount );
      DEBUGLOG(("output_play_samples: reset sem: %lu\n", resetcount));
      // The device might have been stopped meanwhile.
      if (a->status != DEVICE_OPENED)
        return 0;
    }
    
    if( a->trashed ) {
      // This portion of samples should be trashed.
      a->trashed = FALSE;
      return len;
    }

    // A following buffer is already cashed by the audio device. Therefore
    // it is necessary to skip it and to fill the second buffer from current.
    // Don't do this here. Small dropouts are better than jumps.
    /*if( a->mci_to_fill == INFO(a->mci_is_play)->next ) {
      INFO(a->mci_to_fill)->playingpos = a->playingpos;
      a->mci_to_fill = INFO(a->mci_to_fill)->next;
    }*/

    out = a->mci_buffers[current].pBuffer;
    if( len > a->mci_buf_parms.ulBufferSize ) {
      DEBUGLOG(( "os2audio: too many samples.\n" ));
      return 0;
    }

    memcpy( out, buf, len );

    a->buf_positions[current] = posmarker;

    a->nomoredata = FALSE;
    a->trashed    = FALSE;
  }

  return len;
}

/* Returns the posmarker from the buffer that the user
   currently hears. */
ULONG DLLENTRY
output_playing_pos( OS2AUDIO* a ) {
  ULONG     position, playingpos;
  
  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );

  if( a->status == DEVICE_CLOSED ) {
    DosReleaseMutexSem( a->mutex );
    return 0; // Since the timer may wrap around it makes no sense to return an error anyway.
  }

  if( a->noprecpos || a->trashed || a->nomoredata
    || LOUSHORT( output_position( a, &position )) != MCIERR_SUCCESS ) {
    // we simply do not calculate more exact information in this case
    playingpos = a->buf_positions[a->buf_is_play];
    DosReleaseMutexSem( a->mutex );
    return playingpos;
  }

  DosEnterCritSec();
  // This difference may overflow, but this is compensated below.
  playingpos = a->buf_positions[a->buf_is_play] - a->mci_time;
  DosExitCritSec();

  DosReleaseMutexSem( a->mutex );

  return playingpos + position;
}

/* This function is used by visual plug-ins so the user can visualize
   what is currently being played. */
ULONG DLLENTRY
output_playing_samples( OS2AUDIO* a, FORMAT_INFO* info, char* buf, int len )
{
  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );

  if( a->status != DEVICE_OPENED || a->nomoredata ) {
    DosReleaseMutexSem( a->mutex );
    return PLUGIN_FAILED;
  }

  info->bits       = a->mci_mix.ulBitsPerSample;
  info->samplerate = a->mci_mix.ulSamplesPerSec;
  info->channels   = a->mci_mix.ulChannels;
  info->format     = a->mci_mix.ulFormatTag;

  if( buf && len )
  {
    int   current, next;
    ULONG position = 0;
    LONG  offset;

    if( !a->noprecpos )
    { // calculate offset in the currently playling buffer.
      if( LOUSHORT( output_position( a, &position )) != MCIERR_SUCCESS ) {
        DosReleaseMutexSem( a->mutex );
        return PLUGIN_FAILED;
      }

      DosEnterCritSec(); // Capture these two atomically
      current = a->buf_is_play;
      offset  = a->mci_time;
      DosExitCritSec();
      
      offset = ( position - offset ) * a->mci_mix.ulSamplesPerSec / 1000 *
                                       a->mci_mix.ulChannels *
                                       a->mci_mix.ulBitsPerSample / 8;
    } else
    { // Simply start with the current buffer
      current = a->buf_is_play;
      offset = 0;
    }

    for(;;) {
      next = ( current + 1 ) % a->mci_buf_parms.ulNumBuffers;
      if ( next == a->buf_to_fill || offset <= a->mci_buffers[current].ulBufferLength )
        break;
      
      offset -= a->mci_buffers->ulBufferLength;
      current = next;
    }

    while( len && next != a->buf_to_fill )
    {
      MCI_MIX_BUFFER* mci_buffer = a->mci_buffers + current;
      int chunk_size = mci_buffer->ulBufferLength - offset;

      if( chunk_size > len ) {
          chunk_size = len;
      }

      memcpy( buf, (char*)mci_buffer->pBuffer + offset, chunk_size );

      buf += chunk_size;
      len -= chunk_size;

      offset = 0;
      current = next;
      next = ( current + 1 ) % a->mci_buf_parms.ulNumBuffers;
    }

    if( len ) {
      memset( buf, 0, len );
    }
  }

  DosReleaseMutexSem( a->mutex );
  return PLUGIN_OK;
}

/* Trashes all audio data received till this time. */
void DLLENTRY
output_trash_buffers( OS2AUDIO* a, ULONG temp_playingpos )
{
  int i;

  DEBUGLOG(( "os2audio: trashing audio buffers.\n" ));
  DosRequestMutexSem( a->mutex, SEM_INDEFINITE_WAIT );

  if(!( a->status & DEVICE_OPENED )) {
    DosReleaseMutexSem( a->mutex );
    return;
  }

  for( i = 0; i < a->mci_buf_parms.ulNumBuffers; i++ ) {
    memset( a->mci_buffers[i].pBuffer, 0, a->mci_buf_parms.ulBufferSize );
    a->buf_positions[i] = temp_playingpos;
  }

  a->nomoredata = FALSE;
  a->trashed    = TRUE;

  // A following buffer is already cashed by the audio device. Therefore
  // it is necessary to skip it and to fill the second buffer from current.
  a->buf_to_fill = ( a->buf_is_play + 1 ) % a->mci_buf_parms.ulNumBuffers; 

  DosReleaseMutexSem( a->mutex );
  DEBUGLOG(( "os2audio: audio buffers is successfully trashed.\n" ));
}

/* This function is called when the user requests
   the use of output plug-in. */
ULONG DLLENTRY
output_init( OS2AUDIO** A )
{
  OS2AUDIO* a = calloc( sizeof( OS2AUDIO ), 1 );
  *A = a;

  a->numbuffers  = 64;
  a->volume      = 100;
  a->amplifier   = 1.0;
  a->status      = DEVICE_CLOSED;
  a->nomoredata  = TRUE;

  DosCreateEventSem( NULL, &a->dataplayed, 0, FALSE );
  DosCreateMutexSem( NULL, &a->mutex, 0, FALSE );
  return PLUGIN_OK;
}

/* This function is called when another output plug-in
   is request by the user. */
ULONG DLLENTRY
output_uninit( OS2AUDIO* a )
{
  DEBUGLOG(("output_uninit: %p\n", a));

  output_close( a );

  DosCloseEventSem( a->dataplayed );
  DosCloseMutexSem( a->mutex );

  free( a );
  return PLUGIN_OK;
}

/* Returns TRUE if the output plug-in still has some buffers to play. */
BOOL DLLENTRY
output_playing_data( OS2AUDIO* a ) {
  return !a->nomoredata;
}

static ULONG DLLENTRY
output_get_devices( char* name, int deviceid )
{
  char buffer[256];
  MCI_SYSINFO_PARMS mip;

  if( deviceid && name )
  {
    MCI_SYSINFO_LOGDEVICE mid;

    mip.pszReturn    = buffer;
    mip.ulRetSize    = sizeof( buffer );
    mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;
    mip.ulNumber     = deviceid;

    mciSendCommand( 0, MCI_SYSINFO, MCI_WAIT | MCI_SYSINFO_INSTALLNAME, &mip, 0 );

    mip.ulItem = MCI_SYSINFO_QUERY_DRIVER;
    mip.pSysInfoParm = &mid;
    strcpy( mid.szInstallName, buffer );

    mciSendCommand( 0, MCI_SYSINFO, MCI_WAIT | MCI_SYSINFO_ITEM, &mip, 0 );
    sprintf( name, "%s (%s)", mid.szProductInfo, mid.szVersionNumber );
  }

  mip.pszReturn    = buffer;
  mip.ulRetSize    = sizeof(buffer);
  mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;

  mciSendCommand( 0, MCI_SYSINFO, MCI_WAIT | MCI_SYSINFO_QUANTITY, &mip, 0 );
  return atoi( mip.pszReturn );
}

ULONG DLLENTRY
output_command( OS2AUDIO* a, ULONG msg, OUTPUT_PARAMS* params )
{
  DEBUGLOG(("output_command(%i, %p)\n", msg, params));

  switch( msg )
  {
    case OUTPUT_OPEN:
      return output_open( a );

    case OUTPUT_PAUSE:
      return output_pause( a, params->pause );

    case OUTPUT_CLOSE:
      return output_close( a );

    case OUTPUT_VOLUME:
      return output_set_volume( a, params->volume, params->amplifier );

    case OUTPUT_SETUP:
      if( a->status != DEVICE_OPENED ) {
        // Otherwise, information on the current session are modified.
        a->original_info = *params;
      }

      params->always_hungry = FALSE;
      return 0;

    case OUTPUT_TRASH_BUFFERS:
      output_trash_buffers( a, params->temp_playingpos );
      return 0;
  }

  return MCIERR_UNSUPPORTED_FUNCTION;
}

/********** GUI stuff ******************************************************/
static void
save_ini( void )
{
  save_prf_value( device );
  save_prf_value( lockdevice );
  save_prf_value( logvolume );
  save_prf_value( numbuffers );
  save_prf_value( kludge48as44 );
  save_prf_value( noprecpos );
}

static void
load_ini( void )
{
  device       = 0;
  lockdevice   = 0;
  logvolume    = 0;
  numbuffers   = 64;
  kludge48as44 = 0;
  noprecpos    = 0;

  load_prf_value( device );
  load_prf_value( lockdevice );
  load_prf_value( logvolume );
  load_prf_value( numbuffers );
  load_prf_value( kludge48as44 );
  load_prf_value( noprecpos );
}

/* Processes messages of the configuration dialog. */
static MRESULT EXPENTRY
cfg_dlg_proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
    case WM_INITDLG:
    {
      int i;
      int numdevice = output_get_devices( NULL, 0 );

      lb_add_item( hwnd, CB_DEVICE, "Default" );

      for( i = 1; i <= numdevice; i++ ) {
        char name[256];
        output_get_devices( name, i );
        lb_add_item( hwnd, CB_DEVICE, name );
      }

      lb_select( hwnd, CB_DEVICE, device );

      WinCheckButton( hwnd, CB_SHARED,   !lockdevice   );
      WinCheckButton( hwnd, CB_LOGVOLUME, logvolume    );
      WinCheckButton( hwnd, CB_48KLUDGE,  kludge48as44 );
      WinCheckButton( hwnd, CB_NOPRECPOS, noprecpos    );

      WinSendDlgItemMsg( hwnd, SB_BUFFERS, SPBM_SETLIMITS, MPFROMLONG( 200 ), MPFROMLONG( 8 ));
      WinSendDlgItemMsg( hwnd, SB_BUFFERS, SPBM_SETCURRENTVALUE, MPFROMLONG( numbuffers ), 0 );
      break;
    }

    case WM_COMMAND:
      if( SHORT1FROMMP( mp1 ) == DID_OK )
      {
        device = lb_selected( hwnd, CB_DEVICE,  LIT_FIRST );

        lockdevice   = !WinQueryButtonCheckstate( hwnd, CB_SHARED    );
        logvolume    =  WinQueryButtonCheckstate( hwnd, CB_LOGVOLUME );
        kludge48as44 =  WinQueryButtonCheckstate( hwnd, CB_48KLUDGE  );
        noprecpos    =  WinQueryButtonCheckstate( hwnd, CB_NOPRECPOS );

        WinSendDlgItemMsg( hwnd, SB_BUFFERS, SPBM_QUERYVALUE,
                           MPFROMP( &numbuffers ), MPFROM2SHORT( 0, SPBQ_DONOTUPDATE ));

        save_ini();
      }
      break;
  }

  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

/* Configure plug-in. */
void DLLENTRY
plugin_configure( HWND howner, HMODULE module )
{
  HWND  hwnd;

  hwnd = WinLoadDlg( HWND_DESKTOP, howner, cfg_dlg_proc, module, DLG_CONFIGURE, NULL );
  do_warpsans( hwnd );

  if( hwnd ) {
    WinProcessDlg( hwnd );
    WinDestroyWindow( hwnd );
  }
}

int DLLENTRY
plugin_query( PLUGIN_QUERYPARAM* param )
{
  param->type         = PLUGIN_OUTPUT;
  param->author       = "Samuel Audet, Dmitry A.Steklenev, Marcel Mueller";
  param->desc         = "DART Output 1.33";
  param->configurable = TRUE;
  param->interface    = 1;
  return 0;
}

/* init plug-in */
int DLLENTRY
plugin_init( const PLUGIN_CONTEXT* ctx )
{
  context = ctx;
  load_ini();
  return 0;
}

#if defined(__IBMC__) && defined(__DEBUG_ALLOC__)
unsigned long _System _DLL_InitTerm( unsigned long modhandle,
                                     unsigned long flag )
{
  if( flag == 0 ) {
    if( _CRT_init() == -1 ) {
      DEBUGLOG(("os2audio: failed to initialize C runtime"));
      return 0UL;
    }
    return 1UL;
  } else {
    _dump_allocated( 0 );
    _CRT_term();
    return 1UL;
  }
}
#endif
