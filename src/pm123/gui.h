/*
 * Copyright 2007-2010 M.Mueller
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

#ifndef GUI_H
#define GUI_H

#define INCL_WIN
#include "controller.h"
#include "songiterator.h"
#include "playable.h"
#include "pm123.rc.h"
#include <os2.h>

#include <cpp/smartptr.h>
#include <cpp/event.h>
#include <cpp/cpputil.h>


class Module;
class LoadHelper;
class PlaylistMenu;
struct CfgChangeArgs;
class PluginEventArgs;

class GUI
{public:
  // Opens dialog for the specified object.
  enum DialogType
  { DLT_INFOEDIT,
    DLT_METAINFO,
    DLT_TECHINFO,
    DLT_PLAYLIST,
    DLT_PLAYLISTTREE
  };

 protected:
  enum WMUser                             // MP1               MP2
  { WMP_REFRESH_CONTROLS = WM_USER + 1000 // 0                 0
  , WMP_PAINT                             // mask              0
  , WMP_LOAD                              // LoadHelper*
  , WMP_RELOADSKIN                        // 0                 0
  , WMP_LOAD_VISUAL                       // int_ptr<Visual>   TRUE
  , WMP_DISPLAY_MESSAGE                   // message           TRUE (info) or FALSE (error)
  , WMP_DISPLAY_MODE                      // 0                 0
  , WMP_QUERY_STRING                      // buffer            size and type
  , WMP_SHOW_DIALOG                       // int_ptr<Playable> DialogType
  , WMP_PLAYABLE_EVENT                    // APlayable*        Changed, Loaded
  , WMP_CTRL_EVENT                        // EventFlags        0
  , WMP_CTRL_EVENT_CB                     // ControlCommand*   0
  , WMP_REFRESH_ACCEL
  , WMP_SLIDERDRAG                        // pos(x,y),         TRUE: navigate and complete
  , WMP_ARRANGEDOCKING                    //
  };

 protected: // Internal playlist objects
  static int_ptr<Playable> DefaultPL;     // Default playlist, representing PM123.LST in the program folder.
  static int_ptr<Playable> DefaultPM;     // PlaylistManager window, representing PFREQ.LST in the program folder.
  static int_ptr<Playable> DefaultBM;     // Default instance of bookmark window, representing BOOKMARK.LST in the program folder.
  static int_ptr<Playable> LoadMRU;       // Most recent used entries in the load menu, representing LOADMRU.LST in the program folder.
  static int_ptr<Playable> UrlMRU;        // Most recent used entries in the load URL dialog, representing URLMRU.LST in the program folder.

 protected: // Working set
  static HWND              HFrame;        // Frame window
  static HWND              HPlayer;       // Player window
  static HWND              HHelp;         // Help instance
  static SongIterator      IterBuffer[2]; // Two SongIterators. CurrentIter points to one of them.
  static SongIterator*     CurrentIter;   // current SongIterator. NOT NULL!

 public: // Utility functions
  static HWND      GetFrameWindow()       { return HFrame; }
  static HWND      GetHelpMgr()           { return HHelp; }
  static Playable& GetDefaultPL()         { return *DefaultPL; }
  static Playable& GetDefaultPM()         { return *DefaultPM; }
  static Playable& GetDefaultBM()         { return *DefaultBM; }
  static Playable& GetLoadMRU()           { return *LoadMRU; }
  static Playable& GetUrlMRU()            { return *UrlMRU; }

  // Update an MRU list with item ps and maximum size max
  static void      Add2MRU(Playable& list, size_t max, APlayable& ps);

 public: // Manipulating interface   
  static void      PostMessage(MESSAGE_TYPE type, xstring text);
  // Tells the help manager to display a specific help window.
  // TODO: should move to dialog.cpp
  static bool      ShowHelp(SHORT resid)  { DEBUGLOG(("ShowHelp(%u)\n", resid));
                                            return WinSendMsg(HHelp, HM_DISPLAY_HELP, MPFROMSHORT(resid), MPFROMSHORT(HM_RESOURCEID)) == 0; }
  // Opens dialog for the specified object.
  static void      ShowDialog(Playable& item, DialogType dlg);
  static void      ShowConfig()           { PMRASSERT(WinPostMsg(HPlayer, WM_COMMAND, MPFROMSHORT(IDM_M_CFG), 0)); }
  static void      ShowConfig(Module& plugin);
  
  static void      Show(bool visible = true);
  static void      Quit()                 { WinSendMsg(HPlayer, WM_COMMAND, MPFROMSHORT(BMP_POWER), 0); }

  // Load objects into the player.
  // Attention!!! Load takes the exclusive ownership of lhp and deletes it afterwards.
  static void      Load(LoadHelper* lhp)  { WinPostMsg(HFrame, WMP_LOAD, MPFROMP(lhp), 0); }

 public: // Initialization interface
  static void      Init();
  static void      Uninit();  
};

#include "plugman.h"

inline void GUI::ShowConfig(Module& plugin)
{ plugin.Config(HFrame);
} 


#endif /* GUI_H */

