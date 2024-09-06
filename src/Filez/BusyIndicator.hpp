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

#ifndef __busyindicator_h__
#define __busyindicator_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "SectionsCmn.hpp"

#define defaultY 77

class BusyIndicator
   {
   private:
   
   UInt8    cycle;
   UInt8    dotWidth;
   UInt8    dotSpace;
   UInt8    i;
   UInt8    freq;             // update the indicator every freq update calls
   UInt16   y;                // the y-coordinate placement of the indicator
   UInt16   x;                // the x-coordinate placement of the indicator

   public:
   
   BusyIndicator( UInt16 y, UInt8 freq ) SECT_CMN;
   void draw() SECT_CMN;
   void update() SECT_CMN;
   void erase() SECT_CMN;
   };

#endif