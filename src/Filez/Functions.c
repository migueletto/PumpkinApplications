/****************************************************************************
 FileZ
 Copyright (C) 2003  nosleep software

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/

/** @file
 * A bunch of useful functions.
 *
 * Created on 12/21/01 by Tom Bulatewicz
 */
 
#include <PalmOS.h>						// all the system toolbox headers
#include <FeatureMgr.h>					// needed to get the ROM version
#include <ExgMgr.h>
#include <DLServer.h>					// to get hotsync username
#include <VFSMgr.h>

#include "Main.h"
#include "CardMgr.h"
#include "Resource.h"					// application resource defines
#include "Stuph.h"
#include "Functions.h"

#include "ListForm.h"					// shouldn't need this


/**
 * Callback for ExgDBWrite to send data with exchange manager.
 *
 * @param dataP : buffer containing data to send
 * @param sizeP : number of bytes to send
 * @param userDataP: app defined buffer for context (holds exgSocket when using ExgManager)
 * @return error if non-zero
 */
static Err WriteDBData( const void* dataP, UInt32* sizeP, void* userDataP )
	{
   Err		err;

   // try to send as many bytes as were requested by the caller
   *sizeP = ExgSend((ExgSocketPtr)userDataP, (void*)dataP, *sizeP, &err);

   return err;
	}


/**
 * Sends data in the input field using the Exg API.
 *
 * @param cardNo: 	card number of db to send (usually 0)
 * @param dbID:    databaseId of database to send
 * @param nameP:  	public filename for this database. This is the name as it appears on a PC file listing
 *					It should end with a .prc or .pdb extension
 *					description: Optional text description of db to show to user who receives the database.
 * @return error code or zero for no error
 */
static Err SendDatabase( Int16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP )
	{
     	ExgSocketType  exgSocket;
     	Err				err;

     	// Create exgSocket structure
     	MemSet(&exgSocket, sizeof(exgSocket), 0);
     	exgSocket.description = descriptionP;
     	
      
      //Char *fullName = new Char[StrLen( nameP ) + 10];

      // if 4.0, do the ?_send: one, but for earlier
      // could just have a preference to set what the beam menu does
      //  that has 3 choices: normal, ?, local
      
      //StrPrintF( fullName, "?_send:%s", nameP );
      //StrPrintF( fullName, "_local:%s", nameP );
      //exgSocket.name = fullName;
      
      exgSocket.name = nameP;
      
      //ADD another beam option for beaming locally
     	//exgSocket.localMode = 1;
     	
      DmDatabaseSize( cardNo, dbID, 0, &exgSocket.length, 0 );

     	// Start exchange put operation
     	err = ExgPut(&exgSocket);
     	if( !err )
         	{
          	// This function converts a palm database into its external (public)
          	// format. The first parameter is a callback that will be passed parts of
          	// the database to send or write.
          	err = ExgDBWrite(WriteDBData, &exgSocket, NULL, dbID, cardNo);
          	// Disconnect Exg and pass error
          	err = ExgDisconnect(&exgSocket, err);
          	}
     	return err;
	}


/**
 * This routine sends bytes over the IR.
 *
 * @param an IR socket
 * @param what to send
 * @param how much to send
 * @return any errors
 */
static Err BeamBytes( ExgSocketPtr s, void *buffer, UInt32 bytesToSend )
	{
	Err	err = 0;
	
	while( !err && bytesToSend > 0 )									// while still some to send
		{
		UInt32	bytesSent = ExgSend( s, buffer, bytesToSend, &err );		// send some more
		bytesToSend -= bytesSent;									// update our counter
		buffer = ((char*) buffer) + bytesSent;							// update our buffer
		}
		
	return err;														// return any errors
	}


/**
 * Beam a VFS file to another device.
 *
 * @param cardNo: 	card number of db to send (usually 0)
 * @param dbID:    databaseId of database to send
 * @param nameP:  	public filename for this database. This is the name as it appears on a PC file listing
 *					It should end with a .prc or .pdb extension
 *					description: Optional text description of db to show to user who receives the database.
 * @return error code or zero for no error
 */
static Err SendDatabaseVFS( Int16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP, UInt32 size )
	{
   ExgSocketType	exgSocket;
   Err				err;

   // Create exgSocket structure
   MemSet( &exgSocket, sizeof(exgSocket), 0 );
   exgSocket.description = descriptionP;
   exgSocket.name = nameP;
   exgSocket.length = size;
		
   // Start and exchange put operation
   err = ExgPut(&exgSocket);
   if( !err )
     	{
     	// open the vfs file
      FileRef	srcFile;
      UInt32	bufSize = 1024;					// bytes
      UInt32	bytesRead, bytesWritten=0;
      Char		buf[1024];
      Boolean	more=true;
		
      AddFilename( currentPath, cache.getName( selectedDB ) );
		
		// open the source
		err = VFSFileOpen( cache.getCard(), currentPath, vfsModeRead, &srcFile );
		if( err != errNone )
			{
			ErrDisplay( "Unable to open source file." );
			return err;
			}
		
		// loop through and read from source and write to destination				
		while( more )
			{
			err = VFSFileRead( srcFile, bufSize, &buf, &bytesRead );
			
			if( err == vfsErrFileEOF )				// end of the file
				{
				err = errNone;
				more = false;
				}
				
			if( err != errNone )								// some other error
				{
				ErrDisplay( "Error while reading the source file." );
				return err;
				}

			bytesWritten = ExgSend( &exgSocket, &buf, bytesRead, &err );		// send some more
			
			if( err != errNone )								// some other error
				{
				ErrDisplay( "Error while writing the destination file." );
				return err;
				}
			}
		
		// close up the files		
		VFSFileClose( srcFile );          	
      RemoveFilename(currentPath);
          	
     	// Disconnect Exg and pass error
     	err = ExgDisconnect(&exgSocket, err);
     	}
  	return err;
	}


Err BeamDBVFS()
   {
    Err			err = errNone;
    Char		fileName[37];
	LocalID		curID;
	
	curID = cache.getID( selectedDB );

    StrCopy( fileName, cache.getName( selectedDB ));
    StrCat( fileName, ".prc\0" );
     	
     if( curID )  			  								// send it giving external name and description
         SendDatabaseVFS( cache.getCard(), curID, fileName, cache.getName( selectedDB ), cache.getSize( selectedDB ));
     else
       	err = DmGetLastErr();

     return err;
      }

static Err BeamDBVFSToVFS()
	{
	// This is the real tricky one.  In order to beam from a VFS card to the
	//	VFS card of another device, we have to use the IR library because the
	//	built-in exchange manager always loads the receiving db into the heap,
	//	which in some cases might be too small for the file.
	
	return errNone;
	}
	

/**
 * Sends the file over IR.
 *
 * @return Error code or zero for no error
 */
Err BeamDB()
   {
   Err			err;
   Char		fileName[37];
	LocalID		curID;
	Char        message[64];
   
	curID = cache.getID( selectedDB );

   StrCopy( fileName, cache.getName( selectedDB ));
   StrCat( fileName, ".prc\0" );

   if( curID )  			  								// send it giving external name and description
      err = SendDatabase( cache.getCard(), curID, fileName, cache.getName( selectedDB ));
   else
      err = DmGetLastErr();
   
  // if( err != errNone )
      {
      if( err == 5393 )
         FrmAlert( 4010 );
     // }
   
/*   if( err != errNone )
      //FrmAlert( 4010 );
      {
      switch( err )
         {
         case exgMemError:
            StrCopy( message, "Memory Error" );
            break;
         
         case exgErrStackInit:
            StrCopy( message, "Stack could not initialize" );
            break;
            
         case exgErrUserCancel:
            StrCopy( message, "User cancelled" );
            break;
         
         case exgErrNoReceiver:
            StrCopy( message, "Receiver device not found" );
            break;
            
         case exgErrNoKnownTarget:
            StrCopy( message, "Can't find a target app" );
            break;
         
         case exgErrTargetMissing:
            StrCopy( message, "Target app is known but missing" );
            break;
            
         case exgErrNotAllowed:
            StrCopy( message, "Operation not allowed" );
            break;
            
         case exgErrBadData:
            StrCopy( message, "Internal data was not valid" );
            break;
            
         case exgErrAppError:
            StrCopy( message, "Generic application error" );
            break;
            
         case exgErrUnknown:
            StrCopy( message, "Unknown general error" );
            break;
            
         case exgErrDeviceFull:
            StrCopy( message, "Device is full" );
            break;
            
         case  exgErrDisconnected:
            StrCopy( message, "Link disconnected" );
            break;
            
         case exgErrNotFound:
            StrCopy( message, "Requested object not found" );
            break;
            
         case exgErrBadParam:
            StrCopy( message, "Bad parameter to call" );
            break;
            
         case exgErrNotSupported:
            StrCopy( message, "Operation not supported by this library" );
            break;
            
         case exgErrDeviceBusy:
            StrCopy( message, "Device is busy" );
            break;
            
         case exgErrBadLibrary
            StrCopy( message, "Bad or missing ExgLibrary" );
            break;
            
         case exgErrNotEnoughPower:
            StrCopy( message, "Device has not enough power to perform the requested operation" );
            break;
         
         default:
            StrCopy( message, "Error not available" );
         }
 */     
      
      
      //Char text[50];
      //StrPrintF( text, "%d", err );
      //FrmCustomAlert( DebugAlert, text, 0, 0 );
      }
    
    return err;    	
	}


/**
 * Add a trailing slash if necessary.
 *
 * @param string to add to
 */
void AddTrailingSlash( Char *str )
	{
	if( str[StrLen(str)-1] != cardMgr.getCurrentSep()[0] )
		StrCat( str, cardMgr.getCurrentSep() );	
	}


/**
 * Add the filename to the string and add a trailing slash if necessary.
 *
 * @param place for result
 * @param the filename to append
 */
void AddFilename( Char *output, Char *filename )
	{
	Char	sep[2];
	
	// here we assume that if we're dealing with adding a filename to the path
	//	and the current card is the internal card, then we assume we must be
	//	dealing with a sd/mmc/memstick in vfs and not an sfs (since internal-to-sfs
	//	is not currently supported.  if internal-to-sfs support is added, then
	//	this will have to change.
	
	if( cardMgr.currentCardIsInternal())
		StrCopy( sep, "/" );
	else
		StrCopy( sep, cardMgr.getCurrentSep());
	
	if( output[StrLen(output)-1] != sep[0] )
		StrCat( output, sep );

	StrCat( output, filename );
	}


/**
 * Truncate the filename part of the current path global.
 *
 * @param the current path
 */
void RemoveFilename( Char *path )
	{
	Int16	i;
	Char	sep[2];
	
	// here we assume that if we're dealing with adding a filename to the path
	//	and the current card is the internal card, then we assume we must be
	//	dealing with a sd/mmc/memstick in vfs and not an sfs (since internal-to-sfs
	//	is not currently supported.  if internal-to-sfs support is added, then
	//	this will have to change.
	
	if( cardMgr.currentCardIsInternal())
		StrCopy( sep, "/" );
	else
		StrCopy( sep, cardMgr.getCurrentSep());
	
	for( i=StrLen( path )-1; i>=0; i-- )
		{
		if( path[i] == sep[0] )
			{
			path[i+1] = 0;
			return;
			}
		}
	}


/**
 * Return the filename part of the path string.
 *
 * @param the path with filename
 * @return a pointer to just the filename
 */
Char *GetFileName( Char *path )
	{
	Int16	i;
	Char	sep[2];
	
	// here we assume that if we're dealing with adding a filename to the path
	//	and the current card is the internal card, then we assume we must be
	//	dealing with a sd/mmc/memstick in vfs and not an sfs (since internal-to-sfs
	//	is not currently supported.  if internal-to-sfs support is added, then
	//	this will have to change.
	
	if( cardMgr.currentCardIsInternal())
		StrCopy( sep, "/" );
	else
		StrCopy( sep, cardMgr.getCurrentSep());
	
	for( i=StrLen( path )-1; i>=0; i-- )
		{
		if( path[i] == sep[0] )
			{
			return path + i + 1;
			}
		}
	
	ErrFatalDisplay( "Error getting filename." );
	return 0;
	}


/**
 * Convert the attributes to a displayable string.
 *
 * @param the resulting string
 * @param the type
 */
void FilterAttrToString( Char str[], UInt16 attr )
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


/**
 * Delete a folder on an mmc.
 *
 * @return true if the delete occurred,  false if it was canceled
 */
Boolean DeleteDir()
	{
	Int16		buttonHit;
	Err		err=errNone;	
	SfsErr	errs=sfsOK;	

	buttonHit = FrmAlert( DeleteDirectoryAlert );

	if( buttonHit == DeleteDirectoryCancel )					// if cancelled, don't do anything
		return false;
	
	if( !DEBUG )
		{
		switch( cardMgr.getCurrentType())
			{
			case cardInternal:
				ErrFatalDisplay( "DeleteDir() called on a RAM card!" );
				break;
				
			case cardVFS:
				err = VFSFileDelete( cardMgr.getCurrentID(), currentPath );
				switch( err )
					{
					case errNone:		
						ListViewUpOneLevel();
						cache.ResetSelected();
						ListViewUpdateList();
						break;
				
					case vfsErrDirNotEmpty:
						FrmAlert( DirNotEmptyAlert );
						return false;
				
					default:
						FrmAlert( DeleteDirectoryErrorAlert );
						return false;
					}
				break;
				
			case cardSFS:
				Char	str[256];
				StrPrintF( str, "dev0:%s", currentPath );
				errs = SfsRemoveDir( cardMgr.getCurrentID(), str ,NOT_IF_NOT_EMPTY );
				switch( errs )
					{
					case sfsOK:
						ListViewUpOneLevel();
						cache.ResetSelected();
						ListViewUpdateList();
						break;
					
					case sfsErrDirNotEmpty:
						FrmAlert( DirNotEmptyAlert );
						return false;
					
					default:
						FrmAlert( DeleteDirectoryErrorAlert );
						return false;
					}
				break;
			}
		}
			
	return true;
	}


/**
 * This routine deletes a file.
 *
 * @param should the user be prompted?
 * @param the filename to delete
 * @param the card the file is on
 * @param the local id of the file to delete
 * @true if the delete occurred,  false if it was canceled
 */
Boolean DeleteFile( Boolean prompt, Char *filename, UInt16 card, LocalID id )
	{
	Err		err=errNone;	
	UInt16	attr=0;

	if( cardMgr.currentCardIsInternal() && MemLocalIDKind( id ) != memIDHandle )
		{
		FrmAlert( ROMAlert );	
		return false;
		}

	if( prompt )
		{
		if( FrmAlert( DeleteFileAlert ) == DeleteFileCancel )				// if cancelled, don't do anything
			return false;
		}
		
	if( !DEBUG )
		{
		switch( cardMgr.getCurrentType())
			{
			case cardInternal:
				// need to check here and make sure the file does not have the read-only
				//	attribute set.  having it set will prevent the file from being deleted.
			
				err = DmDatabaseInfo( card, id, 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
				attr &= ~dmHdrAttrReadOnly;
				err = DmSetDatabaseInfo( card, id, 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
			
				err = DmDeleteDatabase( card, id );						// otherwise, remove the database
				break;
				
			case cardVFS:
				StrCat( currentPath, filename );
				err = VFSFileDelete( card, currentPath );
				RemoveFilename( currentPath );
				break;
				
			case cardSFS:
				Char		str[256];					// must be better way...	
				StrPrintF( str, "dev0:%s%s", currentPath, filename );
				err = SfsRemoveFile( card, str );
				break;
			}
		}
		
	if( err == errNone )
		{
		cache.Remove( filename );
		}
	else
		{
      Char text[32];
      StrIToA( text, err );
		FrmCustomAlert( DeleteErrorAlert, text, 0, 0 );
		return false;
		}
	
	return true;
	}


/**
 * Export a list of all the files to the memopad.
 *
 * @param an event
 * @return if it was sent successfully
 */
Boolean SendListToMemo()
	{
	DmOpenRef	MemoDB = DmOpenDatabaseByTypeCreator( 'DATA', sysFileCMemo, dmModeReadWrite );
	MemHandle	recH;
	Char			*text;
	UInt16		i=0, lines=0, recID, size=1, thissize, row, recordNum=0, attr=0;
	MemPtr		recP;
	LocalID 		localID;
	MemHandle	namesH;
	Char			name[32], *namep = name, creatorText[5], typeText[5], sizeStr[16];
	Err			err=0;
	UInt32		numRecordsP, totalBytesP, dataBytesP;

	if( !MemoDB )
		return false;

	namesH = MemHandleNew( 7 );
	text = (char*)MemHandleLock( namesH );
	StrCopy( text, "Filez\n" );
	text[6] = 0;
	MemHandleUnlock( namesH );
	size = 7;
	
	if( !cardMgr.currentCardIsInternal() )
		{
		size += StrLen( currentPath ) + 1;
		MemHandleResize( namesH, size );
		text = (char*)MemHandleLock( namesH );
		StrCat( text, currentPath );
		StrCat( text, "\n" );
		MemHandleUnlock( namesH );
		}
		
	for( row = 0; row < cache.getCount(); row++, recordNum++ )
		{
		if( cardMgr.currentCardIsInternal() )
			{
			localID = cache.getID( row );

			IntToStr( cache.getCreator( row ), creatorText );
			IntToStr( cache.getType( row ), typeText );
			DmDatabaseSize( cache.getCard(), localID, &numRecordsP, &totalBytesP, &dataBytesP );
			StrIToA( sizeStr, totalBytesP );
		
			thissize = StrLen( cache.getName( row )) + 4 + 4 + StrLen(sizeStr) + 3 + 1;
			}
		else
			{
			thissize = StrLen( cache.getName( row )) + 1;
			}
		size += thissize;
				
		MemHandleResize( namesH, size );
			
		text = (char*)MemHandleLock( namesH );
		StrCat( text, cache.getName( row ));

		if( cardMgr.currentCardIsInternal() )
			{
			StrCat( text, "," );
			StrCat( text, creatorText );
			StrCat( text, "," );
			StrCat( text, typeText );
			StrCat( text, "," );
			StrCat( text, sizeStr );
			}
		StrCat( text, "\n" );
	
		text[StrLen(text)] = 0;

		MemHandleUnlock( namesH );
		}

	recH = DmNewRecord( MemoDB, &recID, size );
	recP = MemHandleLock( recH );
	
	text = (char*)MemHandleLock( namesH );
	
	DmWrite( recP, 0, text, StrLen(text)+1 );
	MemHandleUnlock( recH );
	
	MemHandleUnlock( namesH );
	DmReleaseRecord( MemoDB, recID, true );
	DmCloseDatabase( MemoDB );
	MemHandleFree( namesH );
	
	return true;
	}


/**
 * Sets the bits of all files to some setting.
 *
 * @param what the user selected
 */
void SetAllBits( UInt16 w )
	{
	UInt16	i, attr;
	Char		c1[25], c2[25];
	
	switch( w )
		{
		case ListSetBackupOn:
		case ListSetBasicBackupOn:
			StrCopy( c1, "backup" );
			StrCopy( c2, "on" );
			break;
		case ListSetBackupOff:
		case ListSetBasicBackupOff:
			StrCopy( c1, "backup" );
			StrCopy( c2, "off" );
			break;
		case ListSetCopyProtectOn:
		case ListSetBasicCopyProtectOn:
			StrCopy( c1, "copy protect" );
			StrCopy( c2, "on" );
			break;
		case ListSetCopyProtectOff:
		case ListSetBasicCopyProtectOff:
			StrCopy( c1, "copy protect" );
			StrCopy( c2, "off" );
			break;		
		}

   // make sure the user actually has some file selected.
   if( cache.getSelectedCount() == 0 )
      { FrmAlert( 3800 ); return; }

	if( FrmCustomAlert( ConfirmAllBitsAlert, c1, c2, 0 ) == ConfirmAllBitsNo )
		return;
	
	for( i=0; i<cache.getCount(); i++ )
		if( cache.IsSelected( cache.getID( i )))
		{
		DmDatabaseInfo( cache.getCard(), cache.getID( i ), 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		if( MemLocalIDKind( cache.getID( i )) != memIDHandle ) continue;							// skip rom files

		switch( w )
			{
			case ListSetBackupOn:
			case ListSetBasicBackupOn:
				attr |= dmHdrAttrBackup;
				break;
			case ListSetBackupOff:
			case ListSetBasicBackupOff:
				if( attr & dmHdrAttrBackup )
					attr ^= dmHdrAttrBackup;
				break;
			case ListSetCopyProtectOn:
			case ListSetBasicCopyProtectOn:
				attr |= dmHdrAttrCopyPrevention;
				break;
			case ListSetCopyProtectOff:
			case ListSetBasicCopyProtectOff:
				if( attr & dmHdrAttrCopyPrevention )
					attr ^= dmHdrAttrCopyPrevention;
				break;		
			}
						
		DmSetDatabaseInfo( cache.getCard(), cache.getID( i ), 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		}
		
	ListViewUpdateList();
	}


#define memHeapFlagReadOnly	0x0001

/**
 * Determines if the file is in flash memory or not.
 *
 * @param the card and id of the file
 * @return true if in flash ram, false if not
 */
Boolean FileIsInFlash( UInt16 cardNo, LocalID localID )
	{
	if( MemLocalIDKind( localID ) == memIDHandle )					// definitely not in flash if we get a handle
		return false;
		
	if( MemHeapFlags( MemPtrHeapID( MemLocalIDToGlobal( localID, cardNo ))) & memHeapFlagReadOnly )
		return true;											// database is in flash or masked ROM
	else
		return false;											// database is in RAM
	}


/**
 * Return the processor id string of the processor.
 *
 * @param the destination string
 */
void GetProcessorID( Char *str )
	{
	// If we're not using a new include file, define things appropriately:
	#ifndef sysFtrNumProcessorID
	#define sysFtrNumProcessorID 2

	// Product id
	// 0xMMMMRRRR, where MMMM is the processor model and RRRR is the revision.
	#define sysFtrNumProcessorMask 0xFFFF0000					// Mask to obtain processor model
	#define sysFtrNumProcessor328 0x00010000					// Motorola 68328 (Dragonball)
	#define sysFtrNumProcessorEZ 0x00020000						// Motorola 68EZ328 (Dragonball EZ)
	#endif

	UInt32	id, chip;
	Err		err;
	UInt16	revision;

	err = FtrGet( sysFtrCreator, sysFtrNumProcessorID, &id );
	if( err )
		{ 
		// Can't get that feature; we must be on an old unit without it defined, thus we're on a DragonBall.
		StrCopy( str, "DragonBall" );
		}
	else
		{
		chip = id & sysFtrNumProcessorMask;
		revision = id & 0x0ffff;

		if( chip==sysFtrNumProcessor328 )
			StrCopy( str, "DragonBall" );
		else if (chip==sysFtrNumProcessorEZ)
			StrCopy( str, "DragonBall EZ" );
		else
			StrCopy( str, "unknown" );
		}
	} 

