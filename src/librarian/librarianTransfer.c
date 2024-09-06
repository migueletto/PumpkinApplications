/***********************************************************************
 *
 * PROJECT:  Librarian
 * FILE:     librarianTransfer.c
 * AUTHOR:   Lonnon R. Foster <author@palmosbible.com>
 *
 * DESCRIPTION:  Exchange manager data transfer routines for Librarian.
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

#include <PalmOS.h>
//#include <CharAttr.h>
#include "librarian.h"
#include "librarianTransfer.h"
#include "librarianRsc.h"


/***********************************************************************
 *
 * FUNCTION:    CleanFileName
 *
 * DESCRIPTION: Removes colons (:) after the first and all periods (.)
 *              except the last from a string to clean it up for transfer
 *              with the exchange manager.
 *
 * PARAMETERS: string <-> pointer to null-terminated string
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void CleanFileName(Char *filename)
{
    Char    *maybeLastPeriod;
    Char    *lastPeriod;
    Char    *foundColon;
    UInt32  periodSize = TxtCharSize(chrFullStop);
    UInt32  colonSize = TxtCharSize(chrColon);


    // Don't process empty or NULL strings.
    if (filename == NULL || *filename == 0)
        return;

    // Remove all periods but the last.
    maybeLastPeriod = StrChr(filename, chrFullStop);
    while (0 != (lastPeriod = StrChr(maybeLastPeriod + periodSize, chrFullStop)))
    {
        StrCopy(maybeLastPeriod, maybeLastPeriod + periodSize);
        maybeLastPeriod = lastPeriod - periodSize;
    }

    foundColon = StrChr(filename, chrColon);
    while (0 != (foundColon = StrChr(foundColon + colonSize, chrColon)))
    {
        StrCopy(foundColon, foundColon + colonSize);
    }
}


/***********************************************************************
 *
 * FUNCTION:    CustomReceiveDialog
 *
 * DESCRIPTION: Presents the user with a custom dialog to prompt the
 *              user for a category in which to put incoming records.
 *              The ExgDoDialog function is only available on Palm OS
 *              3.5 and later, so this function does nothing on earlier
 *              versions of the operating system.  CustomReceiveDialog
 *              shoves the selected category into the exchange socket's
 *              appData field for later use when handling the
 *              sysAppLaunchCmdReceiveData launch code.
 *
 * PARAMETERS:  db - open database that holds category information
 *    askInfo - structure passed from sysAppLaunchCmdAskUser
 *                        launch code
 *
 * RETURNED:    0 if successful; otherwise, returns an error code
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
Err CustomReceiveDialog(DmOpenRef db, ExgAskParamType *askInfo)
{
    ExgDialogInfoType  exgInfo;
    Err      error;
    Boolean  result;
    UInt32   romVersion;
 
 
    // The custom category-enabled dialog is only available on Palm OS
    // 3.5 or later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < sysMakeROMVersion(3,5,0,sysROMStageRelease,0))
        return 1;
 
    // Set the default category to Unfiled.
    exgInfo.categoryIndex = dmUnfiledCategory;
 
    // Store the database reference for use by the event handler.
    exgInfo.db = db;
 
    // Display the custom dialog.
    result = ExgDoDialog(askInfo->socketP, &exgInfo, &error);

    if (! error && result)
    {
        // Accept the data; pretend that the user tapped OK.
        askInfo->result = exgAskOk;
  
        // Stuff the category index into the appData field.
        askInfo->socketP->appData = exgInfo.categoryIndex;
    }
    else
    {
        // Reject the data; pretend that the user tapped Cancel.
        askInfo->result = exgAskCancel;
    }

    return error;
}


/***********************************************************************
 *
 * FUNCTION:    LibImportRecord
 *
 * DESCRIPTION: Imports a single record from an IR input stream.  
 *
 * PARAMETERS: db         -> open reference to Librarian's database
 *              exgSocketP -> pointer to the exchange socket structure
 *              bytes   -> number of bytes to read from the input stream;
 *                         LibImportRecord stops importing when it hits
 *                         the end of the stream or this many bytes,
 *                         whichever comes first
 *              indexP <-  pointer to receive the index of the newly
 *                         created record.
 *
 * RETURNED:    0 if no error; otherwise, returns error code
 *
 * REVISION HISTORY:
 *
 *
 **********************************************************************/
Err LibImportRecord(DmOpenRef db, ExgSocketType *exgSocketP,
                    UInt32 bytes, UInt16 *indexP)
{
    Char     buffer[libImportBufferSize];
    Err      error;
    UInt16   index = 0;
    UInt16   insertIndex;
    UInt32   bytesReceived;
    MemHandle  recordH = NULL;
    Char     *record;
    UInt32   recordSize = 0;
    MemHandle  packedH;
    LibPackedDBRecord  *packed;
    Boolean  allocated = false;
    UInt16   category;
 
 
    do {
        UInt32  bytesToRead = min(bytes, sizeof(buffer));
  
  
        bytesReceived = ExgReceive(exgSocketP, buffer, bytesToRead,
                                   &error);
        bytes -= bytesReceived;
        if (! error)
        {
            if (! recordH)
                recordH = DmNewRecord(db, &index, bytesReceived);
            else
                recordH = DmResizeRecord(db, index, recordSize +
                                         bytesReceived);
   
            if (! recordH)
            {
                error = DmGetLastErr();
                break;
            }
   
            allocated = true;
            record = MemHandleLock(recordH);
            error = DmWrite(record, recordSize, buffer, bytesReceived);
            MemHandleUnlock(recordH);
   
            recordSize += bytesReceived;
        }
  
    } while ( (! error) && (bytesReceived > 0) && (bytes > 0) );
 
    if (recordH)
    {
        DmReleaseRecord(db, index, true);
  
        // Grab the category for the new record from the socket's
        // appData field.
        category = exgSocketP->appData;
 
        // Put the record in the proper category.
        if (category)
        {
            UInt16  attr;


            // Get the record's attributes.
            error = DmRecordInfo(db, index, &attr, NULL, NULL);

            // Set the category and mark the record dirty.
            if ((attr & dmRecAttrCategoryMask) != category)
            {
                attr &= ~dmRecAttrCategoryMask;
                attr |= category | dmRecAttrDirty;
                error = DmSetRecordInfo(db, index, &attr, NULL);
            }
        }
  
        // Move the record to its proper sort position.
        packedH = DmQueryRecord(db, index);
        packed = MemHandleLock(packedH);
        insertIndex = LibFindSortPosition(db, packed);
        error = DmMoveRecord(db, index, insertIndex);
        if (! error)
            index = insertIndex - 1;
        MemHandleUnlock(packedH);
    }
 
    if (error && allocated)
        DmRemoveRecord(db, index);
  
    *indexP = index;
    return error;
}


/***********************************************************************
 *
 * FUNCTION:    LibReceiveData
 *
 * DESCRIPTION: Receives data via IR transfer.  
 *
 * PARAMETERS:  db - open reference to Librarian's database
 *              exgSocketP - pointer to the exchange socket structure
 *
 * RETURNED:    0 if no error; otherwise, returns error code
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
Err LibReceiveData(DmOpenRef db, ExgSocketType *exgSocketP)
{
    Err     error;
    UInt16  index = 0;
    Char    *startOfExtension;
    Boolean  singleRecord = false;
    UInt16  numRecords;
    UInt16  recordSize;
 
 
    // Determine whether the input stream contains a single record
    // or an entire category by looking at the file extension.
    if (exgSocketP->name)
    {
        startOfExtension = exgSocketP->name +
            StrLen(exgSocketP->name) - libFileExtensionLength;
        if (StrCaselessCompare(startOfExtension,
                               libFileExtension) == 0)
            singleRecord = true;
    }
 
    // Accept connection from remote device. 
    error = ExgAccept(exgSocketP);
 
    // Import records from data stream.
    if (! error)
    {
        if (singleRecord)
        {
            // Import a single record.
            error = LibImportRecord(db, exgSocketP, libEntireStream,
                                    &index);
        }
        else
        {
            // Import a whole category, starting with the number of
            // records in the transfer.
            ExgReceive(exgSocketP, &numRecords, sizeof(numRecords),
                       &error);
            while ( (! error) && (numRecords-- > 0) )
            {
                // Retrieve the size of the next record.
                ExgReceive(exgSocketP, &recordSize, sizeof(recordSize),
                           &error);
    
                // Import the record.
                if (! error)
                    error = LibImportRecord(db, exgSocketP, recordSize,
                                            &index);
            }
        }
  
        // Disconnect the transfer.
        ExgDisconnect(exgSocketP, error);
    }
 
    // Set the socket structure's goto information so the system can
    // fire off a sysAppLaunchCmdGoTo launch code to open the newly
    // transmitted record in Librarian.
    if (! error)
    {
        DmRecordInfo(db, index, NULL, &exgSocketP->goToParams.uniqueID,
                     NULL);
        DmOpenDatabaseInfo(db, &exgSocketP->goToParams.dbID, NULL, NULL,
                           &exgSocketP->goToParams.dbCardNo, NULL);
        exgSocketP->goToParams.recordNum = index;
        exgSocketP->goToCreator = libCreatorID;
    }

    return error;
}


/***********************************************************************
 *
 * FUNCTION:    LibRegisterData
 *
 * DESCRIPTION: Register Librarian to receive files with an appropriate
 *              extension.  
 *
 * PARAMETERS: none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void LibRegisterData(void)
{
    UInt32     romVersion;
    
 
    // ExgRegisterDatatype is available only on Palm OS 4.0 and later, and
    // ExgRegisterData is available only on Palm OS 3.0 and later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageRelease,0))
    {
        ExgRegisterDatatype(libCreatorID, exgRegExtensionID,
                            libFileExtension "\t" libCategoryExtension,
                            libExgFileDesc "\t" libExgCategoryDesc,
                            0);
    }
    else if (romVersion >= sysMakeROMVersion(3,0,0,sysROMStageRelease,0))
    {
        ExgRegisterData(libCreatorID, exgRegExtensionID,
                        libFileExtension "\t" libCategoryExtension);
    }
}


/***********************************************************************
 *
 * FUNCTION:    LibSendCategory
 *
 * DESCRIPTION: Send an entire category full of records, except for
 *              private records; private records should only be sent
 *              one at a time, to prevent accidentally throwing private
 *              information onto someone else's handheld during a large
 *              category transfer.
 *
 *              LibSendCategory first sends the number of records, then
 *              it sends both the size of each record and the record
 *              itself.
 *
 * PARAMETERS:  dbP - open database reference
 *              urlPrefix - exchange manager URL prefix; defines
 *                          what method will handle the transfer
 *                          (beam or send)
 *              category - category to send
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void LibSendCategory(DmOpenRef db, const Char * const urlPrefix,
                     UInt16 category)
{
    Err      error;
    Char     desc[dmCategoryLength];
    UInt16   index;
    Boolean  foundAtLeastOneRecord;
    ExgSocketType  exgSocket;
    UInt16   mode;
    LocalID  dbID;
    UInt16   cardNo;
    Boolean  databaseReopened;
    UInt16   numRecords;
    LibPackedDBRecord  *packed;
    MemHandle  packedH;


    // If the database is currently opened to show private records,
    // reopen it with private records hidden to prevent private records
    // from being accidentally sent along with a large batch of normal
    // records.  Private records should only be sent one at a time.
    DmOpenDatabaseInfo(db, &dbID, NULL, &mode, &cardNo, NULL);
    if (mode & dmModeShowSecret)
    {
        db = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
        databaseReopened = true;
    }
    else
        databaseReopened = false;
 
    // Verify that there is at least one record in the category.  It's
    // possible to just use DmNumRecordsInCategory for this purpose, but
    // that function has to look over the entire database, which can
    // be slow if there are many records.  This technique is quicker,
    // since it stops searching at the first successful match.
    index = 0;
    foundAtLeastOneRecord = false;
    while (true)
    {
        if (DmSeekRecordInCategory(db, &index, 0, dmSeekForward,
                                   category) != 0)
            break;
  
        foundAtLeastOneRecord = (DmQueryRecord(db, index) != 0);
        if (foundAtLeastOneRecord)
            break;
  
        index++;
    }
 
    // If at least one record exists in the category, send the category.
    if (foundAtLeastOneRecord)
    {
        // Initialize the exchange socket structure to zero.
        MemSet(&exgSocket, sizeof(exgSocket), 0);

        // Assemble a description of the record to send.  This
        // description is displayed by the system send and receive
        // dialogs on both the sending and receiving devices.
        CategoryGetName(db, category, desc);
        exgSocket.description = desc;
  
        // Create a filename from the description.
        exgSocket.name = MemPtrNew(StrLen(urlPrefix) + StrLen(desc) +
                                   sizeOf7BitChar(chrFullStop) +
                                   StrLen(libCategoryExtension) +
                                   sizeOf7BitChar(chrNull));
        if (exgSocket.name)
        {
            StrCopy(exgSocket.name, urlPrefix);
            StrCat(exgSocket.name, desc);
            StrCat(exgSocket.name, ".");
            StrCat(exgSocket.name, libCategoryExtension);
        }
  
        // Initiate transfer.
        error = ExgPut(&exgSocket);
        if (! error)
        {
            // Now use DmNumRecordsInCategory to get the number of
            // records to send, since it's certain at this point that
            // the category will be sent.
            numRecords = DmNumRecordsInCategory(db, category);

            // Send the number of records across first.
            error = SendData(&exgSocket, &numRecords,
                             sizeof(numRecords));
   
            index = dmMaxRecordIndex;
            while ( (! error) && (numRecords-- > 0) )
            {
                UInt16  seekOffset = 1;
                UInt16  recordSize;
    
    
                // Be sure to check the last record instead of
                // skipping over it.
                if (index == dmMaxRecordIndex)
                    seekOffset = 0;
    
                error = DmSeekRecordInCategory(db, &index, seekOffset,
                                               dmSeekBackward, category);
                if (! error)
                {
                    packedH = DmQueryRecord(db, index);
                    ErrNonFatalDisplayIf(! packedH,
                                         "Couldn't query record.");
     
                    // Send the size of the record.
                    recordSize = MemHandleSize(packedH);
                    error = SendData(&exgSocket, &recordSize,
                                     sizeof(recordSize));
     
                    // Send the record itself.
                    if (! error)
                    {
                        packed = MemHandleLock(packedH);
                        error = SendData(&exgSocket, packed, recordSize);
                        MemHandleUnlock(packedH);
                    }
                }
            }
            ExgDisconnect(&exgSocket, error);
        }
        else if (error != exgErrUserCancel)
        {
            FrmAlert(ExchangeErrorAlert);
        }
 
        // Free the filename string to prevent a memory leak.
        MemPtrFree(exgSocket.name);
    }
 
    if (databaseReopened)
        DmCloseDatabase(db);
}


/***********************************************************************
 *
 * FUNCTION:    LibSendRecord
 *
 * DESCRIPTION: Sends a record.  
 *
 * PARAMETERS:  dbP - open database reference
 *              urlPrefix - exchange manager URL prefix; defines
 *                          what method will handle the transfer
 *                          (beam or send)
 *              recordNum - index of the record to send
 *              noDataAlertID - resource ID of an alert to display if
 *                              the record is empty
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void LibSendRecord(DmOpenRef db, const Char * const urlPrefix,
                   Int16 recordNum, UInt16 noDataAlertID)
{
    LibDBRecordType  record;
    MemHandle  recordH = NULL;
    LibPackedDBRecord  *packed;
    MemHandle  packedH;
    MemHandle  descH;
    UInt16     descSize = 0;
    Coord      descWidth;
    Boolean    descFit;
    UInt16     newDescSize;
    MemHandle  prefixH;
    Char       *prefix;
    MemHandle  nameH;
    Err        error;
    ExgSocketType  exgSocket;
    UInt8      prefixLength;
    Coord      ignoreHeight;
 
 
    // If the record number is actually "no record", return.
    if (recordNum == noRecord)
        return;
 
    // Initialize the exchange socket structure to zero.
    MemSet(&exgSocket, sizeof(exgSocket), 0);
 
    // Assemble a description of the record to send.  This description
    // is displayed by the system send and receive dialogs on both the
    // sending and receiving devices.
    error = LibGetRecord(db, recordNum, &record, &recordH);
    ErrNonFatalDisplayIf(error, "Can't get record.");
 
    if (RecordContainsData(&record))
    {
        // Use the title of the book, if it exists.  If the book record
        // is untitled, use the author's name.  Failing that, fall back
        // to a generic string stored in Librarian's resources.
        descH = NULL;
        exgSocket.description = NULL;
        if (record.fields[libFieldTitle])
        {
            // Use title of book for the description.
            descSize = StrLen(record.fields[libFieldTitle]) +
                sizeOf7BitChar('\0');
            descH = MemHandleNew(descSize);
            if (descH)
            {
                exgSocket.description = MemHandleLock(descH);
                StrCopy(exgSocket.description,
                        record.fields[libFieldTitle]);
            }
        }
        else if (record.fields[libFieldFirstName] ||
                 record.fields[libFieldLastName])
        {
            // Use "a book by <author>" for the description.
            prefixH = DmGetResource(strRsc, UntitledSendString);
            prefix = (Char *) MemHandleLock(prefixH);
            descSize = StrLen(prefix);
   
            if (record.fields[libFieldFirstName] &&
                record.fields[libFieldLastName])
                descSize += sizeOf7BitChar(' ') + sizeOf7BitChar('\0');
            else
                descSize += sizeOf7BitChar('\0');
   
            if (record.fields[libFieldFirstName])
                descSize += StrLen(record.fields[libFieldFirstName]);
   
            if (record.fields[libFieldLastName])
                descSize += StrLen(record.fields[libFieldLastName]);
   
            descH = MemHandleNew(descSize);
            exgSocket.description = MemHandleLock(descH);
            StrCopy(exgSocket.description, prefix);
            MemHandleUnlock(prefixH);
            DmReleaseResource(prefixH);
   
            if (record.fields[libFieldFirstName])
            {
                StrCat(exgSocket.description,
                       record.fields[libFieldFirstName]);
                if (record.fields[libFieldLastName])
                    StrCat(exgSocket.description, " ");
            }
   
            if (record.fields[libFieldLastName])
                StrCat(exgSocket.description,
                       record.fields[libFieldLastName]);
        }

        // Truncate the description if it's too long.
        if (descSize > 0)
        {
            newDescSize = descSize;
            WinGetDisplayExtent(&descWidth, &ignoreHeight);
            FntCharsInWidth(exgSocket.description, &descWidth,
                            (Int16 *) &newDescSize, &descFit);
   
            if (newDescSize > 0)
            {
                if (newDescSize != descSize)
                {
                    exgSocket.description[newDescSize] = nullChr;
                    MemHandleUnlock(descH);
                    MemHandleResize(descH, newDescSize +
                                    sizeOf7BitChar('\0'));
                    exgSocket.description = MemHandleLock(descH);
                }
            }
            else
            {
                MemHandleFree(descH);
            }
            descSize = newDescSize;
        }

        // If the description has a length of 0, it indicates that:
        //   a. The record's title, last name, and first name fields
        //      are empty, or
        //   b. One of those fields contained data, but it consisted
        //      entirely of space or tab characters, and the
        //      FntCharsInWidth call during truncation wiped the
        //      description string out.
        // To remedy either situation, use "a book" for the description.
        if (descSize == 0)
        {
            prefixH = DmGetResource(strRsc, NoAuthorSendString);
            prefix = (Char *) MemHandleLock(prefixH);
            descSize = StrLen(prefix) + sizeOf7BitChar('\0');
   
            descH = MemHandleNew(descSize);
            exgSocket.description = MemHandleLock(descH);
            StrCopy(exgSocket.description, prefix);
            MemHandleUnlock(prefixH);
            DmReleaseResource(prefixH);
        }
            
        // Create a file name from the description. The file name conforms
        // to Internet Mail Consortium spec (maximum 32 characters in
        // length). Prefix the file name with the appropriate URL prefix,
        // and truncate the description so it and the ".lib" file extension
        // fit within the 32-character maximum.
        prefixLength = StrLen(urlPrefix);
        nameH = MemHandleNew(prefixLength + imcFilenameLength);
        exgSocket.name = MemHandleLock(nameH);
        StrCopy(exgSocket.name, urlPrefix);
        StrNCat(exgSocket.name, exgSocket.description,
                prefixLength + imcFilenameLength - libFileExtensionLength -
                sizeOf7BitChar('.'));
        StrCat(exgSocket.name, ".");
        StrCat(exgSocket.name, libFileExtension);

        // Clean up the file name.
        CleanFileName(exgSocket.name);
  
        // Send the record.
        error = ExgPut(&exgSocket);
        if (! error)
        {
            packedH = DmQueryRecord(db, recordNum);
            packed = MemHandleLock(packedH);
            error = SendData(&exgSocket, packed, MemHandleSize(packedH));
            MemHandleUnlock(packedH);

            // Calling ExgDisconnect with an empty library reference causes
            // a fatal error. Because Librarian specifies a library by URL and
            // not by explicit library reference, it is possible that the
            // library reference in the exchange socket structure doesn't get
            // filled in by ExgPut; this happens when the particular handheld
            // Librarian is running on doesn't support the library requested
            // in the URL. This is why the call to ExgDisconnect is located
            // within this branch of the if statement; ExgDisconnect should
            // not be called if ExgPut returned an error, because
            // exgSocket.libraryRef might not be filled in properly.
            ExgDisconnect(&exgSocket, error);
        }
        else if (error != exgErrUserCancel)
        {
            FrmAlert(ExchangeErrorAlert);
        }
  
        if (nameH)
        {
            MemHandleUnlock(nameH);
            MemHandleFree(nameH);
        }
  
        if (descH)
        {
            MemHandleUnlock(descH);
            MemHandleFree(descH);
        }
    }
    else
    {
        FrmAlert(noDataAlertID);
    }
 
    MemHandleUnlock(recordH);
}


/***********************************************************************
 *
 * FUNCTION:    SendData
 *
 * DESCRIPTION: Handles the actual task of sending data.  Be sure to
 *              initiate a transfer first with ExgPut, and then end
 *              the transfer with ExgDisconnect.
 *
 * PARAMETERS:  exgSocket - pointer to an exchange socket structure that
 *                          defines the parameters of the send operation
 *              buffer - pointer to the buffer holding the data to send
 *              bytes - number of bytes to send
 *
 * RETURNED:    0 if successful, or an error code if ExgSend had a
 *              problem sending the data.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err SendData(ExgSocketType *exgSocket, void *buffer, UInt32 bytes)
{
    Err  error = 0;
 
 
    while ( (! error) && (bytes > 0) )
    {
        UInt32  bytesSent = ExgSend(exgSocket, buffer, bytes, &error);
        bytes -= bytesSent;
        buffer = ( (Char *) buffer) + bytesSent;
    }

    return error;
}


