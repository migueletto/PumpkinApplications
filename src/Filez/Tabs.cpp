/****************************************************************************
 Common Code - A Generic Tab UI Component
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
 * A nifty generic tabset control.
 *
 * Created on 7/20/2002 by Tom Bulatewicz
 */


#include <PalmOS.h>
#include "Tabs.h"

	
/**
 * Constructor for the tab.
 *
 * @param The label to be displayed
 * @param a unique ID number
 */
GenericTab::GenericTab( Char *label, int id )
	{
	StrCopy( _label, label );
	_id = id;
	_textWidth = FntCharsWidth( label, StrLen( label ));
		}
	
int GenericTab::GetTextWidth()
	{
	return _textWidth;
	}

Char *GenericTab::GetLabel()
	{
	return _label;
	}

	
/**
 * Constructor for the tabset.
 *
 * @param The width of the screen
 * @param the full height of the tabset
 */
GenericTabSet::GenericTabSet( int screenWidth, int fullHeight )
	{
	_tabCount = 0;
	_screenWidth = screenWidth;
	_fullHeight = fullHeight;
	_padding = 0;
	}

GenericTabSet::~GenericTabSet()
	{
	for( int i=0; i<_tabCount; i++ )
		if( _tabs[i] != NULL )
			{
			delete _tabs[i];
			_tabs[i] = NULL;
			}
	}


/**
 * Adds a tab to the tabset.  Only call before calling FinalizeTabs().
 *
 * @param Its label
 * @param unique ID
 */
void GenericTabSet::AddTab( Char *label, int i )
	{
	if( i >= MAXTABS )								// can't add more than the max number
		return;
			
	_tabs[i] = new GenericTab( label, i );
	_tabCount++;
		
	FinalizeTabs();
	}


/**
 * This finalizes the tab configuration and performs all the calculations.
 * Call this method after all the tabs have been added.
 */
void GenericTabSet::FinalizeTabs()
	{
	int		textWidth=0;
	int		leftOverSpace;
		
	for( int i=0; i<_tabCount; i++ )
		if( _tabs[i] != NULL )				
			textWidth += _tabs[i]->GetTextWidth();		// get the total tab width
		
	leftOverSpace = _screenWidth - textWidth - 3;			// calculate extra space
	_padding = ( leftOverSpace / _tabCount ) / 2;		// calculate padding		
	}
	
	
/**
 * Draws the tabs.  Only call after calling FinalizeTabs().  A good place to
 * put this call is inside your frmUpdateEvent handler.
 * Note: pixels 0-4 are the left side, then a variable amount, then 7 for a divider
 *
 * @param Which tab is selected
 */
void GenericTabSet::DrawTabs( int selected )
	{
	int		y=TOPMARGIN;
	int		x=1;
	//int		c=0;
	int		tabWidth=0;
	RectangleType	border;
	PointType		topLeft, extent;
	
	topLeft.x = 0;  			topLeft.y = TOPMARGIN;
	extent.x = _screenWidth;	extent.y = TABHEIGHT+1;
	
	border.topLeft = topLeft;
	border.extent = extent;
		
   tabWidth = _screenWidth/_tabCount;
   
   RectangleType tab;
  	for( int i=0; i<_tabCount; i++ )
      {
      // adjust the last tab to line up with the edge nicely
      if( i == _tabCount-1 )
         tabWidth = _screenWidth - x - 1;

      tab.topLeft.x = x;			tab.topLeft.y = TOPMARGIN;
      tab.extent.x = tabWidth;	tab.extent.y = TABHEIGHT+3;
     
      WinEraseRectangle( &tab, 2 );
      WinDrawRectangleFrame( roundFrame, &tab );
      
      if( selected != i )
			WinDrawLine( x, TOPMARGIN+TABHEIGHT, x+tabWidth, TOPMARGIN+TABHEIGHT  );

      int pad = ( tabWidth - _tabs[i]->_textWidth) / 2;

		int xCoord = x + pad;
		WinDrawChars( _tabs[i]->GetLabel(), StrLen( _tabs[i]->GetLabel() ), xCoord, y+LEADING );
      
      x+=tabWidth+1;
      }
   
   // now erase the bottom couple pixels
   tab.topLeft.x = 0;				tab.topLeft.y = TOPMARGIN+TABHEIGHT+1;
   tab.extent.x = _screenWidth;	tab.extent.y = 3;
	WinEraseRectangle( &tab, 0 );
   
	WinDrawLine( 0, y+TABHEIGHT, 0, _fullHeight+TOPMARGIN-1 );		// left line
	WinDrawLine( _screenWidth-1, y+TABHEIGHT, _screenWidth-1, _fullHeight+TOPMARGIN-1 );		// left line
		
	WinDrawLine( 1, _fullHeight+TOPMARGIN, _screenWidth-2, _fullHeight+TOPMARGIN );		// draw the bottom line
	}


/**
 * Checks to see which tab the passed point is in (if any).
 *
 * @param x coordination of the tap
 * @param y coordination of the tap
 * @return The id of the tapped tab ( or -1 if no tab was tapped )
 */
int GenericTabSet::CheckForTap( Coord px, Coord py )
	{
	int		x=0;
	int		y=TOPMARGIN;
	int		tabWidth=0;
	int		c=0;
	
	for( int i=0; i<_tabCount; i++ )
		if( _tabs[i] != NULL )
			{
         tabWidth = (_screenWidth/_tabCount)+1;
			
         if( px > x && px < x+tabWidth && py > y && py < y+TABHEIGHT )
            return i;
               	
			x += tabWidth;
			c++;
			}			
	
	return -1;
	}
        
