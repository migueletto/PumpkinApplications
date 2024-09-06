/*
 *	file:		wav_1.c
 *	project:	WavPlayer (GentleMan Plugin)
 *	content:	main app functions
 *	updated:	17. Sep. 2001
 * 
 *	copyright: Collin R. Mulliner <palm@mulliner.org>
 * web: www.mulliner.org/palm
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

#include "wav.h"

// prototypes
Err playWavFile(unsigned char *data);
Err readWavData(void *dataP, UInt32 offset, UInt32 *sizeP, void *userDataP);
		
UInt32 PilotMain (UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	int error;
	FormPtr form;
	
	
	if (cmd == GMPluginAppCmdShowAbout) {
		// load form
		form = FrmInitForm(formID_about);
		// show form
		FrmDoDialog(form);
		// delete form
		FrmDeleteForm(form);
	}
	else if (cmd == GMPluginAppCmdDoAction) {
		return(playWavFile((unsigned char *) cmdPBP));
	}
	
	return(errNone);
}       

/*
 *	play the wav file
 */
Err playWavFile(unsigned char *data)
{
	FileRef fp;
	AudioFormatType auData;
	GMPluginDoActionType actionData;
	UInt32 version, i;
	char *filename;
	FormPtr oldForm, form;
	Err retVal = errNone;
		
	
	oldForm = FrmGetActiveForm();
	form = FrmInitForm(formID_play);
	FrmSetActiveForm(form);
	FrmDrawForm(form);
	
	filename = (char *) (data+sizeof(GMPluginDoActionType));
	i = StrLen(filename);
	while (filename[i] != '/') {
		i--;
	}
	i++;
	filename = &filename[i];
	
	WinDrawChars(filename, StrLen(filename), (156-FntLineWidth(filename, StrLen(filename)))/2, 25);
	
	filename = (char *) (data+sizeof(GMPluginDoActionType));
	
	MemMove(&actionData, data, sizeof(GMPluginDoActionType));

	if (VFSFileOpen(actionData.volRefNum, filename, vfsModeRead, &fp) == errNone) {
		
		// check for HandEra AudioLib
		if (_TRGAudioFeaturePresent(&version)) {
		
			AudioOpenWave(&auData, readWavData, (void*) &fp);

			AudioPlayData();
			
			AudioCloseWave();		
		}

		VFSFileClose(fp);
	}
	else {
		retVal = 1;
	}
	
	FrmEraseForm(form);
	FrmDeleteForm(form);
	FrmSetActiveForm(oldForm);
	
	return(retVal);
}

/*
 *	read wav data - callback function
 */
Err readWavData(void *dataP, UInt32 offset, UInt32 *sizeP, void *userDataP)
{
	Err ret_val = 0;
	
	
	CALLBACK_PROLOGUE
	
	if ((ret_val = VFSFileSeek(*((FileRef*) userDataP), vfsOriginBeginning, offset)) != errNone) {
		CALLBACK_EPILOGUE
		return(ret_val);
	}
	
	ret_val = VFSFileRead(*((FileRef*) userDataP), *sizeP, dataP, sizeP);
			
	CALLBACK_EPILOGUE
	
	return(ret_val);
}
