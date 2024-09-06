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

#include "ItemFolder.hpp"
#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemSet.hpp"
#include "BusyIndicator.hpp"
#include <VFSMgr.h>
#include <MemGlue.h>

#include "Stuph.h"            // for the error checking utils
#include "UI.h"
#include "Main.h"             // for access to "prefs"

#include "FilterForm.h"
#include "CardSet.hpp"
#include "TreeView.hpp"

void ItemFolder::init()
   {
   path = 0;
   items = 0;
   itemCount = 0;
   volumeNum = 0;
   expanded = 0;
   name = 0;
   isVolume = 0;
   }
      

void ItemFolder::setFolderName( Char *newName )
   {
   Err err = 0;
   
   if( name )
      {
      err = MemPtrFree( name );
      checkError( err, "ItemFolder::setFolderName()", newName );
      name = 0;
      }
      
   name = (Char*)MemPtrNew( StrLen( newName ) + 1 );
   StrCopy( name, newName );
   }


// folders are always sorted alphabetically. if we're doing a reverse
//  filename sort, then order them in reverse, otherwise, order them
//  in regular order.
Int16 FolderCompare( Item *x, Item *y )
   {
   if( !x->getFolder() || !y->getFolder())
      showMessage( "No folder in FolderCompare()" );
   
   if( x->getFolder()->getIsVolume() || y->getFolder()->getIsVolume())
      return 0;
   
	if( prefs.list.sortOrder == sortNameZA )
      return StrCaselessCompare( y->getName(), x->getName() );
   else
      return StrCaselessCompare( x->getName(), y->getName() );
   }


// when comparing by name, we always sort from A to Z unless we're
//  specifically sorting them from Z to A.
Int16 CompareName( Char *x, Char *y )
   {
   if( prefs.list.sortOrder == sortNameZA )
      return StrCaselessCompare( y, x );
   else
      return StrCaselessCompare( x, y );
   }
   

Int16 CompareSize( Item *x, Item *y )
   {
   // if they're the same size, then sort by name
   if( x->getFile()->size == y->getFile()->size )
      return CompareName( x->getName(), y->getName());
   
   Int16 value;
   
   if( x->getFile()->size < y->getFile()->size )
      value = 1;
   else
      value = -1;

   if( prefs.list.sortOrder == sortOtherAZ )
      return value;
   else
      return -1 * value;
   }
   

Int16 CompareIAttr( InternalAttr *x, InternalAttr *y )
   {
	Char	xs[5], ys[5];

   if( !x || !y )
      showMessage( "No iattr in CompareIAttr()" );

   switch( prefs.list.column )
      {
      case colCreator:
         IntToStr( x->creator, xs );
         IntToStr( y->creator, ys );
         return StrCaselessCompare( xs, ys );
         break;
					
      case colType:
         IntToStr( x->type, xs );
         IntToStr( y->type, ys );
         return StrCaselessCompare( xs, ys );
         break;
					
      case colRec:
         if( x->recCount == y->recCount )
            return 0;
         
         if( x->recCount < y->recCount )
            return 1;
         else
            return -1;					
         break;
					
      case colAttr:
         attrToString( xs, x->attr );
         attrToString( ys, y->attr );
         return StrCaselessCompare( xs, ys );
         break;

      case colCreate:
         if( x->created == y->created )
            return 0;
         
         if( x->created < y->created )
            return 1;
         else
            return -1;					
         break;

      case colMod:
         if( x->modified == y->modified )
            return 0;
         
         if( x->modified < y->modified )
            return 1;
         else
            return -1;					
         break;

      case colBackup:
         if( x->backedUp == y->backedUp )
            return 0;
         
         if( x->backedUp < y->backedUp )
            return 1;
         else
            return -1;					
         break;
      }
     return 0;
   }


// if we're comparing 2 files, then just compare em.
Int16 FileCompare( Item *x, Item *y )
   {
   if( !x->getFile() || !y->getFile())
      showMessage( "No file in FileCompare()" );

   // if we're sorting by name, then just do the comparison
   if( prefs.list.sortOrder == sortNameAZ || prefs.list.sortOrder == sortNameZA )
      return CompareName( x->getName(), y->getName());

   // if we're sorting by size, then do the comparison
   if( prefs.list.column == colSize )
      return CompareSize( x, y );

   // if we're sorting by iAttr's, then see if both items have them
   if( !x->getFile()->iAttr ) return 1;
   if( !y->getFile()->iAttr ) return -1;
   
	switch( prefs.list.sortOrder )
		{
		case sortOtherAZ:         
			return CompareIAttr( x->getFile()->iAttr, y->getFile()->iAttr );		
		case sortOtherZA:
			return -1 * CompareIAttr( x->getFile()->iAttr, y->getFile()->iAttr );
      }

   return 0;
   }


/**
 * Compares two items.
 *
 * @param r1 item to compare
 * @param r2 item to compare
 * @param other unused
 */
//Int16 ItemCompare( void *r1, void *r2, long other )
Int16 ItemCompare( void *r1, void *r2, Int32 other )
	{
	Item 	*x, *y;
	//Char	xs[5], ys[5];
	
	x = ((Item*)r1);
	y = ((Item*)r2);

   // first of all, we need to see if a volume is being compared. if so,
   //  we just return 0 since they shouldn't move at all.
   if( x->getFolder())
      if( x->getFolder()->getIsVolume())
         return 0;

   if( y->getFolder())
      if( y->getFolder()->getIsVolume())
         return 0;

   if( prefs.list.foldersFirst )
      {
      // this will make sure that folders are always sorted by name
      if( x->getFolder() && y->getFolder()) 
         return FolderCompare( x, y );

      // these two make sure that folders come first
      if( x->getFolder()) return -1;
      if( y->getFolder()) return 1;
      }
   else
      {
      if( x->getFolder() || y->getFolder())
         {
         // if sorting by name, then compare the folder and file/folder by name
         if( prefs.list.sortOrder == sortNameAZ || prefs.list.sortOrder == sortNameZA )
            return CompareName( x->getName(), y->getName());
         else
            {
            if( x->getFolder()) return 1;
            if( y->getFolder()) return -1;
            }
         }
      }

   // at this point, we must be dealing with two files, so sort em
   return FileCompare( x, y );
	}


void ItemFolder::setIsVolume( UInt8 vol )
   {
   isVolume = vol;
   }
   
   
UInt8 ItemFolder::getIsVolume()
   {
   return isVolume;
   }


void ItemFolder::sendSelectedItems()
   {
   for( UInt32 i=0; i<itemCount; i++ )
      items[i].sendSelectedItems();
   }


void ItemFolder::sendToMemo( MemHandle textH, UInt8 depth, BusyIndicator *busyIndicator )
   {
   if( expanded )
      {
      for( UInt32 i=0; i<itemCount; i++ )
         items[i].sendToMemo( textH, depth, busyIndicator );
      }
   }
   

void ItemFolder::setAllAttributes( UInt16 attr, Boolean set )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      items[i].setAllAttributes( attr, set );
   }


void ItemFolder::sortItems()
   {
	if( itemCount > 1 )
		SysQSort( (void*)items, itemCount, sizeof(Item), ItemCompare, 0);

   // restore the parent pointer, and make sure any nested folders are sorted
   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( items[i].getFile() )
         items[i].getFile()->parent = this;
      
      if( items[i].getFolder() )
         items[i].getFolder()->sortItems();
      }
   }


void ItemFolder::setAllSelected( Boolean on )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      items[i].setAllSelected( on );
   }


void ItemFolder::free()
   {
   Err err = 0;

   if( name == 0 )
      showMessage( "Name not allocated in ItemFolder::free()." );
   else
      {
      err = MemPtrFree( name );
      checkError( err, "ItemFolder::free() a", 0 );
      name = 0;
      }
   
   if( path == 0 )
      showMessage( "Path not allocated in ItemFolder::free()." );
   else
      {
      err = MemPtrFree( path );
      checkError( err, "ItemFolder::free() b", 0 );
      path = 0;
      }
   
   freeItems();   
   }

   
void ItemFolder::freeItems()
   {
   if( itemCount )
      {
      for( UInt32 i=0; i<itemCount; i++ )
         items[i].free();
      
      MemPtrFree( items );
      
      items = 0;
      itemCount = 0;
      }
   }

   
void ItemFolder::getSelectedItems( ItemSet *itemSet )
   {
   getSelectedItemsHelper( itemSet );
   }


void ItemFolder::getSelectedItemsHelper( ItemSet *itemSet )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( items[i].getSelected())
         itemSet->addItem( &items[i] );
         
      if( items[i].getFolder() != 0 )
         {
         if( items[i].getFolder()->expanded )
            items[i].getFolder()->getSelectedItemsHelper( itemSet );
         }
      }
   }

   
Char* ItemFolder::getPath()
   {
   return path;
   }

   
void ItemFolder::rememberExpandedItems( ItemSet *expandedItems )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( items[i].getFolder() != 0 )
         {
         if( items[i].getFolder()->expanded )
            {
            Item *eItem = (Item*)MemPtrNew( sizeof( Item ));
     
            eItem->init();

            eItem->setFolder((ItemFolder*)MemPtrNew( sizeof( ItemFolder )));
            eItem->getFolder()->init();
            eItem->getFolder()->setPath( items[i].getFolder()->path );
            eItem->getFolder()->setVolumeNum( items[i].getFolder()->getVolumeNum());

            eItem->setName( items[i].getName() );

            expandedItems->addItem( eItem );
            items[i].getFolder()->rememberExpandedItems( expandedItems );
            }
         }
      }
   }

   
UInt32 ItemFolder::getItemCount()
   {
   UInt32 count=0;
   return getItemCountHelper( count );
   }


UInt32 ItemFolder::getItemCountHelper( UInt32 count )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( !items[i].getFiltered())
         count++;
      }

   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( items[i].getFolder() == 0 && items[i].getFile() == 0 )
         showMessage( "Bad file in ItemFolder::getItemCountHelper(): %s", items[i].getName() );

      if( items[i].getFolder() != 0 )
         {
         if( items[i].getFolder()->expanded && !items[i].getFiltered())
            count = items[i].getFolder()->getItemCountHelper( count );
         }
      }

   return count;
   }


Int32 ItemFolder::findItemWithLetter( Char ch )
   {
   Int32 count=0;
   return findItemWithLetterHelper( ch, count );
   }


// this methods jumps to the first file that has a filename starting
//  with the given letter, except if the list is sorted by creator or
//  type, in which case we jump to the creator/type that has the first
//  letter.
Int32 ItemFolder::findItemWithLetterHelper( Char ch, Int32 count )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      {
      // we want to check to see what we're sorted by. if we're sorted by name,
      //  then check the first letter of the file name, but if we're sorted
      //  by type or creator, then check those letters.

      // start by getting the first letter of the file name
      Char *name = items[i].getName();
      Char fc = name[0];

      // now check to see if we should forget the file name and get the
      //  first letter of the creator or type.
      if( prefs.list.sortOrder == sortOtherAZ || prefs.list.sortOrder == sortOtherZA )
         {
         if( items[i].getFile() )
            {
            if( items[i].getFile()->iAttr )
               {
               if( prefs.list.column == colCreator )
                  {
                  Char creator[5];
                  IntToStr( items[i].getFile()->iAttr->creator, creator );
                  fc = creator[0];
                  }
               if( prefs.list.column == colType )
                  {
                  Char type[5];
                  IntToStr( items[i].getFile()->iAttr->type,type );
                  fc = type[0];
                  }
               }
            }
         }

		if( (UInt8)fc >= 'a' && (UInt8)fc <= 'z')
         fc -= ('a' - 'A');                           // conver to uppercase

//		if( fc == ch )				// find a match
		if( fc == ch && !items[i].getFolder())				// find a match
         return count;
      
      count++;

      if( items[i].getFolder() )
         {
         if( items[i].getFolder()->expanded )
            {
            Int32 num = items[i].getFolder()->findItemWithLetterHelper( ch, count );
            if( num >= 0 )
               return num;
            }
         }
      }

   return -1;     // indicate that it was not found
   }


Item* ItemFolder::getItem( UInt32 target )
   {
   UInt32 count=0;
   Item *item = getItemHelper( count, target );
   if( item == 0 )
      {
      Char str[128];
      StrPrintF( str, "Item not found in ItemFolder::getItem: %ld", target );
      showMessage( str );
      }
      
   return item;
   }


Item* ItemFolder::getItemHelper( UInt32 &count, UInt32 target )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      {
      if( items[i].getFiltered())
         continue;
         
      if( target == count )
         return &items[i];
      
      if( items[i].getFolder() == 0 && items[i].getFile() == 0 )
         showMessage( "File has no file or folder in ItemFolder::getItemHelper(): %s", items[i].getName() );

      count++;

      if( items[i].getFolder() )
         {
         if( items[i].getFolder()->expanded )
            {
            Item *item = items[i].getFolder()->getItemHelper( count, target );
            if( item )
               return item;
            }
         }
      }

   return 0;
   }

   
void ItemFolder::readItems( ItemSet *expandedItems, Int8 indent, Boolean chooser, Boolean filterOn, Boolean expandAll )
   {
	FileRef			dirRef=0;
	Err				err;
	UInt32			fileIterator;
	FileInfoType	fileInfo;
	Char           *fileName;
   UInt16         i=0;
   Char           msg[256];
   MemHandle      itemsH = 0;

   BusyIndicator *busyIndicator = new BusyIndicator( defaultY, 5 );

   if( itemCount != 0 )
      {
      StrPrintF( msg, "Error: itemCount is not zero (%ld) in ItemFolder::readItems()", itemCount );
      showMessage( msg );
      itemCount = 0;
      }
   
	err = VFSFileOpen( volumeNum, path, vfsModeRead, &dirRef );
   checkError( err, "ItemFolder::readItems() a", path );

	fileName = (Char*)MemPtrNew( 256 );
   checkMemPtr( fileName, "ItemFolder::readItems() b" );
	
	fileInfo.nameP = fileName;
   fileInfo.nameBufLen = 256;

   itemCount = 0;
	fileIterator = expIteratorStart;
	while( fileIterator != expIteratorStop )
		{
      busyIndicator->update();

		err = VFSDirEntryEnumerate( dirRef, &fileIterator, &fileInfo );
      
      // now, what the API reference for VFSDirEntryEnumerate() doesn't say
      //  is that if this function encounters a problem, there is no way
      //  for it to skip the bad entry, so it just keeps returning an error
      //  as it continually tries to enumerate the bad entry. VFS is
      //  designed to be a layer on top of various other file systems, and
      //  so cards not formatted by Palm OS often see errors. i don't know
      //  any way to recover from one of these entry enumeration errors, so
      //  the only thing to do is quit enumerating. if you view the details
      //  of a folder, the details screen will now tell you if there were any
      //  problems enumerating the files since i don't want to pop up a message
      //  every time the user expands the card in the file list.

      if( err != errNone )
         break;
      
		if( !(fileInfo.attributes & vfsFileAttrDirectory) && chooser )
         continue;

      // ok, so here we check the file name for illegal characters, : and /
      //  those characters will cause every file operation to fail, so we'll
      //  not even show them.
		if( hasInvalidVFSCharacters( fileName )) continue;

      // adjust the allocated memory to accomodate this next file
      if( itemCount == 0 )
         {
         itemsH = MemHandleNew( sizeof( Item ));
         checkMemPtr( itemsH, "ItemFolder::readItems() c" );
         }
      else
         {
         UInt32 newSize = (itemCount + 1) * sizeof( Item );
         Err err = MemHandleResize( itemsH, newSize );
         checkError( err, "ItemFolder::readItems() e", 0 );
         }

      items = (Item*)MemHandleLock( itemsH );
      checkMemPtr( items, "ItemFolder::readItems() f" );
   
      items[i].init();

		if( fileInfo.attributes & vfsFileAttrDirectory )
         {
         items[i].setFolder((ItemFolder*)MemPtrNew( sizeof( ItemFolder )));
         checkMemPtr( items[i].getFolder(), "ItemFolder::readItems() g" );

         items[i].getFolder()->init();
         items[i].getFolder()->setPath( path, fileName );
         items[i].setName( fileName );
         items[i].getFolder()->setVolumeNum( volumeNum );
         items[i].setIndent( indent + 1 );
         
         if( expandAll )
            items[i].getFolder()->expanded = true;
         else
            {
            if( expandedItems )
               {
               if( expandedItems->isExpanded( items[i].getFolder()->path, items[i].getName(), items[i].getFolder()->getVolumeNum()))
                  items[i].getFolder()->expanded = true;
               }
            }
            
         if( items[i].getFolder()->expanded )
            items[i].getFolder()->readItems( expandedItems, indent + 1, chooser, filterOn, expandAll );
         }
      else
         {
         items[i].setFile((ItemFile*)MemPtrNew( sizeof( ItemFile )));
         checkMemPtr( items[i].getFile(), "ItemFolder::readItems() h" );

         items[i].getFile()->init();
         items[i].setIndent( indent + 1 );
         items[i].setVolumeNum( volumeNum );
         items[i].setName( fileName );

         FileRef	fileRefP;
         Char     *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( path ) + StrLen( fileName ) + 2 );
         checkMemPtr( fullFilenameAndPath, "ItemFolder::readItems() i" );
         StrPrintF( fullFilenameAndPath, "%s%s", path, fileName );

			err = VFSFileOpen( volumeNum, fullFilenameAndPath, vfsModeRead, &fileRefP );
         checkError( err, "ItemFolder::readItems() j", fullFilenameAndPath );

         if( err == errNone && fileRefP )
				{
				err = VFSFileSize( fileRefP, &items[i].getFile()->size );
            checkError( err, "ItemFolder::readItems() k", fullFilenameAndPath );

            items[i].getFile()->eAttr = (ExternalAttr*)MemPtrNew( sizeof( ExternalAttr ));
            checkMemPtr( items[i].getFile()->eAttr, "ItemFolder::readItems() l" );
            
            err = VFSFileGetAttributes( fileRefP, &items[i].getFile()->eAttr->attr );
            checkError( err, "ItemFolder::readItems() m", fullFilenameAndPath );

            err = VFSFileGetDate( fileRefP,  vfsFileDateCreated , &items[i].getFile()->eAttr->created );
            if( err != expErrUnsupportedOperation )
               checkError( err, "ItemFolder::readItems() n", fullFilenameAndPath );
            err = VFSFileGetDate( fileRefP,  vfsFileDateModified , &items[i].getFile()->eAttr->modified );
            if( err != expErrUnsupportedOperation )
               checkError( err, "ItemFolder::readItems() o", fullFilenameAndPath );
            err = VFSFileGetDate( fileRefP,  vfsFileDateAccessed , &items[i].getFile()->eAttr->accessed );
            if( err != expErrUnsupportedOperation )
               checkError( err, "ItemFolder::readItems() p", fullFilenameAndPath );

            Char     dbName[dmDBNameLength];
            UInt16   attributes;
            UInt16   version;
            UInt32   crDate;
            UInt32   modDate;
            UInt32   bckUpDate;
            //UInt32   modNum;
            UInt32   type;
            UInt32   creator;
               
            err = MyVFSFileDBInfo( fileName, fileRefP, dbName, &attributes, &version, &crDate, &modDate, &bckUpDate, &type, &creator );
            if( err == errNone )
               {
               items[i].getFile()->iAttr = (InternalAttr*)MemPtrNew( sizeof( InternalAttr ));
               checkMemPtr( items[i].getFile(), "ItemFolder::readItems() q" );
               
               items[i].getFile()->iAttr->id = 0;
               items[i].getFile()->iAttr->attr = attributes;
               items[i].getFile()->iAttr->version = version;
               items[i].getFile()->iAttr->type = type;
               items[i].getFile()->iAttr->creator = creator;
               items[i].getFile()->iAttr->created = crDate;
               items[i].getFile()->iAttr->modified = modDate;
               items[i].getFile()->iAttr->backedUp = bckUpDate;
               items[i].getFile()->iAttr->recCount = 0;
               }

            err = VFSFileClose( fileRefP );
            checkError( err, "ItemFolder::readItems() r", 0 );
            }

         err = MemPtrFree( fullFilenameAndPath );
			checkError( err, "ItemFolder::readItems() s", 0 );
         fullFilenameAndPath = 0;
         }

      if( filterOn && !chooser )
         items[i].filterCheck();

      err = MemHandleUnlock( itemsH );
      checkError( err, "ItemFolder::readItems() t", 0 );
      
      i++;
      itemCount++;
		}

   delete busyIndicator;
   
   if( fileName )
      {
      err = MemPtrFree( fileName );
      checkError( err, "ItemFolder::readItems() u", 0 );
      }

   err = VFSFileClose( dirRef );
   checkError( err, "ItemFolder::readItems() v", 0 );

   if( itemCount == 0 )
      return;

   Item *itemsHandle = (Item*)MemHandleLock( itemsH );
   checkMemPtr( itemsHandle, "readItems() v.1" );
   
   items = (Item*)MemGluePtrNew( sizeof( Item ) * itemCount );
   checkMemPtr( items, "readItems() v.2" );
   MemMove( items, itemsHandle, sizeof( Item ) * itemCount );

   err = MemHandleUnlock( itemsH );
   checkError( err, "ItemFolder::readItems() w", 0 );
   
   err = MemHandleFree( itemsH );
   checkError( err, "ItemFolder::readItems() x", 0 );

   sortItems();
   }


/**
 * So I'm trying something new here, and rather than try to figure out if a
 * file is a pdb/prc myself (which technically is impossible), I'm just
 * assuming that the file extension can be trusted. - 3/15/05
 */
Err ItemFolder::MyVFSFileDBInfo( Char *fileName, FileRef ref, Char *name, UInt16 *attributes, UInt16 *version, UInt32 *crDate, UInt32 *modDate, UInt32 *bckUpDate, UInt32 *type, UInt32 *creator )
   {
   Err err;
   UInt32   len = sizeof( DatabaseHdrType );
   UInt32   read=0;
	UInt16 i, j;

	UInt16 nameLength = StrLen( fileName );
	Char   extension[5];
	
	// if the length of the name is less than 5, then it can't have a .pdb
	//  or .prc extension, so exit.
	if( nameLength < 5 )
		return vfsErrBadData;
	
	MemSet( extension, 5, 0 );
	
	// figure out what the extension is
	for( i=nameLength-4, j=0; j<5; i++, j++ )
		extension[j] = fileName[i];

	// see if it's a prc or pdb file and exit if not
	if( StrCaselessCompare( extension, ".prc" ) != 0 &&
		StrCaselessCompare( extension, ".pdb" ) != 0 )
		return vfsErrBadData;
	
	// so now we're assuming it's a prc/pdb, so start parsing the header
   DatabaseHdrType *hdr = (DatabaseHdrType*)MemPtrNew( len );
   if( !hdr )
      return memErrNotEnoughSpace;

   /* open the file so that we can read the db header */
   err = VFSFileRead( ref, len, hdr, &read );
   if( err != errNone )
      return err;
   
   /* if the file is too short to contain a database header, then it's
       not a pdb or prc */
   if( read < len )
      {
      MemPtrFree( hdr );
      return vfsErrBadData;
      }

   /* since there is absolutely no possbible way to determine if a file is
      a pdb/prc, all we can do is check the type field and see if its type
      is registered in SystemResources.h */
/*
   switch( hdr->type )
      {
      case sysFileTLibrary:
      case sysFileTLibraryExtension:
      case sysFileTTelTaskSerial:
      case sysFileTBaseATDriver:
      case sysFileTUartPlugIn:
      case sysFileTVirtPlugin:
      case sysFileTPhoneDriver:
      case sysFileTSystem:
      case sysFileTSystemPatch:
      case sysFileTProductUpdate:
      case sysFileTKernel:
      case sysFileTBoot:
      case sysFileTSmallHal:
      case sysFileTBigHal:
      case sysFileTSplash:
      case sysFileTUIAppShell:
      case sysFileTOverlay:
      case sysFileTExtension:
      case sysFileTApplication:
      case sysFileTPanel:
      case sysFileTSavedPreferences:
      case sysFileTPreferences:
      case sysFileTMidi:
      case sysFileTpqa:
      case sysFileTLocaleModule:
      case sysFileTActivationPlugin:
      case sysFileTUserDictionary:
      case sysFileTLearningData:
      case sysFileTGraffitiMacros:
      case sysFileTHtalLib:
      case sysFileTExgLib:
      case sysFileTSlotDriver:
      case sysFileTFileSystem:
      case sysFileTFileStream:
      case sysFileTTemp:
      case sysFileTNetworkPanelPlugin:
      case sysFileTScriptPlugin:
      case sysFileTStdIO:
      case 'DATA':
      case 'data':
         break;
      default:
         MemPtrFree( hdr );
         return vfsErrBadData;
      }
*/
   /* read the database name, being very careful not to allow any sort
      of buffer overflows in case the file is not a prc or pdb */
   for( int i=0; i<dmDBNameLength; i++ )
      {
      if( hdr->name[i] == 0 )
         {
         name[i] = 0;
         break;
         }
      else
         name[i] = hdr->name[i];
      }
   name[dmDBNameLength-1] = 0;        // just in case

   *attributes = hdr->attributes;
   *version = hdr->version;
   *crDate = hdr->creationDate;
   *modDate = hdr->modificationDate;
   *bckUpDate = hdr->lastBackupDate;
   *type = hdr->type;
   *creator = hdr->creator;

   MemPtrFree( hdr );
   return errNone;
   }


void ItemFolder::readCards( ItemSet *expandedItems, Int8 indent, Boolean chooser, Boolean filterOn, Boolean expandAll )
   {
   Char buf[32];

   for( int i=0; i<cardSet.size(); i++ )
      {
      items[i].init();

      items[i].setFolder((ItemFolder*)MemPtrNew( sizeof( ItemFolder )));
      checkMemPtr( items[i].getFolder(), "ItemFolder::readCards()" );

      items[i].getFolder()->init();

      items[i].setIndent( indent );
      items[i].getFolder()->setFolderName( cardSet.getName( i ));
      StrCopy(buf, "/");
      items[i].getFolder()->setPath( buf );
      items[i].getFolder()->setVolumeNum( cardSet.getVolumeNum( i ));
      items[i].getFolder()->setIsVolume( cardSet.getKind( i ));
      
      if( expandAll )
         items[i].getFolder()->expanded = true;
      
      if( expandedItems )
         {
         if( expandedItems->isExpanded( items[i].getFolder()->path, items[i].getName(), items[i].getFolder()->getVolumeNum()))
            items[i].getFolder()->expanded = true;
         }
         
      if( items[i].getFolder()->expanded )
         {
         if( items[i].getFolder()->getIsVolume() == volInternal )
            items[i].getFolder()->readPalmCard( indent, chooser, filterOn );
         else
            items[i].getFolder()->readItems( expandedItems, indent, chooser, filterOn, expandAll );
         }

      itemCount++;
      }
   }


void ItemFolder::remove( Err &err )
   {
   if( itemCount == 0 )
      readItems( 0, 0, false, false, false );
   
   for( UInt32 i=0; i<itemCount; i++ )
      items[i].remove( false, err );
   }


void ItemFolder::filter( Boolean turnOn )
   {
   for( UInt32 i=0; i<itemCount; i++ )
      items[i].filter( turnOn );   
   }


void ItemFolder::readPalmCard( Int8 indent, Boolean chooser, Boolean filterOn )
   {
	//FileRef			dirRef=0;
	Err				err;
	//UInt32			fileIterator;
	//FileInfoType	fileInfo;
	Char           *filename;
   UInt32         i=0, a=0;      // "a" is the actual item count, "i" is the total number of possible items
   LocalID        id;
   Char buf[32];

   if( chooser ) return;

   BusyIndicator *busyIndicator = new BusyIndicator( defaultY, 15 );

   if( itemCount != 0 )
      {
      Char message[128];
      StrPrintF( message, "itemCount is not zero (%ld) in ItemFolder::readPalmCard()", itemCount );
      showMessage( message );
      itemCount = 0;
      }

   filename = (Char*)MemPtrNew( dmDBNameLength );
   checkMemPtr( filename, "ItemFolder::readPalmCard() a" );

	UInt32	heapFree, heapMax;
	UInt16	heapID;

	heapID = MemPtrHeapID( filename );

	itemCount = DmNumDatabases( volumeNum );
   items = (Item*)MemGluePtrNew( itemCount * sizeof( Item ));

	for( i=0; i<itemCount; i++ )
		{
      busyIndicator->update();

		// make sure we have enough memory left to read the rest of the files
		err = MemHeapFreeBytes( heapID, &heapFree, &heapMax );
		if( heapMax < 1000 || heapFree < 1000 )
			{
			Char msg[256];
			StrPrintF( msg, "Low memory (free: %ld, max: %ld)", heapFree, heapMax );
			showMessage( msg );
			break;
			}

		id = DmGetDatabase( volumeNum, i );
      if( !id )
         {
         showMessage( "Unable to get database in readPalmCard() b" );
         continue;
         }

		if( !prefs.list.hideROM && MemLocalIDKind( id ) != memIDHandle )
			continue;

      items[a].init();

      items[a].setFile((ItemFile*)MemPtrNew( sizeof( ItemFile )));
      checkMemPtr( items[a].getFile(), "ItemFolder::readPalmCard() c" );
      items[a].getFile()->init();
      items[a].setIndent( indent + 1 );
      items[a].setVolumeNum( volumeNum );

      items[a].getFile()->iAttr = (InternalAttr*)MemPtrNew( sizeof( InternalAttr ));
      checkMemPtr( items[a].getFile()->iAttr, "ItemFolder::readPalmCard() d" );

      UInt16 attributes;
      UInt16 version;
      UInt32 crDate;
      UInt32 modDate;
      UInt32 bckUpDate;
      UInt32 modNum;
      UInt32 type;
      UInt32 creator;

		err = DmDatabaseInfo( volumeNum, id, filename, &attributes, &version, &crDate, &modDate, &bckUpDate, &modNum, 0, 0, &type, &creator );
      checkError( err, "ItemFolder::readPalmCard() e", filename );

      items[a].getFile()->iAttr->id = id;
      items[a].getFile()->iAttr->attr = attributes;
      items[a].getFile()->iAttr->version = version;
      items[a].getFile()->iAttr->type = type;
      items[a].getFile()->iAttr->creator = creator;
      items[a].getFile()->iAttr->backedUp = bckUpDate;
      items[a].getFile()->iAttr->created = crDate;
      items[a].getFile()->iAttr->modified = modDate;
      items[a].getFile()->parent = this;

		if( !filename || !(*filename) ) {					// the file has no name
         StrCopy(buf, "unnamed");
         items[a].setName( buf );
    }

		items[a].setName( filename );

      UInt32 numRecords;
      UInt32 totalBytes;
      UInt32 dataBytes;

      // this call has been known to crash on bad files
      err = DmDatabaseSize( volumeNum, id, &numRecords, &totalBytes, &dataBytes );
      checkError( err, "ItemFolder::readPalmCard() f", 0 );

      if( err == errNone )
         {
         items[a].getFile()->iAttr->recCount = numRecords;
         items[a].getFile()->size = totalBytes;
         }
      else
			items[a].getFile()->iAttr->recCount = items[a].getFile()->size = 0;

      if( filterOn && !chooser )
         items[a].filterCheck();
      
      a++;
		}

   itemCount = a;

   err = MemPtrFree( filename );
   checkError( err, "ItemFolder::readPalmCard() h", 0 );
   filename = 0;
	
   delete busyIndicator;

   sortItems();
   }
   

void ItemFolder::setPath( Char *pathP, Char *folderName )
   {
   path = (Char*)MemPtrNew( StrLen( pathP ) + 1 + StrLen( folderName ) + 1 + 1 );
   StrPrintF( path, "%s%s/", pathP, folderName );
   }

   
void ItemFolder::setPath( Char *pathP )
   {
   path = (Char*)MemPtrNew( StrLen( pathP ) + 2 );
   StrPrintF( path, "%s", pathP );
   if( path[StrLen(path)-1] != '/' )
      StrCat( path, "/" );
   }


void ItemFolder::setVolumeNum( UInt16 v )
   {
   volumeNum = v;
   }
   

UInt16 ItemFolder::getVolumeNum()
   {
   return volumeNum;
   }
