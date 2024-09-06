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
 * Display a form where the user can select a volume.
 *
 * Created on 12/21/01 by Tom Bulatewicz
 */
  
#include <PalmOS.h>					// all the system toolbox headers
#include <FeatureMgr.h>				// needed to get the ROM version
#include <ExgMgr.h>
#include <DLServer.h>				// to get hotsync username
#include <VFSMgr.h>
#include "resize.h"

#include "Main.h"
#include "Resource.h"				// application resource defines
#include "Stuph.h"
#include "UI.h"
#include "SetAttributes.h"
#include "TreeView.hpp"
#include "Item.hpp"
#include "ItemSet.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "BusyIndicator.hpp"
#include "TreeViewForm.h"


static Boolean SECT8 handleFormOpen( EventPtr event )
	{
   FormPtr     frm = FrmGetActiveForm();
   
   ControlPtr  ctl = GetObjectPtr<ControlType>( SetAttributesSetPushButton );
   CtlSetValue( ctl, true );
   
	FrmDrawForm( frm );
   return true;
	}


static Boolean SECT8 handleCancelButton( EventPtr event )
   {
   FrmGotoForm( TreeViewForm );	
   FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
   return true;
   }
   

static Boolean SECT8 handleOKButton( EventPtr event )
   {
   UInt16      attr = 0;
   ControlPtr  ctl = 0;
   
   ctl = GetObjectPtr<ControlType>( SetAttributesReadOnlyCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrReadOnly;
	
   ctl = GetObjectPtr<ControlType>( SetAttributesOKInstallCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrOKToInstallNewer;
	
   ctl = GetObjectPtr<ControlType>( SetAttributesBackupCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrBackup;

   ctl = GetObjectPtr<ControlType>( SetAttributesResetAfterCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrResetAfterInstall;

   ctl = GetObjectPtr<ControlType>( SetAttributesCopyProtectCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrCopyPrevention;
		
   ctl = GetObjectPtr<ControlType>( SetAttributesHiddenCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrHidden;

   ctl = GetObjectPtr<ControlType>( SetAttributesAppInfoCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrAppInfoDirty;

   ctl = GetObjectPtr<ControlType>( SetAttributesLaunchableCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrLaunchableData;

   ctl = GetObjectPtr<ControlType>( SetAttributesBundleCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrBundle;
   
   ctl = GetObjectPtr<ControlType>( SetAttributesRecyclableCheckbox );
   if( CtlGetValue( ctl ) ) attr |= dmHdrAttrRecyclable;

   ctl = GetObjectPtr<ControlType>( SetAttributesSetPushButton );
   Boolean set = CtlGetValue( ctl );

   tree->setAllAttributes( attr, set );

   FrmGotoForm( TreeViewForm );	
   FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );

   return true;
   }


static Boolean SECT8 handleSelectEvent( EventPtr event )
   {
   Boolean handled = false;
   
   switch( event->data.ctlSelect.controlID )
      {
      case SetAttributesOKButton:
         handled = handleOKButton( event );
         break;

      case SetAttributesCancelButton:
         handled = handleCancelButton( event );
         break;
      }
   
   return handled;
   }


Boolean SetAttributesHandleEvent( EventPtr event )
   {
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleFormOpen( event );
			break;

      case frmCloseEvent:
         break;

		case frmUpdateEvent:
			break;

		case ctlSelectEvent:
         handled = handleSelectEvent( event );
         break;
    default:
      break;
		}
	
	return handled;
   }
