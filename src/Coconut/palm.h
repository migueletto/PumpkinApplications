#ifndef _PALM_H
#define _PALM_H

#define AppID	'CoCn'

#define ProgDir	"/PALM/Programs"
#define AppDir	"/PALM/Programs/Coconut"

typedef struct {
  UInt8 machine;
  UInt8 ramsize;
} AppPrefs;

Boolean ApplicationHandleEvent(EventPtr);
Boolean MainFormHandleEvent(EventPtr);
Boolean EmuFormHandleEvent(EventPtr);
Boolean ListFormHandleEvent(EventPtr);
Boolean ConfigFormHandleEvent(EventPtr);
Boolean AboutFormHandleEvent(EventPtr);

Err AppInit(void *);
void AppFinish(void);
void SetEventHandler(FormPtr, Int16);
void EventLoop(void);
void InitPrefs(AppPrefs *prefs);
void SavePrefs(void);
AppPrefs *LoadPrefs(void);
AppPrefs *GetPrefs(void);
char *GetAppVersion(void);
char *GetRomVersion(void);
Int16 GetRomVersionNumber(void);

UInt8 cas_input(void);
void cas_validate(void);
UInt32 cas_buffer(UInt32 n, UInt8 *buf);
void snd_output(UInt8 b);
void snd_settone(UInt32 freq, UInt32 amp);
void snd_setnoise(UInt32 freq, UInt32 amp);
void snd_calcnoise(void);

Err EmuSelect(void);
Err EmuStart(void);
Err EmuInit(void);
Err EmuLoad(void);
void EmuReset(void);

#endif
