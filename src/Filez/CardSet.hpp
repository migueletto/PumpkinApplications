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
  
#ifndef __cardset_h__
#define __cardset_h__

#include "Sections.hpp"
#include "ItemFolder.hpp"  // for the volume kind defines

class Card
	{
   public:

	void init( Char *nameP, UInt8 kindP, UInt16 volumeNumP, UInt32 free, UInt32 total ) SECT4;
	void free() SECT4;

   //private:

	Char     *name;
	UInt8    kind;
	UInt16	volumeNum;
   UInt32   freeMemory, totalMemory;
	};


class CardSet
	{
   public:

   CardSet() SECT4;
   void addVFSCards() SECT4;
   void setCurrentCard( UInt16 card ) SECT4;
   void addCard( Char *nameP, UInt8 kindP, UInt16 volumeNumP, UInt32 freeP, UInt32 totalP ) SECT4;
   Card* getCard( UInt16 i ) SECT4;
   UInt16 size() SECT4;	
	void init() SECT4;
   UInt16 getCurrentCardVolumeNum() SECT4;
   Card* getCurrentCard() SECT4;
   UInt8 getCurrentCardIndex() SECT4;
   Char* getCurrentName() SECT4;
	void getCurrentFreeMemory( UInt32 *freeMemory, UInt32 *totalMemory ) SECT4;
	void getCurrentFreeMemoryForAll( UInt32 *freeMemory, UInt32 *totalMemory ) SECT4;
   void getInternalMemory( UInt16 cardNo, UInt32 *freeMemory, UInt32 *totalMemory ) SECT4;
   void getExternalMemory( UInt32 *freeMemory, UInt32 *totalMemory ) SECT4;
   UInt8 getCurrentKind() SECT4;
   void free() SECT4;
   Char *getName( UInt16 index ) SECT4;
   UInt16 getVolumeNum( UInt16 index ) SECT4;
   UInt8 getKind( UInt16 index ) SECT4;
   void makeMenu() SECT4;
   char** getMenu() SECT4;

   private:

	Card		*cards;
	UInt8		count;
	UInt8		currentCard;
	Char		**menu;
   };
	
extern CardSet cardSet;
   
#endif
