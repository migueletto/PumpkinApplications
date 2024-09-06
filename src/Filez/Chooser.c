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
 * Display a form where the user can select a volume.
 *
 * Created on 12/21/01 by Tom Bulatewicz
 */
  
#include <PalmOS.h>					// all the system toolbox headers
#include <FeatureMgr.h>				// needed to get the ROM version
#include <ExgMgr.h>
#include <DLServer.h>				// to get hotsync username
#include <VFSMgr.h>
#include "resize.h"

#include "Main.h"
#include "Resource.h"				// application resource defines
#include "Stuph.h"
#include "UI.h"
#include "Chooser.h"

#include "TreeView.hpp"
#include "Item.hpp"
#include "ItemSet.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"

#include "TreeViewForm.h"
#include "BusyIndicator.hpp"
#include "SelectionChecker.hpp"
#include "TreeViewForm.h"

UInt8    chooserOp;

static TreeView   *ctree = 0;

/**
 * Call the record drawing method of the tree object.
 */
static void SECT8 drawRec( void *tableP, Int16 row, Int16 column, RectanglePtr bounds )
   {
   TablePtr table = *((TablePtr*)&tableP);
   if( ctree )
      ctree->drawRecord( row, column, bounds, table );
   }
   

static Boolean SECT8 handleFormClose( EventPtr event )
   {
   Err err = 0;

   if( ctree == 0 )
      showMessage( "tree not allocated in handleFormClose()." );
   else
      {
      ctree->free();
      err = MemPtrFree( ctree );
      checkError( err, "handleFormClose() a", 0 );
      ctree = 0;
      }
   return false;
   }


/**
 */
static Boolean SECT8 handleFormOpen( EventPtr event )
	{
   FormPtr     frm = FrmGetActiveForm();
	FrmDrawForm( frm );

   // if we're copying or moving a file, then hide the name field and 
   //  show the copy/move label.
   if( chooserOp == copyFile || chooserOp == moveFile )
      {
      HideObject( frm, ChooserNameField );
      HideObject( frm, ChooserNameLabel );
      ShowObject( frm, ChooserCopyLabel );
      }
   else
      {
      HideObject( frm, ChooserCopyLabel );
      ShowObject( frm, ChooserNameField );
      ShowObject( frm, ChooserNameLabel );
      }      
   
   // setup the tree view
   ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( ChooserScrollBar );
	TablePtr table = GetObjectPtr<TableType>( ChooserTable );
   ctree = (TreeView*)MemPtrNew( sizeof( TreeView ));
   ctree->init();
	TblSetCustomDrawProcedure( table, 0, drawRec );
   ctree->setup( table, true );
   ctree->updateTable( table, scrollBar, true );

   return true;
	}


Err SECT8 progressCallBack( UInt32 totalBytes, UInt32 offset, void *userData )
   {
   BusyIndicator *busyIndicator = (BusyIndicator*)userData;
   busyIndicator->update();
   
   return errNone;
   }
   

void SECT8 splitPath( Char *path, Char *newPath, Char *newName )
   {
   Int16 pos = -1;
   Int16 len = StrLen( path );

   // find the last / in the path
   for( int i=StrLen( path )-1; i >= 0 ; i-- )
      {
      if( path[i] == '/' )
         {
         pos = i;
         break;
         }
      }

   ErrFatalDisplayIf( pos == -1, "Unable to find / in splitPath()" );

   StrNCopy( newPath, path, pos+1 );
   newPath[pos+1] = 0;
         
   StrNCopy( newName, path+pos+1, len - pos - 1 );
   newName[len - pos - 1] = 0;
   }


static Err SECT8 copyItemVFSToInternal( Item *file, Item *destFolder )
   {
   Err err;
   BusyIndicator *busyIndicator = new BusyIndicator( defaultY, 5 );
   Char origName[dmDBNameLength];
   Char tempFilename[dmDBNameLength] = "FiLeZtEmPfIlE";

   // copying a vfs file to internal memory
   UInt16   volRefNum = file->getVolumeNum();

   Char *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( file->getFile()->parent->path ) + StrLen( file->getName() ) + 1 );
   StrPrintF( fullFilenameAndPath, "%s%s", file->getFile()->parent->path, file->getName() );

   UInt16   cardNo;
   LocalID  dbID;

   err = VFSImportDatabaseFromFileCustom( volRefNum, fullFilenameAndPath, &cardNo, &dbID, progressCallBack, busyIndicator );
   if( err == dmErrAlreadyExists )
      {
      // ask the user if they want to overwrite the file
      UInt16 response = FrmCustomAlert( OverwriteAlert, file->getName(), 0, 0 );
      if( response == OverwriteYes )
         {
         UInt32 origCreator;
         UInt32 origType;
         
         err = DmDatabaseInfo( cardNo, dbID, origName, 0, 0, 0, 0, 0, 0, 0, 0, &origType, &origCreator );
         // they gave us the ok, so just rename the database right now in
         //  case the copy fails, we can recover the original file.
         err = DmSetDatabaseInfo( cardNo, dbID, tempFilename, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
         checkError( err, "copyItemVFSToInternal() a", origName );

         // we have to do this to avoid a bug in PalmOS 5 that does not it when
         //  you rename a database and then delete it shortly thereafter
         SysNotifyBroadcast( NULL );

         UInt16 newCardNo;
         LocalID newDbID;
               
         // now lets try the copy operation again
         if( err == errNone )
            err = VFSImportDatabaseFromFileCustom( volRefNum, fullFilenameAndPath, &newCardNo,  &newDbID, progressCallBack, busyIndicator );

         // now that we're done copying the file, we have to delete the
         //  original file (or rename it back if there was an error).
         if( err == errNone )
            {
            err = DmDeleteDatabase( cardNo, dbID );
            checkError( err, "copyItemVFSToInternal() b", 0 );
            }
         else
            {
            // delete the newly copied (and invalid) database
            // this is only necessary if it is possible for the VFSImport call
            //  to create and leave a database around when it fails. if it
            //  always cleans up afterward, then we don't need this call.
            DmDeleteDatabase( newCardNo, newDbID );

            Err rerr = DmSetDatabaseInfo( cardNo, dbID, origName, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
            checkError( rerr, "copyItemVFSToInternal() c", origName );
            }

         checkError( err, "copyItemVFSToInternal() d", fullFilenameAndPath );
         }
      else
         err = exgErrUserCancel;
      }
   
   Err merr = MemPtrFree( fullFilenameAndPath );
   checkError( merr, "copyItemVFSToInternal() e", 0 );

   delete busyIndicator;
   return err;
   }


static Err SECT8 copyItemVFSToVFS( Item *file, Item *destFolder )
   {
   Err err;
   BusyIndicator *busyIndicator = new BusyIndicator( defaultY, 5 );
   Char tempFilename[dmDBNameLength] = "FiLeZtEmPfIlE";

   Char *srcFullFilenameAndPath = (Char*)MemPtrNew( StrLen( file->getFile()->parent->path ) + StrLen( file->getName() ) + 1 );
   StrPrintF( srcFullFilenameAndPath, "%s%s", file->getFile()->parent->path, file->getName() );
         
   Char *destFullFilenameAndPath = (Char*)MemPtrNew( StrLen( destFolder->getFolder()->path ) + StrLen( file->getName() ) + 1 );
   StrPrintF( destFullFilenameAndPath, "%s%s", destFolder->getFolder()->path, file->getName() );

   err = VFSFileCopy( file->getFile()->parent->getVolumeNum(), srcFullFilenameAndPath, destFolder->getVolumeNum(), destFullFilenameAndPath, progressCallBack, busyIndicator );
   if( err == vfsErrFileAlreadyExists )
      {
      // ask the user if they want to overwrite the file (i.e. delete and copy/move)
      UInt16 response = FrmCustomAlert( OverwriteAlert, destFullFilenameAndPath, 0, 0 );
      if( response == OverwriteYes )
         {
         // they gave us the ok, so only rename the file right now in case
         //  the copy fails, we can recover the original file.
         err = VFSFileRename( destFolder->getVolumeNum(), destFullFilenameAndPath, tempFilename );

         checkError( err, "copyItemVFSToVFS() a", destFullFilenameAndPath );
               
         // now lets try the copy operation again
         if( err == errNone )
            err = VFSFileCopy( file->getFile()->parent->getVolumeNum(), srcFullFilenameAndPath, destFolder->getVolumeNum(), destFullFilenameAndPath, progressCallBack, busyIndicator );

         // find the full path to the temporarily renamed file (mem has an extra 32 bytes)
         Char tempPath[StrLen(destFullFilenameAndPath)+1+32];
         Char tempName[StrLen(destFullFilenameAndPath)+1];
         splitPath( destFullFilenameAndPath, tempPath, tempName );
         StrCat( tempPath, tempFilename );   // add our unique temp name

         // now that we're done copying the file, we have to delete the
         //  original file (or rename it back if there was an error).
         if( err == errNone )
            {
            err = VFSFileDelete( destFolder->getVolumeNum(), tempPath );
            checkError( err, "copyItemVFSToVFS() b", tempPath );
            }
         else
            {
            Err rerr = VFSFileRename( destFolder->getVolumeNum(), tempPath, tempName );
            checkError( rerr, "copyItemVFSToVFS() c", tempPath );
            }

         checkError( err, "copyItemVFSToVFS() d", 0 );
         }
      else
         err = exgErrUserCancel;
      }
   
   Err merr = MemPtrFree( srcFullFilenameAndPath );
   checkError( merr, "copyItemVFSToVFS() e", 0 );
   merr = MemPtrFree( destFullFilenameAndPath );
   checkError( merr, "copyItemVFSToVFS() f", 0 );

   delete busyIndicator;
   return err;
   }


static Err SECT8 copyItemInternalToVFS( Item *file, Item *destFolder )
   {
   Err err;
   BusyIndicator *busyIndicator = new BusyIndicator( defaultY, 5 );
   Char tempFilename[dmDBNameLength] = "FiLeZtEmPfIlE";

   // copying an internal file to a vfs folder
   UInt16   volRefNum = destFolder->getVolumeNum();
         
   Char *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( destFolder->getFolder()->path ) + StrLen( file->getName() ) + 5 );
   StrPrintF( fullFilenameAndPath, "%s%s", destFolder->getFolder()->path, file->getName() );

   if( file->getFile()->iAttr->attr & dmHdrAttrResDB )					// if a resource file
      StrCat( fullFilenameAndPath, ".prc" );
   else
      StrCat( fullFilenameAndPath, ".pdb" );		
         
   err = VFSExportDatabaseToFileCustom( volRefNum, fullFilenameAndPath, file->getVolumeNum(), file->getFile()->iAttr->id, progressCallBack, busyIndicator );
   if( err == vfsErrFileAlreadyExists )
      {
      // ask the user if they want to overwrite the file
      UInt16 response = FrmCustomAlert( OverwriteAlert, file->getName(), 0, 0 );
      if( response == OverwriteYes )
         {
         // they gave us the ok, so rename the file for now in case the
         //  copy fails and we need to recover the original file.
         err = VFSFileRename( volRefNum, fullFilenameAndPath, tempFilename );
         checkError( err, "copyItemInternalToVFS() a", fullFilenameAndPath );
         
         // now lets try the copy operation again
         if( err == errNone )
            err = VFSExportDatabaseToFileCustom( volRefNum, fullFilenameAndPath, file->getVolumeNum(), file->getFile()->iAttr->id, progressCallBack, busyIndicator );

         // find the full path to the temporarily renamed file (mem has an extra 32 bytes)
         Char tempPath[StrLen(fullFilenameAndPath)+1+dmDBNameLength];
         Char tempName[StrLen(fullFilenameAndPath)+1];
         splitPath( fullFilenameAndPath, tempPath, tempName );
         StrCat( tempPath, tempFilename );   // add our unique temp name

         // now that we're done copying the file, we have to delete the
         //  original file (or rename it back if there was an error).
         if( err == errNone )
            {
            err = VFSFileDelete( volRefNum, tempPath );
            checkError( err, "copyItemInternalToVFS() b", tempPath );
            }
         else
            {
            Err rerr = VFSFileRename( volRefNum, tempPath, tempName );
            checkError( rerr, "copyItemInternalToVFS() c", tempPath );
            }
         checkError( err, "copyItemInternalToVFS() d", fullFilenameAndPath );
         }
      else
         err = exgErrUserCancel;
      }
         
   Err merr = MemPtrFree( fullFilenameAndPath );
   checkError( merr, "copyItemInternalToVFS() e", 0 );

   delete busyIndicator;
   return err;
   }


static Err SECT8 copyItem( Item *file, Item *destFolder )
   {
   Err      err = 0;
   Boolean  copyToInternal = false;
   
   // check to make sure that there is enough free space for the copy op
   
   UInt32 fileSize = file->getFile()->getSize();
   UInt32 cardFree = 0;

   if( destFolder->getFolder()->getIsVolume() == volInternal )
      {
      err = MemCardInfo( destFolder->getVolumeNum(), 0, 0, 0, 0, 0, 0, &cardFree );
      }
   else
      {
      UInt32 cardTotal, cardUsed;
      err = VFSVolumeSize( destFolder->getVolumeNum(), &cardUsed, &cardTotal );
      cardFree = cardTotal - cardUsed;
      }
   checkError( err, "copyItem() a", 0 );
/*
   Char msg[256];
   StrPrintF( msg, "file: %ld, free: %ld", fileSize, cardFree );
   showMessage( msg );
  */ 
   if( cardFree < fileSize )
      {
      showMessage( "Not enough space on card." );
      return exgErrUserCancel;
      }

   if( destFolder->getFolder()->getIsVolume() == volInternal )
      copyToInternal = true;
   
   // if we're copying an external file
   if( file->getFile()->eAttr )
      {
      if( copyToInternal )
         err = copyItemVFSToInternal( file, destFolder );
      else
         err = copyItemVFSToVFS( file, destFolder );
      }

   // we're copying an internal file
   else
      {
      if( !copyToInternal )
         err = copyItemInternalToVFS( file, destFolder );
      else
         {
         showMessage( "You cann't copy/move a file from the internal card to the internal card." );
         return exgErrUserCancel;
         }
      }

   return err;
   }


static Boolean SECT8 handleCancelButton( EventPtr event )
   {
   FrmGotoForm( TreeViewForm );
   FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
   return true;
   }
   

static void SECT8 copyMoveFile()
   {
   Err      err = errNone;
   //Boolean  handled = false;
   ItemSet  *itemSet = 0;

   deferSleep = true;

   // figure out where we're copying the file to
   itemSet = ctree->getSelectedItems();
   Item *destItem = itemSet->getItem( 0 );
   delete itemSet;

   itemSet = tree->getSelectedItems();

   for( int i=0; i<itemSet->size(); i++ )
      {
      Item *srcItem = itemSet->getItem( i );
      err = copyItem( srcItem, destItem );

      // the vfsVolumeFull error is already told to the user, so we don't
      //  tell them again here. also, the exgErrUserCancel is returned when
      //  the user cancels an overwrite and hence the original should not
      //  be deleted.
      if( err != errNone && err != vfsErrVolumeFull && err != exgErrUserCancel )
         {
         Char msg[256];
         StrPrintF( msg, "There was an error during copy, stopping.", err );
         showMessage( msg );
         break;
         }

      if( err == errNone && chooserOp == moveFile && err != exgErrUserCancel )
         {
         srcItem->remove( false, err );
         if( err != errNone )
            showMessage( "Unable to delete original file, during move.", srcItem->getName());
         }
         
      if( err == vfsErrVolumeFull )
         break;
      }

   deferSleep = false;
   
   delete itemSet;
   }

   
// we're creating a new folder inside the selected folder
static void SECT8 folderCreate()
   {
   Err      err = errNone;
   //Boolean  handled = false;
   ItemSet  *itemSet = 0;
   FieldPtr fld = GetObjectPtr<FieldType>( ChooserNameField );

   // figure out where we're copying the file to
   itemSet = ctree->getSelectedItems();
   Item *destItem = itemSet->getItem( 0 );
   delete itemSet;

   // get the full path and folder name of the folder we want to create
   Char *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( destItem->getFolder()->path ) + StrLen( FldGetTextPtr( fld )) + 1 );
   StrPrintF( fullFilenameAndPath, "%s%s", destItem->getFolder()->path, FldGetTextPtr( fld ) );

   // create the new folder
   err = VFSDirCreate( destItem->getVolumeNum(), fullFilenameAndPath );
   checkError( err, "createFolder() a", fullFilenameAndPath );
   
   err = MemPtrFree( fullFilenameAndPath );
   checkError( err, "createFolder() b", 0 );
   }
   

static Boolean SECT8 handleOKButton( EventPtr event )
   {
   if( chooserOp == createFolder )
      {
      FieldPtr fld = GetObjectPtr<FieldType>( ChooserNameField );

      // first check: make sure a destination is chosen
      if( !SelectionChecker::check( selectionOne, selectionExternal, selectionFolders, ctree, true, selectionVolOK ))
         return true;
      
      // second check: make sure they enter a folder name
      if( !FldGetTextPtr( fld ))
         {
         showMessage( "Folder name must not be empty" );
         return true;
         }
         
      // third check: make sure the new name is clean
      if( checkForInvalidVFSCharacters( FldGetTextPtr( fld ), RenameFileAlert, RenameFileCancel ))
         return true;
         
      folderCreate();
      }

   if( chooserOp == copyFile || chooserOp == moveFile )
      {
      // make sure the user has actually chosen something
      if( !SelectionChecker::check( selectionOne, selectionIntExt, selectionFolders, ctree, true, selectionVolOK ))
         return true;

      copyMoveFile();
      }

   FrmGotoForm( TreeViewForm );	
   FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );

   return true;
   }


static Boolean SECT8 handleSelectEvent( EventPtr event )
   {
   Boolean handled = false;
   
   switch( event->data.ctlSelect.controlID )
      {
      case ChooserOKButton:
         handled = handleOKButton( event );
         break;

      case ChooserCancelButton:
         handled = handleCancelButton( event );
         break;
      }
   
   return handled;
   }


static Boolean handleFormUpdate( EventPtr event )
   {
   Boolean handled = false;

   ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( ChooserScrollBar );
	TablePtr table = GetObjectPtr<TableType>( ChooserTable );
   
   switch( event->data.frmUpdate.updateCode )
      {
      case frmRedrawTreeUpdateCode:
         if( ctree ) ctree->updateTable( table, scrollBar, true );
         handled = true;
         break;
      case frmReloadTreeUpdateCode:
         if( ctree ) ctree->reloadTree( table, scrollBar );
         handled = true;
         break;
      case frmFilterTreeUpdateCode:
         if( ctree )
            {
            ctree->filterTree();
            ctree->updateTable( table, scrollBar, true );
            }
         handled = true;
         break;
      default:
         if( ctree ) ctree->redrawTree( table );
      }
      
   return handled;
   }


Boolean ChooserHandleEvent( EventPtr event )
   {
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

   if( ctree )
      {
      ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( ChooserScrollBar );
      TablePtr table = GetObjectPtr<TableType>( ChooserTable );
      handled = ctree->handleEvent( event, table, scrollBar );

      if( handled )
         return handled;
      }

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleFormOpen( event );
			break;

      case frmCloseEvent:
         handled = handleFormClose( event );
         break;

		case frmUpdateEvent:
         handled = handleFormUpdate( event );
			break;

		case ctlSelectEvent:
         handled = handleSelectEvent( event );
         break;

    default:
         break;
		}
	
	return handled;
   }
