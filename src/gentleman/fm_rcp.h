/*
 *	file:		fm_rcp.h
 *	project:	GentleMan
 *	content:	resource IDs
 *	updated:	Sep. 17. 2001
 *
 * copyright: Collin R. Mulliner <palm@mulliner.org>
 *
 * web: www.mulliner.org/palm/
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

// -= main form =-
#define formID_main 1201

// the volume list
#define volst_main_lst 1202
#define volst_main_button 1203

// sorting
#define sort_main_ptrg 1204
#define sort1_main_lst 1205
#define sort2_main_lst 1206
// show info - part 1
#define show1_main_ptrg 1207
#define show11_main_lst 1208
#define show12_main_lst 1209
// show info - part 2
#define show2_main_ptrg 1210
#define show21_main_lst 1211
#define show22_main_lst 1212

// scrollbar
#define file_main_slb 1213

// browsers
#define brw_main_button 1214
#define brw_main_lst 1215

#define brw1_main_button 1216
#define brw2_main_button 1217

#define brw_group 1218

// --= menu stuff =--
#define vol_main_menu 1301
#define card_main_menu 1302
#define pluginMgr_menu 1303

#define cmd_copy 1310
#define cmd_move 1311
#define cmd_delete 1312
#define cmd_mkdir 1313
#define cmd_fdetails 1314
#define cmd_prefs 1315
#define cmd_help 1316
#define cmd_about 1317
#define cmd_vdetails 1318
#define cmd_beam 1319
#define cmd_sellall 1320
#define cmd_unsellall 1321
#define cmd_beamGentleMan 1322
#define cmd_pluginMgr 1323
#define cmd_pluginActivate 1324
#define cmd_pluginAbout 1325

// -= mkdir form =-
#define formID_mkdir 1401

#define name_mkdir_fld 1402
#define ok_mkdir_button 1403
#define cancel_mkdir_button 1404

// -= volDetails from =-
#define formID_volDetails 1501

#define name_volDetails_fld 1502

#define archive_volDetails_cbox 1503
#define hidden_volDetails_cbox 1504
#define link_volDetails_cbox 1505
#define readonly_volDetails_cbox 1506
#define system_volDetails_cbox 1507

#define done_volDetails_button 1508
#define save_volDetails_button 1509
#define cancel_volDetails_button 1510

#define name_volDetails_slb 1511

#define size_volDetails_label 1512

#define main_volDetails_menu 1513

#define d1_volDetails_lst 1514
#define d2_volDetails_lst 1515
#define d3_volDetails_lst 1516

#define ds1_volDetails_button 1517
#define ds2_volDetails_button 1518
#define ds3_volDetails_button 1519
#define ds4_volDetails_button 1520
#define ds5_volDetails_button 1521
#define ds6_volDetails_button 1522

// -= cardDetails form =-
#define formID_cardDetails 1601

#define name_cardDetails_fld 1602

#define type_cardDetails_label 1603
#define crid_cardDetails_label 1604
#define size_cardDetails_label 1605
#define recs_cardDetails_label 1606
#define ver_cardDetails_label 1607
#define mnum_cardDetails_label 1608
#define type_cardDetails_fld 1609
#define crid_cardDetails_fld 1610

#define app_cardDetails_label 1611
#define sort_cardDetails_label 1612

#define done_cardDetails_button 1620
#define save_cardDetails_button 1621
#define cancel_cardDetails_button 1622

#define gen_cardDetails_button 1623
#define attr_cardDetails_button 1624

#define cd_group 1625

#define backup_cardDetails_cbox 1630
#define bundled_cardDetails_cbox 1631
#define copy_cardDetails_cbox 1632
#define dirty_cardDetails_cbox 1633
#define hidden_cardDetails_cbox 1634
#define launch_cardDetails_cbox 1635
#define install_cardDetails_cbox 1636
#define open_cardDetails_cbox 1637
#define ro_cardDetails_cbox 1638
#define recycle_cardDetails_cbox 1639
#define reset_cardDetails_cbox 1640
#define res_cardDetails_cbox 1641
#define stream_cardDetails_cbox 1642

#define crd_cardDetails_label 1643
#define mod_cardDetails_label 1644
#define bkd_cardDetails_label 1645

#define ds1_cardDetails_button 1646
#define ds2_cardDetails_button 1647
#define ds3_cardDetails_button 1648
#define ds4_cardDetails_button 1649
#define ds5_cardDetails_button 1650
#define ds6_cardDetails_button 1651

#define d1_cardDetails_lst 1652
#define d2_cardDetails_lst 1653
#define d3_cardDetails_lst 1654

// -= about form =-
#define formID_about 2001
#define done_about_button 2002

// -= alerts =-
#define alertID_cardDetName 1701
#define alertID_cardDBRomBased 1702

#define alertID_volReadOnly 1703
#define alertID_volFileName 1704

#define alertID_volDirName 1705

#define alertID_volDeleteFile 1706
#define alertID_volDeleteFiles 1707

#define alertID_volPerms 1708

#define alertID_volFull 1709

#define alertID_cardDeleteFiles 1710

#define alertID_cardDetailsUnknownError 1711

#define alertID_cardNoSelDB 1712

#define alertID_volNoSelFile 1713

#define alertID_cardCantChangeAttr 1714

#define alertID_volSingleFileOnly 1715

#define alertID_cardSingleDBOnly 1716

#define alertID_volDetailsUnknownError 1717

#define alertID_volUnknownError 1718

#define alertID_volCantRemoveRO 1719

#define alertID_volOverwriteQuestion 1720

#define alertID_volOverwriteQuestionAllways 1721

#define alertID_volSrcEqDest 1722

#define alertID_volPDBBroken 1723

#define alertID_volSrcDestIncompatible 1724

#define alertID_stupid 1725

#define alertID_palmOSVersion 1726

#define alertID_betaExpired 1727

#define alertID_cardRecycleWarning 1728

#define alertID_pluginNotFound 1729

// -= bitmaps and other stuff =-
#define bmpID_ani_1 1801
#define bmpID_ani_2 1802
#define bmpID_ani_3 1803
#define bmpID_ani_4 1804
#define bmpID_ani_5 1805
#define bmpID_ani_6 1806
#define bmpID_ani_7 1807
#define bmpID_ani_8 1808

#define bmpID_ani_big_1 1809
#define bmpID_ani_big_2 1810
#define bmpID_ani_big_3 1811
#define bmpID_ani_big_4 1812
#define bmpID_ani_big_5 1813
#define bmpID_ani_big_6 1814
#define bmpID_ani_big_7 1815
#define bmpID_ani_big_8 1816

#define strID_helpPrefs 1817

// -= copy form =-
#define formID_copy 1901

#define dest_copy_lst 1902

#define copy_copy_button 1903
#define cancel_copy_button 1904

#define name_copy_fld 1905

#define dest_copy_label 1906

// -= prefs form =-
#define formID_prefs 2101

#define sortF_prefs_ptrg 2102
#define sortF_prefs_lst 2103
#define showF1_prefs_ptrg 2104
#define showF1_prefs_lst 2105
#define showF2_prefs_ptrg 2106
#define showF2_prefs_lst 2107
#define sortD_prefs_ptrg 2108
#define sortD_prefs_lst 2109
#define showD1_prefs_ptrg 2110
#define showD1_prefs_lst 2111
#define showD2_prefs_ptrg 2112
#define showD2_prefs_lst 2113

#define ok_prefs_button 2114
#define cancel_prefs_button 2115

#define dtapF_prefs_ptrg 2116
#define dtapF_prefs_lst 2117
#define dtapD_prefs_ptrg 2118
#define dtapD_prefs_lst 2119

// -= pluginMgr form =-
#define formID_pluginMgr 2201

#define done_plugin_button 2202
#define plug_plugin_lst 2203
