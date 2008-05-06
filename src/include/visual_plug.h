#ifndef PM123_VISUAL_PLUG_H
#define PM123_VISUAL_PLUG_H

#include <format.h>
#include <decoder_plug.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

/* see decoder_plug.h and output_plug.h for more information
   on some of these functions */
typedef struct _PLUGIN_PROCS
{
  ULONG  DLLENTRYP(output_playing_samples)( FORMAT_INFO* info, char* buf, int len );
  BOOL   DLLENTRYP(decoder_playing)( void );
  double DLLENTRYP(output_playing_pos)( void );
  ULONG  DLLENTRYP(decoder_status)( void );
  /* name is the DLL filename of the decoder that can play that file */
  ULONG  DLLENTRYP(decoder_fileinfo)( const char* URL, DECODER_INFO2* info, char* name );

  int    DLLENTRYP(pm123_getstring)( int index, int subindex, size_t bufsize, char* buf );
  void   DLLENTRYP(pm123_control)( int index, void* param );

  /* name is the DLL filename of the decoder that can play that track */
  ULONG  DLLENTRYP(decoder_cdinfo)( const char* drive, DECODER_CDINFO* info );
  double DLLENTRYP(decoder_length)( void );

} PLUGIN_PROCS, *PPLUGIN_PROCS;

/*
 * int pm123_getstring( int index, int subindex, int bufsize, char* buf )
 *
 *  index    - which string (see STR_* defines below)
 *  subindex - not currently used
 *  bufsize  - bytes in buf
 *  buf      - buffer for the string
 */

#define STR_NULL         0
#define STR_VERSION      1 /* PM123 version          */
#define STR_DISPLAY_TEXT 2 /* Displayed text         */
#define STR_FILENAME     3 /* Currently loaded file  */
#define STR_DISPLAY_TAG  4 /* Displayed song info    */
#define STR_DISPLAY_INFO 5 /* Displayed tech info    */

/*
 * int pm123_control( int index, void* param );
 *
 *  index - operation
 *  param - parameter for the operation
 */

#define CONTROL_NEXTMODE 1 /* Next display mode */

typedef struct _VISPLUGININIT
{
  int           x, y, cx, cy;   /* Input        */
  HWND          hwnd;           /* Input/Output */
  PPLUGIN_PROCS procs;          /* Input        */
  int           id;             /* Input        */
  char*         param;          /* Input        */
  HAB           hab;            /* Input        */

} VISPLUGININIT, *PVISPLUGININIT;

HWND DLLENTRY vis_init( PVISPLUGININIT initdata );

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif /* PM123_VISUAL_PLUG_H */