/*
 * Copyright 1997-2003 Samuel Audet  <guardia@step.polymtl.ca>
 *                     Taneli Lepp�  <rosmo@sektori.com>
 *
 * Copyright 2004 Dmitry A.Steklenev <glass@ptv.ru>
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
#define  INCL_DOS
#define  INCL_ERRORS

#include <utilfct.h>
#include "pm123.h"
#include "dialog.h" // for global eq vars...
#include "iniman.h"
#include "plugman.h"
#include "properties.h"
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <debuglog.h>

// support xstring
xstring ini_query_xstring(HINI hini, const char* app, const char* key)
{ xstring ret; 
  ULONG len;
  if (PrfQueryProfileSize(hini, app, key, &len))
  { char* data = ret.raw_init(len);
    if (!PrfQueryProfileData(hini, app, key, data, &len))
      ret = NULL;
  }
  return ret;
}

struct ext_pos
{ POINTL pos[2];
  time_t tstmp; // Time stamp when the information has been saved.
};

// Purge outdated ini locations in the profile
static void clean_ini_positions(HINI hini)
{ ULONG size;
  if (!PrfQueryProfileSize(hini, "Positions", NULL, &size))
    return;
  char* names = new char[size+2];
  names[size] = names[size+1] = 0; // ensure termination
  PrfQueryProfileData(hini, "Positions", NULL, names, &size);
  const time_t limit = time(NULL) - cfg.win_pos_max_age * 86400;
  for (char* cp = names; *cp; cp += strlen(cp)+1)
  { if (!memcmp(cp, "POS_", 4))
      continue;
    ext_pos pos;
    pos.tstmp = 0;
    size = sizeof(pos);
    if (PrfQueryProfileData(hini, "Positions", cp, &pos, &size) && pos.tstmp < limit)
    { // Purge this entry
      PrfWriteProfileData(hini, "Positions", cp, NULL, 0);
      memcpy(cp, "WIN", 3);
      PrfWriteProfileData(hini, "Positions", cp, NULL, 0);
    } 
  }
  delete names;
}

void
load_ini( void )
{
  HINI INIhandle;
  xstring tmp;

  cfg = cfg_default;

  if(( INIhandle = open_module_ini()) != NULLHANDLE )
  {
    load_ini_bool ( INIhandle, cfg.playonload );
    load_ini_bool ( INIhandle, cfg.autouse );
    load_ini_bool ( INIhandle, cfg.retainonexit );
    load_ini_bool ( INIhandle, cfg.retainonstop );
    load_ini_bool ( INIhandle, cfg.restartonstart );
    load_ini_value( INIhandle, cfg.altnavig );
    load_ini_bool ( INIhandle, cfg.autoturnaround );
    load_ini_bool ( INIhandle, cfg.recurse_dnd );
    load_ini_bool ( INIhandle, cfg.sort_folders );
    load_ini_bool ( INIhandle, cfg.folders_first );
    load_ini_bool ( INIhandle, cfg.append_dnd );
    load_ini_bool ( INIhandle, cfg.append_cmd );
    load_ini_bool ( INIhandle, cfg.queue_mode );
    load_ini_value( INIhandle, cfg.num_workers );
    load_ini_value( INIhandle, cfg.num_dlg_workers );
    load_ini_value( INIhandle, cfg.mode );
    load_ini_value( INIhandle, cfg.font );
    load_ini_bool ( INIhandle, cfg.floatontop );
    load_ini_value( INIhandle, cfg.scroll );
    load_ini_value( INIhandle, cfg.viewmode );
    load_ini_value( INIhandle, cfg.buff_wait );
    load_ini_value( INIhandle, cfg.buff_size );
    load_ini_value( INIhandle, cfg.buff_fill );
    load_ini_value( INIhandle, cfg.conn_timeout );
    load_ini_string( INIhandle, cfg.pipe_name, sizeof cfg.pipe_name );
    load_ini_bool ( INIhandle, cfg.add_recursive );
    load_ini_bool ( INIhandle, cfg.save_relative );
    load_ini_bool ( INIhandle, cfg.show_playlist );
    load_ini_bool ( INIhandle, cfg.show_bmarks );
    load_ini_bool ( INIhandle, cfg.show_plman );
    load_ini_value( INIhandle, cfg.dock_margin );
    load_ini_bool ( INIhandle, cfg.dock_windows );
    load_ini_value( INIhandle, cfg.insp_autorefresh );
    load_ini_bool ( INIhandle, cfg.insp_autorefresh_on );
    load_ini_bool ( INIhandle, cfg.font_skinned );
    load_ini_value( INIhandle, cfg.font_attrs );
    load_ini_value( INIhandle, cfg.font_size );
    load_ini_value( INIhandle, cfg.main );

    load_ini_string( INIhandle, cfg.filedir,  sizeof( cfg.filedir ));
    load_ini_string( INIhandle, cfg.listdir,  sizeof( cfg.listdir ));
    load_ini_string( INIhandle, cfg.savedir,  sizeof( cfg.savedir ));
    load_ini_string( INIhandle, cfg.proxy,    sizeof( cfg.proxy ));
    load_ini_string( INIhandle, cfg.auth,     sizeof( cfg.auth ));
    load_ini_string( INIhandle, cfg.defskin,  sizeof( cfg.defskin ));

    tmp = ini_query_xstring(INIhandle, INI_SECTION, "decoders_list");
    if (!tmp || Decoders.Deserialize(tmp) == PluginList::RC_Error)
      Decoders.LoadDefaults();

    tmp = ini_query_xstring(INIhandle, INI_SECTION, "outputs_list");
    if (!tmp || Outputs.Deserialize(tmp) == PluginList::RC_Error)
      Outputs.LoadDefaults();

    tmp = ini_query_xstring(INIhandle, INI_SECTION, "filters_list");
    if (!tmp || Filters.Deserialize(tmp) == PluginList::RC_Error)
      Filters.LoadDefaults();

    tmp = ini_query_xstring(INIhandle, INI_SECTION, "visuals_list");
    if (!tmp || Visuals.Deserialize(tmp) == PluginList::RC_Error)
      Visuals.LoadDefaults();

    close_ini( INIhandle );
  }
}

void
save_ini( void )
{
  HINI INIhandle;

  if(( INIhandle = open_module_ini()) != NULLHANDLE )
  {
    save_ini_bool ( INIhandle, cfg.playonload );
    save_ini_bool ( INIhandle, cfg.autouse );
    save_ini_bool ( INIhandle, cfg.retainonexit );
    save_ini_bool ( INIhandle, cfg.retainonstop );
    save_ini_bool ( INIhandle, cfg.restartonstart );
    save_ini_value( INIhandle, cfg.altnavig );
    save_ini_bool ( INIhandle, cfg.autoturnaround );
    save_ini_bool ( INIhandle, cfg.recurse_dnd );
    save_ini_bool ( INIhandle, cfg.sort_folders );
    save_ini_bool ( INIhandle, cfg.folders_first );
    save_ini_bool ( INIhandle, cfg.append_dnd );
    save_ini_bool ( INIhandle, cfg.append_cmd );
    save_ini_bool ( INIhandle, cfg.queue_mode );
    save_ini_value( INIhandle, cfg.num_workers );
    save_ini_value( INIhandle, cfg.num_dlg_workers );
    save_ini_value( INIhandle, cfg.mode );
    save_ini_value( INIhandle, cfg.font );
    save_ini_bool ( INIhandle, cfg.floatontop );
    save_ini_value( INIhandle, cfg.scroll );
    save_ini_value( INIhandle, cfg.viewmode );
    save_ini_value( INIhandle, cfg.buff_wait );
    save_ini_value( INIhandle, cfg.buff_size );
    save_ini_value( INIhandle, cfg.buff_fill );
    save_ini_value( INIhandle, cfg.conn_timeout );
    save_ini_string( INIhandle, cfg.pipe_name );
    save_ini_bool ( INIhandle, cfg.add_recursive );
    save_ini_bool ( INIhandle, cfg.save_relative );
    save_ini_bool ( INIhandle, cfg.show_playlist );
    save_ini_bool ( INIhandle, cfg.show_bmarks );
    save_ini_bool ( INIhandle, cfg.show_plman );
    save_ini_bool ( INIhandle, cfg.dock_windows );
    save_ini_value( INIhandle, cfg.dock_margin );
    save_ini_value( INIhandle, cfg.insp_autorefresh );
    save_ini_bool ( INIhandle, cfg.insp_autorefresh_on );
    save_ini_bool ( INIhandle, cfg.font_skinned );
    save_ini_value( INIhandle, cfg.font_attrs );
    save_ini_value( INIhandle, cfg.font_size );
    save_ini_value( INIhandle, cfg.main );

    save_ini_string( INIhandle, cfg.filedir );
    save_ini_string( INIhandle, cfg.listdir );
    save_ini_string( INIhandle, cfg.savedir );
    save_ini_string( INIhandle, cfg.proxy );
    save_ini_string( INIhandle, cfg.auth );
    save_ini_string( INIhandle, cfg.defskin );

    ini_write_xstring(INIhandle, INI_SECTION, "decoders_list", Decoders.Serialize());
    ini_write_xstring(INIhandle, INI_SECTION, "outputs_list",  Outputs.Serialize());
    ini_write_xstring(INIhandle, INI_SECTION, "filters_list",  Filters.Serialize());
    ini_write_xstring(INIhandle, INI_SECTION, "visuals_list",  Visuals.Serialize());

    clean_ini_positions(INIhandle);

    close_ini(INIhandle);
  }
}

/* Copies the specified data from one profile to another. */
static BOOL
copy_ini_data( HINI ini_from, char* app_from, char* key_from,
               HINI ini_to,   char* app_to,   char* key_to )
{
  ULONG size;
  PVOID data;
  BOOL  rc = FALSE;

  if( PrfQueryProfileSize( ini_from, app_from, key_from, &size )) {
    data = malloc( size );
    if( data ) {
      if( PrfQueryProfileData( ini_from, app_from, key_from, data, &size )) {
        if( PrfWriteProfileData( ini_to, app_to, key_to, data, size )) {
          rc = TRUE;
        }
      }
      free( data );
    }
  }

  return rc;
}

/* Saves the current size and position of the window specified by hwnd.
   This function will also save the presentation parameters. */
BOOL
save_window_pos( HWND hwnd, const char* extkey )
{
  char   key1st[32];
  char   key2[16];
  char   key3[300];
  PPIB   ppib;
  PTIB   ptib;
  SHORT  id   = WinQueryWindowUShort( hwnd, QWS_ID );
  HINI   hini = open_module_ini();
  BOOL   rc   = FALSE;
  SWP    swp;
  ext_pos pos;

  DEBUGLOG(("save_window_pos(%p{%u}, %s)\n", hwnd, id, extkey ? extkey : "<null>" ));

  DosGetInfoBlocks( &ptib, &ppib );

  if( hini == NULLHANDLE )
    return FALSE;

  sprintf( key1st, "WIN_%08lX_%08lX", ppib->pib_ulpid, ptib->tib_ptib2->tib2_ultid );
  sprintf( key2, "WIN_%08X", id );
  if (extkey && cfg.win_pos_by_obj)
  { strcpy( key3, key2 );
    key3[12] = '_';
    strlcpy( key3+13, extkey, sizeof key3 -13 );
  } else
    *key3 = 0;

  if( !WinStoreWindowPos( "PM123", key1st, hwnd ))
  { close_ini( hini );
    return false;
  }
  
  rc = copy_ini_data( HINI_PROFILE, "PM123", key1st, hini, "Positions", key2 );
  if (*key3) 
    rc &= copy_ini_data( HINI_PROFILE, "PM123", key1st, hini, "Positions", key3 );
  PrfWriteProfileData( HINI_PROFILE, "PM123", key1st, NULL, 0 );

  if( rc && WinQueryWindowPos( hwnd, &swp )) {
    pos.pos[0].x = swp.x;
    pos.pos[0].y = swp.y;
    pos.pos[1].x = swp.x + swp.cx;
    pos.pos[1].y = swp.y + swp.cy;
    WinMapDlgPoints( hwnd, pos.pos, 2, FALSE );
    time(&pos.tstmp);
    memcpy(key2, "POS", 3);
    rc = PrfWriteProfileData( hini, "Positions", key2, &pos, sizeof( pos ));
    if (*key3) 
    { memcpy(key3, "POS", 3);
      rc &= PrfWriteProfileData( hini, "Positions", key3, &pos, sizeof( pos ));
    }
  }
  close_ini( hini );
  return rc;
}

/* Restores the size and position of the window specified by hwnd to
   the state it was in when save_window_pos was last called.
   This function will also restore presentation parameters. */
BOOL
rest_window_pos( HWND hwnd, const char* extkey )
{
  char   key1st[32];
  char   key2[16];
  char   key3[300];
  PPIB   ppib;
  PTIB   ptib;
  SHORT  id   = WinQueryWindowUShort( hwnd, QWS_ID );
  HINI   hini = open_module_ini();
  BOOL   rc   = FALSE;
  POINTL pos[2];
  SWP    swp;
  SWP    desktop;
  ULONG  len  = sizeof(pos);

  DEBUGLOG(("rest_window_pos(%p{%u}, %s)\n", hwnd, id, extkey ? extkey : "<null>" ));

  DosGetInfoBlocks( &ptib, &ppib );

  if( hini == NULLHANDLE)
    return FALSE;
  
  if (!WinQueryWindowPos( hwnd, &swp ))
  { close_ini( hini );
    return FALSE;
  }

  sprintf( key1st, "WIN_%08lX_%08lX", ppib->pib_ulpid, ptib->tib_ptib2->tib2_ultid );
  sprintf( key2, "WIN_%08X", id );
  if (extkey && cfg.win_pos_by_obj)
  { strcpy( key3, key2 );
    key3[12] = '_';
    strlcpy( key3+13, extkey, sizeof key3 -13 );
  } else
    *key3 = 0;

  if( (*key3 && copy_ini_data( hini, "Positions", key3, HINI_PROFILE, "PM123", key1st ))
    || copy_ini_data( hini, "Positions", key2, HINI_PROFILE, "PM123", key1st ) ) {
    rc = WinRestoreWindowPos( "PM123", key1st, hwnd );
    PrfWriteProfileData( HINI_PROFILE, "PM123", key1st, NULL, 0 );
  }
  if (!rc)
  { close_ini( hini );
    return FALSE;
  }

  // rc = TRUE
  if (*key3)
  { memcpy(key3, "POS", 3);
    if (!PrfQueryProfileData( hini, "Positions", key3, &pos, &len ) || len < sizeof pos)
      *key3 = 0; // not found
  }
  if (!*key3)
  { memcpy(key2, "POS", 3);
    rc = PrfQueryProfileData( hini, "Positions", key2, &pos, &len ) && len >= sizeof pos;
  }
  if (rc)
  { WinMapDlgPoints( hwnd, pos, 2, TRUE );
    if (!extkey || *key3)
    { swp.x = pos[0].x;
      swp.y = pos[0].y;
    }
    swp.cx = pos[1].x - pos[0].x;
    swp.cy = pos[1].y - pos[0].y;
  } else {
    rc = FALSE;
  }

  if( WinQueryWindowPos( HWND_DESKTOP, &desktop ))
  { // clip right
    if( swp.x > desktop.cx - 8 )
      swp.x = desktop.cx - 8;
    // clip left
    else if( swp.x + swp.cx < 8 )
      swp.x = 8 - swp.cx;
    // clip top
    if( swp.y + swp.cy > desktop.cy )
      swp.y = desktop.cy - swp.cy;
    // clip bottom
    else if( swp.y + swp.cy < 8 )
      swp.y = 8 - swp.cy;
  }

  WinSetWindowPos( hwnd, 0, swp.x, swp.y, swp.cx, swp.cy, SWP_MOVE|SWP_SIZE );
  close_ini( hini );
  return rc;
}

