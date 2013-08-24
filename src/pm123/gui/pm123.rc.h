/*
 * Copyright 2007-2012 M.Mueller
 * Copyright 2004-2006 Dmitry A.Steklenev <glass@ptv.ru>
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

#ifndef  PM123_RC_H
#define  PM123_RC_H

/* ID ranges:
 *
 *     0- 1999   PM123 core
 *     0-   10    PM123 main window
 *   100-  199    PM123 common dialog controls
 *   500-  599    PM123 main menu
 *   700-  799    PM123 Playlist/Playlist Manager misc
 *   800-  999    PM123 Playlist/Playlist Manager menu
 *  1000- 1099    PM123 Help table
 *  1100- 1199    PM123 File, URL etc.
 *  1200- 1699    PM123 properties dialog
 *  1700- 1899    PM123 info dialog
 *  2000- 2999   PM123 Skin IDs
 *  2000- 2099    PM123 Skinned buttons
 *  2100- 2199    PM123 Skin misc
 *  2200- 2399    PM123 Skin digits
 *  2400- 2799    PM123 Skin fonts
 * 10000-19999   PM123 Bookmark browser
 */

#define WIN_MAIN              1
#define HLP_MAIN              1
#define ACL_MAIN              WIN_MAIN // neccessarily identical
#define ICO_MAIN              WIN_MAIN // neccessarily identical

/* Generic controls */
#define ST_GENERIC          100 // Generic, non-unique static text
#define GB_GENERIC          101 // Generic, non-unique group box

#define PB_UNDO             110
#define PB_DEFAULT          111
#define PB_HELP             112
#define PB_APPLY            113
#define PB_RETRY            114
#define PB_REFRESH          115

/* Main menu */
#define MNU_MAIN            500
#define IDM_M_HELP          501
#define IDM_M_CFG           507
#define IDM_M_PLAYLIST      509
#define IDM_M_MINIMIZE      510
#define IDM_M_NORMAL        511
#define IDM_M_SMALL         512
#define IDM_M_TINY          513
#define IDM_M_FONT1         515
#define IDM_M_FONT2         516
#define IDM_M_FONT          517
#define IDM_M_SIZE          518
#define IDM_M_SKIN          519
#define IDM_M_SKINLOAD      520
#define IDM_M_FLOAT         521
#define IDM_M_TAG           523
#define IDM_M_INFO          524
#define IDM_M_PLINFO        525
#define IDM_M_PLOPEN        526
#define IDM_M_PLOPENDETAIL  527
#define IDM_M_PLOPENTREE    528
#define IDM_M_MANAGER       530
#define IDM_M_SAVE          532
#define IDM_M_BOOKMARKS     533
#define IDM_M_ADDBOOK       534
#define IDM_M_EDITBOOK      535
#define IDM_M_EDITBOOKTREE  536
#define IDM_M_PLAYBACK      537
#define IDM_M_VOL_RAISE     538
#define IDM_M_VOL_LOWER     539
#define IDM_M_MENU          540
#define IDM_M_CURRENT_SONG  541
#define IDM_M_DETAILED      542
#define IDM_M_TREEVIEW      543
#define IDM_M_ADDPLBOOK     544
#define IDM_M_ADDBOOK_TIME  545
#define IDM_M_PLSAVE        546
#define IDM_M_PLSAVEAS      547
#define IDM_M_ADDPMBOOK     548
#define IDM_M_CURRENT_PL    549
#define IDM_M_ADDPLBOOK_TIME 550
#define IDM_M_LOAD          551
#define IDM_M_PLRELOAD      552
#define IDM_M_RELOAD        553
#define IDM_M_PLCONTENT     554
#define IDM_M_INSPECTOR     559
#define IDM_M_LOADFILE      560
#define IDM_M_URL           561
#define IDM_M_LOADOTHER     562 /* reserve some ID's for several plug-ins.  */
#define IDM_M_PLUG          580 /* reserve some ID's for several plug-ins.  */
#define IDM_M_PLUG_E        599

#define IDM_M_LAST        10000 /* A lot of IDs after this need to be free. */
#define IDM_M_LAST_E      19999


/* Playlist */
#define DLG_PLAYLIST        700
/* Playlistmanager */
#define DLG_PM              702
#define CO_CONTENT          703

#define MNU_PLAYLIST        710
#define MNU_RECORD          711
#define ACL_PLAYLIST        712
#define PM_MAIN_MENU        720
#define PM_REC_MENU         721
//#define ACL_PLAYLISTMAN     722

#define ICO_WAIT            740
#define ICO_SONG            750
#define ICO_SONG_ACTIVE     751
#define ICO_SONG_PLAY       752
#define ICO_SONG_SHADOW     753
#define ICO_SONG_INVALID    754
#define ICO_PL_EMPTY        755
#define ICO_PL_EMPTY_ACTIVE 756
#define ICO_PL_EMPTY_PLAY   757
#define ICO_PL_EMPTY_SHADOW 758
#define ICO_PL_INVALID      759
#define ICO_PL_CLOSE        760
#define ICO_PL_CLOSE_ACTIVE 761
#define ICO_PL_CLOSE_PLAY   762
#define ICO_PL_CLOSE_SHADOW 763
#define ICO_PL_OPEN         765
#define ICO_PL_OPEN_ACTIVE  766
#define ICO_PL_OPEN_PLAY    767
#define ICO_PL_OPEN_SHADOW  768
#define ICO_PL_RECSV        770
#define ICO_PL_RECSV_ACTIVE 771
#define ICO_PL_RECSV_PLAY   772
//#define ICO_PL_RECSV_SHADOW 773
#define ICO_FL_EMPTY        775
#define ICO_FL_EMPTY_ACTIVE 776
#define ICO_FL_EMPTY_PLAY   777
#define ICO_FL_EMPTY_SHADOW 778
#define ICO_FL_INVALID      779
#define ICO_FL_CLOSE        780
#define ICO_FL_CLOSE_ACTIVE 781
#define ICO_FL_CLOSE_PLAY   782
#define ICO_FL_CLOSE_SHADOW 783
#define ICO_FL_OPEN         785
#define ICO_FL_OPEN_ACTIVE  786
#define ICO_FL_OPEN_PLAY    787
#define ICO_FL_OPEN_SHADOW  788
#define ICO_ALT_CLOSE       790
#define ICO_ALT_CLOSE_ACTIVE 791
#define ICO_ALT_CLOSE_PLAY  792
#define ICO_ALT_CLOSE_SHADOW 793
#define ICO_ALT_OPEN        795
#define ICO_ALT_OPEN_ACTIVE 796
#define ICO_ALT_OPEN_PLAY   797
#define ICO_ALT_OPEN_SHADOW 798

#define IDM_PL_USE          800
#define IDM_PL_USEALL       801
#define IDM_PL_NAVIGATE     802
#define IDM_PL_DETAILED     803
#define IDM_PL_DETAILEDALL  804
#define IDM_PL_TREEVIEW     805
#define IDM_PL_TREEVIEWALL  806
#define IDM_PL_REMOVE       807
#define IDM_PL_CLEAR        808
#define IDM_PL_CLEARALL     809
#define IDM_PL_RELOAD       810
#define IDM_PL_REFRESH      811
#define IDM_PL_EDIT         812
#define IDM_PL_SAVE         813
#define IDM_PL_SAVE_AS      814
#define IDM_PL_PROPERTIES   815
#define IDM_PL_INFO         816
#define IDM_PL_INFOALL      817
#define IDM_PL_MENU         818 /* invoke main menu */
#define IDM_PL_MENUCONT     819 /* invoke record menu */ 
#define IDM_PL_FLATTEN      820
#define IDM_PL_FLATTEN_1    821
#define IDM_PL_FLATTEN_ALL  822
#define IDM_PL_SORT         823
#define IDM_PL_SORT_SIZE    824
#define IDM_PL_SORT_TIME    825
#define IDM_PL_SORT_URL     826
#define IDM_PL_SORT_SONG    827
#define IDM_PL_SORT_ALIAS   828
#define IDM_PL_SORT_ART     829
#define IDM_PL_SORT_ALBUM   830
#define IDM_PL_SORT_RAND    831
#define IDM_PL_SORT_SIZEALL 832
#define IDM_PL_SORT_TIMEALL 833
#define IDM_PL_SORT_URLALL  834
#define IDM_PL_SORT_SONGALL 835
#define IDM_PL_SORT_ALIASALL 836
#define IDM_PL_SORT_ARTALL  837
#define IDM_PL_SORT_ALBUMALL 838
#define IDM_PL_SORT_RANDALL 839
#define IDM_PL_SELECT_ALL   840
#define IDM_PL_CONTENT      841
//#define IDM_PL_CONTENT_ALL  842
#define IDM_PL_APPEND       900
#define IDM_PL_APPFILE      901
#define IDM_PL_APPURL       902
#define IDM_PL_APPCPL       903
#define IDM_PL_APPOTHER     904 /* Need some IDs for plugin extensions.     */
#define IDM_PL_APPENDALL    920
#define IDM_PL_APPFILEALL   921
#define IDM_PL_APPURLALL    922
#define IDM_PL_APPCPLALL    923
#define IDM_PL_APPOTHERALL  924 /* Need some IDs for plugin extensions.     */
#define IDM_PL_OPEN         940
#define IDM_PL_OPENLAST     941 /* A lot of IDs after this need to be free. */

#define MNU_SUBFOLDER       999

/* Helptable */
#define HLP_MAIN_TABLE      100
#define HLP_NULL_TABLE      101
//#define HLP_CONFIG_TABLE    102
#define HLP_MENU_TABLE      103

/* Help page IDs. DO NEVER CHANGE THAT */
#define IDH_MAIN           1000
//#define IDH_ADVANTAGES     1001
#define IDH_ANALYZER       1002
#define IDH_SUPPORT        1003
#define IDH_COPYRIGHT      1004
#define IDH_DRAG_AND_DROP  1005
#define IDH_EQUALIZER      1006
#define IDH_INFODIALOG     1007
#define IDH_INTERFACE      1008
#define IDH_MAIN_MENU      1009
#define IDH_MAIN_WINDOW    1010
#define IDH_PM             1011
#define IDH_NETSCAPE       1012
#define IDH_COMMANDLINE    1013
#define IDH_PL             1014
#define IDH_PROPERTIES     1015
#define IDH_REMOTE         1016
#define IDH_SKIN_GUIDE     1017
#define IDH_SKIN_UTILITY   1018
#define IDH_TROUBLES       1019
#define IDH_PLAYLISTFORMAT 1020
#define IDH_MIGRATE1_40    1021
#define IDH_LOCATION       1022
#define IDH_KEYBOARD       1023
#define IDH_ANALYZERFORMAT 1024
#define IDH_BOOKMARK       1025
#define IDH_PLAYFOLDER     1026
#define IDH_INSPECTOR      1027
#define IDH_ID3_EDITOR     1028
#define IDH_SHUFFLE        1029
#define IDH_SETTINGS1      1050
#define IDH_SETTINGS2      1051
#define IDH_SETTINGSIO     1052
#define IDH_SETTINGSPERF   1054
#define IDH_PLAYBACK       1053
#define IDH_DISPLAY        1055
#define IDH_DEC_PLUGINS    1060
#define IDH_FIL_PLUGINS    1061
#define IDH_OUT_PLUGINS    1062
#define IDH_VIS_PLUGINS    1063
#define IDH_MPG123         1070

/* Dialogs */

/* File */
#define DLG_FILE           1100
#define DLG_DIR            1101
#define CB_RECURSE         1105
#define CB_RELATIVE        1106

/* URL and Bookmark */
#define DLG_URL            1110
#define DLG_BM_ADD         1111
#define ENT_URL            1120
#define EF_DESC            1121

/* Properties */
#define DLG_CONFIG         1200
#define NB_CONFIG          1201

#define CFG_ABOUT          1210

#define ST_BUILT           1213
#define GB_AUTHORS         1214
#define ST_AUTHORS         1215
#define GB_CREDITS         1216
#define ST_CREDITS         1217

#define CFG_SETTINGS1      1250

#define CB_PLAYONLOAD      1260
#define CB_RETAINONEXIT    1261
#define CB_RETAINONSTOP    1262
#define CB_RESTARTONSTART  1263
#define CB_DISCARDSEED     1265

#define CB_TURNAROUND      1270
#define RB_SONGONLY        1271
#define RB_SONGTIME        1272
#define RB_TIMEONLY        1273
#define RB_ALTKEY          1275
#define RB_CTRLKEY         1276
#define RB_SHIFTKEY        1277

#define CFG_SETTINGS2      1300

#define CB_AUTOUSEPL       1310
#define CB_KEEPROOT        1311
#define CB_RECURSEDND      1312
#define CB_FOLDERSFIRST    1313
#define CB_AUTOSAVEPL      1314

#define RB_ITEMNAVTO       1315
#define RB_ITEMLOAD        1316
#define RB_ITEMQUEUE       1317
#define RB_DNDLOAD         1318
#define RB_DNDQUEUE        1319
#define RB_CMDLOAD         1320
#define RB_CMDQUEUE        1321
#define CB_QUEUEMODE       1325

#define CFG_IOSETTINGS     1350

#define EF_PIPE            1360

#define EF_PROXY_HOST      1370
#define EF_PROXY_PORT      1371
#define EF_PROXY_USER      1372
#define EF_PROXY_PASS      1373
#define SB_TIMEOUT         1375
#define SB_BUFFERSIZE      1376
#define CB_FILLBUFFER      1377
#define SB_FILLBUFFER      1378

#define CFG_PERFORMANCE    1400

#define SB_NUMWORKERS      1410
#define SB_DLGWORKERS      1411
#define CB_LOWPRIWORKERS   1412

#define CB_PRI_NORM        1420
#define CB_PRI_HIGH        1421
#define SB_PRI_LIMIT       1422

#define CFG_DISPLAY1       1500

#define CB_DOCK            1510
#define SB_DOCK            1511
#define CB_SAVEWNDPOSBYOBJ 1512
#define RB_SCROLL_INFINITE 1520
#define RB_SCROLL_ONCE     1521
#define RB_SCROLL_DISABLE  1522
#define CB_SCROLL_AROUND   1523
#define RB_DISP_FILENAME   1525
#define RB_DISP_ID3TAG     1526
#define RB_DISP_FILEINFO   1527
#define CB_RESTRICT_META   1528
#define SB_RESTRICT_META   1529
#define GB_FONT            1530
#define CB_USE_SKIN_FONT   1531
#define ST_FONT_SAMPLE     1532
#define PB_FONT_SELECT     1533

#define CFG_PLAYBACK       1550

#define CB_RG_ENABLE       1560
#define LB_RG_LIST         1561
#define LB_RG_AVAILABLE    1562
#define PB_RG_UP           1563
#define PB_RG_DOWN         1564
#define PB_RG_ADD          1565
#define PB_RG_REMOVE       1566
#define SB_RG_PREAMP       1567
#define SB_RG_PREAMP_OTHER 1568

#define SB_SCAN_SPEED      1470

#define CFG_DEC_CONFIG     1600
#define CFG_FIL_CONFIG     1601
#define CFG_OUT_CONFIG     1602
#define CFG_VIS_CONFIG     1603

#define LB_PLUGINS         1611
#define PB_PLG_UNLOAD      1612
#define PB_PLG_ADD         1613
#define PB_PLG_UP          1614
#define PB_PLG_DOWN        1615
#define PB_PLG_ENABLE      1616
#define PB_PLG_CONFIG      1618
#define ST_PLG_AUTHOR      1619
#define ST_PLG_DESC        1620
#define ST_PLG_LEVEL       1621

#define ST_DEC_FILETYPES   1631
#define ML_DEC_FILETYPES   1632
#define CB_DEC_TRYOTHER    1633
#define CB_DEC_SERIALIZE   1634
#define PB_PLG_SET         1639

/* Object Info */
#define DLG_INFO           1700
#define NB_INFO            1701

#define DLG_TECHINFO       1710
#define EF_URL             1711
#define EF_TIMESTAMP       1712
#define EF_FATTR           1713
#define EF_FILESIZE        1714
#define EF_TOTALSIZE       1715
#define EF_OBJTYPE         1716
#define CB_SAVEABLE        1717
#define CB_SAVESTREAM      1718
#define EF_TOTALTIME       1719
#define EF_NUMITEMS        1720
#define EF_SONGITEMS       1721
#define EF_LISTITEMS       1722
#define EF_BITRATE         1724
#define EF_SAMPLERATE      1725
#define EF_NUMCHANNELS     1727
#define EF_DECODER         1729
#define EF_FORMAT          1730
#define EF_INFOSTRINGS     1731
#define EF_METARPGAINT     1741
#define EF_METARPGAINA     1743
#define EF_METARPPEAKT     1746
#define EF_METARPPEAKA     1748

#define DLG_METAINFO       1750
#define EF_METATITLE       1752
#define CB_METATITLE       1753
#define EF_METAARTIST      1755
#define CB_METAARTIST      1756
#define EF_METAALBUM       1758
#define CB_METAALBUM       1759
#define EF_METATRACK       1761
#define CB_METATRACK       1762
#define EF_METADATE        1764
#define CB_METADATE        1765
#define EF_METAGENRE       1767
#define CB_METAGENRE       1768
#define EF_METACOMMENT     1770
#define CB_METACOMMENT     1771
#define EF_METACOPYRIGHT   1773
#define CB_METACOPYRIGHT   1774

#define DLG_ITEMINFO       1800
#define EF_INFOURL         1801
#define EF_INFOALIAS       1805
#define EF_INFOSTART       1806
#define EF_INFOSTOP        1807
#define EF_INFOAT          1808
#define EF_INFOPREGAP      1810
#define EF_INFOPOSTGAP     1811
#define EF_INFOGAIN        1812
#define CB_INFOALTERNATION 1820
#define RB_INFOPLSHINHERIT 1821
#define RB_INFOPLSHFORCE   1822
#define RB_INFOPLSHCLEAR   1823

/* Write meta data */
#define DLG_WRITEMETA      1900
#define EF_WMURL           1901
#define SB_WMBARFG         1902
#define SB_WMBARBG         1903
#define EF_WMSTATUS        1904
#define PB_WMSKIP          1906
#define PB_WMSKIPALL       1907

/* Inspector */
#define DLG_INSPECTOR      1950
#define ST_CONTROLLERQ     1951
#define LB_CONTROLLERQ     1952
#define ST_WORKERQ         1953
#define LB_WORKERQ         1954
#define LB_DEPENDENCIES    1955
#define CB_AUTOREFRESH     1961
#define SB_AUTOREFRESH     1962


/* SKINs ... */

/* Bitmap identifiers for action buttons. */
#define BMP_PLAY           2000 /* Play button down for regular mode.                 */
#define BMP_PAUSE          2001 /* Pause button down for regular mode.                */
#define BMP_REW            2002 /* Rewind button down for regular mode.               */
#define BMP_FWD            2003 /* Fast forward button down for regular mode.         */
#define BMP_POWER          2004 /* Power button down for regular mode.                */
#define BMP_PREV           2005 /* Previous button down for regular mode.             */
#define BMP_NEXT           2006 /* Next button down for regular mode.                 */
#define BMP_SHUFFLE        2007 /* Shuffle button down for regular mode.              */
#define BMP_REPEAT         2008 /* Repeat button down for regular mode.               */
#define BMP_PL             2009 /* Playlist button down for regular mode.             */
#define BMP_STOP           2010 /* Stop button down for regular mode.                 */
#define BMP_FLOAD          2011 /* Load file button down for regular mode.            */

#define BMP_N_PLAY         2020 /* Play button up for regular mode.                   */
#define BMP_N_PAUSE        2021 /* Pause button up for regular mode.                  */
#define BMP_N_REW          2022 /* Rewind button up for regular mode.                 */
#define BMP_N_FWD          2023 /* Fast forward button up for regular mode.           */
#define BMP_N_POWER        2024 /* Power button up for regular mode.                  */
#define BMP_N_PREV         2025 /* Previous button up for regular mode.               */
#define BMP_N_NEXT         2026 /* Next button up for regular mode.                   */
#define BMP_N_SHUFFLE      2027 /* Shuffle button up for regular mode.                */
#define BMP_N_REPEAT       2028 /* Repeat button up for regular mode.                 */
#define BMP_N_PL           2029 /* Playlist button up for regular mode.               */
#define BMP_N_STOP         2030 /* Stop button up for regular mode.                   */
#define BMP_N_FLOAD        2031 /* Load file button up for regular mode.              */

#define BMP_S_PLAY         2040 /* Play button down for small and tiny modes.         */
#define BMP_S_PAUSE        2041 /* Pause button down for small and tiny modes.        */
#define BMP_S_REW          2042 /* Rewind button down for small and tiny modes.       */
#define BMP_S_FWD          2043 /* Fast forward button down for small and tiny modes. */
#define BMP_S_POWER        2044 /* Power button down for small and tiny modes.        */
#define BMP_S_PREV         2045 /* Previous button down for small and tiny modes.     */
#define BMP_S_NEXT         2046 /* Next button down for small and tiny modes.         */
#define BMP_S_SHUFFLE      2047 /* Shuffle button down for small and tiny modes.      */
#define BMP_S_REPEAT       2048 /* Repeat button down for small and tiny modes.       */
#define BMP_S_PL           2049 /* Playlist button down for small and tiny modes.     */
#define BMP_S_STOP         2050 /* Stop button down for small and tiny modes.         */
#define BMP_S_FLOAD        2051 /* Load file button down for small and tiny modes.    */

#define BMP_SN_PLAY        2060 /* Play button up for small and tiny modes.           */
#define BMP_SN_PAUSE       2061 /* Pause button up for small and tiny modes.          */
#define BMP_SN_REW         2062 /* Rewind button up for small and tiny modes.         */
#define BMP_SN_FWD         2063 /* Fast forward button up for small and tiny modes.   */
#define BMP_SN_POWER       2064 /* Power button up for small and tiny modes.          */
#define BMP_SN_PREV        2065 /* Previous button up for small and tiny modes.       */
#define BMP_SN_NEXT        2066 /* Next button up for small and tiny modes.           */
#define BMP_SN_SHUFFLE     2067 /* Shuffle button up for small and tiny modes.        */
#define BMP_SN_REPEAT      2068 /* Repeat button up for small and tiny modes.         */
#define BMP_SN_PL          2069 /* Playlist button up for small and tiny modes.       */
#define BMP_SN_STOP        2070 /* Stop button up for small and tiny modes.           */
#define BMP_SN_FLOAD       2071 /* Load file button down for small and tiny modes.    */

#define BMP_LED            2100 /* Bright led (displayed when PM123 has focus).       */
#define BMP_N_LED          2101 /* Dark   led (displayed when PM123 is not focused).  */
#define BMP_R_BGROUND      2104 /* Background bitmap for regular mode.                */
#define BMP_S_BGROUND      2105 /* Background bitmap for small mode.                  */
#define BMP_T_BGROUND      2106 /* Background bitmap for tiny mode.                   */
#define BMP_SLIDER         2107 /* Seek slider handle.                                */
#define BMP_ALTSLIDER      2108 /* Seek slider handle.                                */
#define BMP_SLIDER_SHAFT   2109 /* Slider shaft (drawn before drawing slider handle). */
#define BMP_NO_CHANNELS    2110 /* No channels (unknown, no file loaded) mode.        */
#define BMP_STEREO         2111 /* Stereo mode.                                       */
#define BMP_MONO           2112 /* Mono mode.                                         */
#define BMP_VOLSLIDER      2113 /* Volume slider handle bitmap (only if enabled).     */
#define BMP_VOLBAR         2114 /* Volume bar.                                        */
#define BMP_SINGLEPLAY     2115 /* Indicator for single file play.                    */
#define BMP_LISTPLAY       2116 /* Indicator for playlist play.                       */
#define BMP_NOFILE         2117 /* Indicator for no file loaded.                      */
#define BMP_NOTL           2120 /* "Time left", dark.                                 */
#define BMP_TL             2121 /* "Time left", bright.                               */
#define BMP_NOPLIST        2122 /* "Playlist left", dark.                             */
#define BMP_PLIST          2123 /* "Playlist left", bright.                           */
#define BMP_BPS            2180 /* Bitmaps for bitrates 0 (no bitrate), 32, 48, 56,   */
                                /* 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256 */
                                /* Not used if resource UL_BPS_DIGITS is enabled.     */

#define DIG_SMALL          2200 /* Digits 0-9 for ???                                 */
#define DIG_BIG            2220 /* Digits 0-9 for the main ("big") timer.             */
#define DIG_TINY           2240 /* Digits 0-9 for time and playlist left timers.      */
#define DIG_PL_INDEX       2260 /* Digits 0-9 for playlist total/index displays.      */
#define DIG_BPS            2280 /* Digits 0-9 for bitrates.                           */

#define BMP_FONT1          2400 /* Font 1.                                            */
#define BMP_FONT2          2600 /* Font 2.                                            */


#endif
