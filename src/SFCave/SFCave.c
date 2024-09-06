/*

This software is open source under "The modified BSD license."

Copyright (c) 1998,2002 Sunflat

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 
* The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

/******************************************************************************
 * SFCave for Palm
 * Ver.0.04
 *****************************************************************************/
 

#include <PalmOS.h>
#include "SFCaveRsc.h"
#include "debug.h"


/***********************************************************************
 * Misc functions
 ***********************************************************************/

static void DrawStrCenter(char const *s,int x,int y)
{
	Int16 l=StrLen(s);
	WinDrawChars(s,l,x-FntCharsWidth(s,l)/2,y);
}

static void * GetObjectPtr(FormPtr frmP,UInt16 objectID)
{
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}

static int Rand(void) {
	return SysRandom(0);
}

static long min32(long a,long b) {
	if (a<b) return a; else return b;
}

/*
static long max32(long a,long b) {
	if (a>b) return a; else return b;
}
*/

/***********************************************************************
 * Games
 ***********************************************************************/

// event interval
#define EVENT_INTERVAL 5

// hardware butttons
#define BUTTON_NUM 6
char * gButtonStr[BUTTON_NUM] = {"PageUp","PageDown","DateBook","Address","ToDo","MemoPad"};
UInt16 gButtonCode[BUTTON_NUM] = {pageUpChr,pageDownChr,hard1Chr,hard2Chr,hard3Chr,hard4Chr};
UInt16 gButtonBitMask[BUTTON_NUM] = {keyBitPageUp,keyBitPageDown,keyBitHard1,keyBitHard2,keyBitHard3,keyBitHard4};

// levels
#define LEVEL_NUM 3
char * gLevelStr[LEVEL_NUM] = {"Easy","Normal","Hard"};

// preferences DB
#define DB_VERSION 3

typedef struct 
{
	int dbVersion;
	int level;
	int highScore[LEVEL_NUM];
	int button;
} TStarterPreference;

#define APP_FILE_CREATOR 'SFCv'
#define APP_PREF_ID 0x00
#define APP_PREF_VERSION 0x01

// backup of key parameters
UInt16 gKeyP1,gKeyP2,gKeyP3;
Boolean gKeyP4;


// scene states
typedef enum {SS_NONE,SS_TITLE,SS_GAME,SS_MISS} TSceneState;

TSceneState gSceneState=SS_NONE;
int gSceneCount=0;

// VRAM
WinHandle gOffScreen;

// key state (pushed/released)
Boolean gKeyState=false;

MemHandle gTitleLogoHandle;

// static parameters
#define SCROLL_SPEED 4
#define ME_X 40

#define TITLE_BAR_HEIGHT 12
#define CAVE_Y_MIN (TITLE_BAR_HEIGHT+3)*256L
#define CAVE_Y_MAX 157*256L
#define MAP_SIZE ((160-ME_X)/SCROLL_SPEED-1)

// global variables (head 'g' means 'global')
int gScore=0;
int gHighScore[LEVEL_NUM]={0,0,0};
int gLevel=0,gControlButton=0;
int gPreviousBarPos=0;

// cave's y-pos & velocity-y
// 256 = 1dot
long gCaveY,gCaveVY;

// my y-pos & velocity-y (ribbon)
// 256 = 1dot
long gMeY,gMeVY;

// current map index
int gMapIndex;

// Map structure
typedef struct {
	// ('#' means wall)
	long caveY1,caveY2; // ####y1   y2####  
	long barY1,barY2;
	int barType; // 0:none  /  1: y1###y2  /  2: ###y1  y2###
} TMapElem;

// Map
TMapElem gMap[MAP_SIZE];

//
static void OnTitle(void);
static void OnGame(void);
static void OnMiss(void);

// set scene
static void SetSceneState(TSceneState a)
{
	gSceneState=a;
	gSceneCount=0; 
}

// init game
static void GameInit(void)
{
	gScore=0;
	SetSceneState(SS_TITLE);
}

// scene step (called periodically)
static void SceneStep(void)
{
	gSceneCount++;
	gKeyState=(Boolean)((KeyCurrentState() & gButtonBitMask[gControlButton])!=0);
	
	switch(gSceneState){
	case SS_TITLE:
		OnTitle();
		break;
	case SS_GAME:
		OnGame();
		break;
	case SS_MISS:
		OnMiss();
		break;
  default:
		break;
	}
}

// game scene step
static void OnGame(void)
{
	int i;
	int barHeightMin=0;
	long barHeight=0;
	int barType=0;
	long caveHeight;
	long oldMeY;
	int caveVY_max=0;
	TMapElem *m;
	int caveChangeInterval=0,gravitation=0;
	
	char buff[32];
	WinHandle tmpHandle;
	RectangleType rect;
	CustomPatternType brush;
	

	///	
	
	if (gSceneCount==1) {
		// initialize
		gCaveY=26*256L;gCaveVY=0;
		gMeY=60*256L;gMeVY=-512;
		gMapIndex=0;
		for(i=0;i<MAP_SIZE;i++) {
			m=&(gMap[i]);
			m->caveY1=CAVE_Y_MIN; m->caveY2=CAVE_Y_MAX;
			m->barY1=0; m->barY2=0;
			m->barType=0;
		}
		
		gScore=0;
		gPreviousBarPos=0;
	}

	// increment score
	gScore++;

	// cource parameters for each level
	switch(gLevel) {
	default:
		caveHeight=120L*700L*256L/(gScore+700L)+2*256L;
		caveVY_max=512;
		caveChangeInterval=8;
		gravitation=128;
		
		barType=0;
		if (gScore-gPreviousBarPos>20 && (Rand()%2)==0) {
			gPreviousBarPos=gScore;
			barType=1;
			barHeight=min32(20*256L,caveHeight*3/4);
			barHeightMin=8*256;
		}
		break;
		
	case 1:
		caveHeight=80L*600L*256L/(gScore+600L)+2*256L;
		caveVY_max=512+256;
		caveChangeInterval=8;
		gravitation=128;
		
		barType=0;
		if (gScore-gPreviousBarPos>20 && (Rand()%2)==0) {
			gPreviousBarPos=gScore;
			barType=2;
			barHeight=min32(20*256L,caveHeight*3/4);
			barHeightMin=4*256;
		}
		break;
		
	case 2:
		caveHeight=45L*400L*256L/(gScore+400L)+2*256L;
		caveVY_max=1024;
		caveChangeInterval=8;
		gravitation=128;
		barType=0;
		break;
	}
	

	// next step
	gMapIndex=(gMapIndex+1)%MAP_SIZE;
	m=&(gMap[gMapIndex]);

	oldMeY=gMeY;
	gMeVY+=gKeyState?-gravitation:gravitation;
	gMeY+=gMeVY;
	
	// miss?
	if ((gMeY<m->caveY1 || m->caveY2<gMeY)
		|| (m->barType==1 &&  m->barY1<gMeY && gMeY<m->barY2) 
		|| (m->barType==2 &&  (gMeY<m->barY1 || m->barY2<gMeY))
	){
		SetSceneState(SS_MISS); // bom!
	}

	// make map
	gCaveY+=gCaveVY;
	if (gCaveY<CAVE_Y_MIN){
		gCaveY=CAVE_Y_MIN;
		gCaveVY=Rand()%caveVY_max;
	}
	if (gCaveY>CAVE_Y_MAX-caveHeight){
		gCaveY=CAVE_Y_MAX-caveHeight;
		gCaveVY=-(Rand()%caveVY_max);
	}
	if (0==(Rand()%caveChangeInterval)) {
		gCaveVY=Rand()%(caveVY_max*2)-caveVY_max;
	}
	
	m->caveY1=gCaveY; m->caveY2=gCaveY+caveHeight;
	m->barY1=0; m->barY2=0;
	m->barType=0;
	
	if (barType==1){
		long bh=barHeight;
		if (bh>caveHeight-barHeightMin) bh=caveHeight-barHeightMin;
		if (barHeightMin<bh) {
			m->barY1=gCaveY+Rand()%(caveHeight-bh);
			m->barY2=m->barY1+bh;
			m->barType=barType;
		}
	}
	if (barType==2){
		long bh=caveHeight-barHeight;
		if (barHeightMin<bh && bh<caveHeight) {
			m->barY1=gCaveY+Rand()%(caveHeight-bh);
			m->barY2=m->barY1+bh;
			m->barType=barType;
		}
	}
	
	// draw ////////////
	
	tmpHandle = WinSetDrawWindow(gOffScreen);
	
	for(i=0;i<8;i++) brush[i]=(unsigned char)((Rand()|Rand()|Rand())&255);
	WinSetPattern(&brush);
	
	if (gSceneCount==1) {
		// initialize
		WinEraseWindow();
	}
		
	// scroll
	rect.topLeft.x = SCROLL_SPEED; rect.topLeft.y = TITLE_BAR_HEIGHT;
	rect.extent.x = 160-SCROLL_SPEED ; rect.extent.y = 160;
	WinCopyRectangle(gOffScreen,gOffScreen,&rect,0,TITLE_BAR_HEIGHT,winPaint);
	
	rect.topLeft.x = 160-SCROLL_SPEED; rect.topLeft.y = TITLE_BAR_HEIGHT;
	rect.extent.x = SCROLL_SPEED ; rect.extent.y = 160;
	WinEraseRectangle(&rect,0);

	// draw next wall
	rect.topLeft.y = TITLE_BAR_HEIGHT;
	rect.extent.y = (int)(m->caveY1/256L - rect.topLeft.y);
	WinFillRectangle(&rect,0);
	rect.topLeft.y = (int)(m->caveY2/256L);
	rect.extent.y = 160 - rect.topLeft.y;
	WinFillRectangle(&rect,0);
	
	// draw bar
	if (m->barType==1){
		rect.topLeft.x=160-SCROLL_SPEED*2;rect.extent.x=SCROLL_SPEED*2;
		rect.topLeft.y = (int)(m->barY1/256L);
		rect.extent.y = (int)(m->barY2/256L) - rect.topLeft.y;
		
		for(i=0;i<8;i++) brush[i]|=(unsigned char)((Rand())&255);
		WinSetPattern(&brush);
		
		WinFillRectangle(&rect,0);
	}
	if (m->barType==2){
		rect.topLeft.x=160-SCROLL_SPEED*2;rect.extent.x=SCROLL_SPEED*2;
		rect.topLeft.y = TITLE_BAR_HEIGHT;
		rect.extent.y = (int)(m->barY1/256L) - rect.topLeft.y;
		WinFillRectangle(&rect,0);
		rect.topLeft.y = (int)(m->barY2/256L);
		rect.extent.y = 160 - rect.topLeft.y;
		WinFillRectangle(&rect,0);
	}
	
	// draw ribbon
	for (i=-2;i<=2;i++) WinDrawLine(ME_X-SCROLL_SPEED+i,(int)(oldMeY/256L),ME_X+i,(int)(gMeY/256L));
	
	// paint
	rect.topLeft.x=0; rect.topLeft.y=TITLE_BAR_HEIGHT;
	rect.extent.x = 160; rect.extent.y = 160 - TITLE_BAR_HEIGHT;
	WinCopyRectangle(gOffScreen,WinGetDisplayWindow(),&rect,0,TITLE_BAR_HEIGHT,winPaint);
	StrPrintF(buff," %d ",gScore);
	WinSetDrawWindow(WinGetDisplayWindow());
	WinDrawInvertedChars(buff,StrLen(buff),2,148);
	
	WinSetDrawWindow(tmpHandle);
}

// miss scene step
static void OnMiss(void)
{
	int i;
	static Boolean bPushed; // for button release check
	
	char buff[32];
	WinHandle tmpHandle;
	RectangleType rect;

	//
		
	tmpHandle = WinSetDrawWindow(gOffScreen);
	
	if (gSceneCount==1) {
		// initialize
		SndPlaySystemSound(sndWarning); // play sound
		bPushed=true;
		
		if (gMeY<0) gMeY=0;
		if (gMeY>158*256L) gMeY=158*256L;
	}
	
	bPushed&=gKeyState;
	
	if (gSceneCount<30) {
		// bom!
		for(i=0;i<5;i++){
			WinDrawLine(ME_X,(int)(gMeY/256L),ME_X+Rand()%(gSceneCount*8)-gSceneCount*4
				,(int)(gMeY/256L)+Rand()%(gSceneCount*8)-gSceneCount*4);
			WinEraseLine(ME_X,(int)(gMeY/256L),ME_X+Rand()%(gSceneCount*4)-gSceneCount*2
				,(int)(gMeY/256L)+Rand()%(gSceneCount*4)-gSceneCount*2);
		}
	}
	if (gSceneCount==30) {
		// draw gameover message
		char buff[32];
		RectangleType rect;
		rect.topLeft.x=40;rect.topLeft.y=40;rect.extent.x=160-80;rect.extent.y=160-80;
		WinEraseRectangle(&rect,0);
		WinDrawRectangleFrame(popupFrame,&rect);
		
		FntSetFont(largeFont);
		WinDrawInvertedChars(" GameOver ",10,48,50);
		FntSetFont(boldFont);
		StrPrintF(buff," Score : %d ",gScore);
		DrawStrCenter(buff,80,85);
		if (gHighScore[gLevel]<gScore) DrawStrCenter(" High Score !! ",80,105);
		
	}
	
	if ((gSceneCount>35 && !bPushed && gKeyState) || gSceneCount==100) {
		// back to title
		if (gHighScore[gLevel]<gScore) gHighScore[gLevel]=gScore;
		SetSceneState(SS_TITLE);
	}
	
	// paint
	rect.topLeft.x=0; rect.topLeft.y=TITLE_BAR_HEIGHT;
	rect.extent.x = 160; rect.extent.y = 160 - TITLE_BAR_HEIGHT;
	WinCopyRectangle(gOffScreen,WinGetDisplayWindow(),&rect,0,TITLE_BAR_HEIGHT,winPaint);
	StrPrintF(buff," %d ",gScore);
	WinSetDrawWindow(WinGetDisplayWindow());
	WinDrawInvertedChars(buff,StrLen(buff),2,148);
	
	WinSetDrawWindow(tmpHandle);
}

// title scene step
static void OnTitle(void)
{
	BitmapPtr bitmap;
	static int logoX;
	static Boolean bPushed; // for button release check
	char buff[64];
	
	WinHandle tmpHandle;
	RectangleType rect;
	
	//
	tmpHandle = WinSetDrawWindow(gOffScreen);
	
	if (gSceneCount==1) { 
		// initalize
		WinEraseWindow();
		logoX=160;
		bPushed=true;
	}
	
	bPushed&=gKeyState;
	
	if (gSceneCount==15) {
		
		// draw title messages
		FntSetFont(boldFont);
		
		StrPrintF(buff,"Level : %s",(gLevel==0?"Easy":gLevel==1?"Normal":gLevel==2?"Hard":"?") );
		DrawStrCenter(buff,80,85);

		StrPrintF(buff,"High Score : %d",gHighScore[gLevel]);
		DrawStrCenter(buff,80,100);
		if (gScore!=0){
			StrPrintF(buff,"Last Score : %d",gScore);
			DrawStrCenter(buff,80,115);
		}
		
		StrPrintF(buff,"push %s button",gButtonStr[gControlButton]);
		DrawStrCenter(buff,80,134);
	}
	
	if (gSceneCount>15 && !bPushed && gKeyState) {
		// game start!
		SndPlaySystemSound(sndClick);
		SetSceneState(SS_GAME);
	}
	
	// draw title
	bitmap = MemHandleLock(gTitleLogoHandle);
	logoX=(logoX-10)*3/4+10;
	WinDrawBitmap(bitmap,logoX,24);
	MemHandleUnlock(gTitleLogoHandle);
	
	// paint
	rect.topLeft.x=0; rect.topLeft.y=TITLE_BAR_HEIGHT;
	rect.extent.x = 160; rect.extent.y = 160 - TITLE_BAR_HEIGHT;
	WinCopyRectangle(gOffScreen,WinGetDisplayWindow(),&rect,0,TITLE_BAR_HEIGHT,winPaint);
	
	WinSetDrawWindow(tmpHandle);
}



/***********************************************************************
 * MainFormInit
 ***********************************************************************/
static void MainFormInit(FormPtr /*frmP*/)
{
	gTitleLogoHandle=DmGetResource('Tbmp',AppTitleBitmap);
	GameInit();
}




/***********************************************************************
 * DoOptionDialog
 ***********************************************************************/
static void DoOptionDialog()
{
	FormPtr frmP;
	
	ListPtr lstLevel,lstButton;
	ControlPtr trgLevel,trgButton;
	UInt16 ret;
	
	MenuEraseStatus(0);	// Clear the menu status from the display.
	
	// setup
	frmP = FrmInitForm (OptionForm);
	
	// level
	lstLevel=GetObjectPtr(frmP, OptionLevelList);
	LstSetListChoices(lstLevel,gLevelStr,LEVEL_NUM);
	LstSetHeight(lstLevel,LEVEL_NUM);
	LstSetSelection(lstLevel,gLevel);
	trgLevel=GetObjectPtr(frmP, OptionLevelPopTrigger);
	CtlSetLabel(trgLevel,gLevelStr[gLevel]);
	
	// button
	lstButton=GetObjectPtr(frmP, OptionButtonList);
	LstSetListChoices(lstButton,gButtonStr,BUTTON_NUM);
	LstSetHeight(lstButton,BUTTON_NUM);
	LstSetSelection(lstButton,gControlButton);
	trgButton=GetObjectPtr(frmP, OptionButtonPopTrigger);
	CtlSetLabel(trgButton,gButtonStr[gControlButton]);
	
	// show dialog
	WinEraseWindow();
	ret=FrmDoDialog(frmP);
	if (ret==OptionOKButton) {
		ControlPtr checkClear;
		int a;
	
		// apply options
		a=LstGetSelection(lstLevel);
		if (a>=0 && a<LEVEL_NUM) gLevel=a;
		a=LstGetSelection(lstButton);
		if (a>=0 && a<BUTTON_NUM) gControlButton=a;
		
		// clear high score?
		checkClear=GetObjectPtr(frmP, OptionClearScoreCheckbox);
		if (CtlGetValue(checkClear)!=0) {
			if (FrmAlert(ClearScoreAlert)==ClearScoreOK) {
				gHighScore[gLevel]=0;
			}
		}
		
		//
		GameInit();
	}
	
	FrmDeleteForm (frmP);
	
	// repaint owner
	frmP = FrmGetActiveForm();
	FrmDrawForm ( frmP);
}

/***********************************************************************
 * MainFormHandleEvent
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
   Boolean handled = false;
   FormPtr frmP;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			MainFormInit( frmP);
			FrmDrawForm ( frmP);
			handled = true;
			break;
			
		case nilEvent:
			SceneStep();
			handled=true;
			break;
					
		case keyDownEvent:
		{
			WChar chr;
			
			chr=eventP->data.keyDown.chr;
			if (EvtKeydownIsVirtual(eventP) && (chr==vchrMenu)) {
				DoOptionDialog();		
			}
			
			handled=true;

			break;

		}
		default:
			break;
		
	}
	
	return handled;
}


/***********************************************************************
 * AppHandleEvent
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
				break;

			}
		return true;
		}
	
	return false;
}


/***********************************************************************
 * AppEventLoop
 ***********************************************************************/
static void AppEventLoop(void)
{
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, EVENT_INTERVAL /*evtWaitForever*/ );
debug(1, "XXX", "got event %d", event.eType);

		// disable Sys/MenuHandleEvent for hardware keys
		if ( event.eType == keyDownEvent && (event.data.keyDown.chr==gButtonCode[gControlButton])) {
			// forward to AppHandleEvent directly
			if (! AppHandleEvent(&event))
				FrmDispatchEvent(&event);
			
		}else {
		
			// normal event handling
			if (! SysHandleEvent(&event))
				if (! MenuHandleEvent(0, &event, &error))
					if (! AppHandleEvent(&event))
						FrmDispatchEvent(&event);
		}

	} while (event.eType != appStopEvent);
}



/***********************************************************************
 *  AppStart
 ***********************************************************************/
static Err AppStart(void)
{
    TStarterPreference prefs;
    UInt16 prefsSize;
	UInt16 err,newKeyP1=255,newKeyP3=255;
	Boolean newKeyP4=false;
	
	// reduce key repeat
	KeyRates(false,&gKeyP1,&gKeyP2,&gKeyP3,&gKeyP4);
	KeyRates(true,&newKeyP1,&newKeyP3,&gKeyP3,&newKeyP4);
	
	// randomize
	SysRandom((long)TimGetSeconds());
	
	// Read the saved preferences / saved-state information.
	MemSet(&prefs,sizeof(prefs),0);
	prefsSize = sizeof(TStarterPreference);
	if (PrefGetAppPreferences(APP_FILE_CREATOR, APP_PREF_ID, &prefs, &prefsSize, true) != 
		noPreferenceFound && prefs.dbVersion<=DB_VERSION)
	{
		int i;
		if (prefs.dbVersion>=3) {
			int a;
			a=prefs.level;
			if (0<=a && a<LEVEL_NUM) gLevel=a;
			a=prefs.button;
			if (0<=a && a<BUTTON_NUM) gControlButton=a;
			for(i=0;i<LEVEL_NUM;++i) {
				gHighScore[i]=prefs.highScore[i];
			}
		}
	}
	
	// create VRAM
	gOffScreen = WinCreateOffscreenWindow(160, 160, screenFormat, &err);

   return errNone;
}


/***********************************************************************
 * AppStop
 ***********************************************************************/
static void AppStop(void)
{
    TStarterPreference prefs;
    
    // restore key rates
	KeyRates(true,&gKeyP1,&gKeyP2,&gKeyP3,&gKeyP4);
 
	// Write the saved preferences / saved-state information.  This data 
	// will be backed up during a HotSync.
	MemSet(&prefs,sizeof(prefs),0);

	{
		int i;
		prefs.dbVersion=DB_VERSION;
		prefs.level=gLevel;
		prefs.button=gControlButton;
		for(i=0;i<LEVEL_NUM;++i) {
			prefs.highScore[i]=gHighScore[i];
		}
	}
			
	PrefSetAppPreferences (APP_FILE_CREATOR, APP_PREF_ID, APP_PREF_VERSION, 
		&prefs, sizeof (prefs), true);
		
	// Close all the open forms.
	FrmCloseAllForms ();
}

/***********************************************************************
 * PilotMain
 ***********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;
	
	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error) 
				return error;
				
			FrmGotoForm(MainForm);
			AppEventLoop();
			AppStop();
			break;

		default:
			break;

	}
	
	return errNone;
}

