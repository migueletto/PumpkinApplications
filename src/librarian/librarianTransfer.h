/***********************************************************************
 *
 * PROJECT:  Librarian
 * FILE:     librarianTransfer.h
 * AUTHOR:   Lonnon R. Foster <author@palmosbible.com>
 *
 * DESCRIPTION:  Exchange manager data transfer header for Librarian.
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

#define libFileExtension        "lib"
#define libCategoryExtension    "lbc"
#define libFileExtensionLength  3
#define libExgFileDesc          "Librarian book record"
#define libExgCategoryDesc      "Librarian category (multiple book records)"

#define libEntireStream        0xffffffff
#define libImportBufferSize    100
#define libMaxBeamDescription  50

/***********************************************************************
 *
 *   Function Prototypes
 *
 ***********************************************************************/

static void    CleanFileName(Char *filename);
       Err     CustomReceiveDialog(DmOpenRef db, ExgAskParamType *askInfo);
       void    LibSendCategory(DmOpenRef db, const Char * const prefix,
                               UInt16 category);
       void    LibSendRecord(DmOpenRef db, const Char * const prefix,
                             Int16 recordNum, UInt16 noDataAlertID);
       Err     LibImportRecord(DmOpenRef db, ExgSocketType *exgSocketP,
                               UInt32 bytes, UInt16 *indexP);
       Err     LibReceiveData(DmOpenRef db, ExgSocketType *exgSocketP);
       void    LibRegisterData(void);
static Err     SendData(ExgSocketType *exgSocket, void *buffer,
                        UInt32 bytes);

