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

#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include <VFSMgr.h>
#include <BtLibTypes.h>

#include "Stuph.h"
#include "Resource.h"
#include "FilterForm.h"
#include "Main.h"


void Item::free()
   {
   Err err = 0;
   
   if( file )
      {
      file->free();
      err = MemPtrFree( file );
      checkError( err, "Item::free()", 0 );
      file = 0;
      }
      
   if( folder )
      {
      folder->free();
      err = MemPtrFree( folder );
      checkError( err, "Item::free()", 0 );
      folder = 0;
      }
   }


Boolean Item::remove( Boolean ask, Err &err )
   {
   err = errNone;

	if( ask )
		{
		if( FrmAlert( DeleteFileAlert ) == DeleteFileCancel )				// if cancelled, don't do anything
			return false;
		}

   if( DEBUG ) return true;

   if( file )
      {
      if( file->eAttr )
         {
         // we're deleting a VFS file

         Char *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( file->parent->path ) + StrLen( getName() ) + 1 );
         StrPrintF( fullFilenameAndPath, "%s%s", file->parent->path, getName() );         
         
         err = VFSFileDelete( file->getVolumeNum(), fullFilenameAndPath );
         checkError( err, "Item::remove() a", fullFilenameAndPath );
         MemPtrFree( fullFilenameAndPath );
         }
      else
         {
         UInt16 attr = 0;
         // we're deleting an internal file

         // need to check here and make sure the file does not have the read-only
         //	attribute set.  having it set will prevent the file from being deleted.
			
         err = DmDatabaseInfo( file->getVolumeNum(), file->iAttr->id, 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
         checkError( err, "Item::remove() b", 0 );
         attr &= ~dmHdrAttrReadOnly;
         err = DmSetDatabaseInfo( file->getVolumeNum(), file->iAttr->id, 0, &attr, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
         checkError( err, "Item::remove() c", 0 );
         err = DmDeleteDatabase( file->getVolumeNum(), file->iAttr->id );
         checkError( err, "Item::remove() d", 0 );
         }
      }
   
   if( folder )
      {
      folder->remove( err );
      err = VFSFileDelete( folder->getVolumeNum(), folder->path );
      checkError( err, "Item::remove() e", folder->path );
      }

   return true;
   }
   
   
void Item::setAllSelected( Boolean on )
   {
   // the select all operation should only select visible (i.e. not filtered)
   //  files.
   if( filtered )
      return;

   // if we're unselecting, we unselect both files and folders.
   if( on == false )
      selected = false;

   // if we're setting it on, then see what we want to affect
   if( prefs.list.selectAll == selectAllBoth )
      selected = on;
   else
      {
      if( prefs.list.selectAll == selectAllFiles && file )
         selected = on;
   
      if( prefs.list.selectAll == selectAllFolders && folder )
         selected = on;
      }
      
   if( folder )
      folder->setAllSelected( on );
   }


void Item::setAllAttributes( UInt16 attr, Boolean set )
   {
   if( file )
      {
      if( file->iAttr && selected )
         {
         if( attr & dmHdrAttrReadOnly )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrReadOnly;
            else
               file->iAttr->attr &= ~dmHdrAttrReadOnly;
            }
            
         if( attr & dmHdrAttrOKToInstallNewer ) 
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrOKToInstallNewer;
            else
               file->iAttr->attr &= ~dmHdrAttrOKToInstallNewer;
            }

         if( attr & dmHdrAttrBackup )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrBackup;
            else
               file->iAttr->attr &= ~dmHdrAttrBackup;
            }

         if( attr & dmHdrAttrResetAfterInstall )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrResetAfterInstall;
            else
               file->iAttr->attr &= ~dmHdrAttrResetAfterInstall;
            }

         if( attr & dmHdrAttrCopyPrevention )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrCopyPrevention;
            else
               file->iAttr->attr &= ~dmHdrAttrCopyPrevention;
            }
            
         if( attr & dmHdrAttrHidden )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrHidden;
            else
               file->iAttr->attr &= ~dmHdrAttrHidden;
            }

         if( attr & dmHdrAttrAppInfoDirty )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrAppInfoDirty;
            else
               file->iAttr->attr &= ~dmHdrAttrAppInfoDirty;
            }
            
         if( attr & dmHdrAttrLaunchableData )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrLaunchableData;
            else
               file->iAttr->attr &= ~dmHdrAttrLaunchableData;
            }
            
         if( attr & dmHdrAttrBundle )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrBundle;
            else
               file->iAttr->attr &= ~dmHdrAttrBundle;
            }

         if( attr & dmHdrAttrRecyclable )
            {
            if( set )
               file->iAttr->attr |= dmHdrAttrRecyclable;
            else
               file->iAttr->attr &= ~dmHdrAttrRecyclable;
            }
            
         updateFile( getName() );
         }
      }
   else
      {
      folder->setAllAttributes( attr, set );
      }
   }


void Item::sendToMemo( MemHandle textH, UInt8 depth, BusyIndicator *busyIndicator )
   {
   Char  *sizeStr = 0, *attrStr = 0, *nameStr = 0;

   busyIndicator->update();

   if( filtered )
      return;

   if( file )
      {
      sizeStr = (Char*)MemPtrNew( 16 );
      SizeToString( file->size, sizeStr, 6 );

      if( file->iAttr )
         {
         Char creatorStr[5];
         Char typeStr[5];

         IntToStr( file->iAttr->creator, creatorStr );
         IntToStr( file->iAttr->type, typeStr );
         
         attrStr = (Char*)MemPtrNew( StrLen( creatorStr ) + StrLen( typeStr ) + 2 );
         StrPrintF( attrStr, "%s,%s", creatorStr, typeStr );
         }
      }

   nameStr = (Char*)MemPtrNew( StrLen( getName()) + 2 );
   StrCopy( nameStr, getName());

   if( folder )
      StrCat( nameStr, "/" );

   UInt32 newSize = MemHandleSize( textH ) + 4 + depth;
   if( nameStr ) newSize += StrLen( nameStr );
   if( sizeStr ) newSize += StrLen( sizeStr ) + 1;
   if( attrStr ) newSize += StrLen( attrStr ) + 1;
   MemHandleResize( textH, newSize );
   
   Char *text = (Char*)MemHandleLock( textH );

   for( UInt8 i=0; i<depth; i++ )
      StrCat( text, "\t" );

   StrCat( text, nameStr );
   if( sizeStr )
      {
      StrCat( text, "," );
      StrCat( text, sizeStr );
      }
   if( attrStr )
      {
      StrCat( text, "," );
      StrCat( text, attrStr );
      }

   StrCat( text, "\n" );

   MemHandleUnlock( textH );

   if( attrStr )  MemPtrFree( attrStr );
   if( nameStr )  MemPtrFree( nameStr );
   if( sizeStr )  MemPtrFree( sizeStr );
   
   if( folder )
      folder->sendToMemo( textH, depth+1, busyIndicator );
   }


Err Item::sendVFSFile()
	{
   ExgSocketType	exgSocket;
   Err				err;
   Char           *final;

   // we check for bluetooth support, and if there is, then we add the
   //  ?_send: option to the filename so that the device will ask the
   //  user how they want to send the file.   
   //UInt32 btVersion;
   //if( FtrGet( btLibFeatureCreator, btLibFeatureVersion, &btVersion ) == errNone )
   if( prefs.sendMenu == 0 )
      {
      final = (Char*)MemPtrNew( StrLen( getName() ) + 32 );
      StrPrintF( final, "?_send;_beam:%s", getName());
      }
   else
      {
      final = (Char*)MemPtrNew( StrLen( getName()) + 1 );
      StrPrintF( final, "%s", getName());
      }

   // create exgSocket structure
   MemSet( &exgSocket, sizeof( exgSocket ), 0 );
   exgSocket.description = getName();
   exgSocket.name = final;
   
   exgSocket.length = file->size;
   exgSocket.count = 1;
   // they don't tell you this, but you have to allocate some sort of string
   //  for the mime type or else Palm OS will crash with a null string error. nice.
   exgSocket.type = (Char*)MemPtrNew( 1 );        // a pointer to the MIME type of the object;
   StrCopy( exgSocket.type, "" );   
		
   err = ExgPut( &exgSocket );
   if( err == exgErrUserCancel )
      return err;
   checkError( err, "Item::sendVFSFile() a", 0 );
   if( err != errNone )
      return err;
   
   // open the vfs file
   FileRef	srcFile;
   UInt32	bufSize = 1024;					// bytes
   UInt32	bytesRead /*, bytesWritten=0*/;
   Char		buf[1024];
   Boolean	more=true;
		   
   Char     *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( file->parent->path ) + StrLen( getName() ) + 2 );
   StrPrintF( fullFilenameAndPath, "%s%s", file->parent->path, getName() );

   // open the source
   err = VFSFileOpen( file->getVolumeNum(), fullFilenameAndPath, vfsModeRead, &srcFile );
   if( err != errNone )
      {
      showMessage( "Unable to open file %s.", getName());
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
         showMessage( "Error while reading source file %s.", getName());
         return err;
         }

      /*bytesWritten =*/ ExgSend( &exgSocket, &buf, bytesRead, &err );
      if( err == exgErrUserCancel )
         break;
      checkError( err, "Item::sendVFSFile() b", 0 );

            
      if( err != errNone )             // some other error
         {
         showMessage( "Error while sending the file %s.", getName());
         return err;
         }
      }
		
   err = VFSFileClose( srcFile );
   checkError( err, "Item::sendVFSFile() c", 0 );
   err = ExgDisconnect( &exgSocket, err );
   if( err != exgErrUserCancel )
      checkError( err, "Item::sendVFSFile() d", 0 );

   delete fullFilenameAndPath;
   delete final;

  	return err;
	}


/**
 * Callback for ExgDBWrite to send data with exchange manager.
 *
 * @param dataP : buffer containing data to send
 * @param sizeP : number of bytes to send
 * @param userDataP: app defined buffer for context (holds exgSocket when using ExgManager)
 * @return error if non-zero
 */
 /*
static Err writeDBData( const void* dataP, UInt32* sizeP, void* userDataP )
	{
   Err		err;

   // try to send as many bytes as were requested by the caller
   *sizeP = ExgSend((ExgSocketPtr)userDataP, (void*)dataP, *sizeP, &err );
   if( err == exgErrUserCancel )
      return err;
   checkError( err, "Item::writeDBData()", 0 );

   return err;
	}
*/
/*
Err Item::sendDBFile()
   {
   ExgSocketType  exgSocket;
   Err				err;
   Char           *filename, *final;

   filename = (Char*)MemPtrNew( StrLen( getName()) + 32 );  // lots of space for extra prefixes
   if( file->iAttr->attr & dmHdrAttrResDB )
      StrPrintF( filename, "%s.prc", getName());
   else
      StrPrintF( filename, "%s.pdb", getName());

   // we check for bluetooth support, and if there is, then we add the
   //  ?_send: option to the filename so that the device will ask the
   //  user how they want to send the file.   
   //UInt32 btVersion;
   //if( FtrGet( btLibFeatureCreator, btLibFeatureVersion, &btVersion ) == errNone )
   if( prefs.sendMenu == 0 )
      {
      final = (Char*)MemPtrNew( StrLen( filename ) + 32 );  // lots of space
      StrPrintF( final, "?_send;_beam:%s", filename );
      }
   else
      {
      final = (Char*)MemPtrNew( StrLen( filename ) + 1 );
      StrPrintF( final, "%s", filename );
      }

   // create exgSocket structure
   MemSet( &exgSocket, sizeof( exgSocket ), 0 );
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
   exgSocket.description = getName();  // set the description (optional)
   // they don't tell you this, but you have to allocate some sort of string
   //  for the mime type or else Palm OS will crash with a null string error. nice.
   exgSocket.type = (Char*)MemPtrNew( 1 );        // a pointer to the MIME type of the object;
   StrCopy( exgSocket.type, "" );   
   exgSocket.name = final;       // the object's name (note that colons here mess everything up since they are used in URLs

   err = DmDatabaseSize( file->getVolumeNum(), file->iAttr->id, 0, &exgSocket.length, 0 );
   checkError( err, "Item::sendDBFile() a", 0 );

   err = ExgPut( &exgSocket );
   if( err == exgErrUserCancel )
      return err;
   checkError( err, "Item::sendDBFile() b", 0 );

   if( err == errNone )
      {
      err = ExgDBWrite( writeDBData, &exgSocket, NULL, file->iAttr->id, file->getVolumeNum() );
      if( err == exgErrUserCancel )
         return err;
      checkError( err, "Item::sendDBFile() c", 0 );
      err = ExgDisconnect( &exgSocket, err );
      if( err != exgErrUserCancel )
         checkError( err, "Item::sendDBFile() d", 0 );
      }

   delete final;
   delete filename;
   return err;
   }
*/

void Item::send()
   {
   Err err = errNone;

   if( !file ) return;
   
   if( file->eAttr )
      err = sendVFSFile();
   else
      err = sendDB( file->iAttr->id, file->getVolumeNum(), prefs.sendMenu == 0 );
   
   if( err != errNone && err != exgErrUserCancel )
      showMessage( "An error occured and the item could not be sent" );
   }


void Item::sendSelectedItems()
   {
   if( file && selected )
      send();
   
   if( folder )
      folder->sendSelectedItems();
   }


void Item::updateFile( Char *oldName )
   {
   if( !file )
      {
      showMessage( "File missing in Item::updateFile(): %s", oldName );
      return;
      }
   file->updateFile( oldName );
   }


void Item::filter( Boolean turnOn )
   {
   if( file )
      {
      // if we're turning the filter off, then just set the flag
      if( !turnOn )
         {
         filtered = false;
         return;
         }

      if( file->eAttr )
         {
         Char *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( file->parent->path ) + StrLen( getName() ) + 1 );
         StrPrintF( fullFilenameAndPath, "%s%s", file->parent->path, getName() );         
         
         filterCheck();

         MemPtrFree( fullFilenameAndPath );
         }
      else
         filterCheck();
      }
   else
      {
      folder->filter( turnOn );
      }
   }


void Item::init()
   {
   indent = 0;
   file = 0;
   folder = 0;
   selected = false;
   filtered = false;
   }


Char* Item::getName()
   {
   if( !file && !folder )
      showMessage( "Item has no file or folder in Item::getName()" );

   if( file )
      return file->name;
   
   if( folder )
      return folder->name;

   return 0;
   }


void Item::setName( Char *newName )
   {
   if( !file && !folder )
      showMessage( "Item has no file or folder in Item::setName( %s )", newName );
   
   if( file )
      file->setFileName( newName );
   
   if( folder )
      folder->setFolderName( newName );
   }


/**
 * Compare two strings.
 *
 * @param the strings
 * @return compare two strings
 */
Boolean Item::filterStringCompare( Char s1[], Char s2[] )
	{
	UInt16	len;
	Char		tmp[maxFilterString], tmp2[maxFilterString];
	
	switch( prefs.filter.comparator )
		{
		case filterBegins:
			len = StrLen( s2 );									// see how long what they entered is
			if( len > StrLen( s1 ) ) return false;						// if they entered something longer than the file
			if( StrNCaselessCompare( s1, s2, len ) ) return false;		// see if their first len chars differ or not
			break;
			
		case filterContains:
			StrToLower( tmp, s1 );
			StrToLower( tmp2, s2 );
			if( !StrStr( tmp, tmp2 ) )	return false;
			break;
			
		case filterEnds:
			len = StrLen( s2 );									// see how long the search string is
			if( len > StrLen( s1 ) ) return false;						// if they entered something longer than the file
			StrCopy( tmp, s1+( StrLen( s1 ) - len ) );					// copy the last n chars of the filename
			if( StrNCaselessCompare( tmp, s2, len ) ) return false;
			break;
		}
		
	return true;
	}


/**
 * Compare two numbers.
 *
 * @param i1 - the name of the file/creator/type
 * @param i2 - what the person typed in
 * @return result
 */
Boolean Item::filterNumberCompare( UInt32 i1, UInt32 i2 )
	{
	switch( prefs.filter.comparator )
		{
		case filterBegins:
			if( i1 < i2 ) return true;
			else return false;
			break;
			
		case filterContains:
			if( (UInt32)(i1/1024) == (UInt32)(i2/1024) ) return true;
			else return false;
			break;
			
		case filterEnds:
			if( i1 > i2 ) return true;
			else return false;
			break;
		}
		
	return true;
	}


/**
 * See if the attributes of the file match the current attribute selection.
 *
 * @param the attributes of the file
 * @return result
 */
Boolean Item::filterAttrCompare( UInt16 attr )
	{
   Boolean filtered = true;
	// compare the attributes passed with the global ones specified in the filter details form
	
	if(( prefs.filter.attr & dmHdrAttrReadOnly ) && ( attr & dmHdrAttrReadOnly ))
		filtered = false;

	if(( prefs.filter.attr & dmHdrAttrBackup ) && ( attr & dmHdrAttrBackup ))
		filtered = false;

	if(( prefs.filter.attr & dmHdrAttrCopyPrevention ) && ( attr & dmHdrAttrCopyPrevention ))
		filtered = false;

	if(( prefs.filter.attr & dmHdrAttrResDB ) && ( attr & dmHdrAttrResDB ))
		filtered = false;

   return !filtered;
   }


/**
 * This checks to see if this file passes the current filter, and then sets
 * the filtered flag accordingly.
 *
 * @return result
 */
void Item::filterCheck()
	{
   filtered = false;

   // ok, now (10/4/04) i'm thinking that folders shouldn't be filtered at
   //  all, so don't for now.
   if( !file )
      {
      filtered = false;
      return;
      }

   // the only criteria that could filter both files and folders is by name,
   //  but for now we don't filter folders.
   switch( prefs.filter.criteria )
		{
		case filterName:
			filtered = !filterStringCompare( getName(), prefs.filter.string );
         goto end;
      }

   // the size criteria can work with any type of file
   switch( prefs.filter.criteria )
      {
		case filterSize:
         {
         UInt32 size = StrAToI( prefs.filter.string );
			size *= 1024;											// convert it to kb
			filtered = !filterNumberCompare( file->size, size );
         goto end;
         }
      }
   
   // if we're filtering by a database-specific criteria, then if a file is
   //  not a database, then it should not be displayed.
   if( !file->iAttr )
      {
      filtered = true;
      goto end;
      }
   
   // if we made it here, then we're filtering by a database-specific criteria   
	switch( prefs.filter.criteria )
		{
		case filterCreator:
         {
         Char creator[5];
         IntToStr( file->iAttr->creator, creator );
			filtered = !filterStringCompare( creator, prefs.filter.string );
         goto end;
			}
         
		case filterType:
         {
         Char type[5];
         IntToStr( file->iAttr->type, type );
			filtered = !filterStringCompare( type, prefs.filter.string );
         goto end;
			}
			
		case filterRec:
         {
         UInt32 recCount = StrAToI( prefs.filter.string );
			filtered = !filterNumberCompare( file->iAttr->recCount, recCount );
         goto end;
         }
         
		case filterAttr:
			filtered = !filterAttrCompare( file->iAttr->attr );
         goto end;
		}

   // we do the horrible goto here to ensure that we always apply the
   //  'not' filter option.
   end:
   
   if( prefs.filter.notbox )
      filtered = !filtered;
   }


void Item::setSelected( Boolean s )
   {
/*   if( s )
      {
      Char msg[256];
      StrPrintF( msg, "Selected: %s", getName());
      showMessage( msg );
      }
  */ 
   selected = s;
   }
   

Boolean Item::getSelected()
   {
   return selected;
   }


void Item::setFiltered( Boolean f )
   {
   filtered = f;
   }
   

Boolean Item::getFiltered()
   {
   return filtered;
   }


void Item::setIndent( Int8 i )
   {
   indent = i;
   }
   

Int8 Item::getIndent()
   {
   return indent;
   }

void Item::setFile( ItemFile *f )
   {
   file = f;
   }
   

ItemFile* Item::getFile()
   {
   return file;
   }
   
   
void Item::setFolder( ItemFolder *f )
   {
   folder = f;
   }


ItemFolder* Item::getFolder()
   {
   return folder;
   }


void Item::setVolumeNum( UInt32 volumeNum )
   {
   if( file )
      file->setVolumeNum( volumeNum );
   
   if( folder )
      folder->setVolumeNum( volumeNum );
   }
   

UInt32 Item::getVolumeNum()
   {
   if( file )
      return file->getVolumeNum();
   
   if( folder )
      return folder->getVolumeNum();
   return 0;
   }
