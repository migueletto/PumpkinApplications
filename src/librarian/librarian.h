/***********************************************************************
 *
 * PROJECT:  Librarian
 * FILE:     librarian.h
 * AUTHOR:   Lonnon R. Foster <author@palmosbible.com>
 *
 * DESCRIPTION:  Main header for Librarian.
 *
 * From Palm OS Programming Bible, Second Edition
 * Copyright ©2000, 2002 Lonnon R. Foster.  All rights reserved.
 * This code is not in the public domain, but you are hereby granted
 * permission to use it in any of your own projects, commercial or
 * otherwise.
 *
 * Portions Copyright ©2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 ***********************************************************************/

#include "librarianDB.h"


/***********************************************************************
 *
 *   Utiity Macros
 *
 ***********************************************************************/
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))


/***********************************************************************
 *
 *   Internal Constants
 *
 ***********************************************************************/

#define libDBName     "LibrarianDB-LFlb"
#define libDBType     'DATA'
#define libCreatorID  'LFlb'

#define libVersionNum      0x01
#define libPrefID          0x00
#define libPrefVersionNum  0x01

// List view constants
#define titleColumn        0
#define bookStatusColumn   1
#define printStatusColumn  2
#define formatColumn       3
#define readColumn         4
#define noteColumn         5

#define ellipsisString     "..."
#define ellipsisLength     3
#define spaceBetweenNames  3
#define spaceBeforeStatus  3

// Edit view column constants
#define labelColumn  0
#define dataColumn   1

// Edit view field index constants
#define editFirstFieldIndex  0
#define editLastFieldIndex   7

// Edit view font constants
#define libEditLabelFont     stdFont
#define libEditBlankFont     stdFont

#define spaceBeforeData  2

#define maxLabelColumnWidth   80 

// Preferences dialog push button group
#define PrefsShowInListGroup  1

// Book status characters
#define libHaveStatusChr          'G'
#define libWantStatusChr          'W'
#define libOnOrderStatusChr       'O'
#define libLoanedStatusChr        'L'

#define libInPrintStatusChr       '+'
#define libOutOfPrintStatusChr    '-'
#define libNotPublishedStatusChr  '!'

#define libHardcoverStatusChr     'H'
#define libPaperbackStatusChr     'P'
#define libTradePaperStatusChr    'T'
#define libOtherStatusChr         '?'

#define libReadStatusChr          'R'
#define libUnreadStatusChr        'U'

#define libWidestBookStatusChr    libWantStatusChr
#define libWidestPrintStatusChr   libInPrintStatusChr
#define libWidestFormatStatusChr  libHardcoverStatusChr
#define libWidestReadUnreadChr    libUnreadStatusChr

// Note view menu resource constants for Palm OS 2.0
#define noteTopOfPageCmdV20       10200
#define noteBottomOfPageCmdV20    10201
#define notePhoneLookupCmdV20     10202

// Number of record view lines to store
#define recordFormLinesMax          55
#define recordFormBlankLine         0xffff
#define recordFormBookStatusLine    0xfffe
#define recordFormPrintStatusLine   0xfffd
#define recordFormFormatStatusLine  0xfffc
#define recordFormReadStatusLine    0xfffb

// Duplicate record indicator
#define maxDupIndicatorString  20

// Untitled beam description string
#define maxUntitledBeamString   20

/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/

// Structure containing information abuot how to draw the Record view
typedef struct {
    UInt16  fieldNum;
    UInt16  length;
    UInt16  offset;
    UInt16  x;
} RecordFormLineType;


/***********************************************************************
 *
 *   Function Prototypes
 *
 ***********************************************************************/
// General functions
static void    ChangeCategory (UInt16 category);
static Boolean CreateNote(void);
static void    DeleteRecord (Boolean archive);
static void    DirtyRecord (UInt16 index);
static UInt16  DuplicateCurrentRecord(UInt16 *numCharsToHighlight,
                                      Boolean deleteCurrentRecord);
static Boolean GetAuthorString(LibDBRecordType *record, char **str,
                               Int16 *length, Int16 *width, Boolean firstLast,
                               char **unnamed);
static FieldType* GetFocusFieldPtr(void);
static MemPtr  GetObjectPtr(UInt16 objectID);
static void    GetTitleString(LibDBRecordType *record, char **str,
                              Int16 *length, Int16 *width, char **unnamed);
static void    GoToItem (GoToParamsPtr goToParams, Boolean launchingApp);
static Boolean HandleCommonMenus(UInt16 menuID);
static Err     RomVersionCompatible(UInt32 requiredVersion,
                                    UInt16 launchFlags);
static Boolean SeekRecord(UInt16 *index, Int16 offset, Int16 direction);
static FontID  SelectFont(FontID curFont);

// About box
static Boolean AboutFormHandleEvent(EventType *event);
static void    AboutFormInit(FormType *form);

// Delete dialog
static Boolean DeleteFormHandleEvent(EventType *event);
static void    DeleteFormInit(FormType *form);

// Details dialog
static Boolean DetailsDeleteRecord(void);
static Boolean DetailsFormHandleEvent(EventType *event);
static void    DetailsFormInit(UInt16 *category);
static UInt16  DetailsFormSave(UInt16 category, Boolean categoryEdited);
static Boolean DetailsSelectCategory(UInt16 *category);

// Edit view
static Boolean EditFormDoCommand(UInt16 command);
static UInt16  EditFormGetFieldHeight (TableType *table, UInt16 fieldIndex,
                                       UInt16 columnWidth, UInt16 maxHeight,
                                       LibDBRecordType *record,
                                       FontID *fontID);
static UInt16  EditFormGetLabelWidth();
static Err     EditFormGetRecordField(MemPtr table, Int16 row, Int16 column, 
                                      Boolean editing, MemHandle *textH,
                                      Int16 *textOffset, Int16 *textAllocSize,
                                      FieldType *fldText);
static Boolean EditFormHandleEvent(EventType *event);
static void    EditFormHandleSelectField(Int16 row, const UInt8 column);
static void    EditFormInit(FormType *form);
static void    EditFormLoadTable (void);
static void    EditFormNewRecord();
static void    EditFormNextField (WinDirectionType direction);
static void    EditFormResizeData(EventType *event);
static void    EditFormRestoreEditState();
static void    EditFormSaveRecord(void);
static Boolean EditFormSaveRecordField(MemPtr table, Int16 row,
                                       Int16 column);
static void    EditFormScroll(WinDirectionType direction);
static void    EditFormSelectCategory(void);
static Boolean EditFormUpdateDisplay(UInt16 updateCode);
static void    EditFormUpdateScrollers(FormType *form, UInt16 bottomFieldIndex,
                                       Boolean lastItemClipped);
static void    EditInitTableRow(TableType *table, UInt16 row,
                                UInt16 fieldIndex, short rowHeight,
                                FontID fontID, LibDBRecordType *record,
                                LibAppInfoType *appInfo);
static void    EditSetGraffitiMode(FieldType *field);

// Find dialog
static Boolean FindFormDoCommand(UInt16 command);
static Boolean FindFormHandleEvent(EventType *eventP);
static void    FindFormInit(FormType *frmP);

// List view
static void    DrawRecordName(LibDBRecordType *record, RectanglePtr bounds,
                              UInt8 showInList, char **noAuthor,
                              char **noTitle);
static Boolean ListFormDoCommand(UInt16 command);
static void    ListFormDrawRecord(MemPtr table, Int16 row, Int16 column, 
                                  RectangleType *bounds);
static Boolean ListFormHandleEvent(EventType *event);
static void    ListFormInit(FormType *form);
static void    ListFormItemSelected(EventType *event);
static void    ListFormLoadTable(void);
static void    ListFormLookup(EventType *event);
static void    ListFormNextCategory(void);
static UInt16  ListFormNumberOfRows(TableType *table);
static void    ListFormScroll(WinDirectionType direction, UInt16 units,
                              Boolean byLine);
static UInt16  ListFormSelectCategory(void);
static void    ListFormSelectFromList(TableType *table, UInt16 row,
                                      UInt16 column);
static void    ListFormSelectRecord(UInt16 recordNum);
static void    ListFormUpdateDisplay(UInt16 updateCode);
static void    ListFormUpdateScrollButtons(void);

// Note view
static void    DeleteNote(void);
static Boolean NoteViewDeleteNote(void);
static Boolean NoteViewDoCommand(UInt16 command);
static void    NoteViewDrawTitle(FormType *form);
static Boolean NoteViewHandleEvent(EventType *event);
static void    NoteViewInit(FormType *form);
static void    NoteViewLoadRecord(void);
static void    NoteViewPageScroll(WinDirectionType direction);
static void    NoteViewSave(void);
static void    NoteViewScroll(Int16 linesToScroll);
static void    NoteViewUpdateScrollBar(void);

// Preferences dialog
static Boolean PrefsFormHandleEvent(EventType *event);
static void    PrefsFormInit(FormType *form);
static void    PrefsFormSave(FormType *form);

// Record view
static void    RecordFormAddField(UInt16 fieldNum, UInt16 *width,
                                  UInt16 maxWidth);
static void    RecordFormAddSpaceForText(char *string, UInt16 *width);
static void    RecordFormCleanup(void);
static Boolean RecordFormDoCommand(UInt16 command);
static void    RecordFormDrawSelectedText(UInt16 currentField,
                                          UInt16 selectPos, UInt16 selectLen,
                                          UInt16 textY);
static void    RecordFormErase();
static Boolean RecordFormHandleEvent(EventType *eventP);
static Boolean RecordFormHandlePen(EventType *event);
static void    RecordFormInit(FormType *frmP);
static void    RecordFormMakeVisible(UInt16 selectFieldNum, UInt16 selectPos,
                                     UInt16 selectLen);
static void    RecordFormNewLine(UInt16 *width);
static void    RecordFormScroll(WinDirectionType direction);
static UInt16  RecordFormScrollOnePage(UInt16 newTopRecordFormLine, 
                                       WinDirectionType direction);
static void    RecordFormUpdate();

// Application backbone
static Boolean AppHandleEvent(EventType *event);
static void    AppEventLoop(void);
UInt32         PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
static Err     AppStart(void);
static void    AppStop(void);
