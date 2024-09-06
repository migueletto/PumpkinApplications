/*
 *	file:		GMPlugin.h
 *	project:	GentleMan
 *	content:	types, defs, ...
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

#ifndef __GMPLUGIN_H__
#define __GMPLUGIN_H__

#include <PalmOS.h>

// type / creator
#define CREATORID	'CRMF'	// GentleMan CreatorID
#define TYPEID		'GMPL'	// Type GMPlugin

// resource id's
#define GM_PLUGIN_NAME_STR_ID						8888
#define GM_PLUGIN_EXTENSIONS_STR_ID				8889
#define GM_PLUGIN_VERSION_INT_ID					8888
#define GM_PLUGIN_INTERFACE_VERSION_INT_ID	8889

// GentleMan Plugin Launchcodes
typedef enum {
	GMPluginAppCmdDoAction = sysAppLaunchCmdCustomBase,
	GMPluginAppCmdShowAbout
} GMPluginCustomActionCodes;

// launch parameter block
typedef struct {
	UInt16 pathLength;
	UInt16 volRefNum;
	// char *path	// "path\0"
} GMPluginDoActionType;

#endif

