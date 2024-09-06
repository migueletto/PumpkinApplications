/****************************************************************************
 FileZ
 Copyright (C) 2005  Tom Bulatewicz, nosleep software

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/

/** @file
 * File manager app for PalmOS.
 *
 * Created on 4/7/00 by Tom Bulatewicz
 */
 
// All of my defined constant values should be zero-based, this is because
//	all the error chcking assumes that zero is an ok number.

#ifndef __main_h__
#define __main_h__

#define DEBUG	0                       // enables various debugging stuff

#include "Sections.hpp"
#include "PreferencesForm.h"

#define sysVersion30	sysMakeROMVersion(3,0,0,sysROMStageRelease,0)
#define sysVersion31	sysMakeROMVersion(3,1,0,sysROMStageRelease,0)
#define sysVersion35	sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define ourMinVersion	sysVersion30

#define appCreatorID       'Filz'      // creator id
#define appVersionNum		0x01        // some version numbers
#define appPrefID          0x00
#define appPrefVersionNum	0x06

#define maxColorTypes		20				// max number of file type colors
#define maxFilterString		64          // max length of a filter string

#define viewBar		0                 // these are the states of the memory/battery in menu
#define viewPercent	1
#define viewNum		2

#define selectAllFiles     1
#define selectAllFolders   2
#define selectAllBoth      3

#define folderSelectName   1
#define folderSelectIcon   2

extern Boolean deferSleep;

/**
 * The preferences struct just for the file list preferences.
 */
typedef struct
	{
	TypeColorType	typeColors;
	UInt8          sortOrder;
	UInt8          column;
	Boolean			hideROM;
	UInt8          selectAll;
   RGBColorType   rowColor;
   UInt8          foldersFirst;
   UInt8          folderSelect;
	} ListPrefType;


/**
 * The preferences struct just for the filter preferences.
 */
typedef struct
	{
	Char			string[maxFilterString];
	UInt8			comparator;
	UInt8			criteria;
	UInt8			notbox;
	UInt8			reserved2;
   UInt16      attr;
	} FilterPrefType;


/**
 * The general preferences struct for the application.
 */
typedef struct
	{
	UInt16			lastForm;
	UInt16			lastCard;
	UInt8          memoryView;
	UInt8          batteryView;
	UInt8          sendMenu;
	UInt8          reserved2;	
	ListPrefType	list;
	FilterPrefType	filter;
	} PrefType;


/**
 * Externally defined variables.
 */
extern PrefType			prefs;						// The preferences for the application.
extern DateFormatType	ShortDateFormat;
extern DateFormatType	LongDateFormat;


#endif
