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


#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "Stuph.h"
#include "UI.h"

#include "ResourceCmn.h"		// only for the selecttimeform
#include "VFSMgr.h"        // for the error check method

#define size1k 				1024
#define size1m 				1048576
#define size10m 				size1m*10
#define defaultTimeButton	SelectTimeHoursPushButton


/**
 * Convert the attributes to a displayable string.
 *
 * @param the resulting string
 * @param the type
 */
void attrToString( Char str[], UInt16 attr )
	{
	StrCopy( str, "" );

	if( attr & dmHdrAttrResDB )			StrCat( str, "r" );
	if( attr & dmHdrAttrReadOnly )			StrCat( str, "o" );
	if( attr & dmHdrAttrAppInfoDirty )		StrCat( str, "a" );
	if( attr & dmHdrAttrOKToInstallNewer )	StrCat( str, "i" );
	if( attr & dmHdrAttrResetAfterInstall )	StrCat( str, "e" );
	if( attr & dmHdrAttrStream )			StrCat( str, "s" );
	if( attr & dmHdrAttrOpen )				StrCat( str, "p" );	
	}


// this copies as much of the src string as possible into the destination
void StrCopyTry( Char *dest, Char *src, UInt16 destSize )
   {
   if( !dest || !src )
      return;
   
   if( StrLen( src ) >= destSize )
      {
      StrNCopy( dest, src, destSize-1 );
      dest[destSize-1] = 0;
      }
   else
      {
      StrCopy( dest, src );
      }
   }


/**
 * Given a filesize in bytes, convert it to a string outstring is either
 * in bytes, kilobytes, or megabytes.
 *
 * @param filesize number of bytes
 * @param str output string
 */
void SizeToString( UInt32 filesize, char *str, UInt8 charLimit )
	{
   for( int decimals=3; decimals>=0; decimals-- )
      {
      // If the size is less then a kilobyte, then just show the byte count
      if( filesize < 1000 )
         {
         StrIToA( str, filesize );
         StrCat( str, " B" );
         }

      else if( filesize < 1024000 )	
         {
         float size = (float)filesize / (float)size1k;
         if( !size ) size = 1;

         StrFToA( str, size, decimals );
         StrCat( str, " k" );					
         }

      else
         {
         float size = (float)filesize / (float)size1m;
         if( !size ) size = 1.0;
         StrFToA( str, size, decimals );
         StrCat( str, " m" );					
         }
      
      if( StrLen( str ) <= charLimit )
         break;
      }
   }


Err VFSFileCopy( UInt16 srcVol, Char *srcPath, UInt16 destVol, Char *destPath, Err (*callBack)(UInt32 total, UInt32 offset, void *user), void *userData )
	{
	FileRef	srcFile, destFile;
	UInt32	bufSize = 16*(1024);			// a 16k buffer for copying, not sure how fast it is on actual device...
	UInt32	bytesRead, bytesWritten;
	Char     *buf;
	Boolean	more=true;
	Err		err = errNone;
   UInt32   attributes = 0;
   
	buf = (Char*)MemGluePtrNew( bufSize );
   checkMemPtr( buf, "VFSFileCopy() a" );
   
	// create the new file
	err = VFSFileCreate( destVol, destPath );
   if( err == vfsErrFileAlreadyExists )
      {
      goto end;
      }

   if( err == vfsErrVolumeFull )
      {
      FrmAlert( VolumeFullAlert );
      err = errNone;                   // skip any more messages
      goto end;
      }
   checkError( err, "VFSFileCopy() b", 0 );
		
	// then open the source and destination files
	err = VFSFileOpen( destVol, destPath, vfsModeWrite, &destFile );
	if( err != errNone )
		{
		showMessage( "Unable to open destination file.", destPath );
		goto end;
		}
			
	err = VFSFileOpen( srcVol, srcPath, vfsModeRead, &srcFile );
	if( err != errNone )
		{
		showMessage( "Unable to open source file.", srcPath );
		err = VFSFileDelete( destVol, destPath );
		checkError( err, "VFSFileCopy() c", destPath );
      goto end;
		}

   // copy the attributes
   err = VFSFileGetAttributes( srcFile, &attributes );
   checkError( err, "VFSFileCopy() c.1", srcPath );
   
   // the volume label, directory, and link attributes can't be set, so
   //  we remove them here if they are present.
   attributes &= ~vfsFileAttrVolumeLabel;
   attributes &= ~vfsFileAttrDirectory;
   attributes &= ~vfsFileAttrLink;

   err = VFSFileSetAttributes( destFile, attributes );
   checkError( err, "VFSFileCopy() c.2", destPath );
		
	// loop through and read from source and write to destination
	while( more )
		{
      callBack( 0, 0, userData );
      
		err = VFSFileRead( srcFile, bufSize, buf, &bytesRead );			
		if( err == vfsErrFileEOF )				// end of the file
			{
			err = errNone;
			more = false;
			}

		if( err != errNone )								// some other error
			{
         checkError( err, "VFSFileCopy() d", srcPath );
			goto end;
			}
			
		err = VFSFileWrite( destFile, bytesRead, buf, &bytesWritten );
      if( err == vfsErrVolumeFull )
         {
         // if the volume is now full, we just close up the files and delete
         //  whatever part of the destination file we've copied so far.
         err = VFSFileClose( srcFile );               // close the files
         checkError( err, "VFSFileCopy() e", destPath );
         err = VFSFileClose( destFile );              
         checkError( err, "VFSFileCopy() f", destPath );
         
         err = VFSFileDelete( destVol, destPath );    // delete the file
         checkError( err, "VFSFileCopy() g", destPath );
         
         // we leave the error to get propagated up so that the source file is
         //  not deleted on a move operation that runs out of space.
         
         goto end;
         }
		checkError( err, "VFSFileCopy() h", 0 );
      if( err != errNone )
         goto end;
		}
		
	// close up the files
	err = VFSFileClose( srcFile );
   checkError( err, "VFSFileCopy() i", srcPath );
	err = VFSFileClose( destFile );
   checkError( err, "VFSFileCopy() j", destPath );
	
   end:
	
   if( buf )
      {
      Err merr = MemPtrFree( buf );
      checkError( merr, "VFSFileCopy() k", 0 );
      buf = 0;
      }
      
	return err;
   }


/**
 * Convert a 1 digit hexadecimal value to a decimal digit.
 *
 * @param num the hex digit
 * @param ch the decimal digit
 */
void ToHex( UInt8 num, Char ch[] )
	{
	ch[0] = ch[1] = 0;
	switch( num )
		{
		case 0:	ch[0] = '0';	break;
		case 1:	ch[0] = '1';	break;
		case 2:	ch[0] = '2';	break;
		case 3:	ch[0] = '3';	break;
		case 4:	ch[0] = '4';	break;
		case 5:	ch[0] = '5';	break;
		case 6:	ch[0] = '6';	break;
		case 7:	ch[0] = '7';	break;
		case 8:	ch[0] = '8';	break;
		case 9:	ch[0] = '9';	break;
		case 10:	ch[0] = 'A';	break;
		case 11:	ch[0] = 'B';	break;
		case 12:	ch[0] = 'C';	break;
		case 13:	ch[0] = 'D';	break;
		case 14:	ch[0] = 'E';	break;
		case 15:	ch[0] = 'F';	break;
		}
	}


/**
 * Convert a 1 digit decimal value to a hex character.
 *
 * @param ch the hex digit
 * @return the decimal digit
 */
UInt8 ToDec( Char ch )
	{
	switch( ch )
		{
		case '0':	return 0;
		case '1':	return 1;
		case '2':	return 2;
		case '3':	return 3;
		case '4':	return 4;
		case '5':	return 5;
		case '6':	return 6;
		case '7':	return 7;
		case '8':	return 8;
		case '9':	return 9;
		case 'A':
		case 'a':	return 10;
		case 'B':
		case 'b':	return 11;
		case 'C':
		case 'c':	return 12;
		case 'D':
		case 'd':	return 13;
		case 'E':
		case 'e':	return 14;
		case 'F':
		case 'f':	return 15;
		}
	
	return 0;
	}


/**
 * Convert a decimal value to a hexadecimal value. It will pad the final hex
 * string (if only a single character) with a leading 0 if the pad flag is set.
 *
 * Be sure to only pass in UInt32 data types into this function. If you have a
 * Char, Int8, UInt8, etc, make sure you properly cast it into a UInt32, and
 * then pass that UInt32 value into this function - do not just cast it to a
 * UInt32 in the call itself.
 *
 * @param pnum the input decimal number
 * @param out the output hex string
 */
void DecToHex( UInt32 decValue, Char hexStr[], Boolean pad )
	{
	Char		ch[2], before[24];
	UInt8    next;
   UInt16   i;

	before[0] = 0;
	
	while( 1 )
		{
		next = decValue % 16;		// get the next hex character
		ToHex( next, ch );
		StrCat( before, ch );
		
		decValue = decValue / 16;
		if( decValue == 0 )
			break;
		}
	
   if( pad )
      {
      if( StrLen( before ) == 1 )				// only one char
         {
         hexStr[0] = '0';
         hexStr[1] = before[0];
         hexStr[2] = 0;
         return;
         }
      
      if( StrLen( before ) == 0 )				// only one char
         {
         hexStr[0] = '0';
         hexStr[1] = '0';
         hexStr[2] = 0;
         return;
         }
      }

	for( i=0; i<StrLen( before ); i++ )		// reverse it
		hexStr[i] = before[StrLen(before) - i - 1];
	hexStr[i] = 0;
	}


/**
 * Raises 16 to a power.
 *
 * @param pow the power to raise 16 to
 * @return the result
 */
static UInt16 Raise( UInt8 pow )
	{
	UInt16	ret=1, i;
	
	if( pow == 0 )
		return ret;
	
	for( i=0; i<pow; i++ )
		{
		ret *= 16;
		}
		
	return ret;
	}


/**
 * Convert a hexadecimal value to a decimal value.
 *
 * @param in the input hexadecimal string
 * @return the decimal value
 */
UInt16 HexToDec( Char in[] )
	{
	Int16		i, b;
	UInt8		st=0;
	UInt16	sum = 0;
	
	for( i=0, b=StrLen(in)-1; i<StrLen( in ); i++, b-- )
		{
		st = ToDec( in[b] );
		sum += Raise( i ) * st;
		}
		
	return sum;	
	}


/**
 * Mark a record dirty (modified).
 *
 * @param db a pointer to the database
 * @param index the record index
 */
void DirtyRecord( DmOpenRef db, UInt16 index )
	{
	UInt16		attr;

	DmRecordInfo( db, index, &attr, NULL, NULL );
	attr |= dmRecAttrDirty;
	DmSetRecordInfo( db, index, &attr, NULL );
	}


/*
void checkMemPtr( MemPtr ptr, const Char *method )
   {
   Char message[128];
   
   if( ptr != NULL ) return;
   StrPrintF( message, "MemErr: Null pointer in %s", method );
   showMessage( message );
   }
*/
   
   
void checkMemPtr( MemHandle handle, const Char *method )
   {
   Char message[128];
   
   if( handle != NULL ) return;
   StrPrintF( message, "MemErr: Null handle in %s", method );
   showMessage( message );
   }


// it will give an alert indicating an error from vfsmgr, datamgr, memorymgr, exchangemgr
// the extra text is optional, the rest is required.
void checkError( Err err, const Char *method, const Char *extra )
   {
   Char error[64], message[256];

   if( err == errNone ) return;
      
   switch( err )
      {
      // see if it's a vfsmgr error
      case vfsErrBufferOverflow:       StrPrintF( error, "vfs: Buffer Overflow" ); break;
      case vfsErrFileGeneric:          StrPrintF( error, "vfs: File Generic" ); break;
      case vfsErrFileBadRef:           StrPrintF( error, "vfs: File Bad Ref" ); break;
      case vfsErrFileStillOpen:        StrPrintF( error, "vfs: File Still Open" ); break;
      case vfsErrFilePermissionDenied: StrPrintF( error, "vfs: File Permission Denied" ); break;
      case vfsErrFileAlreadyExists:    StrPrintF( error, "vfs: File Already Exists" ); break;
      case vfsErrFileEOF:              StrPrintF( error, "vfs: File EOF" );  break;
      case vfsErrFileNotFound:         StrPrintF( error, "vfs: File Not Found" ); break;
      case vfsErrVolumeBadRef:         StrPrintF( error, "vfs: Volume Bad Ref" ); break;
      case vfsErrVolumeStillMounted:   StrPrintF( error, "vfs: Volume Still Mounted" ); break;
      case vfsErrVolumeFull:           StrPrintF( error, "vfs: Volume Full" ); break;
      case vfsErrNoFileSystem:         StrPrintF( error, "vfs: No File System" ); break;
      case vfsErrBadData:              StrPrintF( error, "vfs: Bad Data" ); break;
      case vfsErrNameShortened:        StrPrintF( error, "vfs: Name Shortened" ); break;
      case vfsErrDirectoryNotFound:    StrPrintF( error, "vfs: Directory Not Found" ); break;
      case vfsErrIsADirectory:         StrPrintF( error, "vfs: Is A Directory" ); break;
      case vfsErrNotADirectory:        StrPrintF( error, "vfs: Not A Directory" ); break;
      case vfsErrUnimplemented:        StrPrintF( error, "vfs: Unimplemented" ); break;
      case vfsErrBadName:              StrPrintF( error, "vfs: Bad Name" ); break;
      case vfsErrDirNotEmpty:          StrPrintF( error, "vfs: Dir Not Empty" ); break;

      // see if it's a datamgr error
      case dmErrMemError:              StrPrintF( error, "dm: Mem Error" ); break;
      case dmErrROMBased:              StrPrintF( error, "dm: ROM Based" ); break;
      case dmErrDatabaseNotProtected:  StrPrintF( error, "dm: Database Not Protected" ); break;
      case dmErrDatabaseProtected:     StrPrintF( error, "dm: Database Protected" ); break;
      case dmErrInvalidDatabaseName:   StrPrintF( error, "dm: Invalid Database Name" ); break;
      case dmErrAlreadyExists:         StrPrintF( error, "dm: Already Exists" ); break;
      case dmErrUniqueIDNotFound:      StrPrintF( error, "dm: Unique ID Not Found" ); break;
      case dmErrOpenedByAnotherTask:   StrPrintF( error, "dm: Opened By Another Task" ); break;
      case dmErrAlreadyOpenForWrites:  StrPrintF( error, "dm: Already Open For Writes" ); break;
      case dmErrSeekFailed:            StrPrintF( error, "dm: Seek Failed" ); break;
      case dmErrWriteOutOfBounds:      StrPrintF( error, "dm: Write Out Of Bounds" ); break;
      case dmErrNotValidRecord:        StrPrintF( error, "dm: Not Valid Record" ); break;
      case dmErrInvalidCategory:       StrPrintF( error, "dm: Invalid Category" ); break;
      case dmErrNoOpenDatabase:        StrPrintF( error, "dm: No Open Database" ); break;
      case dmErrResourceNotFound:      StrPrintF( error, "dm: Resource Not Found" ); break;
      case dmErrRecordBusy:            StrPrintF( error, "dm: Record Busy" ); break;
      case dmErrIndexOutOfRange:       StrPrintF( error, "dm: Index Out Of Range" ); break;
      case dmErrInvalidParam:          StrPrintF( error, "dm: Invalid Param" ); break;
      case dmErrReadOnly:              StrPrintF( error, "dm: Read Only" ); break;
      case dmErrDatabaseOpen:          StrPrintF( error, "dm: Database Open" ); break;
      case dmErrCantOpen:              StrPrintF( error, "dm: Cant Open" ); break;
      case dmErrCantFind:              StrPrintF( error, "dm: Cant Find" ); break;
      case dmErrRecordInWrongCard:     StrPrintF( error, "dm: Record In Wrong Card" ); break;
      case dmErrCorruptDatabase:       StrPrintF( error, "dm: Corrupt Database" ); break;
      case dmErrRecordDeleted:         StrPrintF( error, "dm: Record Deleted" ); break;
      case dmErrRecordArchived:        StrPrintF( error, "dm: Record Archived" ); break;
      case dmErrNotRecordDB:           StrPrintF( error, "dm: Not Record DB" ); break;
      case dmErrNotResourceDB:         StrPrintF( error, "dm: Not Resource DB" ); break;

      // see if it's a memorymgr error
      case memErrChunkLocked:          StrPrintF( error, "mem: Chunk Locked" ); break;
      case memErrNotEnoughSpace:       StrPrintF( error, "mem: Not Enough Space" ); break;
      case memErrInvalidParam:         StrPrintF( error, "mem: Invalid Param" ); break;
      case memErrChunkNotLocked:       StrPrintF( error, "mem: Chunk Not Locked" ); break;
      case memErrCardNotPresent:       StrPrintF( error, "mem: Card Not Present" ); break;
      case memErrNoCardHeader:         StrPrintF( error, "mem: No Card Header" ); break;
      case memErrInvalidStoreHeader:   StrPrintF( error, "mem: Invalid Store Header" ); break;
      case memErrRAMOnlyCard:          StrPrintF( error, "mem: RAM Only Card" ); break;
      case memErrWriteProtect:         StrPrintF( error, "mem: Write Protect" ); break;
      case memErrNoRAMOnCard:          StrPrintF( error, "mem: No RAM On Card" ); break;
      case memErrNoStore:              StrPrintF( error, "mem: No Store" ); break;
      case memErrROMOnlyCard:          StrPrintF( error, "mem: ROM Only Card" ); break;

      // see if it's an exchangemgr error
      case exgMemError:                StrPrintF( error, "exg: Mem Error" ); break;
      case exgErrStackInit:            StrPrintF( error, "exg: Stack Init" ); break;
      case exgErrUserCancel:           StrPrintF( error, "exg: User Cancel" ); break;
      case exgErrNoReceiver:           StrPrintF( error, "exg: No Receiver" ); break;
      case exgErrNoKnownTarget:        StrPrintF( error, "exg: No Known Target" ); break;
      case exgErrTargetMissing:        StrPrintF( error, "exg: Target Missing" ); break;
      case exgErrNotAllowed:           StrPrintF( error, "exg: Not Allowed" ); break;
      case exgErrBadData:              StrPrintF( error, "exg: Bad Data" ); break;
      case exgErrAppError:             StrPrintF( error, "exg: App Error" ); break;
      case exgErrUnknown:              StrPrintF( error, "exg: Unknown" ); break;
      case exgErrDeviceFull:           StrPrintF( error, "exg: Device Full" ); break;
      case exgErrDisconnected:         StrPrintF( error, "exg: Disconnected" ); break;
      case exgErrNotFound:             StrPrintF( error, "exg: Not Found" ); break;
      case exgErrBadParam:             StrPrintF( error, "exg: Bad Param" ); break;
      case exgErrNotSupported:         StrPrintF( error, "exg: Not Supported" ); break;
      case exgErrDeviceBusy:           StrPrintF( error, "exg: Device Busy" ); break;
      case exgErrBadLibrary:           StrPrintF( error, "exg: Bad Library" ); break;
      case exgErrNotEnoughPower:       StrPrintF( error, "exg: Not Enough Power" ); break;

      // see if it's a system error (partial check here)
      case sysErrTimeout:              StrPrintF( error, "sys: Timeout" ); break;
      case sysErrParamErr:             StrPrintF( error, "sys: Param Err" ); break;
      case sysErrNoFreeResource:       StrPrintF( error, "sys: No Free Resource" ); break;
      case sysErrNoFreeRAM:            StrPrintF( error, "sys: No Free RAM" ); break;
      case sysErrNotAllowed:           StrPrintF( error, "sys: Not Allowed" ); break;
      case sysErrSemInUse:             StrPrintF( error, "sys: Sem In Use" ); break;
      case sysErrInvalidID:            StrPrintF( error, "sys: Invalid ID" ); break;
      case sysErrOutOfOwnerIDs:        StrPrintF( error, "sys: Out Of Owner IDs" ); break;
      case sysErrNoFreeLibSlots:       StrPrintF( error, "sys: No Free Lib Slots" ); break;
      case sysErrLibNotFound:          StrPrintF( error, "sys: Lib Not Found" ); break;
      case sysErrDelayWakened:         StrPrintF( error, "sys: Delay Wakened" ); break;
      case sysErrRomIncompatible:      StrPrintF( error, "sys: Rom Incompatible" ); break;
      case sysErrBufTooSmall:          StrPrintF( error, "sys: Buf Too Small" ); break;
      case sysErrPrefNotFound:         StrPrintF( error, "sys: Pref Not Found" ); break;

      // see if it's an expansion manager error
      case expErrUnsupportedOperation: StrPrintF( error, "exp: Unsupported Operation" ); break;
      case expErrNotEnoughPower:       StrPrintF( error, "exp: Not Enough Power" ); break;
      case expErrCardNotPresent:       StrPrintF( error, "exp: Card Not Present" ); break;
      case expErrInvalidSlotRefNum:    StrPrintF( error, "exp: Invalid Slot RefNum" ); break;
      case expErrSlotDeallocated:      StrPrintF( error, "exp: Slot Deallocated" ); break;
      case expErrCardNoSectorReadWrite:StrPrintF( error, "exp: Card No Sector ReadWrite" ); break;
      case expErrCardReadOnly:         StrPrintF( error, "exp: Card Read Only" ); break;
      case expErrCardBadSector:        StrPrintF( error, "exp: Card Bad Sector" ); break;
      case expErrCardProtectedSector:  StrPrintF( error, "exp: Card Protected Sector" ); break;
      case expErrNotOpen:              StrPrintF( error, "exp: Not Open" ); break;
      case expErrStillOpen:            StrPrintF( error, "exp: Still Open" ); break;
      case expErrUnimplemented:        StrPrintF( error, "exp: Unimplemented" ); break;
      case expErrEnumerationEmpty:     StrPrintF( error, "exp: Enumeration Empty" ); break;
      case expErrIncompatibleAPIVer:   StrPrintF( error, "exp: Incompatible API Version" ); break;

      // it's none of the above, so just return the error number
      default:                         StrPrintF( error, "Unknown: %d", err ); break;         
      }

   StrPrintF( message, "Error: %s in %s", error, method );
   
   if( extra )
      showMessage( message, extra );
   else
      showMessage( message );
   }


void showMessage( const Char *message )
   {
   FrmCustomAlert( MessageAlert, message, 0, 0 );
   }


void showMessage( const Char *message, const Char *extra )
   {
   Char *text = (Char*)MemPtrNew( StrLen( message ) + StrLen( extra ) + 5 );
   StrPrintF( text, "%s (%s)", message, extra );
   FrmCustomAlert( MessageAlert, text, 0, 0 );
   }


/**
 * Decides if a character is valid or invalid for use in VFS filenames. The
 * full range of possible characters from 0 to 255 should be covered here.
 */
static Boolean SECT_CMN charValid( UInt8 c )
   {
	// apparently only the : and / characters are illegal in names, so only check
	//  for these.
	
	if( c == ':' || c == '/' )
		return false;
	else
		return true;
/*
	
	// the characters from 0 to 31 are all control characters and i think
	//  are all invalid for filenames.
	if( c <= 31 )
		return false;

	// everything from from 32 (space) to 126 (~) are symbols and letters,
	//  and they're all ok cept for ':' and '/' so just fail on those.
	
	if( c == ':' || c == '/' )
		return false;

	if( c >= 32 && c <= 125 )
		return true;

	// the characters from 126 (DEL) to 191 (inverted question mark) are all
	//  really strange characters, so i don't think they're valid.
	
	if( c >= 126 && c <= 191 )
		return false;
	
	// the characters from 192 (accent A) to 255 (umlaut y) are all the
	//  international characters, which should be fine.

	if( c >= 192 && c <= 255 )
		return true;

   return false;
*/	
   }


Boolean hasInvalidVFSCharacters( Char *name )
   {
	for( int i=0; i<StrLen( name ); i++ )
      {
      if( !charValid( name[i] ))
			return true;
		}
   return false;
   }


/**
 * Check here for any illegal characters in the string, and if so, ask if
 * they want to automatically replace them with underscores.
 *
 * @param string to check
 * @return Returns 0 if everything is ok, 1 if the copy should be cancelled
 */
Err checkForInvalidVFSCharacters( Char *name, int alert, int cancel )
	{
	Boolean	invalid=false;
   //Char     invalidChar = ' ';

   // first run through the name to see if there are any illegal characters
	for( int i=0; i<StrLen( name ); i++ )
      {
      if( !charValid( name[i] ))
         {
         invalid = true;
         
         Char msg[256];
         StrPrintF( msg, "Invalid character: %c", name[i] );
         showMessage( msg );
         }
		}

   // if any bad characters were found, replace them with an '_'
	if( invalid )
		{
		if( FrmAlert( alert ) == cancel )
			return 1;
			
		for( int i=0; i<StrLen( name ); i++ )
         {
         if( !charValid( name[i] ))
            name[i] = '_';
         }
		}

	return errNone;
	}


Boolean StrTail( Char *str, Char *tail )
   {
   // first see if the tail is even in there
   if( !StrStr( str, tail ))
      return false;
   
   // it is, so make sure it's the end of the string
   UInt8 t = StrLen( tail ) - 1;
   UInt8 s = StrLen( str ) - 1;
   
   while( true )
      {
      if( str[s] != tail[t] )
         return false;

      if( t == 0 ) break;

      t--;
      s--;      
      }

   return true;
   }


/**
 * Display a progress bar.
 *
 * @param c how much is done
 * @param m how much there is total
 */
void PleaseWait( UInt16 c, UInt16 m, UInt32 colorDepth )
	{
	RectangleType	r;
	float				scale, upto;
   RGBColorType	barColor = 	{ 0, 51, 0, 153 };
	
	scale = 60.0 / (float)m;
	upto = scale * (float)c;
	
	r.topLeft.x = 50;
	r.topLeft.y = 75;
	r.extent.y = 10;

	r.extent.x = (Coord)upto;
	
	r.topLeft.x++;		r.topLeft.y++;
	r.extent.x-=2;		r.extent.y-=2;
	if( r.extent.x < 0 ) r.extent.x = 0;
	
	if( colorDepth >= 8 )
		{
		WinPushDrawState();
		WinSetForeColor( WinRGBToIndex( &barColor ));
		WinDrawRectangle( &r, 0 );
		WinPopDrawState();
		}
	else
		WinDrawRectangle( &r, 0 );
		
	r.topLeft.x--;		r.topLeft.y--;
	r.extent.x+=2;		r.extent.y+=2;

	r.topLeft.x = (Coord)(50 + upto);
	r.extent.x = (Coord)(60 - upto);
	WinEraseRectangle( &r, 0 );
	
	r.extent.x = 60;
	r.topLeft.x = 50;
	WinDrawRectangleFrame( simpleFrame, &r );
	}


/**
 * Get today's date.
 *
 * @return a datetype struct that contains today's date.
 */
DateType GetTodaysDate()
	{
	DateTimeType		today;
	DateType			da;
	
	TimSecondsToDateTime( TimGetSeconds(), &today );
	da.month = today.month;
	da.year = today.year - firstYear;
	da.day = today.day;
	
	return da;
	}


/**
 * Convert a 4 character string into a 4 byte integer.
 *
 * @param str the string to convert
 * @return the integer
 */
UInt32 StrToInt( Char *str )
	{
	UInt32	b1, b2, b3, b4, ret;
	
	if( StrLen( str ) < 4 )
		return 0;
	
	b1 = str[0];		b2 = str[1];
	b3 = str[2];		b4 = str[3];
	
	ret = 0;
	ret |= b1 << 24;		ret |= b2 << 16;
	ret |= b3 << 8;		ret |= b4<< 0;
	
	return ret;
	}


 /**
 * Convert a 4 byte integer into a 4 byte ascii string.
 *
 * @param theID the 4 byte integer
 * @param theStr the output string
 */
void IntToStr( UInt32 theID, Char *theStr )
	{
	UInt8	b1, b2, b3, b4;

	b1 = theID >> 24;
	b2 = theID >> 16;
	b3 = theID >> 8;
	b4 = theID >> 0;

	theStr[0] = b1;
	theStr[1] = b2;
	theStr[2] = b3;
	theStr[3] = b4;
	theStr[4] = 0;
	}


/**
 * This routine substitutes the occurrence a token, within a string,
 * with another string.
 *
 * @param str string containing token string
 * @param token the string to be replaced
 * @param sub the string to substitute for the token
 * @param subLen length of the substitute string.
 * @return a pointer to the string
 */
Char* SubstituteStr( Char* str, const Char* token, Char* sub, UInt16 subLen )
   {
	int 			charsToMove;
	UInt16 		tokenLen;
	UInt16 		strLen;
	UInt16 		blockSize;
	Char* 		ptr;
	MemHandle 	strH;

	// Find the start of the token string, if it doesn't exist, exit.
	ptr = StrStr( str, token );
	if( ptr == NULL ) return str;
	
	tokenLen = StrLen( token );
	charsToMove = subLen - tokenLen;
		
	// Resize the string if necessary.
	strH = MemPtrRecoverHandle( str );
	strLen = StrLen( str );
	blockSize = MemHandleSize( strH );
	if( strLen + charsToMove + 1 >= blockSize )
		{
		MemHandleUnlock( strH );
		MemHandleResize( strH, strLen + charsToMove + 1 );
		str = (Char*)MemHandleLock( strH );
		ptr = StrStr( str, token );
		ErrNonFatalDisplayIf( ptr == NULL, "Msg missing token" );
		}
	
	// Make room for the substitute string.
	if( charsToMove )
		MemMove( ptr + subLen, ptr + tokenLen, StrLen( ptr + tokenLen )+1 );
		
	// Replace the token with the substitute string.
	MemMove( ptr, sub, subLen );
	
	return str;
   }


/**
 * Converts an unsigned long to a string.
 *
 * @param value the number
 * @param strP the output string
 */
static void cvt_ultoa( unsigned long value, char *strP )
   {
   int 	i;
   char 	*cP;
   char 	tmpstr[16];		/* a 32 bit long can give up to 10 digits */
   
   if( value <= 9 )			/* a quick special case */
      {
      *strP++ = value + '0';
      *strP = '\0';
      return;
      }

   /* We convert the number backwards */
   for( cP = tmpstr+9, i = 10; i-- > 0; )
      {
      *cP-- = (value % 10) + '0';
      value /= 10;
      }

   tmpstr[10] = '\0';

   for( cP = tmpstr; *cP && (*cP == '0'); )
      ++cP;
    
   StrCopy(strP,cP);
   }


/**
 * Convert a string to an unsigned long.
 *
 * @param strP the number string
 * @return the number
 */
static unsigned long cvt_atoul( char* strP )
   {
   unsigned long value;

   for( value = 0; (*strP >= '0') && (*strP <= '9'); )
      {
      value *= 10;
      value += *strP++ - '0';
      }

   return value;
   }
   

/**
 * Convert a string to a floating point number.
 *
 * @param strP the number string
 * @return the floating point number
 */
float StrAToF( char *strP )
   {
   int 				i;
   int 				neg;
   char 				*cP;
   unsigned long 	ipart,fpart;
   float 			rslt;

   if( !strP )
      return 0.0;

   while( (*strP == ' ') || (*strP == '\t') )		/* trim whitespace */
      ++strP;

   if( *strP == '-' )					/* check for neg. */
      {
      neg = 1;
      ++strP;
      }
   else
      {
      neg = 0;

      if( *strP == '+' )				/* drop any sign */
         ++strP;
      }

   ipart = cvt_atoul(strP);			/* get integer part */

   if( (cP = StrChr(strP,'.')) )
      {
      i = StrLen(++cP);
      fpart = cvt_atoul(cP);			/* and frac part */
      rslt = (float)fpart;

      while( i-- > 0 )					/* scale as needed */
         rslt /= 10.0;

      rslt += (float)ipart;
      }
   else
      rslt = (float)ipart;

   return( neg?-rslt:rslt );
   }


/**
 * Convert a floating point number to a string.
 *
 * @param strP the output string
 * @param value the floating point number
 */
void StrFToA( char *strP, float value, UInt8 frac )
	{
	//short 			frac = 1;      // number of decimal points
   int 				i;
   unsigned long 	ipart, fpart;
   float 			limit;
   char 				str[16];

   if( frac > (sizeof(str) - 1) )		/* limit frac digits */
      frac = sizeof(str) - 1;

   if( value < 0.0 )
      {
      *strP++ = '-';
      value = -value;
      }

   ipart = (long)value;
   value -= (float)ipart;			/* recover frac part as int */
   for( limit = 1.0, i = frac; i-- > 0; )
      {
      value *= 10.0;
      limit *= 10.0;
      }

   value += 0.5;				/* do some rounding */
   if( value >= limit )
      {
      fpart = 0;				/* overflowed */
      ipart++;
      }
   else
      fpart = (unsigned long)(value);

   cvt_ultoa(ipart,strP);

   if( frac )
      {
      cvt_ultoa(fpart,str);
      strP += StrLen(strP);
      *strP++ = '.';

      for( i = StrLen(str); i++ < frac; )
         *strP++ = '0';			/* need some padding */
      StrCopy(strP,str);			/* and now the value */
      }
   }


/**
 * Display a form showing a time and allow the user to select a different time.
 *
 * @param hour	- pointer to hour to change
 * @param minute - pointer to minute to change
 * @param title - string title for the dialog
 * @return true if the OK button was pressed & true the parameters passed are changed
 */
Boolean SelectATime( Int16 *hour, Int16 *minute, const Char * titleP )
	{
	FormType * originalForm, * frm;
	EventType event;
	Boolean confirmed = false;
	//Boolean handled = false;
	Boolean done = false;
	TimeFormatType timeFormat;			// Format to display time in
	Int16   curHour;
	UInt16	currentTimeButton;
	UInt8	hoursTimeButtonValue;
	Char	hoursTimeButtonString[3];
	UInt8	minuteTensButtonValue;
	Char	minuteTensButtonString[2];
	UInt8	minuteOnesButtonValue;
	Char	minuteOnesButtonString[2];
	Char	separatorString[3];

	timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	originalForm = FrmGetActiveForm();
	frm = (FormType *) FrmInitForm (SelectTimeForm); 
	if (titleP)
		FrmSetTitle (frm, (Char *) titleP);

	FrmSetActiveForm (frm);
	
	curHour = *hour;
	
	if (Use24HourFormat(timeFormat))
		{
		// Hide the AM & PM ui
		HideObject( frm, SelectTimeAMPushButton );
		HideObject( frm, SelectTimePMPushButton );
		}
	else 
		{
		if (curHour < 12)
			CtlSetValue( GetObjectPtr<ControlType>( SelectTimeAMPushButton ), true );
		else
			{
			CtlSetValue( GetObjectPtr<ControlType>( SelectTimePMPushButton ), true );
			curHour -= 12;
			}
		
		if (curHour == 0)
			curHour = 12;
		}

	// Set the time seperator to the system defined one
	separatorString[0] = TimeSeparator(timeFormat);
	separatorString[1] = '\0';
	FrmCopyLabel(frm, SelectTimeSeparatorLabel, separatorString);


	// Now set the time displayed in the push buttons.
	hoursTimeButtonValue = curHour;
	StrIToA(hoursTimeButtonString, hoursTimeButtonValue);
	CtlSetLabel( GetObjectPtr<ControlType>( SelectTimeHoursPushButton ),
		hoursTimeButtonString );
		
	minuteTensButtonValue = *minute / 10;
	StrIToA(minuteTensButtonString, minuteTensButtonValue);
	CtlSetLabel( GetObjectPtr<ControlType>( SelectTimeTensPushButton ),
		minuteTensButtonString );
		
	minuteOnesButtonValue = *minute % 10;
	StrIToA(minuteOnesButtonString, minuteOnesButtonValue);
	CtlSetLabel( GetObjectPtr<ControlType>( SelectTimeOnesPushButton ),
		minuteOnesButtonString);


	// Set the hour time button to be the one set by the arrows
	currentTimeButton = defaultTimeButton;
	CtlSetValue( GetObjectPtr<ControlType>( defaultTimeButton ), true);
	
	FrmDrawForm( frm );
	
	while (!done)
		{
		EvtGetEvent (&event, evtWaitForever);

		if (! SysHandleEvent ((EventType *)&event))	
			FrmHandleEvent (frm,&event); 

		if (event.eType == ctlSelectEvent)
			{
			switch (event.data.ctlSelect.controlID)
				{
				case SelectTimeOKButton:
					frm = FrmGetActiveForm();

					// Set the new time (seconds are cleared).
					if (Use24HourFormat(timeFormat))
						*hour = hoursTimeButtonValue;
					else
						{
						*hour = hoursTimeButtonValue % 12 + // 12am is 0 hours!
							(CtlGetValue( GetObjectPtr<ControlType>(
								SelectTimePMPushButton))
							? 12 : 0);
						}

					*minute = minuteTensButtonValue * 10 + minuteOnesButtonValue;

					done = true;
					confirmed = true;
					break;

				case SelectTimeCancelButton:
					done = true;
					break;
					
				case SelectTimeIncreaseButton:
				case SelectTimeDecreaseButton:
					frm = FrmGetActiveForm();
					switch (currentTimeButton)
						{
						// MemHandle increasing and decreasing the time for each time digit
						case SelectTimeHoursPushButton:
							if (event.data.ctlSelect.controlID == SelectTimeDecreaseButton)
								{
								if (Use24HourFormat(timeFormat))
									if (hoursTimeButtonValue > 0)
										hoursTimeButtonValue--;
									else
										hoursTimeButtonValue = 23;
								else
									if (hoursTimeButtonValue > 1)
										hoursTimeButtonValue--;
									else
										hoursTimeButtonValue = 12;
								}
							else
								{
								if (Use24HourFormat(timeFormat))
									if (hoursTimeButtonValue < 23)
										hoursTimeButtonValue++;
									else
										hoursTimeButtonValue = 0;
								else
									if (hoursTimeButtonValue < 12)
										hoursTimeButtonValue++;
									else
										hoursTimeButtonValue = 1;
								}
								
							StrIToA(hoursTimeButtonString, hoursTimeButtonValue);
							CtlSetLabel( GetObjectPtr<ControlType>( SelectTimeHoursPushButton ), 
								hoursTimeButtonString);
							break;

						case SelectTimeTensPushButton:
							if (event.data.ctlSelect.controlID == SelectTimeDecreaseButton)
								{
								if (minuteTensButtonValue > 0)
									minuteTensButtonValue--;
								else
									minuteTensButtonValue = 5;
								}
							else
								{
								if (minuteTensButtonValue < 5)
									minuteTensButtonValue++;
								else
									minuteTensButtonValue = 0;
								}

							StrIToA(minuteTensButtonString, minuteTensButtonValue );
							CtlSetLabel( GetObjectPtr<ControlType>( 
								SelectTimeTensPushButton ), minuteTensButtonString );
							break;
							
						case SelectTimeOnesPushButton:
							if (event.data.ctlSelect.controlID == SelectTimeDecreaseButton)
								{
								if (minuteOnesButtonValue > 0)
									minuteOnesButtonValue--;
								else
									minuteOnesButtonValue = 9;
								}
							else
								{
								if (minuteOnesButtonValue < 9)
									minuteOnesButtonValue++;
								else
									minuteOnesButtonValue = 0;
								}

							StrIToA(minuteOnesButtonString, minuteOnesButtonValue);
							CtlSetLabel( GetObjectPtr<ControlType>(
								SelectTimeOnesPushButton), minuteOnesButtonString);
							break;
						}

					//handled = true;
					break;	// timeDecreaseButton & timeIncreaseButton
		
				case SelectTimeHoursPushButton:
					currentTimeButton = SelectTimeHoursPushButton;
					break;
					
				case SelectTimeTensPushButton:
					currentTimeButton = SelectTimeTensPushButton;
					break;
					
				case SelectTimeOnesPushButton:
					currentTimeButton = SelectTimeOnesPushButton;
					break;
					
				}
			}

		else if (event.eType == appStopEvent)
			{
			EvtAddEventToQueue (&event);
			done = true;
			break;
			}
			
		}	// end while true
		
	FrmEraseForm (frm);
	FrmDeleteForm (frm);
	
	FrmSetActiveForm(originalForm);
	
	return (confirmed);
   }


/**
 * Get the Palm OS version into a string.
 *
 * @param destination string
 */
void getOSVersion( Char str[] )
	{
	Char	ns[16];
	UInt32	romVersion, n;
	UInt8	len;
	Char	build[12], stage[2], fix[2], minor[2], major[3];

	// 0xMMmfsbbb, where MM is major version, m is minor version
	// f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
	// bbb is build number for non-releases 
	// V1.12b3   would be: 0x01122003
	// V2.00a2   would be: 0x02001002
	// V1.01     would be: 0x01013000
	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
	DecToHex( romVersion, ns, false );
	
	len = StrLen( ns );
	
	build[3] = 0;
	build[2] = ns[len-1];
	build[1] = ns[len-2];
	build[0] = ns[len-3];
	
	stage[1] = 0;
	stage[0] = ns[len-4];
	
	fix[1] = 0;
	fix[0] = ns[len-5];
	
	minor[1] = 0;
	minor[0] = ns[len-6];
	
	if( len == 7 )
		{
		major[1] = 0;
		major[0] = ns[len-7];		
		}
	else
		{
		major[2] = 0;
		major[1] = ns[len-7];
		major[0] = ns[len-8];	
		}

	n = StrAToI( stage );										// get the stage
	switch( n )
		{
		case 3:					// release rom
			StrCopy( stage, "" );
			break;
		case 2:					// beta
			StrCopy( stage, "b" );
			break;
		case 1:					// alpha
			StrCopy( stage, "a" );
			break;
		case 0:					// development
			StrCopy( stage, "d" );
			break;
		}

	n = StrAToI( build );										// get the build number
	if( StrLen( stage ) )
		{
		StrIToA( ns, n );
		StrCopy( build, " Build " );
		StrCat( build, ns );
		}

	StrPrintF( str, "V%s.%s%s%s%s", major, minor, fix, stage, build );	
	}


void SendStringToMemoPad( Char *title, Char *content )
   {
   UInt16 maxMemoSize = 4000;
   UInt16      extraTitleSpace = 4;
	MemPtr		p;
	//Char			zero = 0;
	UInt32		offset = 0;
	UInt16		index;
	MemHandle 	memoRec;
   UInt16      count=0;
   Char        num[extraTitleSpace];

	DmOpenRef MemoDB = DmOpenDatabaseByTypeCreator( 'DATA', sysFileCMemo, dmModeReadWrite );
	if( !MemoDB )
		return;

   UInt16 contentPieceSize=0, written=0;
   UInt16 titleSize = StrLen( title );
   UInt16 totalToWrite = StrLen( content );

   while( totalToWrite > 0 )
      {
      if( count > 0 )
         titleSize += extraTitleSpace;
      
      if( totalToWrite > maxMemoSize )
         contentPieceSize = maxMemoSize;
      else
         contentPieceSize = totalToWrite;
      UInt16 writeSize = contentPieceSize + 1 + titleSize;

      Char *str = (Char*)MemPtrNew( writeSize + 1 );
      StrCopy( str, title );
      if( count > 0 )
         {
         StrPrintF( num, " %d", count+1 );
         StrCat( str, num );
         }
      StrCat( str, "\n" );
      StrNCat( str, content+written, writeSize+1 );
      str[writeSize] = 0;
      
      index = DmNumRecords( MemoDB );
      memoRec = DmNewRecord( MemoDB, &index, writeSize+1 );
      if( !memoRec )
         return;
      p = MemHandleLock( memoRec );
      DmWrite( p, offset, str, writeSize+1 );
      MemPtrUnlock( p );
      DmReleaseRecord( MemoDB, index, true );
      
      totalToWrite -= contentPieceSize;
      written += contentPieceSize;
      MemPtrFree( str );
      count++;
      }

	DmCloseDatabase( MemoDB );
   }


/**
 * This routine checks that a ROM version meets your minimum requirement.
 * (see sysFtrNumROMVersion in SystemMgr.h for format). Note that this 
 * function must not be moved into any code section since it can be called
 * when the app is not running.
 *
 * @param flags that indicate if the application UI is initialized
 * @return error code or zero if rom is compatible
 */
Err RomVersionCompatible( UInt32 requiredVersion, UInt16 launchFlags )
	{
	UInt32	romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
		
	if( romVersion < requiredVersion )
		{
		if( ( launchFlags & ( sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp ) ) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			//FrmCustomAlert( GeneralErrorAlert, "System Version 3.0 or greater is required to run this application.", NULL, NULL );
		
			// pilot 1.0 will continuously relaunch this app unless we switch to another safe one
			if( romVersion < 0x02000000 )
				AppLaunchWithCommand( sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL );
			}
		
		return( sysErrRomIncompatible );
		}

	return 0;
	}


/**
 * Callback for ExgDBWrite to send data with exchange manager.
 *
 * @param buffer containing data to send
 * @param number of bytes to send
 * @param app defined buffer for context (holds exgSocket when using ExgManager)
 * @return error if non-zero
 */
static Err SECT_CMN sendDBWriteData( const void* dataP, UInt32* sizeP, void* userDataP )
   {
   Err err;

   // try to send as many bytes as were requested by the caller
   *sizeP = ExgSend((ExgSocketPtr)userDataP, (void*)dataP, *sizeP, &err );
   if( err == exgErrUserCancel )
      return err;
   checkError( err, "sendDBWriteData()", 0 );

   return err;
	}


/**
 * Sends the database with the given local id. If showMenu is false, then the
 * file is simply beamed via the IR. 
 *
 * @param ??
 * @return error code or zero for no error
 */
Err sendDB( LocalID dbID, UInt32 cardNo, Boolean showMenu )
	{
   Err            err;
   Char           *dbName, *sendName;
	Char           dbNameP[32], fileName[36];
	UInt16         attr;
   ExgSocketType	exgSocket;
	
   if( !dbID )
      {
      showMessage( "Error: No ID in sendDB(). Unable to send." );
      return 0;
      }
   
	dbName = dbNameP;
	err = DmDatabaseInfo( 0, dbID, dbName, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
   checkError( err, "sendDB() a", 0 );

   // put the appropriate file extension - it's just for visual appeal
   if( attr & dmHdrAttrResDB )
      StrPrintF( fileName, "%s.prc", dbName );
   else
      StrPrintF( fileName, "%s.pdb", dbName );

   // we check for bluetooth support, and if there is, then we add the
   //  ?_send: option to the filename so that the device will ask the
   //  user how they want to send the file.   
   //UInt32 btVersion;
   //if( FtrGet( btLibFeatureCreator, btLibFeatureVersion, &btVersion ) == errNone )
   if( showMenu )
      {
      sendName = (Char*)MemPtrNew( StrLen( fileName ) + 32 );  // lots of space
      StrPrintF( sendName, "?_send;_beam:%s", fileName );
      }
   else
      {
      sendName = (Char*)MemPtrNew( StrLen( fileName ) + 1 );
      StrPrintF( sendName, "%s", fileName );
      }

   MemSet( &exgSocket, sizeof(exgSocket), 0 );  // zero it all out
   
   //exgSocket.libraryRef = 0;   // automatically handled
   //exgSocket.socketRef = 0;    // automatically handled
   //exgSocket.target = 'lnch';    // creator id of who should handle this file
   exgSocket.count = 1;          // the number of objects in this connection (optional)
   //exgSocket.length = 0;       // total byte count for all objects being sent (optional)
   //exgSocket.time = 0;         // the last modified time of the object (optional)
   //exgSocket.appData = 0;      // application-specific information (optional)
   //exgSocket.goToCreator = 0;  // creator id of who sysAppLaunchCmdGoto should be sent to
   //exgSocket.goToParams = 0;   // the parameters for the goto launch code above
   //exgSocket.localMode = 0;    // restrict to only local exchanges?
   //exgSocket.packetMode = 0;   // must be 0
   //exgSocket.noGoTo = 0;       // set this to 1 to disable launching the application with the goto
   //exgSocket.noStatus = 0;     // leave this 0 so the exchange manager can show a progress dialog
   //exgSocket.preview = 0;      // this is set to true if the data is being used for a preview
   //exgSocket.reserved = 0;     // not used
   exgSocket.description = fileName;  // set the description (optional)
   // they don't tell you this, but you have to allocate some sort of string
   //  for the mime type or else Palm OS will crash with a null string error. nice.
   exgSocket.type = (Char*)MemPtrNew( 1 );        // a pointer to the MIME type of the object;
   StrCopy( exgSocket.type, "" );   
   exgSocket.name = sendName;       // the object's name (note that colons here mess everything up since they are used in URLs

   // set the length of the data that will be sent
   err = DmDatabaseSize( cardNo, dbID, 0, &exgSocket.length, 0 );
   checkError( err, "sendDB() b", 0 );

   err = ExgPut( &exgSocket );
   if( !err )
      {
      err = ExgDBWrite( sendDBWriteData, &exgSocket, NULL, dbID, cardNo );
      if( err == exgErrUserCancel )
         return err;
      checkError( err, "sendDB() c", 0 );
      err = ExgDisconnect( &exgSocket, err );
      if( err != exgErrUserCancel )
         checkError( err, "sendDB() d", 0 );
      }
   
   delete sendName;
          
   return err;    	
	}
