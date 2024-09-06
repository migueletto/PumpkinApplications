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
 * Handles the folder details form.
 *
 * Created on 8/25/04 by Tom Bulatewicz
 */
  
#include <PalmOS.h>
#include <VFSMgr.h>
#include "resize.h"

#include "Main.h"
#include "Resource.h"
#include "Stuph.h"
#include "UI.h"
#include "FolderDetails.h"
#include "Item.hpp"
#include "ItemSet.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "TreeViewForm.h"
#include "BusyIndicator.hpp"

#include "debug.h"

static Item    *item;
static UInt8   invalid;
static Boolean enumProblem;
static UInt32  count = 0;
static UInt32  size = 0;


static Boolean SECT7 handleOKButton( EventPtr event )
	{
	Err      err = errNone;
   Char     *fullFilenameAndPath = 0;

	// get new name
	FieldPtr	fld = GetObjectPtr<FieldType>( FolderDetailsNameField );

   // first check: make sure they enter a folder name
   if( !FldGetTextPtr( fld ))
      {
      showMessage( "Folder name must not be empty" );
      return true;
      }

   if( StrCompare( FldGetTextPtr( fld ), item->getName()) == 0 )
      goto end;
   
   // second check: make sure that the new folder name is clean
   if( checkForInvalidVFSCharacters( FldGetTextPtr( fld ), RenameFileAlert, RenameFileCancel ) != errNone )
      {
      FrmGotoForm( TreeViewForm );
      FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
      return true;
      }

   if( item->getFolder()->getIsVolume())
      {
      if( item->getFolder()->getIsVolume() == volExternal )
         {
         err = VFSVolumeSetLabel( item->getVolumeNum(), FldGetTextPtr( fld ));
         checkError( err, "handleOKButton() a", 0 );
         
         // also need to update the cardset cache
         }
      }
   else
      {
      // get the full path and folder name of the folder we want to rename
      fullFilenameAndPath = (Char*)MemPtrNew( StrLen( item->getFolder()->path ) + 1 );
      StrPrintF( fullFilenameAndPath, "%s", item->getFolder()->path );
      
      // this takes the original full path/filename, and the new name
      err = VFSFileRename( item->getVolumeNum(), fullFilenameAndPath, FldGetTextPtr( fld ));
      checkError( err, "FolderOps::handleOKButton() b", 0 );
      }
      
   delete fullFilenameAndPath;

   end:

   FrmReturnToForm( TreeViewForm );
   FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );
   return true;
   }
   

static Boolean SECT7 handleCancelButton( EventPtr event )
	{
   FrmReturnToForm( TreeViewForm );
   return true;
   }
   

static Boolean SECT7 handleWarningButton( EventPtr event )
	{
   Char msg[256];

   if( invalid )
      {
      StrPrintF( msg, "%d file name(s) contain illegal characters. These files will not appear in the file list.", invalid );
      showMessage( msg );
      }

   if( enumProblem )
      {
      StrPrintF( msg, "There was an error reading items in this folder. Some items may not appear in the file list." );
      showMessage( msg );
      }

   return true;
   }
   

static Boolean SECT7 handleSelectEvent( EventPtr event )
	{
   Boolean handled = false;
   
   switch( event->data.ctlSelect.controlID )
      {
      case FolderDetailsOKButton:
         handled = handleOKButton( event );
         break;

      case FolderDetailsCancelButton:
         handled = handleCancelButton( event );
         break;

      case FolderDetailsWarningButton:
         handled = handleWarningButton( event );
         break;
      }
   
   return handled;
   }


static void SECT7 FolderDetailsGetStatsVFS( Char *path, UInt32 *count, UInt32 *size, BusyIndicator *busyIndicator )
   {
	FileRef			dirRef=0;
	Err				err;
	UInt32			fileIterator;
	FileInfoType	fileInfo;
	Char           *fileName;
   //UInt16         i=0;
   FileRef        fileRefP;

	err = VFSFileOpen( item->getVolumeNum(), path, vfsModeRead, &dirRef );
   checkError( err, "FolderDetailsGetStats() a", path );

	fileName = (Char*)MemPtrNew( 256 );
   checkMemPtr( fileName, "FolderDetailsGetStats() b" );
	
	fileInfo.nameP = fileName;
   fileInfo.nameBufLen = 256;

	fileIterator = expIteratorStart;
	while( fileIterator != expIteratorStop )
		{
      busyIndicator->update();
      
		err = VFSDirEntryEnumerate( dirRef, &fileIterator, &fileInfo );

      if( err != errNone && err != expErrEnumerationEmpty )
         enumProblem = true;

      if( err != errNone )
         break;

      if( hasInvalidVFSCharacters( fileName ))
         {
         invalid++;
         continue;
         }

      Char *newPath = (Char*)MemPtrNew( StrLen( path ) + StrLen( fileName ) + 3 );
      if( path[StrLen(path)-1] == '/' )
         StrPrintF( newPath, "%s%s", path, fileName );
      else
         StrPrintF( newPath, "%s/%s", path, fileName );

		if( fileInfo.attributes & vfsFileAttrDirectory )
         {
         FolderDetailsGetStatsVFS( newPath, count, size, busyIndicator );
         }
      else
         {
         (*count)++;
         
			err = VFSFileOpen( item->getVolumeNum(), newPath, vfsModeRead, &fileRefP );			
			checkError( err, "ItemFolder::readItems() j", newPath );

         UInt32 fileSize = 0;
         if( err == errNone && fileRefP )
            err = VFSFileSize( fileRefP, &fileSize );

         (*size) += fileSize;
         
         err = VFSFileClose( fileRefP );
         }

      MemPtrFree( newPath );
		}

   if( fileName )
      {
      err = MemPtrFree( fileName );
      checkError( err, "FolderDetailsGetStats() u", 0 );
      }

   err = VFSFileClose( dirRef );
   checkError( err, "FolderDetailsGetStats() v", 0 );
   }


static void SECT7 FolderDetailsGetStatsInternal( UInt32 *count, UInt32 *size )
   {
   UInt32      numRecords;
   UInt32      totalBytes;
   UInt32      dataBytes;
   LocalID     dbID;
   //Err         err;

   (*count) = DmNumDatabases( item->getVolumeNum());
   (*size) = 0;

   for( UInt16 i=0; i<(*count); i++ )
      {
      dbID = DmGetDatabase( item->getVolumeNum(), i );
      /*err =*/ DmDatabaseSize( item->getVolumeNum(), dbID, &numRecords, &totalBytes, &dataBytes );
      (*size) += totalBytes;
      }
   }


static void SECT7 FolderDetailsCalcStats()
   {
   size = 0;
   count = 0;
   
   if( item->getFolder()->getIsVolume() == volInternal ) {
      FolderDetailsGetStatsInternal( &count, &size );
   } else {
      BusyIndicator *busyIndicator = new BusyIndicator( 50, 5 );
      FolderDetailsGetStatsVFS( item->getFolder()->path, &count, &size, busyIndicator );
      busyIndicator->erase();
      delete busyIndicator;
      }
   }
   

static void SECT7 FolderDetailsDrawStats()
   {
   Char     str[16];
      
   StrIToA( str, count );
   WinDrawChars( str, StrLen( str ), 44, 42 );

   SizeToString( size, str, 7 );
   WinDrawChars( str, StrLen( str ), 44, 56 );
   
   FormPtr  frm = FrmGetActiveForm();
   
   if( invalid || enumProblem )
      {
      ShowObject( frm, FolderDetailsWarningButton );
      DrawBitmapSimple( WarningBitmap, 113, 58 );
      }
   }


static Boolean SECT7 handleUpdateEvent()
   {
   FrmDrawForm( FrmGetActiveForm());
   FolderDetailsDrawStats();
   
   return true;
   }
   

static Boolean SECT7 handleFormOpen( EventPtr event )
	{
	Char        *p;
	MemHandle	h;

   invalid = 0;
   enumProblem = false;
   
	FormPtr	frm = FrmGetActiveForm();

   ItemSet *itemSet = tree->getSelectedItems();
   item = itemSet->getItem( 0 );
   delete itemSet;

	h = MemHandleNew( StrLen( item->getName()) + 1 );
	p = (Char*)MemHandleLock( h );
	StrCopy( p, item->getName());
	MemHandleUnlock( h );
	SetFieldTextFromHandle( FolderDetailsNameField, h, false );

	FrmDrawForm( frm );	

   // if it's the internal card folder, you can't rename it
   if( item->getFolder()->getIsVolume() == volInternal )
      FldSetUsable( GetObjectPtr<FieldType>( FolderDetailsNameField ), false );

   FolderDetailsCalcStats();
   FolderDetailsDrawStats();
	
   return true;
   }


/**
 * Handle the directory rename dialog.
 *
 * @param an event
 * @return if it was handled
 */
Boolean FolderDetailsHandleEvent( EventPtr event )
	{
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleFormOpen( event );
			break;

		case ctlSelectEvent:
         handled = handleSelectEvent( event );
         break;
         
      case frmUpdateEvent:
         handled = handleUpdateEvent();
         break;
    default:
      break;
		}
	
	return handled;
	}

