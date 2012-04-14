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

#ifndef PM123_CONFIGURATION_H
#define PM123_CONFIGURATION_H

#define INCL_WIN
#include <config.h>
#include <plugin.h>
#include <cpp/xstring.h>
#include <cpp/cpputil.h>
#include <cpp/mutex.h>
#include <cpp/event.h>
#include <cpp/container/vector.h>
#include <stdlib.h>
#include <os2.h>


// read xstring
const bool ini_query_xstring(HINI hini, const char* app, const char* key, xstring& dst);
// write xstring
inline BOOL ini_write_xstring(HINI hini, const char* app, const char* key, const xstring& str)
{ return PrfWriteProfileData(hini, app, key, (PVOID)str.cdata(), str ? str.length() : 0);
}

#define load_ini_xstring(hini, var) \
  ini_query_xstring((hini), INI_SECTION, #var, var)

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
enum cfg_anav
{ CFG_ANAV_SONG,
  CFG_ANAV_SONGTIME,
  CFG_ANAV_TIME
};

enum cfg_rgtype
{ CFG_RG_NONE,
  CFG_RG_ALBUM,
  CFG_RG_ALBUM_NO_CLIP,
  CFG_RG_TRACK,
  CFG_RG_TRACK_NO_CLIP
};

struct amp_cfg
{ xstring defskin;            // Default skin.

  bool    playonload;         // Start playing on file load.
  bool    autouse;            // Auto use playlist on add.
  bool    retainonexit;       // Retain playing position on exit.
  bool    retainonstop;       // Retain playing position on stop.
  bool    restartonstart;     // Restart playing on startup.
  cfg_anav altnavig;          // Alternate navigation method 0=song only, 1=song&time, 2=time only
  bool    autoturnaround;     // Turn around at prev/next when at the end of a playlist
  bool    recurse_dnd;        // Drag and drop of folders recursive
  bool    folders_first;      // Place subfolders before content
  bool    append_dnd;         // Drag and drop appends to default playlist
  bool    append_cmd;         // Commandline appends to default playlist
  bool    queue_mode;         // Delete played items from the default playlist

  bool    replay_gain;        // Enable replay gain processing
  cfg_rgtype rg_list[4];      // Type of replay gain processing
  int     rg_preamp;          // additional gain for tracks with replaygain information [dB]
  int     rg_preamp_other;    // additional gain for tracks without replaygain information [dB]
  int     pri_normal;         // Normal decoder priority
  int     pri_high;           // High decoder priority
  int     pri_limit;          // Maximum high priority seconds

  int     num_workers;        // Number of worker threads for Playable objects
  int     num_dlg_workers;    // Number of dialog (high priority) worker threads for Playable objects

  int     font;               // Use font 1 or font 2.
  bool    font_skinned;       // Use skinned font.
  FATTRS  font_attrs;         // Font's attributes.
  unsigned font_size;         // Font's point size.

  bool    floatontop;         // Float on top.
  cfg_scroll scroll;          // See CFG_SCROLL_*
  bool    scroll_around;      // Scroller turns around the text instead of scrolling backwards.
  cfg_disp viewmode;          // See CFG_DISP_*
  int     max_recall;         // Number of items in the recall lists
  xstring proxy;              // Proxy URL
  xstring auth;               // HTTP authorization
  int     buff_wait;          // Wait before playing
  int     buff_size;          // Read ahead buffer size (KB)
  int     buff_fill;          // Percent of prefilling of the buffer
  int     conn_timeout;       // Connection timeout
  xstring pipe_name;          // PM123 remote control pipe name
  bool    dock_windows;       // Dock windows?
  int     dock_margin;        // The margin for docking window
  bool    win_pos_by_obj;     // Store object specific window position.
  int     win_pos_max_age;    // Maximum age of window positions in days

  int     insp_autorefresh;   // Auto refresh rate of inspector dialog
  bool    insp_autorefresh_on;// Auto refresh rate of inspector dialog

  // plug-in lists, not necessarily up to date, not changeable this way
  xstring decoders_list;      // Serialized decoders
  xstring filters_list;       // Serialized filters
  xstring outputs_list;       // Serialized outputs
  xstring visuals_list;       // Serialized visual plug-ins

  // Player state
  xstring filedir;            // The last directory used for addition of files.
  xstring listdir;            // The last directory used for access to a playlist.
  xstring savedir;            // The last directory used for saving a stream.

  cfg_mode mode;              // See CFG_MODE_*

  bool    show_playlist;      // Show playlist
  bool    show_bmarks;        // Show bookmarks
  bool    show_plman;         // Show playlist manager
  bool    add_recursive;      // Enable recursive addition
  bool    save_relative;      // Use relative paths in saved playlists
};

struct CfgValidateArgs
{       amp_cfg& New;
  const amp_cfg& Old;
        xstring  Message;
        bool     Fail;
  CfgValidateArgs(amp_cfg& n, const amp_cfg& o) : New(n), Old(o), Fail(false) {}
};

struct CfgChangeArgs
{ const amp_cfg& New;
  const amp_cfg& Old;
  CfgChangeArgs(const amp_cfg& n, const amp_cfg& o) : New(n), Old(o) {}
};

class Cfg
{private:
  static Mutex                Mtx;
  static amp_cfg              Current;
  static event<CfgValidateArgs> Validate;
  static event<const CfgChangeArgs> Change;
  static HINI                 HIni;
 public:
  static const amp_cfg        Default;

 private:
  Cfg(); // static class
  /// Helper to LoadIni. Loads the plug-in type \a type from the ini key \a key.
  static void LoadPlugins(const char* key, xstring amp_cfg::*cfg, PLUGIN_TYPE type);
  /// Helper to SaveIni. Saves the list of plug-ins of type \a type to the ini key \a key.
  static void SavePlugins(const char* key, xstring amp_cfg::*cfg, PLUGIN_TYPE type);

  /// Load configuration from ini file.
  static void LoadIni();
  /// migrate plug-in configuration
  static void MigrateIni(const char* inipath, const char* app);
  /// Purge outdated ini locations in the profile
  static void CleanIniPositions();
 public:
  /// Initialize properties, called from main.
  static void Init();
  /// Deinitialize properties, called from main.
  static void Uninit();

  /// Save configuration to disk.
  static void SaveIni();

  /// Read-only unsynchronized access to the current configuration.
  /// @remarks Note that the values are always subject to change
  /// unless you hold the mutex \c Mtx.
  static volatile const amp_cfg& Get()         { return Current; }
  /// Change the configuration.
  /// @remarks It is recommended to call this function from synchronized context.
  static bool Set(amp_cfg& settings);

  /// Configuration change event, fires when the configuration changes (call to \c Set).
  static event_pub<CfgValidateArgs>& GetValidate()   { return Validate; }
  /// Configuration change event, fires when the configuration changes (call to \c Set).
  static event_pub<const CfgChangeArgs>& GetChange() { return Change; }

  /// INI file handle
  static HINI GetHIni()                        { return HIni; }

  /// Saves the current size and position of the window specified by \a hwnd.
  /// This function will also save the presentation parameters.
  static bool SaveWindowPos(HWND hwnd, const char* extkey = NULL);
  /// Restores the size and position of the window specified by \a hwnd to
  /// the state it was in when save_window_pos was last called.
  /// This function will also restore presentation parameters.
  static bool RestWindowPos(HWND hwnd, const char* extkey = NULL);

 public:
  struct Access;
  friend class Access;
  /// Synchronized access to the configuration
  struct Access
  { Access()                                   { Cfg::Mtx.Request(); }
    ~Access()                                  { Cfg::Mtx.Release(); }
    const amp_cfg& operator*()                 { return Cfg::Current; }
    const amp_cfg* operator->()                { return &Cfg::Current; }
  };
  struct ChangeAccess : private Access, public amp_cfg
  { ChangeAccess()                             : amp_cfg(**this) {}
    ~ChangeAccess()                            { Cfg::Set(*this); }
  };
};


#endif /* PM123_PROPERTIES_H */
