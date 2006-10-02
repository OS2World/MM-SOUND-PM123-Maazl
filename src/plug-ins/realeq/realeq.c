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

/* EQ hard coded to 32 1/3 octave bands, kinda combersome to change realtime
   if we want band widths that are a "good" fraction of an octave */

#define  INCL_PM
#define  INCL_DOS
#include <os2.h>
#include <stdlib.h>

#include <limits.h>
#include <math.h>
#include <float.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>

#define FILTER_PLUGIN_LEVEL 2

#include <utilfct.h>
#include <fftw3.h>
#include <format.h>
#include <filter_plug.h>
#include <plugin.h>
#include "realeq.h"

//#undef DEBUG
//#define DEBUG 2
#include <debuglog.h>

#define VERSION "Real Equalizer 1.22"
#define MAX_COEF 32768
#define MIN_COEF   512
#define MAX_FIR  16384
#define MIN_FIR    256
#define NUM_BANDS   32

// Equalizer filter frequencies
static const float Frequencies[NUM_BANDS] =
{ 15.84893192,
  19.95262315,
  25.11886432,
  31.62277660,
  39.81071706,
  50.11872336,
  63.09573445,
  79.43282347,
  100.0000000,
  125.8925412,
  158.4893192,
  199.5262315,
  251.1886432,
  316.2277660,
  398.1071706,
  501.1872336,
  630.9573445,
  794.3282347,
  1000.000000,
  1258.925412,
  1584.893192,
  1995.262315,
  2511.886432,
  3162.277660,
  3981.071706,
  5011.872336,
  6309.573445,
  7943.282347,
  10000.00000,
  12589.25412,
  15848.93192,
  19952.62315
};

static BOOL eqneedinit;
static BOOL eqneedFIR;
static BOOL eqneedEQ;
static HWND hdialog     = NULLHANDLE;
static BOOL plugininit  = FALSE; // plug-in is initialized (only once)

// note: I originally intended to use 8192 for the finalFIR arrays
// but Pentium CPUs have a too small cache (16KB) and it totally shits
// putting anything bigger.
// Update 2006-07-10 MM: Solved. Now FIR order 12288 works.
// Higher values are possible too, but will require compensation for the latency.

static float bandgain[2][NUM_BANDS]; // gain in dB
static BOOL  mute[2][NUM_BANDS];     // mute flags (TRUE = mute)
static float preamp = 1.0;           // global gain

// for FFT convolution...
static struct
{ short* inbox;               // buffer to collect incoming samples
  float* time_domain;         // buffer in the time domain
  fftwf_complex* freq_domain; // buffer in the frequency domain (shared memory with design)
  float* design;              // buffer to design filter kernel (shared memory with freq_domain)
  float* kernel[2];           // since the kernel is real even, it's even real in the time domain
  float* overlap[2];          // keep old samples for convolution
  fftwf_complex* channel_save;// buffer to keep the frequency domain of a mono input for a second convolution
  fftwf_plan forward_plan;    // fftw plan for time_domain -> freq_domain
  fftwf_plan backward_plan;   // fftw plan for freq_domain -> time_domain
  fftwf_plan DCT_plan;        // fftw plan for design -> time domain
  fftwf_plan RDCT_plan;       // fftw plan for time_domain -> kernel
  int    FIRorder;            // filter kernel length
  int    plansize;            // plansize for the FFT convolution
  int    DCTplansize;         // plansize for the filter design
} FFT;

// settings
static int  newFIRorder; // this is for the GUI only
static int  newPlansize; // this is for the GUI only
static BOOL eqenabled = FALSE;
static BOOL locklr    = FALSE;
static char lasteq[CCHMAXPATH];

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_LN10
#define M_LN10 2.302585093
#endif

/* Hamming window
#define WINDOW_FUNCTION( n, N ) (0.54 - 0.46 * cos( 2 * M_PI * n / N ))
 * Well, since the EQ does not provide more tham 24dB dynamics
 * we should use a much more agressive window function.
 * This is verified by calculations.
 */
#define WINDOW_FUNCTION( n, N ) (0.77 - 0.23 * cos( 2 * M_PI * n / N ))

#define round(n) ((n) > 0 ? (n) + 0.5 : (n) - 0.5)

#define DO_8(p,x) \
{  { const int p = 0; x; } \
   { const int p = 1; x; } \
   { const int p = 2; x; } \
   { const int p = 3; x; } \
   { const int p = 4; x; } \
   { const int p = 5; x; } \
   { const int p = 6; x; } \
   { const int p = 7; x; } \
}

typedef struct {

   ULONG (PM123_ENTRYP output_command)       ( void* a, ULONG msg, OUTPUT_PARAMS2* info );
   int   (PM123_ENTRYP output_request_buffer)( void* a, const FORMAT_INFO2* format, short** buf );
   void  (PM123_ENTRYP output_commit_buffer) ( void* a, int len, int posmarker );
   void* a;
   void  (PM123_ENTRYP error_display)        ( char* );

   FORMAT_INFO2 format;

   int   posmarker;  // starting point of the inbox buffer
   int   inboxlevel; // number of samples in inbox buffer
   int   latency;    // samples to discard before passing them to the output stage
   BOOL  enabled;    // flag whether the EQ was enabled at the last call to request_buffer
   int   temppos;    // >= 0: discard buffer content before next request_buffer

} REALEQ_STRUCT;


/********** Ini file stuff */

static void
save_ini( void )
{
  HINI INIhandle;

  if(( INIhandle = open_module_ini()) != NULLHANDLE )
  {
    save_ini_value ( INIhandle, newFIRorder );
    save_ini_value ( INIhandle, newPlansize );
    save_ini_value ( INIhandle, eqenabled );
    save_ini_value ( INIhandle, locklr );
    save_ini_string( INIhandle, lasteq );

    close_ini( INIhandle );
  }
}

static void
load_ini( void )
{
  HINI INIhandle;

  memset(bandgain, 0, sizeof bandgain);
  memset(mute,     0, sizeof mute    );

  eqenabled   = FALSE;
  lasteq[0]   = 0;
  newPlansize = 8192;
  newFIRorder = 4096;
  locklr      = FALSE;

  if(( INIhandle = open_module_ini()) != NULLHANDLE )
  {
    load_ini_value ( INIhandle, newFIRorder );
    load_ini_value ( INIhandle, newPlansize );
    load_ini_value ( INIhandle, eqenabled );
    load_ini_value ( INIhandle, locklr );
    load_ini_string( INIhandle, lasteq, sizeof( lasteq ));

    close_ini( INIhandle );

    // avoid crash when INI-Content is bad
    if (newPlansize < 16)
      newPlansize = 16;
     else if (newPlansize > MAX_COEF)
      newPlansize = MAX_COEF;
    if (newPlansize <= newFIRorder)
      newFIRorder = newPlansize >> 1;
    if (newFIRorder > MAX_FIR)
      newFIRorder = MAX_FIR;
  }
  eqneedinit  = TRUE;
}

static BOOL
load_eq_file( char* filename, float* gains, BOOL* mutes, float* preamp )
{
  FILE* file;
  int   i = 0;
  char  line[256];

  if (filename == NULL || *filename == 0)
    return FALSE;
  file = fopen( filename, "r" );
  if( file == NULL ) {
    return FALSE;
  }

  while( !feof( file ))
  {
    fgets( line, sizeof(line), file );
    blank_strip( line );
    if( *line && line[0] != '#' && line[0] != ';' && i < 129 )
    {
      if( i < NUM_BANDS*2 ) {
        double gain = atof(line);
        if (gain < 0)
          gain = 0;
         else if (gain <= .2511886432)
          gain = -12.;
         else if (gain >= 3.981071706)
          gain = 12.;
         else
          gain = 20.*log10(gain); 
        gains[i] = gain;
      } else if( i > NUM_BANDS*2-1 && i < NUM_BANDS*4 ) {
        mutes[i-NUM_BANDS*2] = atoi(line);
      } else if( i == NUM_BANDS*4 ) {
        *preamp = atof(line);
      }
      i++;
    }
  }
  fclose( file );
  return TRUE;
}

static void
init_request( void )
{ if (!plugininit) // first time?
  { load_ini();
    load_eq_file( lasteq, bandgain[0], mute[0], &preamp );
    plugininit = TRUE;
  }
}

static void
trash_buffers( REALEQ_STRUCT* f )
{ DEBUGLOG(("realeq::trash_buffers(%p) - %d %d\n", f, f->inboxlevel, f->latency));
  f->inboxlevel = 0;
  f->latency    = -1;
  memset(FFT.overlap[0], 0, FFT.plansize * 2 * sizeof(float)); // initially zero
}

typedef struct
{ float lf; // log frequency
  float lv; // log value
} EQcoef;

/* setup FIR kernel */
static BOOL
fil_setup( REALEQ_STRUCT* f )
{
  static BOOL FFTinit = FALSE;
  int   i, channel;
  float fftspecres;
  // equalizer design data
  EQcoef coef[NUM_BANDS+4]; // including surounding points

  // mute = -36dB. More makes no sense since the stopband attenuation
  // of the window function does not allow significantly better results.
  const float log_mute_level = -4.144653167; // -36dB

  // dispatcher to skip code fragments
  if (!eqneedinit)
  { if (eqneedFIR)
      goto doFIR;
    if (eqneedEQ)
      goto doEQ;
    return TRUE;
  }

  // STEP 1: initialize buffers and FFT plans
   
  // free old resources
  if (FFTinit)
  { DEBUGLOG(("fil_setup: free\n"));
    free(FFT.inbox);
    fftwf_destroy_plan(FFT.forward_plan);
    fftwf_destroy_plan(FFT.backward_plan);
    fftwf_destroy_plan(FFT.RDCT_plan);
    fftwf_free(FFT.freq_domain);
    fftwf_free(FFT.time_domain);
    // do not free aliased arrays!
    free(FFT.overlap[0]);
    fftwf_free(FFT.kernel[0]);
  }

  // copy global parameters for thread safety and round up to next power of 2
  frexp(newPlansize-1, &i); // floor(log2(plansize-1))+1
  FFT.plansize = 1 << i; // 2**x
  DEBUGLOG(("I: %d - %p\n", FFT.plansize, FFT.DCT_plan));

  // allocate buffers
  FFT.inbox       = (short*)malloc(FFT.plansize * 2 * sizeof(short));
  FFT.freq_domain = (fftwf_complex*)fftwf_malloc((FFT.plansize/2+1) * sizeof *FFT.freq_domain);
  FFT.time_domain = (float*)fftwf_malloc((FFT.plansize+1) * sizeof *FFT.time_domain);
  FFT.design      = FFT.freq_domain[0]; // Aliasing!
  FFT.overlap[0]  = (float*)malloc(FFT.plansize * 2 * sizeof(float));
  FFT.overlap[1]  = FFT.overlap[0] + FFT.plansize;
  FFT.channel_save= (fftwf_complex*)FFT.overlap[1] -1; // Aliasing!
  FFT.kernel[0]   = (float*)fftwf_malloc((FFT.plansize/2+1) * 2 * sizeof(float));
  FFT.kernel[1]   = FFT.kernel[0]  + (FFT.plansize/2+1);

  // prepare real 2 complex transformations
  FFT.forward_plan  = fftwf_plan_dft_r2c_1d( FFT.plansize, FFT.time_domain, FFT.freq_domain, FFTW_ESTIMATE );
  FFT.backward_plan = fftwf_plan_dft_c2r_1d( FFT.plansize, FFT.freq_domain, FFT.time_domain, FFTW_ESTIMATE );
  // prepare real 2 real transformations
  FFT.RDCT_plan     = fftwf_plan_r2r_1d( FFT.plansize/2+1, FFT.time_domain, FFT.kernel[0], FFTW_REDFT00, FFTW_ESTIMATE );
  
  trash_buffers(f);

  eqneedinit = FALSE;

 doFIR:
  // STEP 2: setup FIR order
  
  if( newFIRorder < 2 || newFIRorder >= FFT.plansize)
  { (*f->error_display)("very bad! The FIR order and/or the FFT plansize is invalid or the FIRorder is higer or equal to the plansize.");
    eqenabled = FALSE; // avoid crash
    return FALSE;
  }

  // copy global parameters for thread safety
  FFT.FIRorder = (newFIRorder+15) & -16; /* multiple of 16 */

  // calculate optimum plansize for kernel generation
  frexp((FFT.FIRorder<<1) -1, &FFT.DCTplansize); // floor(log2(2*FIRorder-1))+1
  FFT.DCTplansize = 1 << FFT.DCTplansize; // 2**x
  // free old resources
  if (FFTinit)
    fftwf_destroy_plan(FFT.DCT_plan);
  // prepare real 2 real transformations
  FFT.DCT_plan = fftwf_plan_r2r_1d(FFT.DCTplansize/2+1, FFT.design, FFT.time_domain, FFTW_REDFT00, FFTW_ESTIMATE);

  DEBUGLOG(("P: FIRorder: %d, Plansize: %d, DCT plansize: %d\n", FFT.FIRorder, FFT.plansize, FFT.DCTplansize));
  eqneedFIR = FALSE;

 doEQ:
  // STEP 3: setup filter kernel
  
  fftspecres = (float)f->format.samplerate / FFT.DCTplansize;

  // Prepare design coefficients frame
  coef[0].lf = -14; // very low frequency
  coef[0].lv = log_mute_level;
  coef[1].lf = M_LN10; // subsonic point
  coef[1].lv = log_mute_level;
  for (i = 0; i < NUM_BANDS; ++i)
    coef[i+2].lf = log(Frequencies[i]);
  coef[NUM_BANDS+2].lf = log(32000); // keep higher frequencies at 0 dB
  coef[NUM_BANDS+2].lv = 0;
  coef[NUM_BANDS+3].lf = 14; // very high frequency
  coef[NUM_BANDS+3].lv = 0;

  /* for left, right */
  for( channel = 0; channel < 2; channel++ )
  {
    // fill band data
    for (i = 0; i < NUM_BANDS; ++i)
      coef[i+2].lv = mute[channel][i] ? log_mute_level : bandgain[channel][i]/20. * M_LN10;

    // compose frequency spectrum
    { EQcoef* cop = coef;
      FFT.design[0] = 0; // no DC
      for (i = 1; i <= FFT.DCTplansize/2; ++i) // do not start at f=0 to avoid log(0)
      { const float f = i * fftspecres; // current frequency
        double pos;
        double val = log(f);
        while (val > cop[1].lf)
          ++cop;
        // do double logarithmic sine^2 interpolation
        pos = .5 - .5*cos(M_PI * (log(f)-cop[0].lf) / (cop[1].lf-cop[0].lf));
        val = exp(cop[0].lv + pos * (cop[1].lv - cop[0].lv));
        FFT.design[i] = val;
        DEBUGLOG2(("F: %i, %g, %g -> %g = %g dB @ %g\n",
          i, f, pos, FFT.design[i], 20*log(val)/M_LN10, exp(cop[0].lf)));
    } }

    // transform into the time domain
    fftwf_execute(FFT.DCT_plan);
    #if defined(DEBUG) && DEBUG > 1
    for (i = 0; i <= FFT.DCTplansize/2; ++i)
      DEBUGLOG2(("TK: %i, %g\n", i, FFT.time_domain[i]));
    #endif

    // normalize, apply window function and store results symmetrically
    { float* sp = FFT.time_domain;
      for (i = FFT.FIRorder/2; i >= 0; --i)
      { *sp *= WINDOW_FUNCTION(i, FFT.FIRorder) / FFT.DCTplansize / FFT.plansize; // normalize for next FFT
        DEBUGLOG2(("K: %i, %g\n", i, *sp));
        ++sp;
      }
      // padding
      memset(sp, 0, (FFT.plansize-FFT.FIRorder)/2 * sizeof *FFT.time_domain);
    }

    // prepare for FFT convolution
    // transform back into the frequency domain, now with window function
    fftwf_execute_r2r(FFT.RDCT_plan, FFT.time_domain, FFT.kernel[channel]);
    #if defined(DEBUG) && DEBUG > 1
    for (i = 0; i <= FFT.plansize/2; ++i)
      DEBUGLOG2(("FK: %i, %g\n", i, FFT.kernel[channel][i]));
    DEBUGLOG2(("E: kernel completed.\n"));
    #endif
  }

  eqneedEQ = FALSE;
  FFTinit = TRUE;
  return TRUE;
}

/* do convolution and back transformation
 */
static void
do_fft_convolute(fftwf_complex* sp, float* kp)
{ int l;
  fftwf_complex* dp = FFT.freq_domain;
  DEBUGLOG(("realeq:do_fft_convolute(%p, %p) - %p\n", sp, kp, dp));
  // Convolution in the frequency domain is simply a series of complex products.
  // But because of the symmetry of the filter kernel it is just comlpex * real.
  for (l = (FFT.plansize/2+1) >> 3; l; --l)
  { DO_8(p,
      dp[p][0] = sp[p][0] * kp[p];
      dp[p][1] = sp[p][1] * kp[p];
    );
    sp += 8;
    kp += 8;
    dp += 8;
  }
  for (l = (FFT.plansize/2+1) & 7; l; --l)
  { dp[0][0] = sp[0][0] * kp[0];
    dp[0][1] = sp[0][1] * kp[0];
    ++sp;
    ++kp;
    ++dp;
  }
  // do IFFT
  fftwf_execute(FFT.backward_plan);
}

/* fetch saved samples */
INLINE void
do_fft_load_overlap(float* overlap_buffer)
{ memcpy(FFT.time_domain + FFT.plansize - FFT.FIRorder/2, overlap_buffer, FFT.FIRorder/2 * sizeof *FFT.time_domain);
  memcpy(FFT.time_domain, overlap_buffer + FFT.FIRorder/2, FFT.FIRorder/2 * sizeof *FFT.time_domain);
}

/* store last FIRorder samples */
static void
do_fft_save_overlap(float* overlap_buffer, int len)
{
  DEBUGLOG(("realeq:do_fft_save_overlap(%p, %i) - %i\n", overlap_buffer, len, FFT.FIRorder));
  if (len < FFT.FIRorder/2)
  { memmove(overlap_buffer, overlap_buffer + len, (FFT.FIRorder - len) * sizeof *overlap_buffer);
    memcpy(overlap_buffer + FFT.FIRorder - len, FFT.time_domain + FFT.FIRorder/2, len * sizeof *overlap_buffer);
  } else
    memcpy(overlap_buffer, FFT.time_domain + len - FFT.FIRorder/2, FFT.FIRorder * sizeof *overlap_buffer);
}

/* convert samples from short to float and store it in the FFT.time_domain buffer
   The samples are shifted by FIRorder/2 to the right.
   FIRorder samples before the starting position are taken from overlap_buffer.
   The overlap_buffer is updated with the last FIRorder samples before return.
 */
static void
do_fft_load_samples_mono(const short* sp, const int len, float* overlap_buffer)
{ int l;
  float* dp = FFT.time_domain + FFT.FIRorder/2;
  do_fft_load_overlap(overlap_buffer);
  // fetch new samples
  for (l = len >> 3; l; --l) // coarse
  { DO_8(p, dp[p] = sp[p]);
    sp += 8;
    dp += 8;
  }
  for (l = len & 7; l; --l) // fine
    *dp++ = *sp++;
  memset(dp, 0, (FFT.plansize - FFT.FIRorder - len) * sizeof *dp); // padding not required, we simply ignore part of the result.
  do_fft_save_overlap(overlap_buffer, len);
}
/* convert samples from short to float and store it in the FFT.time_domain buffer
 * This will cover only one channel. The only difference to to mono-version is that
 * the source pointer is incremented by 2 per sample.
 */
static void
do_fft_load_samples_stereo(const short* sp, const int len, float* overlap_buffer)
{ int l;
  float* dp = FFT.time_domain + FFT.FIRorder/2;
  do_fft_load_overlap(overlap_buffer);
  for (l = len >> 3; l; --l) // coarse
  { DO_8(p, dp[p] = sp[p<<1]);
    sp += 16;
    dp += 8;
  }
  for (l = len & 7; l; --l) // fine
  { *dp++ = *sp;
    sp += 2;
  }
  memset(dp, 0, (FFT.plansize - FFT.FIRorder - len) * sizeof *dp); // padding not required, we simply ignore part of the result.
  do_fft_save_overlap(overlap_buffer, len);
}

INLINE short
quantize(float f)
{ if (f < -32768)
    return -32768;
  if (f > 32767)
    return 32767;
  return (short)f; // Well, dithering might be nice, but slow either.
}

/* store stereo samples
 * This will cover only one channel. The only difference to to mono-version is that
 * the destination pointer is incremented by 2 per sample.
 */
static void
do_fft_store_samples_stereo(short* sp, const int len)
{ float* dp = FFT.time_domain;
  int l;
  DEBUGLOG(("realeq:do_fft_store_samples_stereo(%p, %i)\n", sp, len));
  // transfer samples
  for (l = len >> 3; l; --l) // coarse
  { DO_8(p, sp[p<<1] = quantize(dp[p]));
    sp += 16;
    dp += 8;
  }
  for (l = len & 7; l; --l) // fine
  { *sp = quantize(*dp++);
    sp += 2;
  }
}

static void
filter_samples_fft( REALEQ_STRUCT* f, short* newsamples, const short* buf, int len )
{ DEBUGLOG(("realeq:filter_samples_fft(%p, %p, %p, %i)\n", f, newsamples, buf, len));

  while (len) // we might need more than one FFT cycle
  { int l2 = FFT.plansize - FFT.FIRorder;
    if (l2 > len)
      l2 = len;

    if ( f->format.channels == 2 )
    { // left channel
      // convert to float (well, that bill we have to pay)
      do_fft_load_samples_stereo(buf, l2, FFT.overlap[0]);
      // do FFT
      fftwf_execute(FFT.forward_plan);
      // convolution
      do_fft_convolute(FFT.freq_domain, FFT.kernel[0]);
      // convert back to short
      do_fft_store_samples_stereo(newsamples, l2);
      // right channel
      // convert to float (well, that bill we have to pay)
      do_fft_load_samples_stereo(buf+1, l2, FFT.overlap[1]);
      // do FFT
      fftwf_execute(FFT.forward_plan);
      // convolution
      do_fft_convolute(FFT.freq_domain, FFT.kernel[1]);
      // convert back to short
      do_fft_store_samples_stereo(newsamples+1, l2);
      // next block
      len -= l2;
      l2 <<= 1; // stereo
      buf += l2;
      newsamples += l2;
    } else
    { // left channel
      // convert to float (well, that bill we have to pay)
      do_fft_load_samples_mono(buf, l2, FFT.overlap[0]);
      // do FFT
      fftwf_execute(FFT.forward_plan);
      // save data for 2nd channel
      memcpy(FFT.channel_save, FFT.freq_domain, (FFT.plansize/2+1) * sizeof *FFT.freq_domain);
      // convolution
      do_fft_convolute(FFT.freq_domain, FFT.kernel[0]);
      // convert back to short
      do_fft_store_samples_stereo(newsamples, l2);
      // right channel
      // convolution
      do_fft_convolute(FFT.channel_save, FFT.kernel[1]);
      // convert back to short
      do_fft_store_samples_stereo(newsamples+1, l2);
      // next block
      len -= l2;
      buf += l2;
      l2 <<= 1; // stereo
      newsamples += l2;
    }
  }
}

/* only update the overlap buffer, no filtering */
static void
filter_samples_new_overlap( REALEQ_STRUCT* f, const short* buf, int len )
{ int l2;
  DEBUGLOG(("realeq:filter_samples_new_overlap(%p, %p, %i)\n", f, buf, len));
  
  l2 = FFT.plansize - FFT.FIRorder;
  if (len > l2)
  { // skip unneeded samples
    buf += (len - l2) * f->format.channels;
    len = l2;
  }

  // Well, there might be a more optimized version without copying the old overlap buffer, but who cares
  if ( f->format.channels == 2 )
  { do_fft_load_samples_stereo(buf  , l2, FFT.overlap[0]);
    do_fft_load_samples_stereo(buf+1, l2, FFT.overlap[1]);
  } else 
  { do_fft_load_samples_mono(buf, l2, FFT.overlap[0]);
  }
}

/* Proxy funtions to remove the first samples to compensate for the filter delay */
static int
do_request_buffer( REALEQ_STRUCT* f, short** buf )
{ DEBUGLOG(("realeq:do_request_buffer(%p, %p) - %d\n", f, buf, f->latency));
  if (f->latency != 0)
  { if (f->latency < 0)
      f->latency = FFT.FIRorder >> 1;
    *buf = NULL; // discard
    return f->latency;
  } else
  { FORMAT_INFO2 fi = f->format;
    fi.channels = 2; // result is always stereo
    return (*f->output_request_buffer)( f->a, &fi, buf );
  }
}

static void
do_commit_buffer( REALEQ_STRUCT* f, int len, int posmarker )
{ DEBUGLOG(("realeq:do_commit_buffer(%p, %d, %d) - %d\n", f, len, posmarker, f->latency));
  if (f->latency != 0)
  { f->latency -= len;
  } else
  { (*f->output_commit_buffer)( f->a, len, posmarker );
  }
}

/* applies the FFT filter to the current inbox content and sends the result to the next plug-in.
 * You should not call this function with f->inboxlevel == 0.
 */
static void
filter_and_send( REALEQ_STRUCT* f )
{
  int    len, dlen;
  short* dbuf;
  DEBUGLOG(("realeq:filter_and_send(%p) - %d\n", f, f->inboxlevel));

  // request destination buffer
  dlen = do_request_buffer( f, &dbuf );
  
  if (f->temppos != -1)
  { f->inboxlevel = 0;
    do_commit_buffer(f, 0, f->temppos); // no-op
    return;
  }

  len = f->inboxlevel; // required destination length
  if (dlen < len)
  { // with fragmentation
    int rem = len;
    short* sp;
  
    filter_samples_fft( f, FFT.inbox, FFT.inbox, f->inboxlevel );
    
    // transfer data
    sp = FFT.inbox;
    for(;;)
    { if (dbuf != NULL)
        memcpy(dbuf, sp, dlen * sizeof(short) * 2);
      do_commit_buffer( f, dlen, f->posmarker + (len-rem)*1000/f->format.samplerate );
      rem -= dlen;
      if (rem == 0)
        break;
      sp += dlen * 2;
      // request next buffer
      dlen = do_request_buffer( f, &dbuf );
      if (f->temppos != -1)
      { f->inboxlevel = 0;
        do_commit_buffer(f, 0, f->temppos); // no-op
        return;
      }
      if (dlen > rem)
        dlen = rem;
    }
    
  } else
  { // without fragmentation
    if (dbuf == NULL)
    { // only save overlap
      filter_samples_new_overlap( f, FFT.inbox, f->inboxlevel );
    } else
    { filter_samples_fft( f, dbuf, FFT.inbox, f->inboxlevel );
    }
    do_commit_buffer( f, len, f->posmarker );
  }

  f->posmarker += f->inboxlevel*1000/f->format.samplerate;
  f->inboxlevel = 0;
}

static void
local_flush( REALEQ_STRUCT* f )
{ // emulate input of some zeros to compensate for filter size
  int   len = (FFT.FIRorder+1) >> 1;
  DEBUGLOG(("realeq:local_flush(%p) - %d %d %d\n", f, len, f->inboxlevel, f->latency));
  while (len != 0)
  { int dlen = FFT.plansize - FFT.FIRorder - f->inboxlevel;
    if (dlen > len)
      dlen = len;
    memset(FFT.inbox + f->inboxlevel * f->format.channels, 0, dlen * f->format.channels * sizeof(short));
    // commit buffer
    f->inboxlevel += dlen;
    if (f->inboxlevel == FFT.plansize - FFT.FIRorder)
    { // enough data, apply filter
      filter_and_send(f);
      if (f->temppos != -1)
        break;
    }
    len -= dlen;
  }

  // flush buffer  
  if (f->inboxlevel != 0)
    filter_and_send(f);

  // setup for restart
  trash_buffers(f);
}

/* Entry point: do filtering */
static int PM123_ENTRY
filter_request_buffer( REALEQ_STRUCT* f, const FORMAT_INFO2* format, short** buf )
{ BOOL enabled;
  #ifdef DEBUG
  if (format != NULL)
    DEBUGLOG(("realeq:filter_request_buffer(%p, {%u, %u, %u}, %p) - %d %d\n",
     f, format->size, format->samplerate, format->channels, buf, f->inboxlevel, f->latency));
   else
    DEBUGLOG(("realeq:filter_request_buffer(%p, %p, %p) - %d %d\n", f, format, buf, f->inboxlevel, f->latency));
  #endif

  enabled = eqenabled && buf != NULL && ( format->channels == 1 || format->channels == 2 );
  if (enabled && !f->enabled)
  { // enable EQ
    f->enabled = TRUE;
    f->latency = -1;
  } else if (!enabled && f->enabled)
  { // disable EQ
    local_flush(f);
    f->enabled = FALSE;
    if (buf == NULL) // global flush();
    { return (*f->output_request_buffer)( f->a, format, buf );
    }
  }

  if ( f->enabled )
  {
    if (f->temppos != -1)
    { trash_buffers(f);
      f->temppos = -1;
    }
    if ( f->format.samplerate != format->samplerate || f->format.channels != format->channels )
    { if (f->format.samplerate != 0)
        local_flush( f );
      f->format.samplerate = format->samplerate;
      f->format.channels   = format->channels;
      eqneedinit = TRUE;
    } else if (eqneedFIR || eqneedinit)
    { if (f->format.samplerate != 0)
        local_flush( f );
    }
    
    fil_setup( f );

    *buf = FFT.inbox + f->inboxlevel * format->channels;
    DEBUGLOG(("realeq:filter_request_buffer: %p, %d\n", *buf, FFT.plansize - FFT.FIRorder - f->inboxlevel));
    return FFT.plansize - FFT.FIRorder - f->inboxlevel;
  }                                                        
  else
  {
    return (*f->output_request_buffer)( f->a, format, buf );
  }
}

static void PM123_ENTRY
filter_commit_buffer( REALEQ_STRUCT* f, int len, int posmarker )
{
  DEBUGLOG(("realeq:filter_commit_buffer(%p, %u, %u) - %d %d\n", f, len, posmarker, f->inboxlevel, f->latency));
  
  if (!f->enabled)
  { (*f->output_commit_buffer)( f->a, len, posmarker );
    return;
  }

  if (f->inboxlevel == 0)
    // remember position and precompensate for filter delay
    f->posmarker = posmarker - (FFT.FIRorder>>1)*1000/f->format.samplerate;

  f->inboxlevel += len;

  if (f->inboxlevel == FFT.plansize - FFT.FIRorder)
  { // enough data, apply filter
    filter_and_send(f);
  }
}

static ULONG PM123_ENTRY
filter_command( REALEQ_STRUCT* f, ULONG msg, OUTPUT_PARAMS2* info )
{ DEBUGLOG(("realeq:filter_command(%p, %u, %p)\n", f, msg, info));
  switch (msg)
  {case OUTPUT_SETUP:
    if (info->formatinfo.channels == 1 && eqenabled)
      info->formatinfo.channels = 2;
    break;
   case OUTPUT_TRASH_BUFFERS:
    f->temppos = info->temp_playingpos;
    break;
   case OUTPUT_CLOSE:
    f->temppos = f->posmarker;
    break;
  }
  return (*f->output_command)( f->a, msg, info );
}

/********** Entry point: Initialize
*/
ULONG PM123_ENTRY
filter_init( void** F, FILTER_PARAMS2* params )
{
  REALEQ_STRUCT* f = (REALEQ_STRUCT*)malloc( sizeof( REALEQ_STRUCT ));
  DEBUGLOG(("filter_init(%p->%p, {%u, ..., %p, ..., %p})\n", F, f, params->size, params->a, params->w));

  *F = f;
 
  init_request();
  eqneedinit = TRUE;

  f->output_command        = params->output_command;
  f->output_request_buffer = params->output_request_buffer;
  f->output_commit_buffer  = params->output_commit_buffer;
  f->a                     = params->a;
  f->error_display         = params->error_display;
  f->inboxlevel            = 0;
  f->enabled               = FALSE; // flag is set later
  f->temppos               = -1;

  f->format.size           = sizeof f->format;
  f->format.samplerate     = 0;
  f->format.channels       = 0;
  
  params->output_command        = (ULONG (PM123_ENTRYP)(void*, ULONG, OUTPUT_PARAMS2*))      &filter_command;
  params->output_request_buffer = (int   (PM123_ENTRYP)(void*, const FORMAT_INFO2*, short**))&filter_request_buffer;
  params->output_commit_buffer  = (void  (PM123_ENTRYP)(void*, int, int))                    &filter_commit_buffer;
  return 0;
}

void PM123_ENTRY
filter_update( void *F, const FILTER_PARAMS2 *params )
{ REALEQ_STRUCT* f = (REALEQ_STRUCT*)F;
  DosEnterCritSec();
  f->output_command        = params->output_command;
  f->output_request_buffer = params->output_request_buffer;
  f->output_commit_buffer  = params->output_commit_buffer;
  f->a                     = params->a;
  f->error_display         = params->error_display;
  DosExitCritSec();
}

BOOL PM123_ENTRY
filter_uninit( void* F )
{
  REALEQ_STRUCT* f = (REALEQ_STRUCT*)F;
  DEBUGLOG(("filter_uninit(%p)\n", F));

  if( f != NULL )
  {
    free( f );
  }
  return TRUE;
}


/********** GUI stuff *******************************************************/

static BOOL
save_eq( HWND hwnd, float* gains, BOOL* mutes, float preamp )
{
  FILEDLG filedialog;
  FILE*   file;
  int     i = 0;

  memset( &filedialog, 0, sizeof(FILEDLG));
  filedialog.cbSize   = sizeof(FILEDLG);
  filedialog.fl       = FDS_CENTER | FDS_SAVEAS_DIALOG;
  filedialog.pszTitle = "Save Equalizer as...";

  if( lasteq[0] == 0 ) {
    strcpy( filedialog.szFullFile, "*.REQ" );
  } else {
    strcpy( filedialog.szFullFile, lasteq );
  }

  WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &filedialog );

  if( filedialog.lReturn == DID_OK )
  {
    strcpy( lasteq, filedialog.szFullFile );
    file = fopen( filedialog.szFullFile, "w" );
    if( file == NULL ) {
      return FALSE;
    }

    fprintf( file, "#\n# Equalizer created with %s\n# Do not modify!\n#\n", VERSION );
    fprintf( file, "# Band gains\n" );
    for( i = 0; i < NUM_BANDS*2; i++ ) {
      fprintf( file, "%g\n", pow(10,gains[i]/20) );
    }
    fprintf(file, "# Mutes\n" );
    for( i = 0; i < NUM_BANDS*2; i++ ) {
      fprintf(file, "%u\n", mutes[i]);
    }
    fprintf( file, "# Preamplifier\n" );
    fprintf( file, "%g\n", preamp );

    fprintf( file, "# End of equalizer\n" );
    fclose ( file );
    return TRUE;
  }

  return FALSE;
}

static
void drivedir( char* buf, char* fullpath )
{
  char drive[_MAX_DRIVE],
       path [_MAX_PATH ];

  _splitpath( fullpath, drive, path, NULL, NULL );
  strcpy( buf, drive );
  strcat( buf, path  );
}

static BOOL
load_eq( HWND hwnd, float* gains, BOOL* mutes, float* preamp )
{
  FILEDLG filedialog;

  memset( &filedialog, 0, sizeof( FILEDLG ));
  filedialog.cbSize = sizeof(FILEDLG);
  filedialog.fl = FDS_CENTER | FDS_OPEN_DIALOG;
  filedialog.pszTitle = "Load Equalizer";
  drivedir( filedialog.szFullFile, lasteq );
  strcat( filedialog.szFullFile, "*.REQ" );

  WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &filedialog );

  if( filedialog.lReturn == DID_OK )
  {
    strcpy( lasteq, filedialog.szFullFile );
    return load_eq_file( filedialog.szFullFile, gains, mutes, preamp );
  }
  return FALSE;
}

int PM123_ENTRY
plugin_query( PLUGIN_QUERYPARAM *param )
{ // Check PM123 core version roughly
  if ( param->size < sizeof (PLUGIN_QUERYPARAM) )
  { param->type = 0;
    return -1;
  }
  param->type         = PLUGIN_FILTER;
  param->author       = "Samuel Audet, Marcel M�ller";
  param->desc         = VERSION;
  param->configurable = TRUE;
  param->interface    = FILTER_PLUGIN_LEVEL;
  return 0;
}

static void
set_slider( HWND hwnd, int channel, int band, double value )
{ MRESULT rangevalue;
  DEBUGLOG2(("realeq:set_slider(%p, %d, %d, %f)\n", hwnd, channel, band, value));

  if (value < -12)
  { DEBUGLOG(("load_dialog: value out of range %d, %d, %f\n", channel, band, value));
    value = -12;
  } else if (value > 12)
  { DEBUGLOG(("load_dialog: value out of range %d, %d, %f\n", channel, band, value));
    value = 12;
  }

  rangevalue = WinSendDlgItemMsg( hwnd, 200+NUM_BANDS*channel+band, SLM_QUERYSLIDERINFO,
                                  MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ), 0 );

  WinSendDlgItemMsg( hwnd, 200+NUM_BANDS*channel+band, SLM_SETSLIDERINFO,
                     MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                     MPFROMSHORT( (value/24.+.5) * (SHORT2FROMMR(rangevalue) - 1) +.5 ));
}

static void
load_dialog( HWND hwnd )
{
  int i, e;

  for( e = 0; e < 2; e++ ) {
    for( i = 0; i < NUM_BANDS; i++ ) {
      SHORT   range;

      // mute check boxes
      WinSendDlgItemMsg( hwnd, 100 + NUM_BANDS*e + i, BM_SETCHECK, MPFROMCHAR( mute[e][i] ), 0 );

      // sliders
      range = SHORT2FROMMR( WinSendDlgItemMsg( hwnd, 200 + NUM_BANDS*e + i, SLM_QUERYSLIDERINFO,
                                               MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ), 0 ) );
          
      WinSendDlgItemMsg( hwnd, 200 + NUM_BANDS*e + i, SLM_ADDDETENT,
                         MPFROMSHORT( range - 1 ), 0 );
      WinSendDlgItemMsg( hwnd, 200 + NUM_BANDS*e + i, SLM_ADDDETENT,
                         MPFROMSHORT( range >> 1 ), 0 );
      WinSendDlgItemMsg( hwnd, 200 + NUM_BANDS*e + i, SLM_ADDDETENT,
                         MPFROMSHORT( 0 ), 0 );

      DEBUGLOG2(("load_dialog: %d %d %g\n", e, i, bandgain[e][i]));
      set_slider( hwnd, e, i, bandgain[e][i] );
    }
  }

  // eq enabled check box
  WinSendDlgItemMsg( hwnd, EQ_ENABLED, BM_SETCHECK, MPFROMSHORT( eqenabled ), 0 );
  WinSendDlgItemMsg( hwnd, ID_LOCKLR,  BM_SETCHECK, MPFROMSHORT( locklr ), 0 );

  WinSendDlgItemMsg( hwnd, ID_FIRORDER, SPBM_SETMASTER, MPFROMLONG( NULLHANDLE ), 0 );
  WinSendDlgItemMsg( hwnd, ID_FIRORDER, SPBM_SETLIMITS, MPFROMLONG( MAX_FIR ), MPFROMLONG( MIN_FIR ));
  WinSendDlgItemMsg( hwnd, ID_FIRORDER, SPBM_SETCURRENTVALUE, MPFROMLONG( newFIRorder ), 0 );
  WinSendDlgItemMsg( hwnd, ID_PLANSIZE, SPBM_SETMASTER, MPFROMLONG( NULLHANDLE ), 0 );
  WinSendDlgItemMsg( hwnd, ID_PLANSIZE, SPBM_SETLIMITS, MPFROMLONG( MAX_COEF ), MPFROMLONG( MIN_COEF ));
  WinSendDlgItemMsg( hwnd, ID_PLANSIZE, SPBM_SETCURRENTVALUE, MPFROMLONG( newPlansize ), 0 );
}

static MRESULT EXPENTRY
ConfigureDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  static char nottheuser = FALSE;

  switch( msg )
  {
    case WM_INITDLG:
      nottheuser = TRUE;
      load_dialog( hwnd );
      nottheuser = FALSE;
      break;

    case WM_CLOSE:
      save_ini();
      WinDestroyWindow( hwnd );
      hdialog = NULLHANDLE;
      break;

    case WM_COMMAND:
      switch( SHORT1FROMMP( mp1 ))
      {
        case EQ_SAVE:
          save_eq( hwnd, bandgain[0], mute[0], preamp );
          break;

        case EQ_LOAD:
          load_eq( hwnd, bandgain[0], mute[0], &preamp );
          nottheuser = TRUE;
          load_dialog( hwnd );
          nottheuser = FALSE;
          eqneedEQ = TRUE;
          break;

        case EQ_ZERO:
        {
          int e,i;

          nottheuser = TRUE;
          memset(bandgain, 0, sizeof bandgain);
          memset(mute, 0, sizeof mute);
          for( e = 0; e < 2; e++ ) {
            for( i = 0; i < NUM_BANDS; i++ ) {
              set_slider( hwnd, e, i, 0. );
              WinSendDlgItemMsg( hwnd, 100+NUM_BANDS*e+i, BM_SETCHECK, 0, 0 );
            }
          }
          nottheuser = FALSE;
          
          eqneedEQ = TRUE;
          //*lasteq = 0; maybe it's better to keep the file...
          break;
        }
      }

    case WM_CONTROL:
    {
      int id = SHORT1FROMMP( mp1 );

      if( nottheuser ) {
        return 0;
      }

      switch( id )
      {
        ULONG temp;
        int i;

        case EQ_ENABLED:
          eqenabled = WinQueryButtonCheckstate( hwnd, id );
          break;

        case ID_LOCKLR:
          locklr = WinQueryButtonCheckstate( hwnd, id );
          break;

        case ID_FIRORDER:
          switch( SHORT2FROMMP( mp1 ))
          {
            case SPBN_CHANGE:
              WinSendDlgItemMsg( hwnd, id, SPBM_QUERYVALUE, MPFROMP( &temp ), 0 );
              if (temp > MAX_FIR || temp >= newPlansize)
                WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG( newFIRorder ), 0 ); // restore
               else
              { newFIRorder = temp;
                eqneedFIR = TRUE; // no init required when only the FIRorder changes
              }
              break;

            case SPBN_UPARROW:
              frexp(newFIRorder, &i); // floor(log2(FIRorder-1))+1
              i = 1 << (i-1);
              i >>= 2;
              temp = (newFIRorder & ~(MIN_FIR-1)) + max(MIN_FIR, i);
              if (temp < newPlansize)
                WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG( temp ), 0 );
              break;

            case SPBN_DOWNARROW:
              frexp(newFIRorder-1, &i); // floor(log2(FIRorder-1))+1
              i = 1 << (i-1);
              i >>= 2;
              temp = (newFIRorder & ~(MIN_FIR-1)) - max(MIN_FIR, i);
              if (temp >= 16)
                WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG( temp ), 0 );
              break;
          }
          break;

        case ID_PLANSIZE:
          switch( SHORT2FROMMP( mp1 ))
          {
            case SPBN_CHANGE:
              WinSendDlgItemMsg( hwnd, id, SPBM_QUERYVALUE, MPFROMP(&temp), 0 );
              if (temp > MAX_COEF || temp <= newFIRorder)
                WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG(newPlansize), 0 ); // restore
               else
              { newPlansize = temp;
                eqneedinit = TRUE;
              }
              break;

            case SPBN_UPARROW:
              temp = newPlansize << 1;
              if (temp > MAX_COEF)
                temp = MAX_COEF;
              WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG(temp), 0 );
              break;

            case SPBN_DOWNARROW:
              temp = newPlansize >> 1;
              if (temp <= newFIRorder)
              { int i;
                frexp(newFIRorder, &i);
                temp = 1 << i; // round up to next power of two
              }
              WinSendDlgItemMsg( hwnd, id, SPBM_SETCURRENTVALUE, MPFROMLONG(temp), 0 );
              break;
          }
          break;

        default:

          // mute check boxes
          if( id >= 100 && id < 200 ) {
            switch( SHORT2FROMMP( mp1 ))
            {
              case BN_CLICKED:
              case BN_DBLCLICKED:
              {
                int channel = 0, band = 0, raw = SHORT1FROMMP(mp1);

                raw -= 100;
                while( raw - NUM_BANDS >= 0 ) {
                  raw -= NUM_BANDS;
                  channel++;
                }
                band = raw;

                mute[channel][band] = SHORT1FROMMR( WinSendDlgItemMsg( hwnd, 100+NUM_BANDS*channel+band, BM_QUERYCHECK, 0, 0 ) );

                if( locklr )
                {
                  nottheuser = TRUE;
                  WinSendDlgItemMsg( hwnd, 100+NUM_BANDS*(channel^1)+band, BM_SETCHECK,
                                     MPFROMSHORT( mute[channel^1][band] = mute[channel][band] ), 0 );
                  nottheuser = FALSE;
                }

                eqneedEQ = TRUE;
              }
            }
          // sliders
          } else if( id >= 200 && id < 300 ) {
            switch( SHORT2FROMMP( mp1 ))
            {
              case SLN_CHANGE:
              {
                int channel = 0, band = 0, raw = SHORT1FROMMP(mp1);
                MRESULT rangevalue;

                raw -= 200;
                while( raw - NUM_BANDS >= 0 ) {
                  raw -= NUM_BANDS;
                  channel++;
                }
                band = raw;
                
                rangevalue = WinSendDlgItemMsg( hwnd, 200+NUM_BANDS*channel+band, SLM_QUERYSLIDERINFO,
                                                MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ), 0 );
                bandgain[channel][band] = (int)(24.*SHORT1FROMMR( rangevalue )/(SHORT2FROMMR( rangevalue ) - 1) +.5) - 12;

                if( locklr )
                {
                  nottheuser = TRUE;
                  set_slider( hwnd, channel^1, band, bandgain[channel^1][band] = bandgain[channel][band] ); 
                  nottheuser = FALSE;
                }
                eqneedEQ = TRUE;
              }

              case SLN_SLIDERTRACK:
                // copy the behavior of one channel slider to the other
                if( locklr )
                {
                  int channel = 0, band = 0, raw = SHORT1FROMMP(mp1);
                  MRESULT rangevalue;

                  raw -= 200;
                  while( raw - NUM_BANDS >= 0 ) {
                    raw -= NUM_BANDS;
                    channel++;
                  }
                  band = raw;
                  nottheuser = TRUE;
                  rangevalue = WinSendDlgItemMsg( hwnd, 200+NUM_BANDS*channel+band, SLM_QUERYSLIDERINFO,
                                  MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ), 0 );

                  WinSendDlgItemMsg( hwnd, 200+NUM_BANDS*(channel^1)+band, SLM_SETSLIDERINFO,
                                     MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                                     MPFROMSHORT( SHORT1FROMMR( rangevalue )));

                  nottheuser = FALSE;
                }
              }
              break;
            }
          }
          return 0;
      }
  }

  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

int PM123_ENTRY
plugin_configure( HWND hwnd, HMODULE module )
{
  init_request();

  if( !hdialog ) {
    hdialog = WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP, ConfigureDlgProc, module, ID_EQ, NULL );
    do_warpsans( hdialog );
  }

  WinShowWindow( hdialog, TRUE );
  WinSetFocus  ( HWND_DESKTOP, WinWindowFromID( hdialog, EQ_ENABLED ));
  return 0;
}

