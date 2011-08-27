#ifndef PM123_OUTPUT_PLUG_H
#define PM123_OUTPUT_PLUG_H

#define INCL_BASE
#include <format.h>
#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

// forward declaration
struct _DECODER_INFO;
struct _DECODER_INFO2;

ULONG DLLENTRY output_init  (void** a);
ULONG DLLENTRY output_uninit(void*  a);

typedef enum
{ OUTPUT_OPEN          = 1,
  OUTPUT_CLOSE         = 2,
  OUTPUT_VOLUME        = 3,
  OUTPUT_PAUSE         = 4,
  OUTPUT_SETUP         = 5,
  OUTPUT_TRASH_BUFFERS = 6,
  OUTPUT_NOBUFFERMODE  = 7 /* obsolete */
} OUTMSGTYPE;

#define OUTPUT_SIZE_1 76  /* size of the OUTPUT_PARAMS structure prior PM123 1.32 */
#define OUTPUT_SIZE_2 80  /* size of the OUTPUT_PARAMS structure since PM123 1.32 */

typedef struct _OUTPUT_PARAMS
{
  /* --- see OUTPUT_SIZE definitions */
  unsigned int size;

  /* --- OUTPUT_SETUP */
  FORMAT_INFO  formatinfo;

  int          buffersize;

  unsigned short boostclass, normalclass;
  signed   short boostdelta, normaldelta;

  void DLLENTRYP(error_display)(const char*);

  /* info message function the output plug-in should use */
  /* this information is always displayed to the user right away */
  void DLLENTRYP(info_display)(const char*);

  HWND hwnd; /* commodity for PM interface, sends a few messages to this handle. */

  /* --- OUTPUT_VOLUME */
  unsigned char volume;
  float amplifier;

  /* --- OUTPUT_PAUSE */
  BOOL  pause;

  /* --- OUTPUT_NOBUFFERMODE */
  BOOL  nobuffermode;

  /* --- OUTPUT_TRASH_BUFFERS */
  ULONG temp_playingpos;  /* used until new buffers come in. */

  /* --- OUTPUT_OPEN */
  const char* filename;   /* filename, URL or track now being played,
                             useful for disk output */

  /* --- OUTPUT_SETUP ***OUTPUT*** */
  BOOL  always_hungry;    /* the output plug-in imposes no time restraint
                             it is always WM_OUTPUT_OUTOFDATA */

  /* --- OUTPUT_SETUP */
  const struct _DECODER_INFO* info;     /* added since PM123 1.32 */

} OUTPUT_PARAMS;

typedef enum
{ OUTEVENT_END_OF_DATA,   // The flush signal is passed to the ouput plugin and the last sample has been played
  OUTEVENT_LOW_WATER,     // The buffers of the output plug-in are getting low. Try to speed up the data source.
  OUTEVENT_HIGH_WATER,    // The buffers of the output plug-in are sufficiently filled.
  OUTEVENT_PLAY_ERROR     // The plug-in detected a fatal error stop immediately.
} OUTEVENTTYPE; 

typedef struct _OUTPUT_PARAMS2
{
  /* callback event */
  void DLLENTRYP(OutEvent)(void* w, OUTEVENTTYPE event);
  void* W;  /* only to be used with the precedent function */

  /* --- OUTPUT_VOLUME */
  float Volume;           // [0...1]
  float Amplifier;

  /* --- OUTPUT_PAUSE */
  BOOL  Pause;

  /* --- OUTPUT_TRASH_BUFFERS and OUTPUT_OPEN and OUTPUT_CLOSE */
  PM123_TIME  PlayingPos; // related playing position

  /* --- OUTPUT_SETUP and OUTPUT_OPEN */
  xstring URL;            // filename, URL or track now being played,
                          // useful for disk output
                          
  const INFO_BUNDLE_CV* Info;// Information on the object to play.

} OUTPUT_PARAMS2;

#if !defined(PLUGIN_INTERFACE_LEVEL) || PLUGIN_INTERFACE_LEVEL <= 1
ULONG  DLLENTRY output_command(void* a, ULONG msg, OUTPUT_PARAMS* info);
int    DLLENTRY output_play_samples(void* a, FORMAT_INFO* format, char* buf, int len, int posmarker);
ULONG  DLLENTRY output_playing_pos(void* a);
#else
ULONG  DLLENTRY output_command(void* a, OUTMSGTYPE msg, const OUTPUT_PARAMS2* info);
int    DLLENTRY output_request_buffer(void* a, const TECH_INFO* format, short** buf);
void   DLLENTRY output_commit_buffer(void* a, int len, PM123_TIME posmarker);
PM123_TIME DLLENTRY output_playing_pos(void* a);
#endif
ULONG  DLLENTRY output_playing_samples(void* a, FORMAT_INFO* info, char* buf, int len);
BOOL   DLLENTRY output_playing_data(void* a);

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif /* PM123_OUTPUT_PLUG_H */

