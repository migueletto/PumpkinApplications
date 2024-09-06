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
 * An object to hold info about all the cards on a device.
 *
 * Created on 8/30/02 by Tom Bulatewicz
 */

#include <PalmOS.h>
#include <VFSMgr.h>

#include "Stuph.h"            // for error checking
#include "CardSet.hpp"


CardSet::CardSet()
   {
	cards = 0;
	count = 0;
	currentCard = 0;
	menu = 0;
   }


/**
 * Determines the amount of total/free memory of an internal card.
 */
void CardSet::getInternalMemory( UInt16 cardNo, UInt32 *freeMemory, UInt32 *totalMemory )
	{
	#define		memoryBlockSize		(1024L)
	UInt32		heapFree;
	UInt32		max;
	Int16       i;
	UInt16		heapID;
	UInt32		free=0, total=0;//, dynamic=0;

	// iterate through the RAM heaps on a card (excludes ROM)
	for( i=0; i< MemNumRAMHeaps( cardNo ); i++ )
		{
		heapID = MemHeapID( cardNo, i );					// obtain the ID of the heap
			
		// if the heap is dynamic, increment the dynamic memory total
		if( MemHeapDynamic( heapID ))
			{
			//dynamic += MemHeapSize( heapID );
			}
		else											// the heap is nondynamic
			{
			// calculate the total memory and free memory of the heap
			total += MemHeapSize( heapID );
			MemHeapFreeBytes( heapID, &heapFree, &max );
			free += heapFree;
			}
		}
	
	*freeMemory = free;
	*totalMemory = total;
	}


void Card::init( Char *nameP, UInt8 kindP, UInt16 volumeNumP, UInt32 freeP, UInt32 totalP )
   {
   name = (Char*)MemPtrNew( StrLen( nameP ) + 1 );
   StrCopy( name, nameP );
   kind = kindP;
   volumeNum = volumeNumP;
   freeMemory = freeP;
   totalMemory = totalP;
   }
   
   
void Card::free()
   {
   Err err = errNone;
   if( name )
      {
      err = MemPtrFree( name );
      checkError( err, "Card::~Card()", 0 );
      }
   name = 0;
   }


void CardSet::addCard( Char *nameP, UInt8 kindP, UInt16 volumeNumP, UInt32 freeP, UInt32 totalP )
   {
   Err         err = errNone;
   MemHandle   cardsH = 0;

   if( count == 0 )
      {
      cardsH = MemHandleNew( sizeof( Card ));
      checkMemPtr( cardsH, "CardSet::addCard()" );
      }
   else
      {
      cardsH = MemPtrRecoverHandle( cards );
      checkMemPtr( cardsH, "CardSet::addCard()" );
      err = MemHandleUnlock( cardsH );
      checkError( err, "CardSet::addCard()", 0 );
      err = MemHandleResize( cardsH, sizeof( Card ) * (count + 1) );
      checkError( err, "CardSet::addCard()", 0 );
      }

   cards = (Card*)MemHandleLock( cardsH );
   checkMemPtr( cardsH, "CardSet::addCard()" );

   cards[count].init( nameP, kindP, volumeNumP, freeP, totalP );
   count++;
   }
   

void CardSet::addVFSCards()
	{
	UInt16	volRefNum;
	UInt32	volIterator = vfsIteratorStart;
	Char     label[256];
	Err		err;	
	UInt32	expMgrVersion;

	// if the library exists, then we _might_ have one or _more_ volumes mounted
	
	err = FtrGet( sysFileCExpansionMgr, expFtrIDVersion, &expMgrVersion );
	if( err ) return;
	
	while( volIterator != vfsIteratorStop )
		{
		err = VFSVolumeEnumerate( &volRefNum, &volIterator );
		if( err == errNone )
			{
			err = VFSVolumeGetLabel( volRefNum, label, 256 );
			if( !StrLen( label ) )
				StrCopy( label, "External Card" );

         UInt32 free, total, used;

         err = VFSVolumeSize( volRefNum, &used, &total );
         checkError( err, "addVFSCards()", 0 );
         free = total - used;
	
         addCard( label, volExternal, volRefNum, free, total );
			}
		}
	}


/**
 * This allocates an array of strings that can be used by a list control.
 */
void CardSet::makeMenu()
	{
   menu = (Char**)MemPtrNew( sizeof( Char* ) * count );
   
	for( int i=0; i<count; i++ )
		{
		menu[i] = (Char*)MemPtrNew( StrLen( cards[i].name ) + 1 );
		StrCopy( menu[i], cards[i].name );
		}
	}


UInt8 CardSet::getCurrentKind()
   {
   return cards[currentCard].kind;
   }


UInt16 CardSet::getCurrentCardVolumeNum()
   {
   return cards[currentCard].volumeNum;
   }


Char* CardSet::getName( UInt16 index )
   {
   return cards[index].name;
   }


UInt16 CardSet::getVolumeNum( UInt16 index )
   {
   return cards[index].volumeNum;
   }


UInt8 CardSet::getKind( UInt16 index )
   {
   return cards[index].kind;
   }


UInt8 CardSet::getCurrentCardIndex()
   {
   return currentCard;
   }


char** CardSet::getMenu()
	{
	return menu;
	}
	
   
void CardSet::free()
   {
   Err err = errNone;
   
   for( int i=0; i<count; i++ )
      cards[i].free();

   if( menu )
      {
      for( int i=0; i<count; i++ )
         {
         err = MemPtrFree( menu[i] );
         checkError( err, "CardSet::free() a", 0 );
         }
      
      err = MemPtrFree( menu );
      checkError( err, "CardSet::free() b", 0 );
      menu = 0;
      }

   if( cards )
      {
      err = MemPtrFree( cards );
      checkError( err, "CardSet::free() c", 0 );
      }
   }
   

// XXX tem 1 memory leak de 32 bytes
void CardSet::init()
   {
   UInt32   freeMem, totalMem;
   Char     cardName[32];

   free();

   cards = 0;
   count = 0;
   menu = 0;

	// add all the internal cards
   for( int i=0; i<MemNumCards(); i++ )
      {
      getInternalMemory( i, &freeMem, &totalMem );
      if( i > 0 )
         StrPrintF( cardName, "Internal %d", i );
      else
         StrPrintF( cardName, "Internal" );

      addCard( cardName, volInternal, i, freeMem, totalMem );
      }
   
	// add all the external cards	
	addVFSCards();
		
	makeMenu();
	
   if( currentCard >= count )
      currentCard = 0;
   
	setCurrentCard( currentCard );				// always start with the internal card
   }


void CardSet::setCurrentCard( UInt16 card )
   {
   currentCard = card;
   }


UInt16 CardSet::size()
   {
   return count;
   }


Card* CardSet::getCard( UInt16 i )
   {
   return &cards[i];
   }


Char* CardSet::getCurrentName()
   {
   return cards[currentCard].name;
   }


void CardSet::getCurrentFreeMemory( UInt32 *free, UInt32 *total )
   {
   *free = cards[currentCard].freeMemory;
   *total = cards[currentCard].totalMemory;
   }


void CardSet::getCurrentFreeMemoryForAll( UInt32 *free, UInt32 *total )
   {
   *free = *total = 0;
   for( UInt8 i=0; i<count; i++ )
      {
      *free += cards[i].freeMemory;
      *total += cards[i].totalMemory;
      }
   }


// this sums memory on all external cards, including any virtual cards on the
//  newer devices.
void CardSet::getExternalMemory( UInt32 *free, UInt32 *total )
   {
   *free = *total = 0;
   for( UInt8 i=0; i<count; i++ )
      {
      if( cards[i].kind == volInternal ) continue;
      *free += cards[i].freeMemory;
      *total += cards[i].totalMemory;
      }
   }
