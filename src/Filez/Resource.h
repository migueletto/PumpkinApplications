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
 * All the constants for the resources.
 *
 */

#ifndef __resource_h__
#define __resource_h__

#include "ResourceCmn.h"

#define resizeIndex 1

#define PREF_ID_RESIZE      4
#define PREF_VERSION_RESIZE 0


/**
 * TreeView Form
 */
#define TreeViewForm                      3000
#define TreeViewTable                     3001
#define TreeViewScrollBar                 3002
#define TreeViewColumnHeaderButton        3005
#define TreeViewColumnList                3006
#define TreeViewFilenameButton            3003
#define TreeViewCloseButton               3008
#define TreeViewDetailsButton             3004
#define TreeViewFilterButton              3007
#define TreeViewUnFilterButton            3009
#define TreeViewHideROMCheckbox           3010
#define TreeViewColumnButton              3012
#define TreeViewSendButton                3013

/* File Menu */
#define TreeItemDetails                   1401
#define TreeItemSend                      1402
#define TreeItemDelete                    1403
#define TreeItemEdit                      1404
#define TreeItemCopy                      1405
#define TreeItemMove                      1406
#define TreeItemCreateFolder              1407
/* Select Menu */
#define TreeSelectSelectAll               1409
#define TreeSelectUnselectAll             1410
#define TreeSelectSetAttributes           1411
/* Options Menu */
#define TreeOptionsFilter                 1412
#define TreeOptionsSendList               1413
#define TreeOptionsSetTypeColors          1414
/* Tree Menu */
#define TreeTreeCollapse                  1415
#define TreeTreeExpand                    1416
#define TreeTreeGoto                   1417


/**
 * Chooser Form
 */
#define ChooserForm                       1300
#define ChooserTable                      1301
#define ChooserScrollBar                  1302
#define ChooserOKButton                   1303
#define ChooserCancelButton               1304
#define ChooserNameLabel                  1305
#define ChooserNameField                  1306
#define ChooserCopyLabel                  1307


/**
 * Folder Details Form
 */
#define FolderDetailsForm                    2500
#define FolderDetailsNameLabel               2501
#define FolderDetailsNameField               2502
#define FolderDetailsOKButton                2503
#define FolderDetailsCancelButton            2504
#define FolderDetailsWarningButton           2507
//#define FolderDetailsWarningBitmap           250


/**
 * About Box Form
 */
#define AboutBoxForm                              1900
#define AboutBoxOKButton                          1901
#define AboutBoxPropsButton                       1908
#define AboutBoxUnnamed1904BitMap                 3300
#define AboutBoxUnnamed1902Label                  1902
#define AboutBoxUnnamed1903Label                  1903
#define AboutBoxUnnamed1905Label                  1905


/*
 * Details Form
 */
#define DetailsRForm                              1100
#define DetailsRCloseButton                      1111
#define DetailsRSaveButton                        1112
#define DetailsRDeleteButton                      1113
#define DetailsRBeamButton                        1130
#define DetailsRCreatorSizeField                1139
/* General Tab */
#define DetailsRVersionIDField                    1101
#define DetailsRNameField                         1104
#define DetailsRTypeField                         1105
#define DetailsRCreatorField                      1106
#define DetailsRSizeField                         1109
#define DetailsRUniqueIDField                     1110
#define DetailsRNameLabel                         1102
#define DetailsRTypeLabel                         1103
#define DetailsRCreatorLabel                      1107
#define DetailsRVersionIDLabel                    1108
#define DetailsRSizeLabel                         1114
#define DetailsRUniqueIDLabel                     1115
/* Attributes Tab */
#define DetailsRStreamCheckbox                    1117
#define DetailsROpenCheckbox                      1118
#define DetailsRROCheckbox                        1119
#define DetailsRBackupCheckbox                    1120
#define DetailsRHiddenCheckbox                    1121
#define DetailsRAppDirtyCheckbox                  1122
#define DetailsRResDBCheckbox                     1123
#define DetailsRInstallNewCheckbox                1124
#define DetailsRResetCheckbox                     1125
#define DetailsRProtectCheckbox                   1126
#define DetailsRLaunchableCheckbox                1127
#define DetailsRBundleCheckbox                     1144
#define DetailsRRecyclableCheckbox                1145
/* Dates Tab */
#define DetailsRCreatedLabel                      1136
#define DetailsRModifiedLabel                     1137
#define DetailsRBackedUpLabel                     1138
#define DetailsRDateChoicesList                   1128
#define DetailsRCreatedDateSelTrigger             1129
#define DetailsRModifiedDateSelTrigger            1131
#define DetailsRBackedUpDateSelTrigger            1132
#define DetailsRCreatedTimeSelTrigger             1133
#define DetailsRModifiedTimeSelTrigger            1134
#define DetailsRBackedUpTimeSelTrigger            1135


/**
 * Details VFS Form
 */
#define DetailsVForm                              3100
#define DetailsVCloseButton                      3111
#define DetailsVSaveButton                        3112
#define DetailsVDeleteButton                      3113
#define DetailsVBeamButton                        3130
/* General Tab */
#define DetailsVNameLabel                         3102
#define DetailsVSizeLabel                         3114
#define DetailsVNameField                         3104
#define DetailsVSizeField                         3109
#define DetailsVShiftGraffitiShift                3116
#define DetailsVViewButton                      3143
/* Attributes Tab */
#define DetailsVROCheckbox                         3119
#define DetailsVHiddenCheckbox                     3121
#define DetailsVSystemCheckbox                     3122
#define DetailsVArchivedCheckbox                   3123
#define DetailsVLinkCheckbox                       3124
/* Dates Tab */
#define DetailsVCreatedLabel                      3136
#define DetailsVModifiedLabel                     3137
#define DetailsVAccessedLabel                     3140
#define DetailsVCreatedDateSelTrigger             3129
#define DetailsVModifiedDateSelTrigger            3131
#define DetailsVAccessedDateSelTrigger            3132
#define DetailsVCreatedTimeSelTrigger             3133
#define DetailsVModifiedTimeSelTrigger            3134
#define DetailsVAccessedTimeSelTrigger            3135
#define DetailsVDateChoicesList                   3128


/**
 * Filter Form
 */
#define FilterForm                               1200
#define FilterOKButton                           1207
#define FilterCancelButton                       1208
#define FilterBackupCheckbox                     1210
#define FilterCopyProtectCheckbox                1211
#define FilterReadOnlyCheckbox                   1212
#define FilterResourceDBCheckbox                 1213
#define FilterStringField                        1206
#define FilterHowList                            1202
#define FilterCompTypeSList                      1204
#define FilterCompTypeNList                      1205
#define FilterHowPopTrigger                      1201
#define FilterCompTypePopTrigger                 1203
#define FilterNotCheckbox                        1214


/**
 * Set Attributes Form
 */
#define SetAttributesForm                 3600
#define SetAttributesOKButton             3601
#define SetAttributesCancelButton         3602
#define SetAttributesAppInfoCheckbox      3603
#define SetAttributesBackupCheckbox       3604
#define SetAttributesBundleCheckbox       3605
#define SetAttributesCopyProtectCheckbox  3606
#define SetAttributesHiddenCheckbox       3607
#define SetAttributesLaunchableCheckbox   3608
#define SetAttributesOKInstallCheckbox    3609
#define SetAttributesReadOnlyCheckbox     3610
#define SetAttributesRecyclableCheckbox   3611
#define SetAttributesResetAfterCheckbox   3612
#define SetAttributesSetPushButton        3613
#define SetAttributesUnsetPushButton      3614


/**
 * Record List Form
 */
#define RecordListForm                            1600
#define RecordListRecordList                      1601
#define RecordListCloseButton                   1602
#define RecordListViewButton                      1609
#define RecordListEditButton                      1610
#define RecordListDeleteButton                    1611
#define RecordListSizeLabel                       1603
#define RecordListIDLabel                         1604
#define RecordListCategoryLabel                   1605
#define RecordListBusyLabel                       1606
#define RecordListSetAttrButton                    1612
#define RecordListDirtyCheckbox                      1607
#define RecordListSecretCheckbox                     1608


/**
 * Goto Form
 */
#define GotoForm										1700
#define GotoFormPathLabel							1701
#define GotoFormPathField							1702
#define GotoFormGotoButton							1703
#define GotoFormCancelButton						1704


/**
 * Hex Editor Form
 */
#define HexEditorForm                             2100
#define HexEditorGoButton                         2103
#define HexEditorDoneButton                       2104
#define HexEditorInsertButton                     2105
#define HexEditorDeleteButton                     2106
#define HexEditorOffsetField                      2102
#define HexEditorUnnamed2101Label                 2101
#define HexEditorScrollUpRepeating                2107
#define HexEditorScrollDownRepeating              2108


/**
 * Menu Form
 */
#define MainViewForm                              2200
#define MainViewFilesButton                       2201
#define MainViewInformationButton                 2202
#define MainViewPreferencesButton                 2211

#define MenuOptionsAbout                          1301

/**
 * Information Form
 */
#define InformationForm                           2700
#define InformationDoneButton                     2701
#define InformationCardList                       2703
#define InformationCardPopTrigger                 2702


/**
 * Pref List Form
 */
#define PrefListForm                              2300
#define PrefListDoneButton                        2302
#define PrefListViewButton                        2303
#define PrefListDeleteButton                      2305
#define PrefListExportListButton                  2308
#define PrefListExportButton                      2309
#define PrefListNumberLabel                       2307
#define PrefListPrefList                          2301
#define PrefListSavedPushButton                   2304
#define PrefListUnsavedPushButton                 2306
#define PrefListGroupID                           1


/**
 * Pref View Form
 */
#define PrefViewForm                              2400
#define PrefViewDoneButton                        2401
#define PrefViewUpRepeating                       2402
#define PrefViewDownRepeating                     2403


/**
 * App Preferences Form
 */
#define PreferencesForm                            2600
#define PreferencesCloseButton                     2601
#define PreferencesAddButton                       2603
#define PreferencesRemoveButton                    2604
#define PreferencesTypeField                       2605
#define PreferencesTypeLabel                       2607
#define PreferencesColorLabel                      2608
#define PreferencesTypeList                        2602
#define PreferencesOKButton                        2609
#define PreferencesListColorLabel                  2610
#define PreferencesSelectAllLabel                  2611
#define PreferencesFilesCheckbox                   2612
#define PreferencesFoldersCheckbox                 2613
#define PreferencesSendLabel                       2614
#define PreferencesSendMenuPushButton              2615
#define PreferencesBeamPushButton                  2616
#define PreferencesFoldersFirstCheckbox            2617
#define PreferencesTapLabel                        2618
#define PreferencesTapNamePushButton               2619
#define PreferencesTapIconPushButton               2620

#define PreferencesHelpID                          1005

#define BlankAlert                                 2301

//	Resource: Talt 1001
#define RomIncompatibleAlert                       1001
#define RomIncompatibleOK                          0

//	Resource: Talt 1000
#define BeamErrorAlert                            1000
#define BeamErrorOK                               0

//	Resource: Talt 1300
#define ROMAlert                                  1300
#define ROMOK                                     0

//	Resource: Talt 1100
#define SaveErrorAlert                            1100
#define SaveErrorOK                               0

//	Resource: Talt 1200
#define MemoAlert                                 1200
#define MemoOK                                    0

//	Resource: Talt 1500
#define SeriouslyAlert                            1500
#define SeriouslyYes                              0
#define SeriouslyNo                               1

//	Resource: Talt 1600
#define SelectAlert                               1600
#define SelectOK                                  0

//	Resource: Talt 1400
#define DeleteErrorAlert                          1400
#define DeleteErrorOK                             0

//	Resource: Talt 1700
#define HelpAlert                                 1700
#define HelpOK                                    0

//	Resource: Talt 1800
#define ConfirmAllBitsAlert                       1800
#define ConfirmAllBitsYes                         0
#define ConfirmAllBitsNo                          1

//	Resource: Talt 1801
#define OverwriteAlert                             1801
#define OverwriteYes                               0
#define OverwriteNo                                1

//	Resource: Talt 1900
#define SelectDateAlert                           1900
#define SelectDateOK                              0

//	Resource: Talt 2000
#define CopySuccessAlert                          2000
#define CopySuccessOK                             0

//	Resource: Talt 2100
#define CopyFailAlert                             2100
#define CopyFailOK                                0

//	Resource: Talt 2200
#define SelectCardAlert                           2200
#define SelectCardOK                              0

//	Resource: Talt 2300
#define DeleteDirectoryAlert                      2300
#define DeleteDirectoryDelete                     0
#define DeleteDirectoryCancel                     1

//	Resource: Talt 2400
#define DeleteFileAlert                           2401
#define DeleteFileDelete                          0
#define DeleteFileCancel                          1

#define DeleteItemsAlert                           2400
#define DeleteItemsDelete                          0
#define DeleteItemsCancel                          1

//	Resource: Talt 2500
#define DeleteDirectoryErrorAlert                 2500
#define DeleteDirectoryErrorOK                    0

//	Resource: Talt 2600
#define DirNotEmptyAlert                          2600
#define DirNotEmptyOK                             0

//	Resource: Talt 2700
#define DirCreateAlert                            2700
#define DirCreateOK                               0

//	Resource: Talt 2900
#define MoveSuccessAlert                          2900
#define MoveSuccessOK                             0

//	Resource: Talt 3000
#define MoveFailAlert                             3000
#define MoveFailOK                                0

//	Resource: Talt 3100
#define Mem2MemNoSupportAlert                     3100
#define Mem2MemNoSupportOK                        0

//	Resource: Talt 3200
#define DestinationExistsAlert                    3200
#define DestinationExistsOK                       0

//	Resource: Talt 3300
#define SelectRecordAlert                         3300
#define SelectRecordOK                            0

//	Resource: Talt 3400
#define SelectByteAlert                           3400
#define SelectByteOK                              0

//	Resource: Talt 3500
#define DeleteRecordAlert                         3500
#define DeleteRecordDelete                        0
#define DeleteRecordCancel                        1

//	Resource: Talt 2800
#define GeneralErrorAlert                         2800
#define GeneralErrorOK                            0

//	Resource: Talt 3700
#define NoChangesAlert                            3700
#define NoChangesOK                               0

//	Resource: Talt 3800
#define SelectSomethingAlert                      3800
#define SelectSomethingOK                         0

//	Resource: Talt 3900
#define UnableCompleteAlert                       3900
#define UnableCompleteOK                          0

//	Resource: Talt 4000
#define UnableToBeamAlert                         5000
#define UnableToBeamOK                            0

//	Resource: Talt 4100
#define EnterPathAlert                            5100
#define EnterPathOK                               0

//	Resource: Talt 4200
#define NoRenameRootAlert                         5200
#define NoRenameRootOK                            0

#define InvalidCharacterAlert                     5201

//	Resource: Talt 4300
#define UnableToRenameAlert                       5300
#define UnableToRenameOK                          0

//	Resource: Talt 4400
#define DeletePrefAlert                           5400
#define DeletePrefYes                             0
#define DeletePrefNo                              1

//	Resource: Talt 4500
#define RenameFileAlert                           5500
#define RenameFileRenameandCopy                   0
#define RenameFileCancel                          1

//	Resource: Talt 4600
#define ColorOnlyAlert                            5600
#define ColorOnlyOK                               0

//	Out of Memory Alert
#define AlertNoMemory                             5700
#define AlertNoMemoryOK                           0


#define ListVFSMenuBar                            1300
#define FileDetailsMenuBar                        1100
#define ListBasicMenuBar                          1200
#define MainViewMenuBar                           2200
#define ListSFSMenuBar                            1500


#define DatesString                               1010
#define AttributesString                          1020
#define GeneralString                             1030
#define SelectTimeString                          1040
#define SelectDateString                          1050




#define AppBitmap						2000
#define CopyFlagBitmap				2100
#define ROMFlagBitmap				2200
#define BackupFlagBitmap			2300
#define CheckFileOnBitmap        2400
#define CheckFileOffBitmap       2500

#define FolderColOffBitmap       3000
#define FolderColOnBitmap        3100
#define FolderExpOffBitmap       3200
#define FolderExpOnBitmap        3300

#define FilesButtonBitmap			2600
#define InfoButtonBitmap			2700
#define PrefsButtonBitmap			2800
#define WarningBitmap				2950

#endif
