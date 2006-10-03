/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 *
 * Copyright 2004-2005 Dmitry A.Steklenev <glass@ptv.ru>
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

#define  INCL_WIN
#define  INCL_GPI
#define  INCL_DOS
#define  INCL_OS2MM
#include <os2.h>
#include <os2me.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include <utilfct.h>

#include "pm123.h"
#include "plugman.h"
#include "httpget.h"

/* Reads ID3 tag from the specified file. */
BOOL
amp_gettag( const char* filename, const META_INFO* info, tune* tag )
{
  int  handle;
  BOOL rc = FALSE;

  emptytag(tag);

  if( filename != NULL && *filename && is_file( filename )) {
    handle = open( filename, O_RDONLY | O_BINARY );
    if( handle != -1 ) {
      rc = gettag( handle, tag );
      close( handle );
    }
  }

  if( !rc && info ) {
    strcpy( tag->title,   info->title   );
    strcpy( tag->artist,  info->artist  );
    strcpy( tag->album,   info->album   );
    strcpy( tag->year,    info->year    );
    strcpy( tag->comment, info->comment );
    strcpy( tag->genre,   info->genre   );
    rc = TRUE;
  }

  if( rc && cfg.auto_codepage )
  {
    // autodetect codepage
    char cstr[ sizeof( tag->title   ) +
               sizeof( tag->artist  ) +
               sizeof( tag->album   ) +
               sizeof( tag->comment ) ];
    strcpy( cstr, tag->title   );
    strcat( cstr, tag->artist  );
    strcat( cstr, tag->album   );
    strcat( cstr, tag->comment );

    tag->codepage = ch_detect( cfg.codepage, cstr );
  }
  
  if ( rc && tag->codepage != CH_CP_NONE )
  {
    tune stag = *tag;
    // replace values in *tag by codepage converted values
    ch_convert( tag->codepage, stag.title,   CH_CP_NONE, tag->title,   sizeof( stag.title   ));
    ch_convert( tag->codepage, stag.artist,  CH_CP_NONE, tag->artist,  sizeof( stag.artist  ));
    ch_convert( tag->codepage, stag.album,   CH_CP_NONE, tag->album,   sizeof( stag.album   ));
    ch_convert( tag->codepage, stag.comment, CH_CP_NONE, tag->comment, sizeof( stag.comment ));
  }

  return rc;
}

/* Wipes ID3 tag from the specified file. */
BOOL
amp_wipetag( const char* filename )
{
  int  handle;
  BOOL rc = FALSE;

  handle = open( filename, O_RDWR | O_BINARY );
  if( handle != -1 ) {
    rc = wipetag( handle );
    close( handle );
  }

  return rc;
}

/* Writes ID3 tag to the specified file. */
BOOL
amp_puttag( const char* filename, const tune* tag )
{
  int  handle;
  BOOL rc = 0;
  tune wtag = *tag;

  // if the codepage is not yet specified use the global configuration setting by default
  if( wtag.codepage == CH_CP_NONE ) {
    wtag.codepage = cfg.codepage;
  }
  
  ch_convert( CH_CP_NONE, tag->title,   tag->codepage, wtag.title,   sizeof( wtag.title   ));
  ch_convert( CH_CP_NONE, tag->artist,  tag->codepage, wtag.artist,  sizeof( wtag.artist  ));
  ch_convert( CH_CP_NONE, tag->album,   tag->codepage, wtag.album,   sizeof( wtag.album   ));
  ch_convert( CH_CP_NONE, tag->comment, tag->codepage, wtag.comment, sizeof( wtag.comment ));

  handle = open( filename, O_RDWR | O_BINARY );
  if( handle != -1 ) {
    rc = settag( handle, &wtag );
    close( handle );
  }

  return rc;
}

/* Constructs a string of the displayable text from the ID3 tag. */
char*
amp_construct_tag_string( char* result, const META_INFO* tag )
{
  *result = 0;

  if( *tag->artist ) {
    strcat( result, tag->artist );
    if( *tag->title ) {
      strcat( result, ": " );
    }
  }

  if( *tag->title ) {
    strcat( result, tag->title );
  }

  if( *tag->album && *tag->year )
  {
    strcat( result, " (" );
    strcat( result, tag->album );
    strcat( result, ", " );
    strcat( result, tag->year  );
    strcat( result, ")" );
  }
  else
  {
    if( *tag->album && !*tag->year )
    {
      strcat( result, " (" );
      strcat( result, tag->album );
      strcat( result, ")" );
    }
    if( !*tag->album && *tag->year )
    {
      strcat( result, " (" );
      strcat( result, tag->year);
      strcat( result, ")" );
    }
  }

  if( *tag->comment )
  {
    strlcat( result, " -- ", sizeof( result ));
    strlcat( result, tag->comment, sizeof( result ));
  }

  return result;
}

/* Converts time to two integer suitable for display by the timer. */
void
sec2num( long seconds, int* major, int* minor )
{
  int mi = seconds % 60;
  int ma = seconds / 60;

  if( ma > 99 ) {
    mi = ma % 60; // minutes
    ma = ma / 60; // hours
  }

  if( ma > 99 ) {
    mi = ma % 24; // hours
    ma = ma / 24; // days
  }

  *major = ma;
  *minor = mi;
}

ULONG handle_dfi_error(ULONG rc, const char *file)
{
 char buf[256];

 if (rc == 0) return 0;

 *buf = '\0';

 if (rc == 100)
   sprintf(buf, "The file %s could not be read.", file);

 if (rc == 200)
   sprintf(buf, "The file %s cannot be played by PM123. The file might be corrupted or the necessary plug-in not loaded or enabled.", file);

 if (rc == 1001)
  {
   amp_stop(); /* baa */
   sprintf(buf, "%s: HTTP error occurred: %s", file, http_strerror());
  }

 if (strcmp(file, "") == 0)
  {
   sprintf(buf, "%s: Error occurred: %s", file, strerror(errno));
  }

 WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, buf, "Error", 0, MB_ERROR | MB_OK);
 return 1;
}

void PM123_ENTRY pm123_control(int index, void *param)
{
 switch (index)
  {
   case CONTROL_NEXTMODE:
    amp_display_next_mode();
    break;
  }
}

int PM123_ENTRY pm123_getstring(int index, int subindex, size_t bufsize, char *buf)
{
 switch (index)
  {
   case STR_NULL: *buf = '\0'; break;
   case STR_VERSION:
     strlcpy( buf, AMP_FULLNAME, bufsize );
     break;
   case STR_DISPLAY_TEXT:
     strlcpy( buf, bmp_query_text(), bufsize );
     break;
   case STR_FILENAME:
   { const MSG_PLAY_STRUCT* current = amp_get_playing_file();
     strlcpy(buf, current != NULL ? current->url : "", bufsize);
     break;
   }
   default: break;
  }
 return(0);
}


