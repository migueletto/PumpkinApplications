/****************************************************************************
 Common Code
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
 * This just has some really useful functions for all kinds of stuph.
 *
 * Created on 9/18/2000 by Tom Bulatewicz
 */

#ifndef _stuph_h_
#define _stuph_h_

#include "SectionsCmn.hpp"

// string-related functions
void StrCopyTry( Char *dest, Char *src, UInt16 destSize ) SECT_CMN;
UInt32 StrToInt( Char *str ) SECT_CMN;
Boolean StrTail( Char *str, Char *tail ) SECT_CMN;
float StrAToF( char *strP ) SECT_CMN;
void StrFToA( char *strP, float value, UInt8 frac ) SECT_CMN;
void IntToStr( UInt32 theID, Char *theStr ) SECT_CMN;
Char* SubstituteStr( Char* str, const Char* token, Char* sub, UInt16 subLen ) SECT_CMN;
void SizeToString( UInt32 filesize, char *str, UInt8 charLimit ) SECT_CMN;
void attrToString( Char str[], UInt16 attr ) SECT_CMN;

// error checking
void checkMemPtr( MemPtr ptr, const Char *method ) SECT_CMN;
void checkMemPtr( MemHandle handle, const Char *method ) SECT_CMN;
void checkError( Err err, const Char *method, const Char *extra ) SECT_CMN;

// filename checking code
Boolean hasInvalidVFSCharacters( Char *filename ) SECT_CMN;
Err checkForInvalidVFSCharacters( Char *name, int alert, int cancel ) SECT_CMN;

// hexadecimal/decimal conversion
void ToHex( UInt8 num, Char ch[] ) SECT_CMN;
UInt8 ToDec( Char ch ) SECT_CMN;
void DecToHex( UInt32 decValue, Char hexStr[], Boolean pad ) SECT_CMN;
//static UInt16 Raise( UInt8 pow ) SECT_CMN;
UInt16 HexToDec( Char in[] ) SECT_CMN;

// misc
void showMessage( const Char *message ) SECT_CMN;
void showMessage( const Char *message, const Char *extra ) SECT_CMN;
Boolean SelectATime( Int16 *hour, Int16 *minute, const Char * titleP ) SECT_CMN;
Err VFSFileCopy( UInt16 srcVol, Char *srcPath, UInt16 destVol, Char *destPath, Err (*callBack)(UInt32 total, UInt32 offset, void *user), void *userData ) SECT_CMN;
DateType GetTodaysDate() SECT_CMN;
//static void cvt_ultoa( unsigned long value, char *strP ) SECT_CMN;
//static unsigned long cvt_atoul( char* strP ) SECT_CMN;
void getOSVersion( Char str[] ) SECT_CMN;
void SendStringToMemoPad( Char *title, Char *content ) SECT_CMN;
void DirtyRecord( DmOpenRef db, UInt16 index ) SECT_CMN;
void PleaseWait( UInt16 c, UInt16 m, UInt32 colorDepth ) SECT_CMN;
Err sendDB( LocalID dbID, UInt32 cardNo, Boolean showMenu ) SECT_CMN;

// do not put the following prototype inside a section
Err RomVersionCompatible( UInt32 requiredVersion, UInt16 launchFlags );

/**
 * A class to make getting a resource string easy.
 */
class ResString
	{
	MemHandle	resH;
	Char			*string;
	
	public:

	ResString( UInt16 resourceID )
		{
		resH = DmGetResource( strRsc, resourceID );
		string = (Char*)MemHandleLock( resH );
		}

	Char *GetString()
		{
		return string;
		}

	~ResString()
		{
		MemHandleUnlock( resH );
		}
	};

#endif
