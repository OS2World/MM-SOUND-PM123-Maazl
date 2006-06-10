/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 *
 * Copyright 2004-2006 Dmitry A.Steklenev <glass@ptv.ru>
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

#ifndef __PM123_UTILS_H
#define __PM123_UTILS_H

#include "rel2abs.h"
#include "abs2rel.h"
#include "bufstream.h"
#include "charset.h"
#include "wildcards.h"
#include "filefind.h"
#include "fileutil.h"
#include "url.h"
#include "errorstr.h"
#include "inimacro.h"
#include "queue.h"

#ifndef bool
  /* This may be defined by STLport. */
  typedef int   bool;
  #define true  1
  #define false 0
#endif

#ifndef  BKS_TABBEDDIALOG
  /* Tabbed dialog. */
  #define BKS_TABBEDDIALOG 0x00000800UL
#endif
#ifndef  BKS_BUTTONAREA
  /* Reserve space for buttons. */
  #define BKS_BUTTONAREA   0x00000200UL
#endif

#if __cplusplus
extern "C" {
#endif

/* Returns TRUE if WarpSans is supported by operating system. */
BOOL check_warpsans( void );
/* Assigns the 9.WarpSans as default font for a specified window if it is supported by
   operating system. Otherwise assigns the 8.Helv as default font. */
void do_warpsans( HWND hwnd );

/* Queries a module handle and name. */
void getModule ( HMODULE* hmodule, char* name, int name_size );
/* Queries a program name. */
void getExeName( char* name, int name_size );

/* Removes leading and trailing spaces. */
char* blank_strip( char* string );
/* Removes leading and trailing spaces and quotes. */
char* quote_strip( char* string );
/* Removes comments starting with "#". */
char* uncomment  ( char* string );

/* Places the current thread into a wait state until another thread
   in the current process has ended. Kills another thread if the
   time expires. */
void  wait_thread( TID tid, ULONG msec );

/* Makes a menu item selectable. */
BOOL  mn_enable_item( HWND menu, SHORT id, BOOL enable );
/* Places a a check mark to the left of the menu item. */
BOOL  mn_check_item ( HWND menu, SHORT id, BOOL check  );

/* Delete all the items in the list box. */
BOOL  lb_remove_all( HWND hwnd, SHORT id );
/* Deletes an item from the list box control. */
SHORT lb_remove_item( HWND hwnd, SHORT id, SHORT i );
/* Adds an item into a list box control. */
SHORT lb_add_item( HWND hwnd, SHORT id, const char* item );
/* Queries the indexed item of the list box control. */
SHORT lb_get_item( HWND hwnd, SHORT id, SHORT i, char* item, LONG size );
/* Queries a size the indexed item of the list box control. */
SHORT lb_get_item_size( HWND hwnd, SHORT id, SHORT i );
/* Sets the handle of the specified list box item. */
BOOL  lb_set_handle( HWND hwnd, SHORT id, SHORT i, PVOID handle );
/* Returns the handle of the indexed item of the list box control. */
PVOID lb_get_handle( HWND hwnd, SHORT id, SHORT i );
/* Sets the selection state of an item in a list box. */
BOOL  lb_select( HWND hwnd, SHORT id, SHORT i );
/* Returns the current cursored item. */
SHORT lb_cursored( HWND hwnd, SHORT id );
/* Returns the current selected item. */
SHORT lb_selected( HWND hwnd, SHORT id, SHORT starti );
/* Returns a count of the number of items in the list box control. */
SHORT lb_size( HWND hwnd, SHORT id );
/* Searches an item in a list box control. */
SHORT lb_search( HWND hwnd, SHORT id, SHORT starti, char *item );

#if __cplusplus
}
#endif
#endif /* _UTILFCT_H */
