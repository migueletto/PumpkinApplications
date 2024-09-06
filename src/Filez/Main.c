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
 * File manager app for PalmOS.
 *
 * Created on 4/7/00 by Tom Bulatewicz
 */
 
#include <PalmOS.h>								// all the system toolbox headers
#include <VFSMgr.h>
#include "resize.h"
#include "DIA.h"

#include "Main.h"
#include "Stuph.h"
#include "UI.h"
#include "Resource.h"							// application resource defines
#include "DetailsForm.h"
#include "FilterForm.h"
#include "Chooser.h"
#include "RecListForm.h"
#include "HexForm.h"
#include "MenuForm.h"
#include "InformationForm.h"
#include "PrefForm.h"
#include "PreferencesForm.h"
#include "TreeViewForm.h"
#include "FolderDetails.h"
#include "SetAttributes.h"
#include "CardSet.hpp"

#include "debug.h"

#undef keyBitJogBack
//#include <SonyCLIE.h>

UInt16 gHRRefNum = sysInvalidRefNum;


CardSet cardSet;

DateFormatType		ShortDateFormat;			// the system date settings
DateFormatType		LongDateFormat;
PrefType				prefs;
extern UInt16		fileOrder;					// declared in list.c
UInt32				original;					// the original colormode
Boolean           deferSleep;             // if we should not go to sleep now

//static UInt16 *ftrCardNoP, oldftrCardNo;
//static LocalID *ftrDbIDP, oldftrDbID;

//static UInt16** maskPP;
//static UInt16* oldmaskP;

#define NUMBER_OF_MASK 1
#if 0
UInt16 mask[] = { sonyJogAstMaskType1,
                  NUMBER_OF_MASK,	// number of forms to mask
                  TreeViewForm, sonyJogAstMaskUp | sonyJogAstMaskDown
                  	// mask JogUp and JogDown for the list form
                  	// see SonyJogAssist.h for other mask values
                  	// <formID>, <mask>,
                  	// ...
                };
#endif


/**
 * This routine loads a form resource and sets the event handler for the form.
 *
 * @param an event
 * @return True if the event has been handled and should not be passed to a higher level handler
 */
static Boolean SECT2 appHandleEvent( EventPtr eventP )
	{
	UInt16	formId;
	FormPtr	frmP;

	if( eventP->eType == frmLoadEvent )
		{
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm( formId );
		FrmSetActiveForm( frmP );

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an event.
		switch( formId )
			{
			case DetailsRForm:
            SetResizePolicy( DetailsRForm );
				FrmSetEventHandler( frmP, DetailsRHandleEvent );
				break;
			case FilterForm:
            SetResizePolicy( FilterForm );
				FrmSetEventHandler( frmP, FilterHandleEvent );
				break;
			case DetailsVForm:
            SetResizePolicy( DetailsVForm );
				FrmSetEventHandler( frmP, DetailsVHandleEvent );
				break;
			case MainViewForm:
            SetResizePolicy( MainViewForm );
				FrmSetEventHandler( frmP, MainViewHandleEvent );
				break;
			case ChooserForm:
            SetResizePolicy( ChooserForm );
				FrmSetEventHandler( frmP, ChooserHandleEvent );
				break;
			case FolderDetailsForm:
            SetResizePolicy( FolderDetailsForm );
				FrmSetEventHandler( frmP, FolderDetailsHandleEvent );
				break;
			case RecordListForm:
            SetResizePolicy( RecordListForm );
				FrmSetEventHandler( frmP, RecordListHandleEvent );
				break;
			case HexEditorForm:
            SetResizePolicy( HexEditorForm );
				FrmSetEventHandler( frmP, HexEditorHandleEvent );
				break;
			case InformationForm:
            SetResizePolicy( InformationForm );
				FrmSetEventHandler( frmP, InfoHandleEvent );
				break;
			case PrefListForm:
            SetResizePolicy( PrefListForm );
				FrmSetEventHandler( frmP, PrefListHandleEvent );
				break;
			case PrefViewForm:
            SetResizePolicy( PrefViewForm );
				FrmSetEventHandler( frmP, PrefViewHandleEvent );
				break;
			case PreferencesForm:
            SetResizePolicy( PreferencesForm );
				FrmSetEventHandler( frmP, PreferencesHandleEvent );
				break;
			case TreeViewForm:
            SetResizePolicy( TreeViewForm );
				FrmSetEventHandler( frmP, TreeViewHandleEvent );
				break;
			case SetAttributesForm:
            SetResizePolicy( SetAttributesForm );
				FrmSetEventHandler( frmP, SetAttributesHandleEvent );
				break;
//			case GotoForm:
//            SetResizePolicy( GotoForm );
//				FrmSetEventHandler( frmP, GotoFormHandleEvent );
//				break;
			}
		return true;
		}
	
	return false;
	}


/**
 * A simple loop that obtains events from the Event Manager and passes them on to various applications and
 * system event handlers before passing them on to FrmDispatchEvent for default processing.
 *
 * @param an event
 * @return True if the event has been handled and should not be passed to a higher level handler
 */
static void SECT2 appEventLoop()
	{
	UInt16		error;
	EventType	event;

	do
		{
		EvtGetEvent( &event, evtWaitForever );

		if( !SysHandleEvent( &event ))
			if( !MenuHandleEvent( 0, &event, &error ))
				if( !appHandleEvent( &event ))
					FrmDispatchEvent( &event );
		}
	while( event.eType != appStopEvent );
	}


static void SECT2 appStartJogDialSupport()
	{
#if 0
	SonySysFtrSysInfoP infoP;

	// get Sony system information
	if (!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32 *)&infoP))
		{
		// disable JogAssist
		if (infoP && (infoP->extn & sonySysFtrSysInfoExtnJogAst)
		    && !FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMaskP, (UInt32*) &maskPP))
			{
			// save the old mask to restore later
			oldmaskP = *maskPP;
			
			// set the JogAssist mask
			*maskPP = mask;

			// set JogAssist mask owner
			if (!FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMOCardNoP, (UInt32*) &ftrCardNoP)
			    && !FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMODbIDP, (UInt32*) &ftrDbIDP))
				{
				// save the old mask owner to restore later
				oldftrCardNo = *ftrCardNoP;
				oldftrDbID = *ftrDbIDP;
				
				// set the mask owner;
				SysCurAppDatabase(ftrCardNoP, ftrDbIDP);
				}
			}	
		}
#endif
	}
	
static void SECT2 appStopJogDialSupport()
	{
#if 0
	SonySysFtrSysInfoP infoP;

	// get Sony system information
	if (!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32 *)&infoP))
		{
		// restore original JogAssist mask
		if(maskPP)	*maskPP = oldmaskP;

		// restore original JogAssist mask owner
		*ftrCardNoP = oldftrCardNo;
		*ftrDbIDP = oldftrDbID;	
		}
#endif
	}


static Err appStartCheckNotify()
   {
	UInt32 romVersion;
	Err err;

	err = FtrGet( sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion );
	if( !err )
      {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase( &cardNo, &dbID );
		if( !err )
         {
			SysNotifyRegister( cardNo, dbID, sysNotifyVolumeMountedEvent, NULL, sysNotifyNormalPriority, NULL );
			SysNotifyRegister( cardNo, dbID, sysNotifyVolumeUnmountedEvent, NULL, sysNotifyNormalPriority, NULL );
			SysNotifyRegister( cardNo, dbID, sysNotifySleepRequestEvent, NULL, sysNotifyNormalPriority, NULL );
         }
      }

	return err;
   }
   

static Err appStopCheckNotify()
   {
	UInt32 romVersion;
	Err err;
	
	err = FtrGet( sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion ); 
	if( !err )
      {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase( &cardNo, &dbID );
		if( !err )
         {
			SysNotifyUnregister( cardNo, dbID, sysNotifyVolumeUnmountedEvent, sysNotifyNormalPriority );
			SysNotifyUnregister( cardNo, dbID, sysNotifyVolumeMountedEvent, sysNotifyNormalPriority );
			SysNotifyUnregister( cardNo, dbID, sysNotifySleepRequestEvent, sysNotifyNormalPriority );
         }
      }
	
	return err;
   }


/**
 * Do the initial setup stuff.
 */
static Err SECT2 appStart()
	{
	SystemPreferencesType	sysPrefs;
   //Err                     err;

   // setup the memory card cache
	cardSet.init();

   deferSleep = false;

   InitializeResizeSupport( resizeIndex );
   LoadResizePrefs( ( UInt32 )appCreatorID, PREF_ID_RESIZE );
	
   // setup our own color management settings
	SetupColorSupport();
   
   // get the device's date preferences
	PrefGetPreferences( &sysPrefs );
	LongDateFormat = (DateFormatType)PrefGetPreference( prefLongDateFormat );
	ShortDateFormat = (DateFormatType)PrefGetPreference( prefDateFormat );
	
   // read the application preferences
	UInt16 prefsSize = sizeof( PrefType );
   UInt16 prefVersion = PrefGetAppPreferences( appCreatorID, appPrefID, &prefs, &prefsSize, true );
   // if the preferences are an older version or do not exist at all, reset em all
   if( prefVersion != appPrefVersionNum )
		{
      prefs.lastForm = MainViewForm;
      prefs.lastCard = 0;
      prefs.memoryView = viewBar;
      prefs.batteryView = viewBar;
      prefs.sendMenu = 0;
      prefs.reserved2 = 0;	

		prefs.list.typeColors.count = 5;
		prefs.list.typeColors.types[0] = 'appl';
		prefs.list.typeColors.colors[0] = purple;
		prefs.list.typeColors.types[1] = 'panl';
		prefs.list.typeColors.colors[1] = blue;
		prefs.list.typeColors.types[2] = 'DATA';
		prefs.list.typeColors.colors[2] = green;
		prefs.list.typeColors.types[3] = 'data';
		prefs.list.typeColors.colors[3] = green;
		prefs.list.typeColors.types[4] = 'libr';
		prefs.list.typeColors.colors[4] = olive;

      prefs.list.sortOrder = sortNameAZ;
      prefs.list.column = colSize;
      prefs.list.hideROM = true;
      prefs.list.rowColor = paleBlue;
      prefs.list.selectAll = selectAllFiles;
      prefs.list.foldersFirst = 0;
      prefs.list.folderSelect = folderSelectName;
		
      prefs.filter.string[0] = 0;
      prefs.filter.comparator = 0;
      prefs.filter.criteria = 0;
      prefs.filter.notbox = false;
      prefs.filter.reserved2 = 0;
      prefs.filter.attr = 0;
      }

   if( prefs.list.folderSelect != folderSelectName && prefs.list.folderSelect != folderSelectIcon )
      prefs.list.folderSelect = folderSelectName;

	appStartJogDialSupport();
	appStartCheckNotify(); 		// not fatal error if not avalaible

	return errNone;
	}


/**
 * Do any shutdown stuff.
 */
static void SECT2 appStop()
	{
	prefs.lastCard = cardSet.getCurrentCardIndex();
   appStopCheckNotify();
   freeTree();
   PrefSetAppPreferences( appCreatorID, appPrefID, appPrefVersionNum, &prefs, sizeof(prefs), true );
	cardSet.free();

   SaveResizePrefs( (UInt32)appCreatorID, PREF_ID_RESIZE, PREF_ID_RESIZE );
   TerminateResizeSupport();

	appStopJogDialSupport();
	FrmCloseAllForms();
	}


static Err handleCmdNotify( UInt16 launchFlags, SysNotifyParamType *pData )
   {
   // first see if the app is actually running so that we don't try to handle
   //  an event if we're not running. this somehow happens (i have reports)
	//  so we should just bail if we get it.
   Boolean appIsActive = launchFlags & sysAppLaunchFlagSubCall; 
   if( !appIsActive )
      {
		//showMessage( "Error: FileZ received a notification while not running." );
      return errNone;
      }
      
   // see if it's a display change event, and if it's handled, then just exit
   if( HandleResizeNotification( pData->notifyType ))
      return errNone;

   // so it wasn't a display-related notification
   switch( pData->notifyType )
      {
      case sysNotifyVolumeMountedEvent:
      case sysNotifyVolumeUnmountedEvent:
         cardSet.init();											// setup all the memory cards
         FrmGotoForm( MainViewForm );
         break;
         
      case sysNotifySleepRequestEvent:
         if( deferSleep )
            {
            SleepEventParamType *se = (SleepEventParamType*)pData->notifyDetailsP;
            se->deferSleep += 1;
            }
         break;
      }

   return errNone;
   }


static Err handleCmdNormalLaunch()
   {
   Err err;
   
   err = appStart();
   if( err )
      return err;
				
   FrmGotoForm( prefs.lastForm );
   appEventLoop();
   appStop();

   return err;
   }


UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags )
	{
	Err err;

	err = RomVersionCompatible( ourMinVersion, launchFlags );
	if( err ) return err;

	switch( cmd )
		{
      case sysAppLaunchCmdNotify:
         err = handleCmdNotify( launchFlags, (SysNotifyParamType*)cmdPBP );
         break;

      case sysAppLaunchCmdNormalLaunch:
         err = handleCmdNormalLaunch();
			break;
		}
	
	return err;
	}
