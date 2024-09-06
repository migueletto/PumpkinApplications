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
 * This file contains all the code to support the preference
 *	viewer. The code in this file was taken from PrefEdit
 *	(under GPL) written by Bodo Bellut (bodo@bellut.net).
 *
 * Created on 1/3/03 by Tom Bulatewicz
 */

#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "PrefForm.h"
#include "Resource.h"
#include "Stuph.h"
#include "UI.h"
#include "Main.h"
#include "resize.h"


#undef DISPLAY_ID_TOO				// toggles whether or not the id for each is shown

// this struct is populated and the preference list is drawn based on it
typedef struct
	{
	UInt32		creator;
#ifdef DISPLAY_ID_TOO
	Int16		id;
#endif
    Char 		*name;
    } listItemType;

// there are two types of preferences, saved, and unsaved
typedef enum
	{
	SAVED = 0,
    UNSAVED
    } prefsType;

// these are used to display the hex contents of a preference
static char hex[16] = {'0', '1' ,'2', '3', '4', '5', '6', '7', '8', '9', 'a',
                'b','c','d','e','f'};

// this is the bounds of the display area when viewing a preference
static RectangleType dataRect = {{0, 20}, {160, 115}};

#define MEMOMAX 	4093			// max size of a memo record
#define MAXDB 		700				// max number of entries in the list
#define LINEWIDTH 	8				// how many chars are on each line when viewing
#define LINEHEIGHT 	11				// how may lines are displayed when viewing

#ifdef DISPLAY_ID_TOO
	#define MEMOLINE (32 + 5 + 3 + 6 + 7)
#else
	#define MEMOLINE (32 + 5 + 3 + 6)
#endif

// globals for the list form
static listItemType	items[2][MAXDB];
static Int16 			nitems[2];
static Int16 			currentItem[2] = { -1, -1};
static Int16 			currentTopItem[2] = { -1, -1};
static prefsType 		ptype = SAVED;

// globals for the view form
static MemPtr			ptr = NULL;
static MemHandle		hand = NULL;
static UInt32			prefoffset;

static DmOpenRef SECT8 OpenRecord();
static void SECT8 CloseRecord( DmOpenRef ref );


//#pragma mark -------- Export Functions --------


/**
 * Create a memo entry with the given text.
 *
 * @param The text for the memo
 * @return True if it was exported, false otherwise
 */
static Boolean SECT8 CreateMemo( const Char *text )
	{
    Boolean		retval;
    DmOpenRef 	dbref;
    UInt16 		len;
    
    retval = false;
    len = StrLen( text ) + 1;
    
    dbref = DmOpenDatabaseByTypeCreator( 'DATA', 'memo', dmModeReadWrite );
    if( dbref )
    	{
		MemHandle 	h;
		UInt16 		index;
	
		h = DmNewRecord( dbref, &index, len );
		if( h )
			{
	 	   	MemPtr	p;
	    
		    p = MemHandleLock( h );
		    if( p )
		    	{
				retval = true;
		
				if( DmSet( p, 0, len, 0 ))
				    retval = false;

				if( DmWrite( p, 0, (MemPtr) text, len ))
				    retval = false;
		
				if( MemPtrUnlock( p ))
				    retval = false;
	    		}
	    
	    	if( DmReleaseRecord( dbref, index, true ))
				retval = false;
	    
	    	if( !retval )
				DmDeleteRecord( dbref, index );
			}
	
		if( DmCloseDatabase( dbref ))
	    	retval = false;
    	}
    
    return retval;
	}


/**
 * Export the list of preferences to a memo.
 *
 * @return True if exported, false if it failed
 */
static Boolean SECT8 ExportList()
	{
    Boolean		retval = false;
    Char 		*str;
    int 		i;
    
    str = (Char*)MemPtrNew( MEMOMAX );
    
    if( str == NULL )
		return false;
	
    MemSet( str, MEMOMAX, 0 );
	
    StrCopy( str, "List of " );
    StrCat( str, (ptype == SAVED) ? "Saved" : "Unsaved" );
    StrCat( str, " Preferences\n\n" );
    
    for( i=0; ((i < nitems[ptype]) && (StrLen(str) < ( MEMOMAX - MEMOLINE ))); i++ )
    	{
		Char	creator[5];
		Char	num[5];
#ifdef DISPLAY_ID_TOO
		Char	id[6];

		StrIToA(id, (UInt) (items[ptype][i].id));
#endif
		MemMove(creator, &items[ptype][i].creator, sizeof(items[ptype][i].creator));

		creator[4] = '\0';
	
		StrIToA(num, i + 1);
	
		StrCat(str, num);
		StrCat(str, ". ");

		StrCat(str, creator);
		StrCat(str, "\t");
#ifdef DISPLAY_ID_TOO
		StrCat(str, id);
		StrCat(str, "\t");
#endif
		StrCat(str, items[ptype][i].name);
		StrCat(str, "\n");
    	}

	retval = CreateMemo( str );
	    
    MemPtrFree( str );

    return retval;
	}


/**
 * Export the contents of a preference to a memo.
 *
 * @param text to export
 */
static void SECT8 ExportRecordContent( Char *str )
	{
    static DmOpenRef ref = NULL;

    char			bytes[2];
    char			line[LINEWIDTH + 1];
    unsigned char 	byte;
    int 			i;
    int 			pos;

    currentItem[ptype] = LstGetSelection(GetObjectPtr<ListType>(PrefListPrefList));
    currentTopItem[ptype] = LstGlueGetTopItem(GetObjectPtr<ListType>(PrefListPrefList));

    // this sets the "ptr" global variable.
    ref = OpenRecord();

    Char 			*ptrC = (Char*)ptr;

   if( ptrC == NULL )
      ErrFatalDisplay( "Pointer is null in ExportRecordContent()." );

    i = 0;
    
    MemPtrSize(ptr);
    
    MemSet(line, LINEWIDTH, 0);

    for( pos = 0; pos < (int)MemPtrSize(ptr); pos++ )
    	{
		byte     = ptrC[pos];
		bytes[0] = hex[(byte >> 4) & 0xf];
		bytes[1] = hex[byte        & 0xf];
	
		StrCat(str, bytes);
	
		if ((byte > 31) && (byte < 127))
		    line[i] = byte;
		else
		    line[i] = '.';
	
		i++;
	
		if (i == LINEWIDTH)
			{
	    	StrCat(str, "\t");
	    	StrCat(str, line);
	    	StrCat(str, "\n");
	    	MemSet(line, LINEWIDTH, 0);
	    	i = 0;
			} 
		else
	    	StrCat(str, " ");
    	}
    	
    StrCat(str, "\t");
    StrCat(str, line);
    StrCat(str, "\n");

    CloseRecord(ref);
    DmCloseDatabase( ref );
	}


/**
 * Export a preference to a memo.
 *
 * @return if it was handled
 */
static Boolean SECT8 ExportItem()
	{
    Boolean 	retval;
    Char 		*str;
    Char 		num[5];
    Char 		creator[5];
    int			idx;

#ifdef DISPLAY_ID_TOO
    Char    id[6];
#endif

//showMessage( "2" );
    
    retval = false;
    
    str = (Char*)MemPtrNew( MEMOMAX );
    
    if( str == NULL )
		return false;
	
    MemSet( str, MEMOMAX, 0 );

//showMessage( "3" );
	
    idx = currentItem[ptype];
    
    if( !(idx > 0))
	idx = 0;

    StrIToA( num, idx + 1 );

    StrCopy( str, (ptype == SAVED) ? "Saved" : "Unsaved" );
    StrCat( str, " Preferences Record # " );
    StrCat( str, num );
    StrCat( str, "\n\n" );

//showMessage( "4" );
    
#ifdef DISPLAY_ID_TOO
    StrIToA( id, (UInt) (items[ptype][idx].id));
#endif
    MemMove( creator, &items[ptype][idx].creator, sizeof(items[ptype][idx].creator));

    creator[4] = '\0';
	
    StrCat( str, creator );
    StrCat( str, "\t" );
#ifdef DISPLAY_ID_TOO
    StrCat( str, id );
    StrCat( str, "\t" );
#endif
    StrCat( str, items[ptype][idx].name );
    StrCat( str, "\n" );


//showMessage( "5" );

    ExportRecordContent(str);
       

    retval = CreateMemo(str);
    
    MemPtrFree(str);


//showMessage( "6" );
    
    return (retval);
	}

//#pragma mark -------- Pref View Functions --------

typedef union {
  uint32_t t;
  uint8_t c[4];
} creator_id_t;

static void id2s(UInt32 ID, char *s) {
  creator_id_t id;

  id.t = ID;
  s[0] = id.c[3];
  s[1] = id.c[2];
  s[2] = id.c[1];
  s[3] = id.c[0];
  s[4] = 0;
}

/**
 * Display the preference data on the screen.
 */
static void SECT8 DisplayData()
	{
    char			bytes[2];
    char			line[LINEWIDTH];
    char			type[8];
    unsigned char	byte;
    int				i,j;
    int				pos;
    Char			*ptrC = (Char*)ptr;

    i = 0;
    j = 0;    
    
    WinEraseRectangle( &dataRect, 0 );
    
    line[0] = hex[(prefoffset >> 12) & 0xf];
    line[1] = hex[(prefoffset >>  8) & 0xf];
    line[2] = hex[(prefoffset >>  4) & 0xf];
    line[3] = hex[(prefoffset >>  0) & 0xf];
    line[4] = ':';
    
    //*((UInt32*)type) = items[ptype][currentItem[ptype]].creator;
    id2s(items[ptype][currentItem[ptype]].creator, type);
    
    WinDrawChars( type, 4, 30, 15 );
    WinDrawChars( line, 5,  0, 15 );
   
    {
	UInt32 size = MemPtrSize(ptr);
	Char sizeStr[32];
	StrPrintF( sizeStr, "(%ld bytes)", size );
	WinDrawChars( sizeStr, StrLen(sizeStr), 60, 15 );
    }
        
    MemSet( line, LINEWIDTH, ' ' );
    
    for( pos=prefoffset; pos < (int)MemPtrSize(ptr); pos++ )
    	{
		byte     = ptrC[pos];
		bytes[0] = hex[(byte >> 4) & 0xf];
		bytes[1] = hex[byte        & 0xf];
	
		WinDrawChars(bytes, 2, i * 12, 25 + j * 10);
	
		line[i] = byte;
		i++;
	
		if( i==LINEWIDTH ) 
			{
		    WinDrawChars( line, LINEWIDTH, 100, 25 + j * 10 );
		    MemSet( line, LINEWIDTH, ' ' );
		    i = 0;
		    j++;
	    
	    	if( j==LINEHEIGHT )
	    		{
				break;
	    		}
			}
    	}
    WinDrawChars( line, LINEWIDTH, 100, 25 + j * 10 );
	}


/**
 * Open a preference record for reading.
 *
 * @return A reference to the db
 */
static DmOpenRef SECT8 OpenRecord()
	{
    Err					err;
    LocalID				chunkID;
    UInt16				cardNo;
    LocalID				dbID;
    DmOpenRef			ref;
    Boolean				newSearch = 1;
    DmSearchStateType	state;
    char				tmpbuf[50];
    
    switch( ptype )
    	{
	    case SAVED:
			err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'sprf', 'psys', false, &cardNo, &dbID);
			ErrFatalDisplayIf(err == dmErrCantFind, "No Saved Preferences");
			break;
    
    	case UNSAVED:
			err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'pref', 'psys', false, &cardNo, &dbID);
			ErrFatalDisplayIf(err == dmErrCantFind, "No Unsaved Preferences");
			break;
    	}

    ref = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
    
    if( ref == 0 )
    	{
		err = DmGetLastErr();
		StrCopy(tmpbuf, "DmOpenDatabase: condition ");
		StrIToA((tmpbuf + StrLen(tmpbuf)), err);
		ErrFatalDisplay(tmpbuf);
	    }
    
    err = DmResourceInfo(ref, currentItem[ptype], NULL, NULL, &chunkID);
    ErrFatalDisplayIf(err, "DmResourceInfo");
    
    if( MemLocalIDKind(chunkID) == memIDHandle )
    	{
		hand = (MemHandle)MemLocalIDToGlobal(chunkID, cardNo);
		ptr = MemHandleLock(hand);
    	} 
    else
    	{
		ptr = MemLocalIDToGlobal(chunkID, cardNo);
		hand = NULL;
    	}
    
   ErrFatalDisplayIf( ptr == NULL, "Pointer is null in OpenRecord()." );
    
    return ref;
	}


/**
 * Close any open handles or pointers to records.
 *
 * @param The database reference
 */
static void SECT8 CloseRecord( DmOpenRef ref )
	{
    if( hand != NULL )
    	{
		MemHandleUnlock( hand );
		hand =NULL;
    	}
      
	}


/**
 * Update the scrollers (enable/disable).
 */
static void SECT8 UpdateScrollers()
	{
    if (prefoffset + LINEWIDTH * LINEHEIGHT < MemPtrSize(ptr))
		CtlSetLabel(GetObjectPtr<ControlType>(PrefViewDownRepeating), "\002");
    else
		CtlSetLabel(GetObjectPtr<ControlType>(PrefViewDownRepeating), "\004");

    if (prefoffset >= LINEWIDTH * LINEHEIGHT)
		CtlSetLabel(GetObjectPtr<ControlType>(PrefViewUpRepeating), "\001");
    else
		CtlSetLabel(GetObjectPtr<ControlType>(PrefViewUpRepeating), "\003");
	}


/**
 * Scroll the data up.
 */
static void SECT8 ScrollUp()
	{
    if( prefoffset >= LINEWIDTH * LINEHEIGHT )
        prefoffset -= LINEWIDTH * LINEHEIGHT;
    
    UpdateScrollers();
    DisplayData();
	}


/**
 * Scroll the data down.
 */
static void SECT8 ScrollDown()
	{
    if( prefoffset + LINEWIDTH * LINEHEIGHT < MemPtrSize( ptr ))
        prefoffset += LINEWIDTH * LINEHEIGHT;
    
    UpdateScrollers();
    DisplayData();
	}
	

static Boolean SECT8 handleKeyDown( EventPtr event )
	{
	Boolean handled = false;
	
	if( EvtKeydownIsVirtual( event ))
		{
		switch( event->data.keyDown.chr )
			{
			case vchrPageUp:
				ScrollUp();
				handled = true;
				break;
			case vchrPageDown:
				ScrollDown();
				handled = true;
				break;
			}
		}

	return handled;
	}


/**
 * Handle any events in the preference view form.
 *
 * @param an event
 * @return if it was handled
 */
Boolean PrefViewHandleEvent( EventPtr e )
	{
    Boolean				handled = false;
    FormPtr				frm;
    static DmOpenRef	ref = NULL;
    
    switch( e->eType )
    	{
 	   	case frmOpenEvent:
			frm = FrmGetActiveForm();
			FrmDrawForm( frm );
			
			ref = OpenRecord();
	
			prefoffset = 0;

			DisplayData();
			UpdateScrollers();

			handled = true;
			break;

		case frmCloseEvent:
			CloseRecord( ref );
			DmCloseDatabase( ref );
			break;

    	case ctlSelectEvent:
			switch( e->data.ctlSelect.controlID )
				{
				case PrefViewDoneButton:
	    			FrmGotoForm( PrefListForm );
	    			break;
				}
			break;

		case ctlRepeatEvent:
			switch( e->data.ctlRepeat.controlID )
				{
				case PrefViewUpRepeating:
					ScrollUp();
					break;
				
				case PrefViewDownRepeating:
					ScrollDown();
					break;
				}
		
    	case keyDownEvent:
			handled = handleKeyDown( e );
			break;
			default:
			break;
    	}

    return handled;
	}

//#pragma mark -------- Pref List Functions --------


/**
 * Draws the preference list.
 *
 * @param An item
 * @param its bounds
 * @param not used
 */
static void SECT8 DrawPrefList( Int16 itemNum, RectangleType *bounds, Char **unusedP )
	{
    Char 	*name = NULL;
    Char    creator[5];
#ifdef DISPLAY_ID_TOO
    Char    id[6];
#endif

    name = items[ptype][itemNum].name;
#ifdef DISPLAY_ID_TOO
    StrIToA(id, (UInt) (items[ptype][itemNum].id));
#endif
    //MemMove(creator, &items[ptype][itemNum].creator, sizeof(items[ptype][itemNum].creator));
    creator[0] = (items[ptype][itemNum].creator >> 24) & 0xFF;
    creator[1] = (items[ptype][itemNum].creator >> 16) & 0xFF;
    creator[2] = (items[ptype][itemNum].creator >> 8) & 0xFF;
    creator[3] = (items[ptype][itemNum].creator) & 0xFF;

    creator[4] = '\0';
    ErrFatalDisplayIf(!name, "Oops");
    
    WinDrawChars(creator, StrLen(creator), bounds->topLeft.x, bounds->topLeft.y);
#ifdef DISPLAY_ID_TOO
    // need to calculate the max width, so that it's not wider than the list!
    WinDrawChars(id, StrLen(id)    , bounds->topLeft.x + 30, bounds->topLeft.y);
    WinDrawChars(name, StrLen(name), bounds->topLeft.x + 60, bounds->topLeft.y);
	
#else
	// see how many chars should be drawn
	Int16	stringWidth=bounds->extent.x - (bounds->topLeft.x+30), stringLength = StrLen(name);
	Boolean	fits;
	FntCharsInWidth( name, &stringWidth, &stringLength, &fits );
	
    WinDrawChars(name, stringLength, bounds->topLeft.x + 30, bounds->topLeft.y);
#endif
	}


/**
 * Compares two items.
 *
 * @param r1 item to compare
 * @param r2 item to compare
 * @param other unused
 */
	/*
static Int16 PrefCompare( void *s1, void *s2, long other )
	{
	Item 	*x, *y;
	Char	xs[5], ys[5];
	
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

	return 0;
	}
*/
/*
static void SECT8 sortItems()
	{
	showMessage( items[0].creator );
	
	//SysQSort( (void*)items, itemCount, sizeof(listItemType), PrefCompare, NULL);
	}
*/

/**
 * Reads all the preference info and creates an array of it.
 *
 * @param Saved/Unsaved
 * @param the card number
 * @param the database id
 */
static void SECT8 FillList( prefsType type, UInt16 cardNo, LocalID dbID )
	{
    Err					err;
    DmOpenRef			ref;
    DmSearchStateType	state;
    Boolean				newSearch = 1;
    UInt32				numRecords;
    UInt32				resType;
#ifdef DISPLAY_ID_TOO
    Int16				resID;
#endif
    Int16				j;
    Char				tmpbuf[50];
    
    ref = DmOpenDatabase( cardNo, dbID, dmModeReadOnly );
    if( ref == 0 )
    	{
		err = DmGetLastErr();
		StrCopy( tmpbuf, "DmOpenDatabase: condition " );
		StrIToA( (tmpbuf + StrLen( tmpbuf )), err );
		ErrFatalDisplay( tmpbuf );
    	}
    
    err = DmDatabaseSize( cardNo, dbID, &numRecords, NULL, NULL );
    ErrFatalDisplayIf( err != 0, "DmDatabaseSize" );

    for( j=0; j < nitems[type]; j++ )
		MemPtrFree( items[type][j].name );
    nitems[type] = 0;

    for( nitems[type]=0; nitems[type] < (Int32)numRecords && nitems[type] < MAXDB-1; nitems[type]++ )
    	{
		char name[50];
    
		items[type][nitems[type]].name = (Char*)MemPtrNew(40);
	
		ErrFatalDisplayIf(items[type][nitems[type]].name == NULL, "MemPtrNew");
	
		MemSet(items[type][nitems[type]].name, 40, 0);
#ifdef DISPLAY_ID_TOO
		err = DmResourceInfo(ref, nitems[type], &resType, &resID, NULL);
#else
		err = DmResourceInfo(ref, nitems[type], &resType, NULL, NULL);
#endif
		ErrFatalDisplayIf(err, "Couldn't open record");
		items[type][nitems[type]].creator = resType;
#ifdef DISPLAY_ID_TOO
		items[type][nitems[type]].id      = resID;
#endif
		if (resType == 'psys')
			{
	    	StrCopy( items[type][nitems[type]].name, "PalmOS" );
			}
		else
			{
		    err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'appl', resType, false, &cardNo, &dbID);
		    if (err == dmErrCantFind)
		    	{
				err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'HACK', resType, false, &cardNo, &dbID);
			    }
	    	if (err == dmErrCantFind) 
	    		{
				err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'panl', resType, false, &cardNo, &dbID);
	    		}
	    	if (err == dmErrCantFind) 
	    		{
				err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'exgl', resType, false, &cardNo, &dbID);
	    		}
	    	if (err == dmErrCantFind) 
	    		{
				err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'DAcc', resType, false, &cardNo, &dbID);
	    		}
	    	if (err == dmErrCantFind) 
	    		{
				StrCopy(items[type][nitems[type]].name, "-none-");
	    		} 
	    	else 
	    		{
				err = DmDatabaseInfo(cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		        ErrFatalDisplayIf(err != 0, "DmDatabaseInfo");
				StrNCopy(items[type][nitems[type]].name, name, 39);
	    		}
			}
    	}
		
    DmCloseDatabase(ref);
	 
	 
	// sortItems();
	}


/**
 * Setup the pref list form.
 */
static void SECT8 PrefListInit()
	{
    ListType 			*listP;
    Err					err;
    UInt16				cardNo;
    LocalID				dbID;
    DmSearchStateType	state;
    Boolean				newSearch = 1;
    
    prefs.lastForm = PrefListForm;
        
    err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'sprf', 'psys', false, &cardNo, &dbID);
    ErrFatalDisplayIf(err == dmErrCantFind, "No Saved Preferences");
    FillList( SAVED, cardNo, dbID );

    err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'pref', 'psys', false, &cardNo, &dbID);
    ErrFatalDisplayIf( err == dmErrCantFind, "No Unsaved Preferences");    
    FillList( UNSAVED, cardNo, dbID );

    if( nitems[ptype] )
    	{
		listP = GetObjectPtr<ListType>( PrefListPrefList );
	
		LstSetListChoices( listP, NULL, nitems[ptype] );
		LstSetHeight( listP, 10 );
		LstSetDrawFunction( listP, DrawPrefList );
	
		if( currentItem[ptype] != -1 && currentItem[ptype] < nitems[ptype] )
		    LstSetSelection( listP, currentItem[ptype] );
		if( currentTopItem[ptype] != -1 && currentTopItem[ptype] < nitems[ptype] )
		    LstSetTopItem( listP, currentTopItem[ptype] );
    	}

    FrmDrawForm( FrmGetActiveForm());

	}


/**
 * Remove an entry from the preference list.
 *
 * @param The index to remove
 */
static void SECT8 RemoveEntryFromList( Int16 index )
	{
    ListPtr listP = GetObjectPtr<ListType>( PrefListPrefList );
	
    MemPtrFree( items[ptype][index].name );
    if( index < nitems[ptype] )
    	{
		MemMove(&items[ptype][index], &items[ptype][index+1], sizeof(items[ptype][index]) * (nitems[ptype] - index));
    	}
    items[ptype][nitems[ptype]--].name = NULL;

    if( currentItem[ptype] == index )
	currentItem[ptype]--;

    LstSetListChoices( listP, NULL, nitems[ptype] );
    LstDrawList( listP );
	}


/**
 * Deletes a preference.
 *
 * @param The index of the preference to delete
 */
static void SECT8 DeleteRecord( Int16 index)
	{
    UInt16				cardNo;
    LocalID				dbID;
    Err					err;
    DmOpenRef			ref;
    DmSearchStateType	state;
    Boolean				newSearch = 1;
    Char				tmpbuf[50];
    
    if( DEBUG ) return;
    
    switch( ptype )
    	{
 	   	case SAVED:
			err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'sprf', 'psys', false, &cardNo, &dbID);
			ErrFatalDisplayIf(err == dmErrCantFind, "No Saved Preferences");
			break;
    
    	case UNSAVED:
			err = DmGetNextDatabaseByTypeCreator(newSearch, &state, 'pref', 'psys', false, &cardNo, &dbID);
			ErrFatalDisplayIf(err == dmErrCantFind, "No Unsaved Preferences");
			break;
    	}

    ref = DmOpenDatabase(cardNo, dbID, dmModeReadWrite);
    
    if( ref == 0 )
    	{
		err = DmGetLastErr();
		StrCopy(tmpbuf, "DmOpenDatabase: condition ");
		StrIToA((tmpbuf + StrLen(tmpbuf)), err);
		ErrFatalDisplay(tmpbuf);
    	}

    err = DmRemoveResource(ref, index);
    
    ErrNonFatalDisplayIf(err != 0, "DmRemoveResource");
    DmCloseDatabase(ref);
    RemoveEntryFromList(index);
	}


static void SECT8 SetNumber( FormPtr frm )
	{
    char tmpbuf[20];
    RectangleType r;

    FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, PrefListNumberLabel), &r);
    WinEraseRectangle(&r, 0);

    if( nitems[ptype] > 0 )
    	{
		if( currentItem[ptype] > 0 )
	    	StrIToA(tmpbuf, currentItem[ptype]+1);
		else
	    	StrIToA(tmpbuf, 1);
		StrCat(tmpbuf, "/");
		StrIToA((tmpbuf + StrLen(tmpbuf)), nitems[ptype]);

		FrmCopyLabel(frm, PrefListNumberLabel, tmpbuf);
	    }
	}


/**
 * Deallocate the list memory.
 */
static void SECT8 PrefListFree()
	{	
    int i;

    for (i=0; i < nitems[SAVED]; i++)
		MemPtrFree(items[SAVED][i].name);
    nitems[SAVED] = 0;

    for (i=0; i < nitems[UNSAVED]; i++)
		MemPtrFree(items[UNSAVED][i].name);
    nitems[UNSAVED] = 0;	
	}


/**
 * Handle any events in the preference list.
 *
 * @param an event
 * @return if it was handled
 */
Boolean PrefListHandleEvent( EventPtr e )
	{
    Boolean 	handled = false;
    FormPtr 	frm = FrmGetActiveForm();

   if ( ResizeHandleEvent( e ) )
      return true;
    
    switch( e->eType )
    	{     
		case keyDownEvent:
			if( e->data.keyDown.chr == pageUpChr || e->data.keyDown.chr == pageDownChr )
				{
	    		ListPtr listP = GetObjectPtr<ListType>(PrefListPrefList);
	    		LstScrollList(listP, (e->data.keyDown.chr == pageUpChr) ? winUp : winDown,  LstGetVisibleItems(listP) - 1);
				//RecordListScroll( winUp );
				handled = true;
				}
         break;


	    case frmOpenEvent:

			PrefListInit();

			switch (ptype)
				{
	    		case SAVED:
					CtlSetValue( GetObjectPtr<ControlType>( PrefListSavedPushButton ), true );
					break;
	    		case UNSAVED:
					CtlSetValue( GetObjectPtr<ControlType>( PrefListUnsavedPushButton ), true );
					break;
				}


			SetNumber(frm);
	
			handled = true;
			break;

    	case ctlSelectEvent:
			switch(e->data.ctlSelect.controlID)
				{
				case PrefListExportButton:
				
//					showMessage( "1" );
				
               if (nitems[ptype] > 0)
                  ExportItem();
					handled = true;
					break;	
		
				case PrefListExportListButton:
				    ExportList();
					handled = false;
					break;
	
				case PrefListDoneButton:
					FrmGotoForm( MainViewForm );
					handled = true;
					break;
		
				case PrefListDeleteButton:
					{
				    Int16 	sel;
				    Char 	creator[5];
				    Char 	*name = NULL;
	
				    if (nitems[ptype] > 0)
				    	{
						sel = LstGetSelection(GetObjectPtr<ListType>(PrefListPrefList));
		
						name = items[ptype][sel].name;
						MemMove(creator, &items[ptype][sel].creator, sizeof(items[ptype][sel].creator));
                  creator[4] = 0;
      
                     if (!FrmCustomAlert(DeletePrefAlert, name, creator, " "))
							{
						    DeleteRecord(sel);
						    SetNumber(frm);
							}
		    			}
	    			handled = true;
	    			}
	    			break;
	
		case PrefListViewButton:
	    	if (nitems[ptype] > 0)
	    		{
				currentItem[ptype] = LstGetSelection(GetObjectPtr<ListType>(PrefListPrefList));
				currentTopItem[ptype] = LstGlueGetTopItem(GetObjectPtr<ListType>(PrefListPrefList));
		
				FrmGotoForm( PrefViewForm );
	    		}
	    	handled = true;
	    	break;
		
		case PrefListSavedPushButton:
	    	if (ptype == SAVED)
	    		{
				handled = true;
				break;
	    		}
	    	// else fall through
		case PrefListUnsavedPushButton:
			{
	    	ListPtr listP = GetObjectPtr<ListType>(PrefListPrefList);
	    	if (e->data.ctlSelect.controlID == PrefListUnsavedPushButton && ptype == UNSAVED)
	    		{
				handled = true;
				break;
	    		}
	    
		    currentItem[ptype]    = LstGetSelection(listP);
		    currentTopItem[ptype] = LstGlueGetTopItem(listP);
	    
	 	   	if (ptype == UNSAVED)
				ptype = SAVED;
		    else
				ptype = UNSAVED;
		    
		    LstSetListChoices(listP, NULL, nitems[ptype]);
	    	if (currentItem[ptype] != -1 && currentItem[ptype] < nitems[ptype])
				LstSetSelection(listP, currentItem[ptype]);
	    	else
				if (nitems[ptype] > 0)
		    		LstSetSelection(listP, 0);
	    	if (currentTopItem[ptype] != -1 && currentTopItem[ptype] < nitems[ptype])
			LstSetTopItem(listP, currentTopItem[ptype]);
	    	else
			if (nitems[ptype] > 0)
		    	LstSetTopItem(listP, 0);
	    	LstDrawList(listP);
	    	SetNumber(frm);
	    	handled = true;
	    	}
	    	break;
		}
		break;

    case lstSelectEvent:
		currentItem[ptype] = e->data.lstSelect.selection;
		SetNumber(frm);
		// DON'T set handled
		break;


	case frmCloseEvent:
		PrefListFree();
		break;
  default:
    break;
    }

    return handled;
	}
