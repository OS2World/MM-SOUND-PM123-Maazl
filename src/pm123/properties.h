/*
 * Copyright 1997-2003 Samuel Audet  <guardia@step.polymtl.ca>
 *                     Taneli Lepp�  <rosmo@sektori.com>
 * Copyright 2004 Dmitry A.Steklenev <glass@ptv.ru>
 * Copyright 2007-2008 Marcel Mueller
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

#ifndef PM123_PROPERTIES_H
#define PM123_PROPERTIES_H

#define INCL_WIN
#include <config.h>
#include <cpp/xstring.h>
#include <cpp/cpputil.h>
#include <stdlib.h>
#include <os2.h>

// Number of items in the recall lists.
#define MAX_RECALL            9

// read xstring
xstring ini_query_xstring(HINI hini, const char* app, const char* key);
// write xstring
inline BOOL ini_write_xstring(HINI hini, const char* app, const char* key, const xstring& str)
{ return PrfWriteProfileData(hini, app, key, (PVOID)str.cdata(), str ? str.length() : 0);
}

#define load_ini_xstring(hini, var) \
  var = ini_query_xstring((hini), INI_SECTION, #var)

#define save_ini_xstring(hini, var) \
  ini_write_xstring((hini), INI_SECTION, #var, var)


/* Possible sizes of the player window. */
enum cfg_mode
{ CFG_MODE_REGULAR,
  CFG_MODE_SMALL,
  CFG_MODE_TINY
};

/* Possible scroll modes. */
enum cfg_scroll
{ CFG_SCROLL_INFINITE,
  CFG_SCROLL_ONCE,
  CFG_SCROLL_NONE
};

/* Possible display modes. */
enum cfg_disp
{ CFG_DISP_FILENAME,
  CFG_DISP_ID3TAG,
  CFG_DISP_FILEINFO
};

// Alternate navigation methods
enum
{ CFG_ANAV_SONG,
  CFG_ANAV_SONGTIME,
  CFG_ANAV_TIME
};

typedef struct _amp_cfg {
  // TODO: buffers too small for URLs!!!
  char   defskin[_MAX_PATH];  // Default skin.
                                
  bool   playonload;          // Start playing on file load.
  bool   autouse;             // Auto use playlist on add.
  bool   retainonexit;        // Retain playing position on exit.
  bool   retainonstop;        // Retain playing position on stop.
  bool   restartonstart;      // Restart playing on startup.
  int    altnavig;            // Alternate navigation method 0=song only, 1=song&time, 2=time only
  bool   autoturnaround;      // Turn around at prev/next when at the end of a playlist
  bool   recurse_dnd;         // Drag and drop of folders recursive
  bool   sort_folders;        // Automatically sort filesystem folders (by name)
  bool   folders_first;       // Place subfolders before content
  bool   append_dnd;          // Drag and drop appends to default playlist
  bool   append_cmd;          // Commandline appends to default playlist
  bool   queue_mode;          // Delete played items from the default playlist
  int    num_workers;         // Number of worker threads for Playable objects
  int    num_dlg_workers;     // Number of dialog (high priority) worker threads for Playable objects
                                
  int    font;                // Use font 1 or font 2.
  bool   font_skinned;        // Use skinned font.
  FATTRS font_attrs;          // Font's attributes.
  LONG   font_size;           // Font's point size.
                                
  bool   floatontop;          // Float on top.
  int    scroll;              // See CFG_SCROLL_*
  bool   scroll_around;       // Scroller turns around the text instead of scrolling backwards.       
  int    viewmode;            // See CFG_DISP_*
  char   proxy[1024];         // Proxy URL.
  char   auth [1024];         // HTTP authorization.
  int    buff_wait;           // Wait before playing.
  int    buff_size;           // Read ahead buffer size (KB).
  int    buff_fill;           // Percent of prefilling of the buffer.
  int    conn_timeout;        // Connection timeout.
  char   pipe_name[_MAX_PATH];// PM123 remote control pipe name
  bool   dock_windows;        // Dock windows?
  int    dock_margin;         // The marging for docking window.
  bool   win_pos_by_obj;      // Store object specific window position.
  int    win_pos_max_age;     // Maximum age of window positions in days.
  
  int    insp_autorefresh;    // Autorefresh rate of inspector dialog.
  bool   insp_autorefresh_on; // Autorefresh rate of inspector dialog.

// Player state
  char   filedir[_MAX_PATH];  /* The last directory used for addition of files.    */
  char   listdir[_MAX_PATH];  /* The last directory used for access to a playlist. */
  char   savedir[_MAX_PATH];  /* The last directory used for saving a stream.      */

  int    mode;                /* See CFG_MODE_*                         */

  bool   show_playlist;       /* Show playlist.                         */
  bool   show_bmarks;         /* Show bookmarks.                        */
  bool   show_plman;          /* Show playlist manager.                 */
  bool   add_recursive;       /* Enable recursive addition.             */
  bool   save_relative;       /* Use relative paths in saved playlists. */
  SWP    main;                /* Position of the player.                */

} amp_cfg;

extern amp_cfg cfg;
extern const amp_cfg cfg_default;

extern const HINI& amp_hini;

void load_ini();
void save_ini();

/* Saves the current size and position of the window. */
BOOL save_window_pos( HWND, const char* extkey = NULL );
/* Restores the current size and position of the window. */
BOOL rest_window_pos( HWND, const char* extkey = NULL );

/* Initialize properties, called from main. */
void cfg_init();
/* Deinitialize properties, called from main. */
void cfg_uninit();

/* Creates the properties dialog. */
void cfg_properties( HWND owner );

#endif /* PM123_PROPERTIES_H */

