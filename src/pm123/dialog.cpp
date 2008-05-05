/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 * Copyright 2004-2006 Dmitry A.Steklenev <glass@ptv.ru>
 * Copyright 2007-2008 M.Mueller
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
#include "dialog.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>

#include <utilfct.h>
#include <cpp/xstring.h>
#include <cpp/container.h>

#include "properties.h"
#include "controller.h"
#include "copyright.h"
#include "skin.h"
#include "iniman.h"
#include "pm123.h"
#include <os2.h>
#include "pm123.rc.h"

#include <debuglog.h>


#define  AMP_REFRESH_CONTROLS   ( WM_USER + 1000 ) /* 0,         0                            */


/* file dialog additional flags */
#define  FDU_DIR_ENABLE   0x0001
#define  FDU_RECURSEBTN   0x0002
#define  FDU_RECURSE_ON   0x0004
#define  FDU_RELATIVBTN   0x0008
#define  FDU_RELATIV_ON   0x0010


/* Equalizer stuff. */
static HWND  heq        = NULLHANDLE;

float gains[20];
BOOL  mutes[20];
float preamp;


static HWND  hhelp      = NULLHANDLE;


/* Default dialog procedure for the file dialog. */
MRESULT EXPENTRY amp_file_dlg_proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{ DEBUGLOG(("amp_file_dlg_proc(%x, %x, %x, %x)\n", hwnd, msg, mp1, mp2));
  FILEDLG* filedialog =
    (FILEDLG*)WinQueryWindowULong( hwnd, QWL_USER );

  switch( msg )
  {
    case WM_INITDLG:
      if( filedialog && !(filedialog->ulUser & FDU_RECURSEBTN )) {
        WinShowWindow( WinWindowFromID( hwnd, CB_RECURSE ), FALSE );
      } else {
        WinCheckButton( hwnd, CB_RECURSE, cfg.add_recursive );
      }
      if( filedialog && !(filedialog->ulUser & FDU_RELATIVBTN )) {
        WinShowWindow( WinWindowFromID( hwnd, CB_RELATIV ), FALSE );
      } else {
        WinCheckButton( hwnd, CB_RELATIV, cfg.save_relative );
      }
      if( filedialog && filedialog->ulUser & FDU_DIR_ENABLE ) {
        WinEnableControl( hwnd, DID_OK, TRUE  );
      }
      do_warpsans( hwnd );
      break;

    case WM_HELP:
      amp_show_help( IDH_MAIN );
      return 0;

    case WM_CONTROL:
      if( SHORT1FROMMP(mp1) == DID_FILENAME_ED && SHORT2FROMMP(mp1) == EN_CHANGE )
      {
        char file[_MAX_PATH];
        WinQueryDlgItemText( hwnd, DID_FILENAME_ED, sizeof(file), file );

        if( filedialog->ulUser & FDU_RECURSEBTN ) {
          if( !*file || strcmp( file, "*"   ) == 0 ||
                        strcmp( file, "*.*" ) == 0 )
          {
            WinEnableControl( hwnd, CB_RECURSE, TRUE  );
          } else {
            WinEnableControl( hwnd, CB_RECURSE, FALSE );
          }
        }

        // Prevents DID_OK from being greyed out.
        if( filedialog->ulUser & FDU_DIR_ENABLE ) {
          return 0;
        }
      }
      break;

    case WM_COMMAND:
      if( SHORT1FROMMP(mp1) == DID_OK )
      {
        if( filedialog->ulUser & FDU_RELATIVBTN ) {
          if( !WinQueryButtonCheckstate( hwnd, CB_RELATIV )) {
            filedialog->ulUser &= ~FDU_RELATIV_ON;
            cfg.save_relative = FALSE;
          } else {
            filedialog->ulUser |=  FDU_RELATIV_ON;
            cfg.save_relative = TRUE;
          }
        }

        if( filedialog->ulUser & FDU_DIR_ENABLE )
        {
          char file[_MAX_PATH];
          WinQueryDlgItemText( hwnd, DID_FILENAME_ED, sizeof(file), file );

          if( !*file ||
              strcmp( file, "*"   ) == 0 ||
              strcmp( file, "*.*" ) == 0 )
          {
            if( !is_root( filedialog->szFullFile )) {
              filedialog->szFullFile[strlen(filedialog->szFullFile)-1] = 0;
            }

            filedialog->lReturn    = DID_OK;
            filedialog->ulFQFCount = 1;

            if( filedialog->ulUser & FDU_RECURSEBTN ) {
              if( !WinQueryButtonCheckstate( hwnd, CB_RECURSE )) {
                filedialog->ulUser &= ~FDU_RECURSE_ON;
                cfg.add_recursive = FALSE;
              } else {
                filedialog->ulUser |=  FDU_RECURSE_ON;
                cfg.add_recursive = TRUE;
              }
            }

            WinDismissDlg( hwnd, DID_OK );
            return 0;
          }
        }
      }
      break;

    case FDM_FILTER:
    {
      HWND  hcbox = WinWindowFromID( hwnd, DID_FILTER_CB );
      ULONG pos   = WinQueryLboxSelectedItem( hcbox );
      ULONG len   = LONGFROMMR( WinSendMsg( hcbox, LM_QUERYITEMTEXTLENGTH, MPFROMSHORT(pos), 0 ));
      char* type  = (char*)malloc( len );
      BOOL  rc    = FALSE;
      char* filt;
      char  file[_MAX_PATH];

      if( !type ) {
        return WinDefFileDlgProc( hwnd, msg, mp1, mp2 );
      }

      WinQueryLboxItemText( hcbox, pos, type, len );
      WinQueryDlgItemText ( hwnd, DID_FILENAME_ED, sizeof(file), file );

      // If the selected type is not have extensions list - that it <All Files>
      // which OS/2 always adds in the list.
      if( !strchr( type, '(' )) {
        rc = TRUE;
      } else {
        strtok( type, "(" );

        while(( filt = strtok( NULL, ";)" )) != NULL ) {
          if( wildcardfit( filt, (char*)mp1 )) {
            rc = TRUE;
            break;
          }
        }
      }

      if( rc && ( strchr( file, '*' ) || strchr( file, '?' ))) {
        rc = wildcardfit( file, (char*)mp1 );
      }

      free( type );
      return MRFROMLONG( rc );
    }
  }
  return WinDefFileDlgProc( hwnd, msg, mp1, mp2 );
}

/* Wizzard function for the default entry "File..." */
ULONG DLLENTRY amp_file_wizzard( HWND owner, const char* title, DECODER_WIZZARD_CALLBACK callback, void* param )
{ DEBUGLOG(("amp_file_wizzard(%p, %s, %p, %p)\n", owner, title, callback, param));

  FILEDLG filedialog = { sizeof( FILEDLG ) };
  { char  buf[2048]; // well, static buffer size...
    buf[2047] = 0;
    dec_fill_types( buf, sizeof buf-1 );
    xstring type_audio = xstring::sprintf(FDT_AUDIO"%s)", buf);
    xstring type_all = xstring::sprintf(FDT_AUDIO_ALL"%s)", buf);

    APSZ types[] = {
      { (PSZ)&*type_audio }, // OS/2 and const...
      { FDT_PLAYLIST },
      { (PSZ)&*type_all }, // OS/2 and const...
      { NULL } };

    xstring wintitle = xstring::sprintf(title, " file(s)");

    filedialog.fl             = FDS_CENTER | FDS_OPEN_DIALOG | FDS_CUSTOM | FDS_MULTIPLESEL;
    filedialog.ulUser         = FDU_DIR_ENABLE | FDU_RECURSEBTN;
    filedialog.pszTitle       = (PSZ)&*wintitle; // OS/2 and const...
    filedialog.hMod           = NULLHANDLE;
    filedialog.usDlgId        = DLG_FILE;
    filedialog.pfnDlgProc     = amp_file_dlg_proc;
    filedialog.papszITypeList = types;
    filedialog.pszIType       = (PSZ)&*type_all; // OS/2 and const...

    strlcpy( filedialog.szFullFile, cfg.filedir, sizeof filedialog.szFullFile );
    PMRASSERT( WinFileDlg( HWND_DESKTOP, owner, &filedialog ));
  }

  ULONG ret = 300; // Cancel unless DID_OK

  if( filedialog.lReturn == DID_OK ) {
    ret = 0;

    char* file = filedialog.ulFQFCount > 1
      ? **filedialog.papszFQFilename
      : filedialog.szFullFile;

    if (*file)
      sdrivedir( cfg.filedir, file, sizeof( cfg.filedir ));

    ULONG count = 0;
    while (*file)
    { DEBUGLOG(("amp_file_wizzard: %s\n", file));
      char fileurl[_MAX_FNAME+25]; // should be sufficient in all cases
      strcpy(fileurl, "file:///");
      strcpy(fileurl + (url123::isPathDelimiter(file[0]) && url123::isPathDelimiter(file[1]) ? 5 : 8), file);
      char* dp = fileurl + strlen(fileurl);
      if (is_dir(file))
      { // Folder => add trailing slash
        if (!url123::isPathDelimiter(dp[-1]))
          *dp++ = '/';
        if (filedialog.ulUser & FDU_RECURSE_ON)
        { strcpy(dp, "?recursive");
          dp += 10;
        } else
          *dp = 0;
      }
      // convert slashes
      dp = strchr(fileurl+7, '\\');
      while (dp)
      { *dp = '/';
        dp = strchr(dp+1, '\\');
      }
      // Callback
      (*callback)(param, fileurl);
      // next file
      if (++count >= filedialog.ulFQFCount)
        break;
      file = (*filedialog.papszFQFilename)[count];
    }
  }

  WinFreeFileDlgList( filedialog.papszFQFilename );

  return ret;
}

/* Default dialog procedure for the URL dialog. */
static MRESULT EXPENTRY amp_url_dlg_proc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ switch( msg )
  {case WM_CONTROL:
    if (MPFROM2SHORT(ENT_URL, EN_CHANGE) != mp1)
      break;
   case WM_INITDLG:
    // Update enabled status of the OK-Button
    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), WinQueryDlgItemTextLength(hwnd, ENT_URL) != 0);
    break;

   case WM_COMMAND:
    DEBUGLOG(("amp_url_dlg_proc: WM_COMMAND: %i\n", SHORT1FROMMP(mp1)));
    if (SHORT1FROMMP(mp1) == DID_OK)
    { HWND ent = WinWindowFromID(hwnd, ENT_URL);
      LONG len = WinQueryWindowTextLength(ent);
      xstring text;
      WinQueryWindowText(ent, len+1, text.raw_init(len));
      if (url123::normalizeURL(text))
        break; // everything OK => continue
      WinMessageBox(HWND_DESKTOP, hwnd, xstring::sprintf("The URL \"%s\" is not well formed.", text.cdata()),
        NULL, 0, MB_CANCEL|MB_WARNING|MB_APPLMODAL|MB_MOVEABLE);
      return 0; // cancel
    }
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* Adds HTTP file to the playlist or load it to the player. */
ULONG DLLENTRY amp_url_wizzard( HWND owner, const char* title, DECODER_WIZZARD_CALLBACK callback, void* param )
{ DEBUGLOG(("amp_url_wizzard(%x, %s, %p, %p)\n", owner, title, callback, param));

  HWND hwnd = WinLoadDlg( HWND_DESKTOP, owner, amp_url_dlg_proc, NULLHANDLE, DLG_URL, 0 );
  if (hwnd == NULLHANDLE)
    return 500;

  do_warpsans(hwnd);

  xstring wintitle = xstring::sprintf(title, " URL");
  WinSetWindowText(hwnd, (PSZ)&*wintitle);

  // TODO: absolute size limit???
  char durl[2048];
  WinSendDlgItemMsg( hwnd, ENT_URL, EM_SETTEXTLIMIT, MPFROMSHORT(sizeof durl), 0 );

  // TODO: last URL
  // TODO: 2. recent URLs

  ULONG ret = 300;
  if (WinProcessDlg(hwnd) == DID_OK)
  { WinQueryDlgItemText(hwnd, ENT_URL, sizeof durl, durl);
    DEBUGLOG(("amp_url_wizzard: %s\n", durl));
    url123 nurl = url123::normalizeURL(durl);
    DEBUGLOG(("amp_url_wizzard: %s\n", nurl.cdata()));
    (*callback)(param, nurl);
    ret = 0;
  }
  WinDestroyWindow(hwnd);
  return ret;
}

/* Adds a user selected bookmark. */
void amp_add_bookmark(HWND owner, PlayableSlice* item)
{ DEBUGLOG(("amp_add_bookmark(%x, %p{%s})\n", owner, item, item->GetPlayable()->GetURL().cdata()));
  // TODO: !!!!!! request information before
  const META_INFO& meta = *item->GetPlayable()->GetInfo().meta;
  xstring desc = "";
  if (*meta.artist)
    desc = xstring(meta.artist) + "-";
  if (*meta.title)
    desc = desc + meta.title;
   else
    desc = desc + item->GetPlayable()->GetURL().getShortName();

  HWND hdlg = WinLoadDlg(HWND_DESKTOP, owner, &WinDefDlgProc, NULLHANDLE, DLG_BM_ADD, NULL);

  WinSetDlgItemText(hdlg, EF_BM_DESC, desc);

  if (WinProcessDlg(hdlg) == DID_OK)
  { xstring alias;
    char* cp = alias.raw_init(WinQueryDlgItemTextLength(hdlg, EF_BM_DESC));
    WinQueryDlgItemText(hdlg, EF_BM_DESC, alias.length()+1, cp);
    if (alias == desc)
      alias = NULL; // Don't set alias if not required.
    amp_get_default_bm()->InsertItem(*item);
    // TODO !!!!!
    //bm_save( owner );
  }

  WinDestroyWindow(hdlg);
}

/* Saves a playlist */
void amp_save_playlist(HWND owner, PlayableCollection* playlist)
{
  APSZ  types[] = {{ FDT_PLAYLIST_LST }, { FDT_PLAYLIST_M3U }, { 0 }};

  FILEDLG filedialog = {sizeof(FILEDLG)};
  filedialog.fl             = FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_CUSTOM | FDS_ENABLEFILELB;
  filedialog.pszTitle       = "Save playlist";
  filedialog.hMod           = NULLHANDLE;
  filedialog.usDlgId        = DLG_FILE;
  filedialog.pfnDlgProc     = amp_file_dlg_proc;
  filedialog.ulUser         = FDU_RELATIVBTN;
  filedialog.papszITypeList = types;
  filedialog.pszIType       = FDT_PLAYLIST_LST;

  if ((playlist->GetFlags() & Playable::Mutable) == Playable::Mutable && playlist->GetURL().isScheme("file://"))
  { // Playlist => save in place allowed => preselect our own file name
    const char* cp = playlist->GetURL().cdata() + 5;
    if (cp[2] == '/')
      cp += 3;
    strlcpy(filedialog.szFullFile, cp, sizeof filedialog.szFullFile);
    // preselect file type
    if (playlist->GetURL().getExtension().compareToI(".M3U") == 0)
      filedialog.pszIType = FDT_PLAYLIST_M3U;
    // TODO: other playlist types
  } else
  { // not mutable => only save as allowed
    // TODO: preselect directory
  }

  PMXASSERT(WinFileDlg(HWND_DESKTOP, owner, &filedialog), != NULLHANDLE);

  if(filedialog.lReturn == DID_OK)
  { url123 file = url123::normalizeURL(filedialog.szFullFile);
    if (!(Playable::IsPlaylist(file)))
    { if (file.getExtension().length() == 0)
      { // no extension => choose automatically
        if (strcmp(filedialog.pszIType, FDT_PLAYLIST_M3U) == 0)
          file = file + ".m3u";
        else // if (strcmp(filedialog.pszIType, FDT_PLAYLIST_LST) == 0)
          file = file + ".lst";
        // TODO: other playlist types
      } else
      { amp_error(owner, "PM123 cannot write playlist files with the unsupported extension %s.", file.getExtension().cdata());
        return;
      }
    }
    const char* cp = file.cdata() + 5;
    if (cp[2] == '/')
      cp += 3;
    if (amp_warn_if_overwrite(owner, cp))
    { PlayableCollection::save_options so = PlayableCollection::SaveDefault;
      if (file.getExtension().compareToI(".m3u") == 0)
        so |= PlayableCollection::SaveAsM3U;
      if (filedialog.ulUser & FDU_RELATIV_ON)
        so |= PlayableCollection::SaveRelativePath;
      // now save
      if (!playlist->Save(file, so))
        amp_error(owner, "Failed to create playlist \"%s\". Error %s.", file.cdata(), xio_strerror(xio_errno()));
    }
  }
}

/* Loads a skin selected by the user. */
void amp_loadskin( HAB hab, HWND hwnd, HPS hps )
{
  FILEDLG filedialog;
  APSZ types[] = {{ FDT_SKIN }, { 0 }};

  memset( &filedialog, 0, sizeof( FILEDLG ));

  filedialog.cbSize         = sizeof( FILEDLG );
  filedialog.fl             = FDS_CENTER | FDS_OPEN_DIALOG | FDS_CUSTOM;
  filedialog.pszTitle       = "Load PM123 skin";
  filedialog.hMod           = NULLHANDLE;
  filedialog.usDlgId        = DLG_FILE;
  filedialog.pfnDlgProc     = amp_file_dlg_proc;
  filedialog.papszITypeList = types;
  filedialog.pszIType       = FDT_SKIN;

  sdrivedir( filedialog.szFullFile, cfg.defskin, sizeof( filedialog.szFullFile ));
  WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &filedialog );

  if( filedialog.lReturn == DID_OK ) {
    bmp_load_skin( filedialog.szFullFile, hab, hwnd, hps );
    strcpy( cfg.defskin, filedialog.szFullFile );
  }
}


static BOOL amp_save_eq( HWND owner, float* gains, BOOL *mutes, float preamp )
{
  FILEDLG filedialog;
  FILE*   file;
  int     i;
  char    ext[_MAX_EXT];
  APSZ    types[] = {{ FDT_EQUALIZER }, { 0 }};

  memset( &filedialog, 0, sizeof( FILEDLG ));

  filedialog.cbSize         = sizeof( FILEDLG );
  filedialog.fl             = FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_CUSTOM;
  filedialog.pszTitle       = "Save equalizer";
  filedialog.hMod           = NULLHANDLE;
  filedialog.usDlgId        = DLG_FILE;
  filedialog.pfnDlgProc     = amp_file_dlg_proc;
  filedialog.papszITypeList = types;
  filedialog.pszIType       = FDT_EQUALIZER;

  strcpy( filedialog.szFullFile, cfg.lasteq );
  WinFileDlg( HWND_DESKTOP, owner, &filedialog );

  if( filedialog.lReturn == DID_OK )
  {
    if( strcmp( sfext( ext, filedialog.szFullFile, sizeof( ext )), "" ) == 0 ) {
      strcat( filedialog.szFullFile, ".eq" );
    }

    if( amp_warn_if_overwrite( owner, filedialog.szFullFile ))
    {
      strcpy( cfg.lasteq, filedialog.szFullFile );
      file = fopen( filedialog.szFullFile, "w" );

      if( file == NULL ) {
        return FALSE;
      }

      fprintf( file, "#\n# Equalizer created with %s\n# Do not modify!\n#\n", AMP_FULLNAME );
      fprintf( file, "# Band gains\n" );
      for( i = 0; i < 20; i++ ) {
        fprintf( file, "%g\n", gains[i] );
      }
      fprintf( file, "# Mutes\n" );
      for( i = 0; i < 20; i++ ) {
        fprintf( file, "%lu\n", mutes[i] );
      }
      fprintf( file, "# Preamplifier\n" );
      fprintf( file, "%g\n", preamp );

      fprintf( file, "# End of equalizer\n" );
      fclose( file );
      return TRUE;
    }
  }
  return FALSE;
}

BOOL amp_load_eq_file( char* filename, float* gains, BOOL* mutes, float* preamp )
{
  FILE* file;
  char  vz[CCHMAXPATH];
  int   i;

  file = fopen( filename, "r" );
  if( !file ) {
    return FALSE;
  }

  i = 0;
  while( !feof( file ))
  {
    fgets( vz, sizeof( vz ), file );
    blank_strip( vz );
    if( *vz && vz[0] != '#' && vz[0] != ';' && i < 41 )
    {
      if( i < 20 ) {
        gains[i] = atof(vz);
      }
      if( i > 19 && i < 40 ) {
        mutes[i-20] = atoi(vz);
      }
      if( i == 40 ) {
        *preamp = atof(vz);
      }
      i++;
    }
  }
  fclose( file );
  return TRUE;
}

static BOOL amp_load_eq( HWND hwnd, float* gains, BOOL* mutes, float* preamp )
{
  FILEDLG filedialog;
  APSZ    types[] = {{ FDT_EQUALIZER }, { 0 }};

  memset( &filedialog, 0, sizeof( FILEDLG ));

  filedialog.cbSize         = sizeof( FILEDLG );
  filedialog.fl             = FDS_CENTER | FDS_OPEN_DIALOG | FDS_CUSTOM;
  filedialog.pszTitle       = "Load equalizer";
  filedialog.hMod           = NULLHANDLE;
  filedialog.usDlgId        = DLG_FILE;
  filedialog.pfnDlgProc     = amp_file_dlg_proc;
  filedialog.papszITypeList = types;
  filedialog.pszIType       = FDT_EQUALIZER;

  sdrivedir( filedialog.szFullFile, cfg.lasteq, sizeof( filedialog.szFullFile ));
  WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &filedialog );

  if( filedialog.lReturn == DID_OK ) {
    strcpy( cfg.lasteq, filedialog.szFullFile );
    return amp_load_eq_file( filedialog.szFullFile, gains, mutes, preamp );
  }
  return FALSE;
}

/* Returns TRUE if the save stream feature has been enabled. */
void amp_save_stream( HWND hwnd, BOOL enable )
{
  if( enable )
  {
    FILEDLG filedialog;

    memset( &filedialog, 0, sizeof( FILEDLG ));
    filedialog.cbSize     = sizeof( FILEDLG );
    filedialog.fl         = FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_CUSTOM;
    filedialog.pszTitle   = "Save stream as";
    filedialog.hMod       = NULLHANDLE;
    filedialog.usDlgId    = DLG_FILE;
    filedialog.pfnDlgProc = amp_file_dlg_proc;

    strcpy( filedialog.szFullFile, cfg.savedir );
    WinFileDlg( HWND_DESKTOP, hwnd, &filedialog );

    if( filedialog.lReturn == DID_OK ) {
      if( amp_warn_if_overwrite( hwnd, filedialog.szFullFile ))
      {
        Ctrl::PostCommand(Ctrl::MkSave(filedialog.szFullFile), &amp_control_event_callback);
        sdrivedir( cfg.savedir, filedialog.szFullFile, sizeof( cfg.savedir ));
      }
    }
  } else {
    Ctrl::PostCommand(Ctrl::MkSave(xstring()));
  }
}

static void amp_eq_update()
{ DEBUGLOG(("amp_eq_update()\n"));

  Ctrl::EQ_Data data;
  for (int i = 0; i < 20; i++)
    data.bandgain[0][i] = gains[i] * preamp * !mutes[i]; // Attension: dirty out of bounds access to data.bandgain[1][...]

  Ctrl::PostCommand(Ctrl::MkEqualize(&data));
}

static MRESULT EXPENTRY
amp_eq_dlg_proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  static ULONG slider_range;
  static BOOL  nottheuser = FALSE;

  switch( msg )
  {
    case WM_CLOSE:
      heq = NULLHANDLE;
      WinDestroyWindow( hwnd );
      return 0;

    case WM_DESTROY:
      save_window_pos( hwnd, WIN_MAP_POINTS );
      return WinDefDlgProc( hwnd, msg, mp1, mp2 );

    case WM_HELP:
      amp_show_help( IDH_EQUALIZER );
      return 0;

    case AMP_REFRESH_CONTROLS:
    {
      float db;
      ULONG value;
      int   i;

      nottheuser = TRUE;

      /* TODO: what the hell is that?
      _control87( EM_INVALID   | EM_DENORMAL | EM_ZERODIVIDE | EM_OVERFLOW |
                  EM_UNDERFLOW | EM_INEXACT, MCW_EM );
      */

      for( i = 0; i < 10; i++ )
      {
        db = 20 * log10( gains[i] );
        value = (ULONG)(( db * slider_range / 2 ) / 12 + slider_range / 2);

        WinSendDlgItemMsg( hwnd, SL_EQ_0 + i, SLM_SETSLIDERINFO,
                           MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                           MPFROMSHORT( value ));

        WinSendDlgItemMsg( hwnd, CB_EQ_0 + i, BM_SETCHECK,
                           MPFROMSHORT( mutes[i] ), 0 );
      }

      db = 20 * log10( preamp );
      value = (ULONG)(( db * slider_range / 2 ) / 12 + slider_range / 2);

      WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_SETSLIDERINFO,
                         MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                         MPFROMSHORT( value ));

      WinSendDlgItemMsg( hwnd, CB_EQ_ENABLED, BM_SETCHECK,
                         MPFROMSHORT( cfg.eq_enabled ), 0 );

      nottheuser = FALSE;
      break;
    }

    case WM_COMMAND:
      switch( COMMANDMSG(&msg)->cmd )
      {
        case BT_EQ_LOAD: // load button
          if( amp_load_eq( hwnd, gains, mutes, &preamp )) {
            WinSendMsg( hwnd, AMP_REFRESH_CONTROLS, 0, 0 );
          }
          if( WinQueryButtonCheckstate( hwnd, 121 )) {
            amp_eq_update();
          }
          break;

        case BT_EQ_SAVE: // save button
          amp_save_eq( hwnd, gains, mutes, preamp );
          break;

        case BT_EQ_DEFAULT: // default button
        {
          int i;

          for( i = 0; i < 20; i++ ) {
            gains[i] = 1.0;
          }
          for( i = 0; i < 20; i++ ) {
            mutes[i] = 0;
          }
          for( i = 0; i < 10; i++ ) {
            WinSendDlgItemMsg( hwnd, SL_EQ_0 + i, SLM_SETSLIDERINFO,
                               MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                               MPFROMLONG( slider_range / 2 ));
          }
          for( i = 0; i < 10; i++ ) {
            WinSendDlgItemMsg( hwnd, CB_EQ_0 + i, BM_SETCHECK, MPFROMSHORT( FALSE ), 0 );
          }

          WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_SETSLIDERINFO,
                             MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                             MPFROMLONG( slider_range / 2 ));

          if( WinQueryButtonCheckstate( hwnd, CB_EQ_ENABLED )) {
            amp_eq_update();
          } else {
            Ctrl::PostCommand(Ctrl::MkEqualize(NULL));
          }
          break;
        }
      }
      break;

    case WM_CONTROL:
    {
      int id = SHORT1FROMMP(mp1);
      if( nottheuser ) {
        break;
      }

      if( SHORT2FROMMP( mp1 ) == BN_CLICKED )
      {
        if( id >= CB_EQ_0 && id < CB_EQ_0 + 10 ) // Mute
        {
          mutes[id - CB_EQ_0] = WinQueryButtonCheckstate( hwnd, id );       // Left
          mutes[id - CB_EQ_0 + 10] = WinQueryButtonCheckstate( hwnd, id );  // Right

          if( WinQueryButtonCheckstate( hwnd, CB_EQ_ENABLED )) {
            amp_eq_update();
            break;
          }
        }

        if( id == 121 ) {
          cfg.eq_enabled = WinQueryButtonCheckstate( hwnd, CB_EQ_ENABLED );
          if( cfg.eq_enabled ) {
            amp_eq_update();
          } else {
            Ctrl::PostCommand(Ctrl::MkEqualize(NULL));
          }
          break;
        }
      }

      switch( SHORT2FROMMP( mp1 ))
      {
        case SLN_CHANGE:
          // Slider adjust
          if(( id >= SL_EQ_0 && id < SL_EQ_0 + 10 ) || id == SL_EQ_PREAMP )
          {
            float g2;

            /* TODO: what the hell is that?
            _control87( EM_INVALID  | EM_DENORMAL  | EM_ZERODIVIDE |
                        EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT, MCW_EM);*/

            // -12 to 12 dB
            g2 = ((float)LONGFROMMP(mp2) - slider_range / 2 ) / ( slider_range / 2 ) * 12;
            // transforming into voltage gain
            g2 = pow( 10.0, g2 / 20.0 );

            if( id == SL_EQ_PREAMP ) {
              preamp = g2;
            } else {
              gains[SHORT1FROMMP(mp1) - SL_EQ_0] = g2;      // Left
              gains[SHORT1FROMMP(mp1) - SL_EQ_0 + 10] = g2; // Right
            }

            if( WinQueryButtonCheckstate( hwnd, CB_EQ_ENABLED )) {
              amp_eq_update();
            }
          }
      }
      break;
    }

    case WM_INITDLG:
    {
      int i;

      nottheuser = TRUE;
      slider_range = SHORT2FROMMR( WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_QUERYSLIDERINFO,
                                   MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                                   MPFROMLONG(0))) - 1;

      for( i = SL_EQ_0; i < SL_EQ_0 + 10; i++ )
      {
        WinSendDlgItemMsg( hwnd, i, SLM_ADDDETENT, MPFROMSHORT( 0 ), 0 );
        WinSendDlgItemMsg( hwnd, i, SLM_ADDDETENT, MPFROMSHORT( slider_range / 4 ), 0 );
        WinSendDlgItemMsg( hwnd, i, SLM_ADDDETENT, MPFROMSHORT( slider_range / 2 ), 0 );
        WinSendDlgItemMsg( hwnd, i, SLM_ADDDETENT, MPFROMSHORT( 3 * slider_range / 4 ), 0 );
        WinSendDlgItemMsg( hwnd, i, SLM_ADDDETENT, MPFROMSHORT( slider_range ), 0 );
        WinSendDlgItemMsg( hwnd, i, SLM_SETSLIDERINFO,
                           MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                           MPFROMLONG( slider_range / 2 ));
      }

       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_SETTICKSIZE, MPFROM2SHORT( SMA_SETALLTICKS, 0 ), 0 );

       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_ADDDETENT, MPFROMSHORT( 0 ), 0 );
       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_ADDDETENT, MPFROMSHORT( slider_range / 4 ), 0 );
       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_ADDDETENT, MPFROMSHORT( slider_range / 2 ), 0 );
       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_ADDDETENT, MPFROMSHORT( 3 * slider_range / 4 ), 0 );
       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_ADDDETENT, MPFROMSHORT( slider_range ), 0 );
       WinSendDlgItemMsg( hwnd, SL_EQ_PREAMP, SLM_SETSLIDERINFO,
                          MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE ),
                          MPFROMLONG( slider_range / 2 ));

       WinSendMsg( hwnd, AMP_REFRESH_CONTROLS, 0, 0 );
       nottheuser = FALSE;
       break;
    }

  default:
    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
  }

  return 0;
}

void amp_eq_show( void )
{
  if( heq == NULLHANDLE )
  {
    heq = WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP,
                      amp_eq_dlg_proc, NULLHANDLE, DLG_EQUALIZER, NULL );

    do_warpsans( heq );
    rest_window_pos( heq, WIN_MAP_POINTS );
  }

  WinSetWindowPos( heq, HWND_TOP, 0, 0, 0, 0,
                        SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE );
}

static USHORT amp_message_box( HWND owner, const char* title,
                             const char* message, ULONG style  )
{
  char padded_title[60];
  sprintf( padded_title, "%-59s", title );

  if( owner == NULLHANDLE )
  { owner  =  HWND_DESKTOP;
    style &= ~MB_APPLMODAL;
  } else {
    style |=  MB_APPLMODAL;
  }

  return WinMessageBox( HWND_DESKTOP, owner, (PSZ)message,
                                      padded_title, 0, style );
}

/* Creates and displays a error message window.
   Use the player window as message window owner. */
void amp_player_error( const char* format, ... )
{ va_list args;
  va_start(args, format);
  xstring message = xstring::vsprintf(format, args);
  va_end(args);

  DEBUGLOG(("ERROR: %s\n", message.cdata()));
  amp_message_box( amp_player_window(), "PM123 Error", message, MB_ERROR | MB_OK | MB_MOVEABLE );
}

/* Creates and displays a error message window.
   The specified owner window is disabled. */
void amp_error( HWND owner, const char* format, ... )
{ va_list args;
  va_start(args, format);
  xstring message = xstring::vsprintf(format, args);
  va_end(args);

  DEBUGLOG(("ERROR: %x, %s\n", owner, message.cdata()));
  amp_message_box( owner, "PM123 Error", message, MB_ERROR | MB_OK | MB_MOVEABLE );
}

/* Creates and displays a message window. */
void amp_info( HWND owner, const char* format, ... )
{ va_list args;
  va_start(args, format);
  xstring message = xstring::vsprintf(format, args);
  va_end(args);

  DEBUGLOG(("INFO: %s\n", message.cdata()));
  amp_message_box( owner, "PM123 Information", message, MB_INFORMATION | MB_OK | MB_MOVEABLE );
}

/* Requests the user about specified action. Returns
   TRUE at confirmation or FALSE in other case. */
BOOL amp_query( HWND owner, const char* format, ... )
{ va_list args;
  va_start(args, format);
  xstring message = xstring::vsprintf(format, args);
  va_end(args);

  return amp_message_box( owner, "PM123 Query", message, MB_QUERY | MB_YESNO | MB_MOVEABLE ) == MBID_YES;
}

/* Requests the user about overwriting a file. Returns
   TRUE at confirmation or at absence of a file. */
BOOL amp_warn_if_overwrite( HWND owner, const char* filename )
{
  struct stat fi;
  if( stat( filename, &fi ) == 0 ) {
    return amp_query( owner, "File %s already exists. Overwrite it?", filename );
  } else {
    return TRUE;
  }
}

/* Tells the help manager to display a specific help window. */
void amp_show_help( SHORT resid )
{
  WinSendMsg( hhelp, HM_DISPLAY_HELP, MPFROMLONG( MAKELONG( resid, NULL )),
                                      MPFROMSHORT( HM_RESOURCEID ));
}



/* global init */
void dlg_init()
{
  xstring infname = xstring::sprintf("%s%s", startpath, "pm123.inf");
  struct stat fi;
  if( stat( infname, &fi ) != 0  )
    // If the file of the help does not placed together with the program,
    // we shall give to the help manager to find it.
    infname = "pm123.inf";

  HELPINIT hinit = { sizeof( hinit ) };
  hinit.phtHelpTable = (PHELPTABLE)MAKELONG( HLP_MAIN, 0xFFFF );
  hinit.pszHelpWindowTitle = "PM123 Help";
  hinit.fShowPanelId = CMIC_SHOW_PANEL_ID;
  hinit.pszHelpLibraryName = (PSZ)infname.cdata();

  hhelp = WinCreateHelpInstance( amp_player_hab(), &hinit );
  if( !hhelp )
    amp_error( amp_player_window(), "Error create help instance: %s", infname.cdata() );
  else
    WinAssociateHelpInstance( hhelp, amp_player_window() );

}

void dlg_uninit()
{
}
