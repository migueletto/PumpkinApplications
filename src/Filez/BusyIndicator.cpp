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

#include "BusyIndicator.hpp"
#include "UI.h"

#include "debug.h"

BusyIndicator::BusyIndicator( UInt16 py, UInt8 pfreq )
   {
   cycle = 0;

   dotWidth = 5;
   dotSpace = 2;
   y = py;
   x = 77;
   freq = pfreq;
   i = freq;

   
   }


void BusyIndicator::draw()
   {
	RectangleType	r;
   
   r.topLeft.x = x;          r.topLeft.y = y;
   r.extent.x = dotWidth;		r.extent.y = dotWidth;

   ColorSet( &black, NULL, NULL, NULL, NULL );
   
   switch( cycle )
      {
      case 0:  // top left
         WinDrawRectangle( &r, 0 );
         break;

      case 1:  // top right
         r.topLeft.x += dotWidth+dotSpace;
         WinDrawRectangle( &r, 0 );
         break;
         
      case 2: // bottom right
         r.topLeft.x += dotWidth+dotSpace;
         r.topLeft.y += dotWidth+dotSpace;
         WinDrawRectangle( &r, 0 );
         break;
         
      case 3: // bottom left
         r.topLeft.y += dotWidth+dotSpace;
         WinDrawRectangle( &r, 0 );	
         break;
      }

   ColorUnset();
   }
   

void BusyIndicator::update()
   {
	RectangleType	r;

   i++;
   if( i < freq )
      return;
   else
      i = 0;

	r.topLeft.x = x;                      r.topLeft.y = y;
	r.extent.x = dotWidth*2 + dotSpace;		r.extent.y = dotWidth*2 + dotSpace;
   WinEraseRectangle( &r, 0 );

	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &r );
	ColorUnset();

   draw();

   cycle++;
   if( cycle > 3 )
      cycle = 0;
   }
   
void BusyIndicator::erase()
   {
	RectangleType	r;

	r.topLeft.x = x - 1;
   r.topLeft.y = y - 1;
	r.extent.x = dotWidth*2 + dotSpace + 2;
   r.extent.y = dotWidth*2 + dotSpace + 2;
   
   WinEraseRectangle( &r, 0 );
   }
