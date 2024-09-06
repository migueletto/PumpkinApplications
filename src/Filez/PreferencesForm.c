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
 * The file type color selection form.
 *
 * Created on 7/14/03 by Tom Bulatewicz
 */
 
#include <PalmOS.h>						// all the system toolbox headers

#include "Main.h"
#include "Tabs.h"
#include "PreferencesForm.h"
#include "Stuph.h"
#include "UI.h"
#include "Resource.h"
#include "TreeViewForm.h"
#include "resize.h"

static RGBColorType	theColor;			// The currently selected color

static GenericTabSet	*prefTabs;
static UInt8			curTab;

#define generalTab	0
#define typecolorTab	1

#define top			32
#define col1		3
#define col2		70
#define height		13

// the bounds of the color box in the type colors tab
#define colorTopLeftx	61
#define colorTopLefty	113
#define colorExtentx		23
#define colorExtenty 	8

// the bounds of the row color box in the general tab
#define rowcolorTopLeftx	88
#define rowcolorTopLefty	42
#define rowcolorExtentx		23
#define rowcolorExtenty 	8

/**
 * Draw the type as a string, in its color.
 *
 * @param The item
 * @param it's bounds
 * @param it's text (which is null)
 * @return if it was handled
 */
static void SECT6 TypeColorDrawRecord( Int16 itemNum, RectangleType *bounds, Char **itemsText )
	{
	Char		text[5];
	
	IntToStr( prefs.list.typeColors.types[itemNum], text );
	ColorSet( NULL, NULL, &prefs.list.typeColors.colors[itemNum], NULL, NULL );
	WinDrawChars( text, StrLen( text ), bounds->topLeft.x, bounds->topLeft.y );
	ColorUnset();
	}


/**
 * Draw the type list.
 */
static void SECT6 TypeColorDrawList()
	{
	ListPtr		lst;
	lst = GetObjectPtr<ListType>( PreferencesTypeList );
	LstSetDrawFunction( lst, TypeColorDrawRecord );
	LstSetListChoices( lst, NULL, prefs.list.typeColors.count );
	LstDrawList( lst );
	}


/**
 * Hides the small details area.
 */
static void SECT6 TypeColorHideDetails()
	{
	RectangleType	rect;
	
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, PreferencesTypeLabel );
	HideObject( frm, PreferencesTypeField );
	HideObject( frm, PreferencesColorLabel );
	HideObject( frm, PreferencesOKButton );
	
	rect.topLeft.x = colorTopLeftx-1;
	rect.topLeft.y = colorTopLefty-1;
	rect.extent.x = colorExtentx+2;
	rect.extent.y = colorExtenty+2;
	
	WinEraseRectangle( &rect, 0 );
	}


/**
 * Draws the current color in the details area.
 */
static void SECT6 TypeColorDrawColor()
	{
	RectangleType	rect;

	rect.topLeft.x = colorTopLeftx;
	rect.topLeft.y = colorTopLefty;
	rect.extent.x = colorExtentx;
	rect.extent.y = colorExtenty;
	
	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &rect );
	ColorUnset();
	
	ColorSet( &theColor, NULL, NULL, NULL, NULL );
	WinDrawRectangle( &rect, 0 );	
	ColorUnset();
	}


/**
 * Draws the frame around the details area.
 */
static void SECT6 TypeColorDrawFrame()
	{
	RectangleType	rect;
	
	rect.topLeft.x = 20;
	rect.topLeft.y = 91;
	rect.extent.x = 114;
	rect.extent.y = 35;

	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &rect );
	ColorUnset();
	}


/**
 * Draws the details area.
 */
static void SECT6 TypeColorShowDetails()
	{
	FormPtr		frm = FrmGetActiveForm();
		
	ShowObject( frm, PreferencesTypeLabel );
	ShowObject( frm, PreferencesTypeField );
	ShowObject( frm, PreferencesColorLabel );
	ShowObject( frm, PreferencesOKButton );

	TypeColorDrawColor();	
	}


/**
 * Handle what happens when the OK button is tapped.
 */
static void SECT6 TypeColorHandleOKButton()
	{
	Char 		textid[5];
	FieldPtr 	fld;
	UInt32		numid;
	
	TypeColorHideDetails();

	fld = GetObjectPtr<FieldType>( PreferencesTypeField );
   if( FldGetTextPtr( fld ))
      {
      StrCopy( textid, FldGetTextPtr( fld ));
      numid = StrToInt( textid );
      prefs.list.typeColors.types[prefs.list.typeColors.count] = numid;
      prefs.list.typeColors.colors[prefs.list.typeColors.count] = theColor;
      prefs.list.typeColors.count++;
      }
      
	TypeColorDrawList();
	}
	

/**
 * Handle what happens when the Add button is tapped.
 */
static void SECT6 TypeColorHandleAddButton()
	{
   if( prefs.list.typeColors.count >= maxTypeColors )
      FrmAlert( 1201 );
   
	FieldPtr	fld;
	fld = GetObjectPtr<FieldType>( PreferencesTypeField );
	FldDelete( fld, 0, FldGetTextLength( fld ));
	theColor = black;
	TypeColorShowDetails();
	}
		

/**
 * Handle what happens when the Remove button is tapped.
 */
static void SECT6 TypeColorHandleRemoveButton()
	{
	Int16    index;
	ListPtr	lst;
	
	lst = GetObjectPtr<ListType>( PreferencesTypeList );
	index = LstGetSelection( lst );
	
   // can't delete anything if there's nothing in the list.
   if( index < 0 )
      return;
   
	for( int i=index; i<prefs.list.typeColors.count-1; i++ )
		{
		prefs.list.typeColors.types[i] = prefs.list.typeColors.types[i+1];
		prefs.list.typeColors.colors[i] = prefs.list.typeColors.colors[i+1];
		}

	prefs.list.typeColors.count--;
	LstSetListChoices( lst, NULL, prefs.list.typeColors.count );
	TypeColorDrawList();
	}


static void SECT6 PrefsHideTypeColorTab()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, PreferencesAddButton );
	HideObject( frm, PreferencesRemoveButton );
	HideObject( frm, PreferencesTypeField );
	HideObject( frm, PreferencesTypeLabel );
	HideObject( frm, PreferencesColorLabel );
	HideObject( frm, PreferencesTypeList );
	HideObject( frm, PreferencesOKButton );
	}
   

static void SECT6 PrefsHideGeneralTab()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, PreferencesListColorLabel );
	HideObject( frm, PreferencesSelectAllLabel );
	HideObject( frm, PreferencesFilesCheckbox );
	HideObject( frm, PreferencesFoldersCheckbox );
	HideObject( frm, PreferencesSendLabel );
	HideObject( frm, PreferencesSendMenuPushButton );
	HideObject( frm, PreferencesBeamPushButton );
	HideObject( frm, PreferencesFoldersFirstCheckbox );
	HideObject( frm, PreferencesTapLabel );
	HideObject( frm, PreferencesTapNamePushButton );
	HideObject( frm, PreferencesTapIconPushButton );
	}
   

/**
 * Draws the current color in the details area.
 */
static void SECT6 PreferencesDrawRowColor()
	{
	RectangleType	rect;

	rect.topLeft.x = rowcolorTopLeftx;
	rect.topLeft.y = rowcolorTopLefty;
	rect.extent.x = rowcolorExtentx;
	rect.extent.y = rowcolorExtenty;
	
	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &rect );
	ColorUnset();
	
   RGBColorType col = prefs.list.rowColor;
	ColorSet( &col, NULL, NULL, NULL, NULL );
	WinDrawRectangle( &rect, 0 );	
	ColorUnset();
	}


/**
 * Draw the hotsync tab.
 */
static void SECT6 PrefsShowGeneralTab()
	{
   FormPtr frm = FrmGetActiveForm();
	ShowObject( frm, PreferencesListColorLabel );
	ShowObject( frm, PreferencesSelectAllLabel );
	ShowObject( frm, PreferencesFilesCheckbox );
	ShowObject( frm, PreferencesFoldersCheckbox );
	ShowObject( frm, PreferencesSendLabel );
	ShowObject( frm, PreferencesSendMenuPushButton );
	ShowObject( frm, PreferencesBeamPushButton );
	ShowObject( frm, PreferencesFoldersFirstCheckbox );
	ShowObject( frm, PreferencesTapLabel );
	ShowObject( frm, PreferencesTapNamePushButton );
	ShowObject( frm, PreferencesTapIconPushButton );

	PreferencesDrawRowColor();
   
	if( prefTabs )
		prefTabs->DrawTabs( generalTab );
	}


/**
 * Erase the contents of the tabs.
 */
static void SECT6 PrefsHideTabs( Boolean erase )
	{
   PrefsHideGeneralTab();
   PrefsHideTypeColorTab();

   if( erase )
      {
      RectangleType	r;
      r.topLeft.x = 2;		r.topLeft.y = top;		r.extent.x = 156;	r.extent.y = 106;
      WinEraseRectangle( &r, 0 );
      }
	}


/**
 * Draw the hotsync tab.
 */
static void SECT6 PrefsShowTypeColorTab()
	{
	ListPtr		lst;

	lst = GetObjectPtr<ListType>( PreferencesTypeList );
	LstSetSelection( lst, -1 );		

	TypeColorHideDetails();	
	TypeColorDrawFrame();
	TypeColorDrawList();

   FormPtr frm = FrmGetActiveForm();
	ShowObject( frm, PreferencesAddButton );
	ShowObject( frm, PreferencesRemoveButton );
	ShowObject( frm, PreferencesTypeList );
	
	if( prefTabs )
		prefTabs->DrawTabs( typecolorTab );	
	}


/**
 * Tell the current tab to draw itself.
 */
static Boolean SECT6 PrefsShowCurrentTab()
	{
	switch( curTab )
		{
		case generalTab:
			PrefsHideTabs( true );
			PrefsShowGeneralTab();
			return true;
		
		case typecolorTab:
			PrefsHideTabs( true );
			PrefsShowTypeColorTab();
			return true;		
		}
		
	return false;
	}


/**
 * Draw the background of the tabs.
 */
static void SECT6 PrefsDrawBackground()
	{
	ColorSet( &bgColor, 0, 0, 0, 0 );
	RectangleType	r;

	r.topLeft.x = 0;		r.topLeft.y = 135;		r.extent.x = 159;	r.extent.y = 21;	
	WinDrawRectangle( &r, 0 );

	r.topLeft.x = 0;		r.topLeft.y = 12;		r.extent.x = 159;	r.extent.y = 5;	
	WinDrawRectangle( &r, 0 );
  
	ColorUnset();

	FormPtr	frm = FrmGetActiveForm();

	// this is to get around a bug in old PalmOS versions (like 3.0) where the
	//	whole control is not drawn by FrmShowObject like it should be.
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, PreferencesCloseButton ), &r );
	WinEraseRectangle( &r, 1 );

	ShowObject( frm, PreferencesCloseButton );
	}


/**
 * Sets up the form.
 */
static void SECT6 handleOpenEvent()
	{
	ControlPtr	ctl;
  Char buf[32];

   FormPtr frm = FrmGetActiveForm();
	PrefsHideTabs( false );
	FrmDrawForm( frm );
	PrefsDrawBackground();

 	prefTabs = new GenericTabSet( 156, 123 );
	
  StrCopy(buf, "General");
	prefTabs->AddTab( buf, generalTab );
  StrCopy(buf, "Type Colors");
	prefTabs->AddTab( buf, typecolorTab );
	
	prefTabs->FinalizeTabs();
	
   ctl = GetObjectPtr<ControlType>( PreferencesFilesCheckbox );			
   if( prefs.list.selectAll == selectAllFiles || prefs.list.selectAll == selectAllBoth )
      CtlSetValue( ctl, true );
      
   ctl = GetObjectPtr<ControlType>( PreferencesFoldersCheckbox );			
   if( prefs.list.selectAll == selectAllFolders || prefs.list.selectAll == selectAllBoth )
      CtlSetValue( ctl, true );

   if( prefs.sendMenu == 0 )
      ctl = GetObjectPtr<ControlType>( PreferencesSendMenuPushButton );
   else
      ctl = GetObjectPtr<ControlType>( PreferencesBeamPushButton );
   CtlSetValue( ctl, true );

   if( prefs.list.folderSelect == folderSelectName )
      ctl = GetObjectPtr<ControlType>( PreferencesTapNamePushButton );
   else
      ctl = GetObjectPtr<ControlType>( PreferencesTapIconPushButton );
   CtlSetValue( ctl, true );

   if( prefs.list.foldersFirst == 1 )
      {
      ctl = GetObjectPtr<ControlType>( PreferencesFoldersFirstCheckbox );
      CtlSetValue( ctl, true );
      }
      
   curTab = generalTab;
	PrefsShowCurrentTab();
	}


static void SECT6 handleCloseButton()
   {
	ControlPtr	fileCtl, folderCtl, sendCtl, foldersCtl, tapCtl;

   fileCtl = GetObjectPtr<ControlType>( PreferencesFilesCheckbox );			
   folderCtl = GetObjectPtr<ControlType>( PreferencesFoldersCheckbox );			
   if( CtlGetValue( fileCtl ) && CtlGetValue( folderCtl ))
      prefs.list.selectAll = selectAllBoth;
   else
      {
      if( CtlGetValue( fileCtl ))
         prefs.list.selectAll = selectAllFiles;
      else
         prefs.list.selectAll = selectAllFolders;
      }

   sendCtl = GetObjectPtr<ControlType>( PreferencesSendMenuPushButton );			
   if( CtlGetValue( sendCtl ))
      prefs.sendMenu = 0;
   else
      prefs.sendMenu = 1;      

   tapCtl = GetObjectPtr<ControlType>( PreferencesTapNamePushButton );			
   if( CtlGetValue( tapCtl ))
      prefs.list.folderSelect = folderSelectName;
   else
      prefs.list.folderSelect = folderSelectIcon;

   foldersCtl = GetObjectPtr<ControlType>( PreferencesFoldersFirstCheckbox );
   if( CtlGetValue( foldersCtl ))
      prefs.list.foldersFirst = 1;
   else
      prefs.list.foldersFirst = 0;
   }
   

/**
 * Sets up the form.
 *
 * @param The direction to scroll
 */
static void SECT6 TypeColorScroll( WinDirectionType direction )
	{
	ListPtr	lst = GetObjectPtr<ListType>( PreferencesTypeList );
	LstScrollList( lst, direction, LstGetVisibleItems( lst ));
	}


/**
 * Handle when the user taps on the form.
 *
 * @param x coordinate of the tap
 * @param y coordinate of the tap
 * @return if it was handled
 */
static Boolean SECT6 TypeColorHandleTap( Int16 x, Int16 y )
	{
   if( colorDepth == 1 )
      return false;
   
	if( x > colorTopLeftx && x < colorTopLeftx+colorExtentx && y > colorTopLefty && y < colorTopLefty+colorExtenty )
		{
		UIPickColor( NULL, &theColor, 0, NULL, NULL );		
		TypeColorDrawColor();
		return true;
		}
	return false;
	}


static Boolean SECT6 handleUpdateEvent()
   {
   FrmDrawForm( FrmGetActiveForm());
   PrefsShowCurrentTab();

   return true;
   }


/**
 * Handle when the user taps on the form.
 *
 * @param x coordinate of the tap
 * @param y coordinate of the tap
 * @return if it was handled
 */
static Boolean SECT6 PrefsRowColorHandleTap( Int16 x, Int16 y )
	{
   if( colorDepth == 1 )
      return false;
   
	if( x > rowcolorTopLeftx && x < rowcolorTopLeftx+rowcolorExtentx && y > rowcolorTopLefty && y < rowcolorTopLefty+rowcolorExtenty )
		{
      RGBColorType col = prefs.list.rowColor;
		UIPickColor( NULL, &col, 0, NULL, NULL );		
      prefs.list.rowColor = col;
      PreferencesDrawRowColor();
		return true;
		}
	return false;
	}


/**
 * Given a set of coordinates, determine if they tapped on a tab.
 *
 * @param x coordinate
 * @param y coordingate
 * @return if it was handled
 */
static Boolean SECT6 PrefsHandleTap( Int16 x, Int16 y )
	{
	int	whichTab = prefTabs->CheckForTap( x, y );
	
	switch( whichTab )
		{
		case -1:
			return false;

		case generalTab:
			if( curTab != generalTab )
				{
				curTab = generalTab;
				PrefsShowCurrentTab();
				}
			return true;
		
		case typecolorTab:
			if( curTab != typecolorTab )
				{
				curTab = typecolorTab;
				PrefsShowCurrentTab();
				}
			return true;		
		}

	return false;
	}


/**
 * Setup the info form.
 */
static void SECT6 PrefsFreeMemory()
	{
	delete prefTabs;
	}


// do not use pageUpChr/pageDownChr
static Boolean SECT6 handleKeyDown( EventPtr event )
	{
	Boolean handled = false;
	
	if( EvtKeydownIsVirtual( event ))
		{
		switch( event->data.keyDown.chr )
			{
			case vchrPageUp:
				TypeColorScroll( winUp );
				handled = true;
				break;
			case vchrPageDown:
				TypeColorScroll( winDown );
				handled = true;
				break;
			}
		}

	return handled;
	}


/**
 * Handle all form events.
 *
 * @param an event
 * @return if it was handled
 */
Boolean PreferencesHandleEvent( EventPtr event )
	{
	Boolean handled = false;

   if( ResizeHandleEvent( event ))
      return true;

	switch( event->eType )
		{
		case penDownEvent:											// if they did anything at all		
			handled = PrefsHandleTap( event->screenX, event->screenY );
			if( !handled )
            handled = TypeColorHandleTap( event->screenX, event->screenY );
			if( !handled )
            handled = PrefsRowColorHandleTap( event->screenX, event->screenY );
			break;
		
		case keyDownEvent:
			handled = handleKeyDown( event );
			break;

		case ctlSelectEvent:
			switch( event->data.ctlSelect.controlID )
				{
				case PreferencesAddButton:
					TypeColorHandleAddButton();
					handled=true;
					break;

				case PreferencesRemoveButton:
					TypeColorHandleRemoveButton();
					handled=true;
					break;

				case PreferencesOKButton:
					TypeColorHandleOKButton();
					handled=true;
					break;

				case PreferencesCloseButton:
               handleCloseButton();
					FrmGotoForm( TreeViewForm );
               FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );
					handled=true;
					break;
				}
			break;

		case frmOpenEvent:
			handleOpenEvent();
			handled = true;
			break;

		case frmUpdateEvent:
         handled = handleUpdateEvent();
			break;

		case frmCloseEvent:
			PrefsFreeMemory();
			break;			
    default:
			break;			
		}
	
	return handled;
	}
