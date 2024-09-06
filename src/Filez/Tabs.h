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


#ifndef __tabs_h__
#define __tabs_h__

// the tab id is the same as its index in the tab set tab array

#include <PalmOS.h>
#include "SectionsCmn.hpp"

#define MAXTABS			10
#define TABHEIGHT			14
#define TOPMARGIN			15
#define DIVIDERWIDTH		7
#define EDGEWIDTH			4
#define LEADING			2		// the space between text lines


/**
 * A generic tab UI control.  The tab id is the same as its index in the
 * tab set tab array.
 */
class GenericTab
	{
	public:
	
	Char	_label[32];			// the tab's label
	int	_id;					// the tab's id number
	int	_textWidth;			// the width of the label text
	
	public:
	
	GenericTab( Char *label, int id ) SECT_CMN;
	int GetTextWidth() SECT_CMN;
	Char *GetLabel() SECT_CMN;
	};


/**
 * A generic tabset UI control.
 */
class GenericTabSet
	{
	private:
	
	GenericTab	*_tabs[MAXTABS];						// pointers to tab objects
	int			_tabCount;								// how many tabs there are
	int			_screenWidth;							// how wide the tabset should be
	int			_fullHeight;							// how high the tabset is
	int			_padding;								// whitespace in the tab	
	
	public:
	
	GenericTabSet( int screenWidth, int fullHeight ) SECT_CMN;
	~GenericTabSet() SECT_CMN;	
	void AddTab( Char *label, int i ) SECT_CMN;
	void FinalizeTabs() SECT_CMN;	
	void DrawTabs( int selected ) SECT_CMN;
	int CheckForTap( Coord px, Coord py ) SECT_CMN;
	};

#endif
