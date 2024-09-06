/*
 *	file:		fm_11-prefs.c
 *	project:	GentleMan
 *	content:	prefs
 *	updated:	Sep. 16. 2001
 *
 * copyright: Collin R. Mulliner <palm@mulliner.org>
 * web: www.mulliner.org/palm/
 *
 */

/*
 *  This file is part of GentleMan.
 *
 *  GentleMan is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GentleMan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GentleMan; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fm.h"

/*
 *	setup the lists
 */
void setupPrefsLists(global_data_type *glb)
{
	// -= sort, show, show : defaults VOL
	LstSetSelection(GetObjectPtr(sortF_prefs_lst), glb->prefs.defaultVfsSort);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(sortF_prefs_ptrg), LstGetSelectionText(GetObjectPtr(sortF_prefs_lst), glb->prefs.defaultVfsSort));
	
	LstSetSelection(GetObjectPtr(showF1_prefs_lst), glb->prefs.defaultVfsShow1);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(showF1_prefs_ptrg), LstGetSelectionText(GetObjectPtr(showF1_prefs_lst), glb->prefs.defaultVfsShow1));
	
	LstSetSelection(GetObjectPtr(showF2_prefs_lst), glb->prefs.defaultVfsShow2);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(showF2_prefs_ptrg), LstGetSelectionText(GetObjectPtr(showF2_prefs_lst), glb->prefs.defaultVfsShow2));
	
	// -= sort, show, show : defaults CARD
	LstSetSelection(GetObjectPtr(sortD_prefs_lst), glb->prefs.defaultCardSort);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(sortD_prefs_ptrg), LstGetSelectionText(GetObjectPtr(sortD_prefs_lst), glb->prefs.defaultCardSort));
	
	LstSetSelection(GetObjectPtr(showD1_prefs_lst), glb->prefs.defaultCardShow1);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(showD1_prefs_ptrg), LstGetSelectionText(GetObjectPtr(showD1_prefs_lst), glb->prefs.defaultCardShow1));
	
	LstSetSelection(GetObjectPtr(showD2_prefs_lst), glb->prefs.defaultCardShow2);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(showD2_prefs_ptrg), LstGetSelectionText(GetObjectPtr(showD2_prefs_lst), glb->prefs.defaultCardShow2));
	
	// -= double-tap VOL
	LstSetSelection(GetObjectPtr(dtapF_prefs_lst), glb->prefs.dTapVfs);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(dtapF_prefs_ptrg), LstGetSelectionText(GetObjectPtr(dtapF_prefs_lst), glb->prefs.dTapVfs));
	
	// -= double-tap CARD
	LstSetSelection(GetObjectPtr(dtapD_prefs_lst), glb->prefs.dTapCard);
	// set popuptrigger text
	CtlSetLabel(GetObjectPtr(dtapD_prefs_ptrg), LstGetSelectionText(GetObjectPtr(dtapD_prefs_lst), glb->prefs.dTapCard));
}

/*
 *	get the list values
 */
void getPrefsLists(global_data_type *glb)
{
	glb->prefs.defaultVfsSort = LstGetSelection(GetObjectPtr(sortF_prefs_lst));
	glb->prefs.defaultVfsShow1 = LstGetSelection(GetObjectPtr(showF1_prefs_lst));
	glb->prefs.defaultVfsShow2 = LstGetSelection(GetObjectPtr(showF2_prefs_lst));
	
	glb->prefs.defaultCardSort = LstGetSelection(GetObjectPtr(sortD_prefs_lst));
	glb->prefs.defaultCardShow1 = LstGetSelection(GetObjectPtr(showD1_prefs_lst));
	glb->prefs.defaultCardShow2 = LstGetSelection(GetObjectPtr(showD2_prefs_lst));
	
	glb->prefs.dTapVfs = LstGetSelection(GetObjectPtr(dtapF_prefs_lst));
	glb->prefs.dTapCard = LstGetSelection(GetObjectPtr(dtapD_prefs_lst));
}

/*
 *	save/load (init prefs - default settings)
 */
void loadSavePrefs(prefsType *prefs, Boolean save)
{
	UInt16 prefsSize;
	
	if (save) {
		PrefSetAppPreferences(CRID, 0, 0, prefs, sizeof(prefsType), true);
	}
	else {
		prefsSize = sizeof(prefsType);
		
		if (PrefGetAppPreferences(CRID, 0, prefs, &prefsSize, true) == noPreferenceFound || prefsSize != sizeof(prefsType)) {
			prefs->defaultCardSort = cardSortNameAscending;
			prefs->defaultCardShow1 = cardShowTypeID;
			prefs->defaultCardShow2 = cardShowCrid;
	
			prefs->defaultVfsSort = vfsSortNameAscending;
			prefs->defaultVfsShow1 = vfsShowAttr;
			prefs->defaultVfsShow2 = vfsShowSize;
			
			prefs->dTapVfs = doubleTapDetails;
			prefs->dTapCard = doubleTapDetailsGeneral;	
		}
	}
}
