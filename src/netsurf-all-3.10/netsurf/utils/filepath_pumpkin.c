/*
 * Copyright 2010 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * Provides utility functions for finding readable files.
 *
 * These functions are intended to make finding resource files more
 *  straightforward.
 */

#include <PalmOS.h>
#include <VFSMgr.h>

#include "utils/dirent.h" /** \todo why is this necessary for atari to get PATH_MAX and is there a better way */
#include "utils/utils.h"
#include "utils/config.h"
#include "utils/filepath.h"

/** maximum number of elements in the resource vector */
#define MAX_RESPATH 128

/* exported interface documented in filepath.h */
char *filepath_vsfindfile(char *str, const char *format, va_list ap)
{
	char *realpathname;
	char *pathname;
	int len;
  FileRef f;

	pathname = sys_malloc(PATH_MAX);
	if (pathname == NULL)
		return NULL; /* unable to allocate memory */

	len = sys_vsnprintf(pathname, PATH_MAX, format, ap);

	if ((len < 0) || (len >= PATH_MAX)) {
		/* error or output exceeded PATH_MAX length so
		 * operation is doomed to fail.
		 */
		sys_free(pathname);
		return NULL;
	}

	if (VFSRealPath(1, pathname, str, PATH_MAX) == errNone) {
	  realpathname = str;
  } else {
	  realpathname = NULL;
  }

	sys_free(pathname);

	if (realpathname != NULL) {
		/* sucessfully expanded pathname */
    if (VFSFileOpen(1, realpathname, vfsModeRead, &f) != errNone) {
			/* unable to read the file */
			return NULL;
		}
    VFSFileClose(f);
	}

	return realpathname;
}


/* exported interface documented in filepath.h */
char *filepath_sfindfile(char *str, const char *format, ...)
{
	sys_va_list ap;
	char *ret;

	sys_va_start(ap, format);
	ret = filepath_vsfindfile(str, format, ap);
	sys_va_end(ap);

	return ret;
}


/* exported interface documented in filepath.h */
char *filepath_findfile(const char *format, ...)
{
	char *ret;
	sys_va_list ap;

	sys_va_start(ap, format);
	ret = filepath_vsfindfile(NULL, format, ap);
	sys_va_end(ap);

	return ret;
}

/* exported interface documented in filepath.h */
char *filepath_sfind(char **respathv, char *filepath, const char *filename)
{
	int respathc = 0;

	if ((respathv == NULL) || (respathv[0] == NULL) || (filepath == NULL))
		return NULL;

	while (respathv[respathc] != NULL) {
		if (filepath_sfindfile(filepath, "%s/%s", respathv[respathc], filename) != NULL) {
			return filepath;
		}

		respathc++;
	}

	return NULL;
}


/* exported interface documented in filepath.h */
char *filepath_find(char **respathv, const char *filename)
{
	char *ret;
	char *filepath;

	if ((respathv == NULL) || (respathv[0] == NULL))
		return NULL;

	filepath = sys_malloc(PATH_MAX);
	if (filepath == NULL)
		return NULL;

	ret = filepath_sfind(respathv, filepath, filename);

	if (ret == NULL)
		sys_free(filepath);

	return ret;
}


/* exported interface documented in filepath.h */
char *
filepath_sfinddef(char **respathv,
		  char *filepath,
		  const char *filename,
		  const char *def)
{
	char t[PATH_MAX];
	char *ret;

	if ((respathv == NULL) || (respathv[0] == NULL) || (filepath == NULL))
		return NULL;

	ret = filepath_sfind(respathv, filepath, filename);

	if ((ret == NULL) && (def != NULL)) {
		/* search failed, return the path specified */
		ret = filepath;
		if (def[0] == '~') {
			sys_snprintf(t, PATH_MAX, "%s/%s/%s", sys_getenv((char *)"HOME"), def + 1, filename);
		} else {
			sys_snprintf(t, PATH_MAX, "%s/%s", def, filename);
		}
		if (VFSRealPath(1, t, ret, PATH_MAX) == errNone) {
			sys_strncpy(ret, t, PATH_MAX);
		}

	}
	return ret;
}


/* exported interface documented in filepath.h */
char **
filepath_generate(char * const *pathv, const char * const *langv)
{
	char **respath; /* resource paths vector */
	int pathc = 0;
	int langc = 0;
	int respathc = 0;
	char *tmppath;
	int tmppathlen;
  UInt32 attributes;

	respath = sys_calloc(MAX_RESPATH, sizeof(char *));

	while ((respath != NULL) &&
	       (pathv[pathc] != NULL)) {
    if (VFSGetAttributes(1, pathv[pathc], &attributes) == errNone &&
        (attributes & vfsFileAttrDirectory)) {
			/* path element exists and is a directory */
			langc = 0;
			while (langv[langc] != NULL) {
				tmppathlen = sys_snprintf(NULL,
						      0,
						      "%s/%s",
						      pathv[pathc],
						      langv[langc]);
				tmppath = sys_malloc(tmppathlen + 1);
				if (tmppath == NULL) {
					break;
				}
				sys_snprintf(tmppath,
					 tmppathlen + 1,
					 "%s/%s",
					 pathv[pathc],
					 langv[langc]);

        if (VFSGetAttributes(1, tmppath, &attributes) == errNone &&
            (attributes & vfsFileAttrDirectory)) {
					/* path element exists and is a directory */
					respath[respathc++] = tmppath;
				} else {
					sys_free(tmppath);
				}

				langc++;
			}
			respath[respathc++] = sys_strdup(pathv[pathc]);
		}
		pathc++;
	}
	return respath;
}


/**
 * expand ${} in a string into environment variables.
 *
 * @param path The pathname to expand.
 * @param pathlen The length of the path element.
 * @return A string with the expanded path or NULL on empty expansion or error.
 */
static char *
expand_path(const char *path, int pathlen)
{
	char *exp;
	int explen;
	int cstart = -1;
	int cloop = 0;
	char *envv;
	int envlen;
	int replen; /* length of replacement */

	exp = sys_malloc(pathlen + 1);
	if (exp == NULL)
		return NULL;

	sys_memcpy(exp, path, pathlen);
	exp[pathlen] = 0;

	explen = pathlen;

	while (exp[cloop] != 0) {
		if ((exp[cloop] == '$') &&
		    (exp[cloop + 1] == '{')) {
			cstart = cloop;
			cloop++;
		}

		if ((cstart != -1) &&
		    (exp[cloop] == '}')) {
			replen = cloop - cstart;
			exp[cloop] = 0;
			envv = sys_getenv(exp + cstart + 2);
			if (envv == NULL) {
				sys_memmove(exp + cstart,
					exp + cloop + 1,
					explen - cloop);
				explen -= replen;
			} else {
				char *tmp;
				envlen = sys_strlen(envv);
				tmp = sys_realloc(exp, explen + envlen - replen);
				if (tmp == NULL) {
					sys_free(exp);
					return NULL;
				}
				exp = tmp;
				sys_memmove(exp + cstart + envlen,
					exp + cloop + 1,
					explen - cloop );
				sys_memmove(exp + cstart, envv, envlen);
				explen += envlen - replen;
			}
			cloop -= replen;
			cstart = -1;
		}

		cloop++;
	}

	if (explen == 1) {
		sys_free(exp);
		exp = NULL;
	}

	return exp;
}


/* exported interface documented in filepath.h */
char **
filepath_path_to_strvec(const char *path)
{
	char **strvec;
	int strc = 0;
	const char *estart; /* path element start */
	const char *eend; /* path element end */
	int elen;

	strvec = sys_calloc(MAX_RESPATH, sizeof(char *));
	if (strvec == NULL)
		return NULL;

	estart = eend = path;

	while (strc < (MAX_RESPATH - 2)) {
		while ( (*eend != 0) && (*eend != ':') )
			eend++;
		elen = eend - estart;

		if (elen > 1) {
			/* more than an empty colon */
			strvec[strc] = expand_path(estart, elen);
			if (strvec[strc] != NULL) {
				/* successfully expanded an element */
				strc++;
			}
		}

		/* skip colons */
		while (*eend == ':')
			eend++;

		/* check for termination */
		if (*eend == 0)
			break;

		estart = eend;
	}
	return strvec;
}


/* exported interface documented in filepath.h */
void filepath_free_strvec(char **pathv)
{
	int p = 0;

	while (pathv[p] != NULL) {
		sys_free(pathv[p++]);
	}
	sys_free(pathv);
}
