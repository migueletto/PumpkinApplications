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

#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include <VFSMgr.h>
#include "Stuph.h"

void ItemFile::init()
   {
   size = 0;
   iAttr = 0;
   eAttr = 0;
   parent = 0;
   name = 0;
   }


void ItemFile::setFileName( Char *newName )
   {
   Err err = 0;
   
   if( name )
      {
      err = MemPtrFree( name );
      checkError( err, "Item::setName()", newName );
      name = 0;
      }
      
   name = (Char*)MemPtrNew( StrLen( newName ) + 1 );
   StrCopy( name, newName );
   }


void ItemFile::free()
   {
   Err err = 0;
   
   if( name == 0 )
      showMessage( "Item name not allocated in ItemFile::free()" );
   else
      {
      err = MemPtrFree( name );
      checkError( err, "ItemFile::free() a", 0 );
      name = 0;
      }

   if( iAttr )
      {
      err = MemPtrFree( iAttr );
      checkError( err, "ItemFile::free() b", 0 );
      iAttr = 0;
      }
   
   if( eAttr )
      {
      err = MemPtrFree( eAttr );
      checkError( err, "ItemFile::free() c", 0 );
      eAttr = 0;
      }
   }
   

void ItemFile::updateFile( Char *oldName )
   {
   Err   err;

   if( DEBUG ) return;

   if( !iAttr && !eAttr )
      {
      showMessage( "File has no attributes in ItemFile::updateFile(): %s", oldName );
      return;
      }
   
   /*
    * if it's a VFS file, then we just save the VFS file properties
    */
   if( eAttr )
      {
      FileRef  fileRef = 0;

      Char     *fullFilenameAndPath = (Char*)MemPtrNew( StrLen( parent->path ) + StrLen( oldName ) + 2 );
      StrPrintF( fullFilenameAndPath, "%s%s", parent->path, oldName );

      err = VFSFileOpen( volumeNum, fullFilenameAndPath, vfsModeReadWrite, &fileRef );
      checkError( err, "updateFile() a", fullFilenameAndPath );
      if( err == errNone )
         {
         err = VFSFileSetAttributes( fileRef, eAttr->attr );
         checkError( err, "updateFile() b", fullFilenameAndPath );

         err = VFSFileSetDate( fileRef, vfsFileDateCreated, eAttr->created );
         if( err != expErrUnsupportedOperation )
            checkError( err, "updateFile() c", fullFilenameAndPath );
         err = VFSFileSetDate( fileRef, vfsFileDateModified, eAttr->modified );
         if( err != expErrUnsupportedOperation )
            checkError( err, "updateFile() d", fullFilenameAndPath );
         err = VFSFileSetDate( fileRef, vfsFileDateAccessed, eAttr->accessed );
         if( err != expErrUnsupportedOperation )
            checkError( err, "updateFile() e", fullFilenameAndPath );

         err = VFSFileClose( fileRef );
         checkError( err, "updateFile() f", fullFilenameAndPath );
         }

      if( StrCompare( oldName, name ) != 0 )
         {
         err = VFSFileRename( volumeNum, fullFilenameAndPath, name );
         checkError( err, "updateFile() g", fullFilenameAndPath );
         }
      }
   else
      {
      if( StrCompare( oldName, name ) != 0 )
         {
         err = DmSetDatabaseInfo( volumeNum, iAttr->id, name, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
         checkError( err, "updateFile() h", oldName );
         }
      
      err = DmSetDatabaseInfo( volumeNum, iAttr->id, 0, &iAttr->attr, &iAttr->version, &iAttr->created, &iAttr->modified, &iAttr->backedUp, 0, 0, 0, &iAttr->type, &iAttr->creator );
      checkError( err, "updateFile() i", oldName );
      }
   }


void ItemFile::setVolumeNum( UInt32 v )
   {
   volumeNum = v;
   }
   
   
UInt32 ItemFile::getVolumeNum()
   {
   return volumeNum;
   }


void ItemFile::setSize( UInt32  s )
   {
   size = s;
   }
   

UInt32 ItemFile::getSize()
   {
   return size;
   }
