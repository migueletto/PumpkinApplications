/*********************************************************************
 * iRogue - Rogue adapted for the PalmPilot.                         *
 * Copyright (C) 1999 Bridget Spitznagel                             *
 * Note: This program is derived from rogue 5.3-clone for Linux.     *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License       *
 * as published by the Free Software Foundation; either version 2    *
 * of the License, or (at your option) any later version.            *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, write to                         *
 * The Free Software Foundation, Inc.,                               *
 * 59 Temple Place - Suite 330,                                      *
 * Boston, MA  02111-1307, USA.                                      *
 *********************************************************************/

#define MainForm              1000
#define MainFormMenu          1010
#define menu_mainJump         1011
#define menu_mainWiz          1012
#define menu_mainWiztog       1013
#define menu_mainMap          1014
#define menu_mainGraffiti     1015
#define menu_mainSaveTog      1016
#define menu_mainAbout        1017
#define menu_mainMoveInstruct 1018
#define menu_mainHelpInstruct 1019
#define menu_mainSettings     1020
#define menu_mainMsgs         1021
#define menu_mainScroll       1022
#define menu_mainWhatsit      1023
#define menu_mainFont         1024
#define menu_mainAutosave     1025
#define menu_mainSave         1026
#define menu_mainRestore      1027
#define menu_mainRefresh      1028
#define menu_mainScores       1029
#define menu_mainBindings     1030

#define DirectionForm         1100
#define btn_d_nw              1101
#define btn_d_n               1102
#define btn_d_ne              1103
#define btn_d_w               1104
#define btn_d_cancel          1105
#define btn_d_e               1106
#define btn_d_sw              1107
#define btn_d_s               1108
#define btn_d_se              1109
#define popup_d               1110
#define list_d                1111

#define MsgLogForm            1200
#define btn_iv_ok             1201
#define field_iv              1202
#define repeat_iv_up          1203
#define repeat_iv_down        1204

#define InvSelectForm         1210
#define btn_i_ok              1211
#define btn_i_cancel          1212
#define list_i                1213
#define label_i_fingers       1219
#define pbtn_i_left1          1220
#define pbtn_i_left2          1221
#define pbtn_i_left3          1222
#define pbtn_i_left4          1223
#define pbtn_i_right1         1224
#define pbtn_i_right2         1225
#define pbtn_i_right3         1226
#define pbtn_i_right4         1227

#define InvFrobForm           1250
#define btn_if_ok             1251
#define btn_if_cancel         1252
#define list_if               1253
// keep these in order, I have a 'for' statement to hide 'em in "inventory.c"
#define pbtn_if_frob          1255
#define pbtn_if_drop          1256
#define pbtn_if_throw         1257
#define pbtn_if_wield         1258

#define WizForm               1300
#define list_wiza             1301
#define list_wizb             1302
#define btn_wiz_ok            1303
#define btn_wiz_cancel        1304

#define MapForm               1400
#define btn_map_done          1401

#define About                 1990
#define MoveStr               1991
#define HelpStr               1992
#define QuitStr               1993
#define InvStr                2004
#define WizStr                2005
#define PrefStr               2006
#define IvStr                 2007
#define AboutStr              2008
#define ButtonsStr            2009
#define RevivStr              2010
#define CreditStr             2011

#define QuitP                 2110
#define WizardP               2111
#define SaveDeleteP           2112
#define SaveOverP             2113
#define NoTiles               2114


#define WitsEndForm          3000

#define TopTenForm           3010
#define field_topten         3011
#define btn_dead_ok          3012
#define repeat_topten_up     3013
#define repeat_topten_down   3014
#define btn_dead_reviv       3015
#define btn_dead_done        3016

#define AboutForm            3020
#define btn_about_ok         3021
#define bitmap_rogue         3022
#define bitmap_cat           3023
#define btn_about_more       3024
#define btn_about_credits    3025

#define RevivifyForm         3030
#define btn_reviv_alive      3031
#define btn_reviv_undead     3032
#define btn_reviv_cancel     3033
#define popup_reviv          3034
#define list_reviv           3035

#define SnapshotForm         3040
#define btn_snap_delete      3041
#define btn_snap_save        3042
#define btn_snap_cancel      3043
#define btn_dead_snap        3044
#define popup_snap           3044
#define list_snap            3045


#define ItsyFont              3456


#define MAGIC_MENU_NUMBER 4000

#define menu_cmd_rest   4046
#define menu_cmd_m      4109
#define menu_cmd_pickup 4044
#define menu_cmd_d      4100

#define menu_cmd_s 4115
#define menu_cmd_i 4105
#define menu_cmd_l 4108
#define menu_cmd_f 4102
#define menu_cmd_F 4070

#define menu_cmd_e 4101
#define menu_cmd_q 4113
#define menu_cmd_r 4114
#define menu_cmd_z 4122

#define menu_cmd_w 4119
#define menu_cmd_t 4116
#define menu_cmd_P 4080
#define menu_cmd_R 4082

#define menu_cmd_T       4084
#define menu_cmd_W       4087
#define menu_cmd_ascend  4060
#define menu_cmd_descend 4062

#define menu_cmd_quit    4081
#define menu_cmd_test    4099

#define menu_cmd_trap    4094



#define MoreForm          5000
#define btn_more_ok       5001


#define PrefsForm         5050
#define btn_bul_ok        5051
#define btn_bul_cancel    5052
#define list_bul_1        5053
#define list_bul_2        5055
#define btn_bul_draw      5057
#define btn_bul_clear     5058
#define check_bul_1       5059
#define check_bul_2       5060
#define field_usr         5061
#define btn_bul_save      5062
#define check_bul_3       5063
#define check_bul_4       5064
#define btn_bul_hw        5065
#define check_bul_5       5066
#define check_bul_6       5067
#define check_bul_7       5068
#define check_bul_8       5069
#define check_bul_9       5070
#define check_bul_10      5071
#define check_bul_11      5072
#define check_bul_12      5073
#define check_bul_13      5074

#define check_hwb        5079
#define HwButtonsForm    5080
#define btn_hwb_ok       5081
#define btn_hwb_cancel   5082
#define popup_hwb_1      5083
#define popup_hwb_2      5084
#define popup_hwb_3      5085
#define popup_hwb_4      5086
#define popup_hwb_5      5087
#define popup_hwb_6      5088
#define popup_hwb_7      5089
#define popup_hwb_8      5090
#define list_hwb_1       5091
#define list_hwb_2       5092
#define list_hwb_3       5093
#define list_hwb_4       5094
#define list_hwb_5       5095
#define list_hwb_6       5096
#define list_hwb_7       5097
#define list_hwb_8       5098

#define FontAlert         5198
