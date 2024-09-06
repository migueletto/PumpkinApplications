/*
 *	file:		fm.h
 *	project:	GentleMan
 *	content:	types, globals, etc...
 *	updated:	Oct. 30. 2001
 *
 * copyright: Collin R. Mulliner <palm@mulliner.org>
 * web: www.mulliner.org/palm/
 *
 */

/*
 *  This file is part of GentleMan.
 *
 *  GentleMan is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GentleMan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GentleMan; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FM_H__
#define __FM_H__

//#define ALLOW_ACCESS_TO_INTERNALS_OF_SCROLLBARS

#include <PalmOS.h>
#include <ExpansionMgr.h>
#include <VFSMgr.h>
//#include "special_headers/SonyChars.h"
//#include <TrgChars.h>
//#include <Vga.h>
//#include <Silk.h>
//#include <Audio.h>
//#include <power.h>
//#include "special_headers/callback.h"
#include "special_headers/multicode.h"
#include "fm_rcp.h"
#include "GMPlugin.h"

// -= constants =-

// -= MemPlug (www.memplug.com) =-
#define MEMPLUG_LABEL_STR "MEMPLUG"
#define MEMPLUG_LABEL_LENG 7

// font id
#define MINI_DATE_4x11_FONT 128

// creatorID
#define CRID 'CRMF'

// file copy/move destination bit fields
#define destVfs	0x0001
#define destCard	0x0002
#define destCardVfs (destVfs | destCard)

// custom event for closing forms
#define GMCloseFormEvent (sysEventFirstUserEvent + 1)

// -= MACROS =-
#define BIG(x) ((double)x * (double)1.5)


// -= structures =-

// what to do on a double tap (Vfs)
typedef enum {
	doubleTapDetails = 0,
	doubleTapRunPlugin
} doubleTapActionVfsType;

// what to do on a double tap (Card)
typedef enum {
	doubleTapDetailsGeneral = 0,
	doubleTapDetailsAttributes
} doubleTapActionCardType;
	
// browser modes
typedef enum {
	brwModeVfs = 1,
	brwModeCard
} brwModeType;

// card RAM or ROM
typedef enum {
	cardTypeRom = 1,
	cardTypeRam
} cardTypeType;

// screen states
typedef enum {
	screen160x160 = 1,
	screen240x240,
	screen240x300
} screenModeType;

// vfs show item type
typedef enum {
	vfsShowSize = 0,
	vfsShowAttr,
	vfsShowCrDate,
	vfsShowMoDate,
	vfsShowAcDate,
	vfsShowCrTime,
	vfsShowMoTime,
	vfsShowAcTime,
	vfsShowNone
} vfsShowType;

// card show item type
typedef enum {
	cardShowTypeID = 0,
	cardShowCrid,
	cardShowSize,
	cardShowRecs,
	cardShowRW,
	cardShowVer,
	cardShowMNum,
	cardShowCrDate,
	cardShowMoDate,
	cardShowBkDate,
	cardShowCrTime,
	cardShowMoTime,
	cardShowBkTime,
	cardShowNone
} cardShowType;

// vfs sort item type
typedef enum {
	vfsSortNameAscending = 0,
	vfsSortNameDescending,
	vfsSortSizeAscending,
	vfsSortSizeDescending,
	vfsSortcDateAscending,
	vfsSortcDateDescending,
	vfsSortmDateAscending,
	vfsSortmDateDescending,
	vfsSortaDateAscending,
	vfsSortaDateDescending
} vfsSortType;

// card sort item type
typedef enum {
	cardSortNameAscending = 0,
	cardSortNameDescending,
	cardSortSizeAscending,
	cardSortSizeDescending,
	cardSortTypeIdAscending,
	cardSortTypeIdDescending,
	cardSortCridAscending,
	cardSortCridDescending,
	cardSortcDateAscending,
	cardSortcDateDescending,
	cardSortmDateAscending,
	cardSortmDateDescending,
	cardSortbDateAscending,
	cardSortbDateDescending
} cardSortType;
	
// database info
typedef struct {
	UInt16 cardNo;
	LocalID lID;
	UInt32 typeId;
	UInt32 crid;
	UInt16 attribs;
	UInt16 version;
	UInt32 modNum;
	UInt32 crDate;
	UInt32 moDate;
	UInt32 acDate;
	UInt32 size;
	UInt32 numRecs;
	// database selected in current list
	Boolean selected;
	char name[32];
} dbInfoType;

// vfs file info
typedef struct {
	UInt32 attribs;
	UInt32 size;
	UInt32 crDate;
	UInt32 moDate;
	UInt32 acDate;
	// file selected in current list
	Boolean selected;
	// filename
	char *name;
} fileInfoType;

// vfs volume info
typedef struct {
	// volume reference number
	UInt16 volRefNum;
	// volume info
	VolumeInfoType volInfo;
	// size of volume in bytes
	UInt32 volSize;
	// bytes used on volume
	UInt32 volUsed;
	// volume label
	char label[256];
} myVolumeInfoType;

// vfs file list
typedef struct {
	// pointer to list of files in current dir
	fileInfoType *fileList;
	// number of files in list
	UInt16 numFiles;
	// volume info
	myVolumeInfoType volInfo;
	// num selected files
	UInt16 numSelFiles;
	UInt32 sizeSelFiles;
	// current dir
	char currentDir[2048];
	// name ptrs
	char *fileNames;
	UIntPtr fileNamesSize;
} fileListType;

// mem card info
typedef struct {
	// card type
	cardTypeType typeId;
	// card number
	UInt16 cardNo;
	// size in bytes
	UInt32 ramSize;
	// unused bytes
	UInt32 ramUsed;
	// creation date
	UInt32 crDate;
	// version number
	UInt16 version;
	// manufacturer
	char manuf[33];
	// card label
	char label[33];
} myCardInfoType;

// database list
typedef struct {
	// card info
	myCardInfoType cardInfo;
	// number of dbs
	UInt16 numDBs;
	// pointer to list of dbs
	dbInfoType *dbList;
	// num selected dbs
	UInt16 numSelDBs;
	// size of all selected DBs
	UInt32 sizeSelDBs;
} dbListType;

// list
typedef struct {
	brwModeType mode;
	union {
		fileListType vol;
		dbListType card;
	} data;
} listInfoType;

// volume list item
typedef struct {
	brwModeType mode;
	union {
		myVolumeInfoType vol;
		myCardInfoType card;
	} data;
} volListItemType;

// volume list
typedef struct {
	// number of volumes
	UInt16 numVols;
	// list data
	volListItemType *list;
} volListType;

// browser status
typedef struct {
	// file list for this browser
	listInfoType lst;
	// index to the current volume for this browser
	UInt16 volIndex;
	// this is used for showing the lists
	brwModeType brwMode;
	// sort selection
	Int16 sort;
	// show1 selection
	Int16 show1;
	// show2 selection
	Int16 show2;
	// scroll position for scrollbar
	Int16 scrollPos;
} brwLstType;

// plugin type
typedef struct {
	LocalID lID;
	UInt16 cardNo;
	char name[100];
	char extensions[50];
} pluginInfoType;

// plugin list type
typedef struct {
	UInt16 numPlugins;
	pluginInfoType *list;
} pluginListType;

// details from data
typedef struct {
	// details type
	brwModeType mode;
	// single or multiple file/db
	Boolean single;
	
	union {
		fileInfoType vol[2];
		dbInfoType card[2];
	} data;
	
	// filename for vol(vfs) mode
	char name[2][256];
	
	MemHandle nameFldHand;
	MemHandle typeFldHand;
	MemHandle cridFldHand;
	
	// general or attribs
	Boolean general;
} detailsType;

// copy/move form data
typedef struct {
	// only one (1) source file/database!
	Boolean single;
	// destination file name changeable
	Boolean changeName;
	
	// type of destination
	UInt16 destType;
	// type of source
	UInt16 sourceType;
	
	// index of destination browser
	UInt16 destBrowser;
	
	// destination name (and path)
	MemHandle destName;	
} copyType;

// app prefs
// this will be the structure that will be saved as the app prefs
typedef struct {
	// ask before deleting a file/db
	Boolean askDelete;
	// when deleting multiple files/dbs ask only once
	Boolean askDeleteOnlyOnce;

	// superUser? if false bring up more warnings!!!
	Boolean superUserMode;
	
	// do what on double-tap
	doubleTapActionVfsType dTapVfs;
	doubleTapActionCardType dTapCard;
	
	// default stuff for CARD browsers
	cardSortType defaultCardSort;
	cardShowType defaultCardShow1;
	cardShowType defaultCardShow2;
	
	// default stuff for VFS browsers
	vfsSortType defaultVfsSort;
	vfsShowType defaultVfsShow1;
	vfsShowType defaultVfsShow2;
} prefsType;

// runtime configuration
typedef struct {
	// screen Type
	screenModeType screenState;
	// true if form was modified
	Boolean formModified;
	
	// number of files to display
	UInt16 numDisplay;
	
	// xStart, xEnd, ...
	UInt16 xS, yS, xE, yE;
	
	// attrib show pos
	UInt16 sX1, sX2;

	// height in pixels of the font with id 0
	UInt8 font0Height;
	// font 0
	FontID font0;

	// font id for date / time view !
	FontID dateTimeFont;
		
	UInt16 nameLengthShort;	// pixel
	UInt16 nameLengthLong;	// pixel
	
	UInt16 scrollBarLength;	// pixels
	
	UInt16 selInfoStrX;
	UInt16 selInfoStrY;
	
	UInt16 volListStrX;
	UInt16 volListStrY;
	UInt16 volListStrLeng;
	
	RectangleType volListArea;
	RectangleType selInfoArea;
	RectangleType dirInfoArea;
		
	// OLD screen depth/color/etc..
	UInt32 oldDepth;
	Boolean oldColor;
	
} runTimeDataType;

// GLOBAL DATA TYPE
typedef struct {
	// -= prefs =-
	prefsType prefs;
	runTimeDataType cfg;
	
	// -= browser stuff =-
	// volume list
	volListType vLst;
	// the 4 file browsers
	brwLstType brwLst[4];
	// current browser
	UInt8 currentBrw;
	// number of browsers
	UInt8 numBrw;
	
	// -= just global stuff =-
	Int16 pos;
	Boolean isInverted;
	Int8 dirTapLast;
	UInt32 dirTapTicks;
	
	// -= font stuff =-
	FontPtr font128;
		
	// --= other forms global data =--
	detailsType *details;
	copyType *copy;
	
	// -= plugin stuff =-
	pluginListType pluginLst;	
} global_data_type;


// -= globals =-

extern global_data_type *gglobals;
#define glob gglobals

#endif

// -= prototypes =-

// fm_2-file_db.c
UInt8 getFilesInDir(fileListType *flst) CODE2_SECTION;
UInt8 getDBList(dbListType *dblst) CODE2_SECTION;
UInt8 makeVolList(volListType *lst) CODE2_SECTION;
void refreshVolumeInfo(volListType *lst) CODE2_SECTION;

// fm_3-init.c
void initRunTimeData(runTimeDataType *cfg) CODE2_SECTION;
void deInitRunTimeData(runTimeDataType *cfg) CODE2_SECTION;
void initBrowsers(global_data_type *glb) CODE2_SECTION;
void init(void) CODE2_SECTION;
void deinit() CODE2_SECTION;
// --= they should be in the main code section ... just to be on the save side
Err myNotifyRemoveHandler(SysNotifyParamType *notifyParamsP);
Err myNotifyInsertHandler(SysNotifyParamType *notifyParamsP);
// --=
void setDisplayStuff(runTimeDataType *cfg, brwModeType mode) CODE2_SECTION;
void handleResizeDisplayEvent() CODE2_SECTION;
Boolean checkPalmOSVersion() CODE2_SECTION;
void beamGentleMan() CODE2_SECTION;
Boolean checkBetaDate() CODE2_SECTION;

// fm_4-gui.c
void setUpVolList(volListType *lst, Int16 index) CODE2_SECTION;
void myVolListDraw(Int16 item, RectangleType *rect, char **text) CODE2_SECTION;
void setUpSortShow(brwLstType *brw) CODE2_SECTION;
void doShowPopupList(brwLstType *brw, UInt16 id) CODE2_SECTION;
void doVolPopupList(brwLstType *brw, volListType *lst) CODE2_SECTION;
void *GetObjectPtr(UInt16 id) CODE2_SECTION;
void doSortPopupList(brwLstType *brw) CODE2_SECTION;
void setUpFileBrowser(brwLstType *brw) CODE2_SECTION;
void makeVolInfo(volListItemType *vInfo, char *drawStr, UInt8 length) CODE2_SECTION;

void doBrwSelect(global_data_type *glb, UInt16 id) CODE2_SECTION;

// fm_5-filelist.c
Boolean handleKeyDown(brwLstType *brw, global_data_type *glb, EventPtr event) CODE2_SECTION;
Boolean handlePenDown(brwLstType *brw, global_data_type *glob, UInt16 x, UInt16 y) CODE2_SECTION;
Boolean handlePenUp(brwLstType *brw, global_data_type *glob, UInt16 x, UInt16 y) CODE2_SECTION;
Boolean handlePenMove(brwLstType *brw, global_data_type *glob, UInt16 x, UInt16 y) CODE2_SECTION;
Boolean handleDoubelTapVfs(brwLstType *brw, global_data_type *glb) CODE2_SECTION;
void handleDoubelTapCard(brwLstType *brw, global_data_type *glb) CODE2_SECTION;
		
void makeDisplayName(char *in, char *out, UInt8 length, UInt32 attribs) CODE2_SECTION;
void makeDisplaySize(UInt32 size, char *out) CODE2_SECTION;
void makeDisplayAttr(UInt32 attribs, char *out) CODE2_SECTION;
void makeDisplayDate(UInt32 date, char *out) CODE2_SECTION;
void makeDisplayTime(UInt32 date, char *out) CODE2_SECTION;

void drawDirectoryInfo(brwLstType *brw) CODE2_SECTION;
void drawSelectedInfo(brwLstType *brw) CODE2_SECTION;
UInt8 drawFileList(brwLstType *brw) CODE2_SECTION;
void doRunPlugin(brwLstType *brw, pluginListType *pluginLst) CODE2_SECTION;

// fm_6-anim.c
void inProgress(UInt16 num, UInt8 doWhat, WinHandle *bg, screenModeType screenState, char *title) CODE2_SECTION;

// fm_7-sorting.c
void sort(brwLstType *brw) CODE2_SECTION;
Int16 doCompareVol(fileInfoType *file1, fileInfoType *file2, vfsSortType sortItem) CODE2_SECTION;
Int16 doCompareCard(dbInfoType *db1, dbInfoType *db2, cardSortType sortItem) CODE2_SECTION;

// fm_8-details.c
void doDetails(global_data_type *glb, UInt32 pos, Boolean usePos) CODE3_SECTION;
void freeDetails(global_data_type *glb, Boolean card) CODE3_SECTION;
void preFormatNumberStr(char *in) CODE3_SECTION;

void showHideCardDetailsButton(Boolean show) CODE3_SECTION;
void drawCardDetailsDateTime(detailsType *details, screenModeType screenState) CODE3_SECTION;
void showCardDetails(Boolean general, Boolean single, screenModeType screenState, Boolean hide) CODE3_SECTION;
void setUpCardDetails(detailsType *details, screenModeType screenState) CODE3_SECTION;
Boolean getNewDateCardDetails(detailsType *details, UInt16 controlID) CODE3_SECTION;
Boolean getNewTimeCardDetails(detailsType *details, UInt16 controlID) CODE3_SECTION;
void getCardDetailsAttrName(detailsType *details, Boolean general) CODE3_SECTION;
Err doChangeCardDetails(global_data_type *glb) CODE3_SECTION;

void getVolDetailsAttrName(detailsType *details) CODE3_SECTION;
void drawVolDetailsDateTime(detailsType *details, screenModeType screenState) CODE3_SECTION;
void setUpVolDetails(detailsType *details, screenModeType screenState) CODE3_SECTION;
void showHideVolDetailsButton(Boolean show) CODE3_SECTION;
Boolean getNewTimeVolDetails(detailsType *details, UInt16 controlID) CODE3_SECTION;
Boolean getNewDateVolDetails(detailsType *details, UInt16 controlID) CODE3_SECTION;
Err doChangeVolDetails(global_data_type *glb) CODE3_SECTION;

// fm_9-files.c
Err doMkDir(global_data_type *glb) CODE2_SECTION;
void doDelete(brwLstType *brw, prefsType *prefs) CODE2_SECTION;
Err deleteStuff(brwLstType *brw, prefsType *prefs) CODE2_SECTION;
Err doDeleteDir(char *pathname, UInt16 volRefNum) CODE2_SECTION;

void doUnSelect(brwLstType *brw, Boolean select) CODE2_SECTION;

void doBeam(brwLstType *brw) CODE2_SECTION;
Err myDBWrite(const void* data, UInt32 *size, void *userdata) CODE2_SECTION;
Err beamDB(UInt16 cNo, LocalID lID, char *name, char *displayname) CODE2_SECTION;
Err beamFile(UInt16 refNum, char *fullpath, char *name, UInt32 size) CODE2_SECTION;

// fm_10-copy.c
void myDestListDraw(Int16 item, RectangleType *rect, char **text) CODE2_SECTION;
void makeVolIndexLst(UInt16 destType, volListType *vLst, brwLstType *brw, copyType *copyData) CODE2_SECTION;

UInt16 analyzeSourceFiles(brwLstType *brw) CODE2_SECTION;

Err copyFiles(global_data_type *glb) CODE2_SECTION;
Boolean doFileCopy(global_data_type *glb) CODE2_SECTION;

void doCopy(global_data_type *glb) CODE2_SECTION;
Err initCopy(global_data_type *glb) CODE2_SECTION;
void setUpCopyForm(global_data_type *glb) CODE2_SECTION;
void freeCopy(global_data_type *glb) CODE2_SECTION;

Err copyVfsToVfs(char *src, UInt16 srcRefNum, UInt32 srcSize, char *dest, UInt16 destRefNum, Boolean overwrite) CODE2_SECTION;
Err copyVfsDirToVfs(char *src, UInt16 srcRefNum, char *dest, UInt16 destRefNum) CODE2_SECTION;

// fm_11-prefs.c
void setupPrefsLists(global_data_type *glb);
void getPrefsLists(global_data_type *glb);
void loadSavePrefs(prefsType *prefs, Boolean save);

// fm_12-plugin.c
void myPluginListDraw(Int16 item, RectangleType *rect, char **text);
void makePluginList(pluginListType *lst);
Err runPlugin(pluginListType *pluginLst, UInt16 volRefNum, char *fullpath);
void initPluginMgr(global_data_type *glb);
void deInitPluginMgr(global_data_type *glb);
void runAboutPlugin(pluginListType *pluginLst, UInt16 index);
void drawPluginInfo(pluginListType *pluginLst, Int16 index, screenModeType screenState);


extern void setUpBrwList(global_data_type *glb);
