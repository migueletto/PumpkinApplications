/*
 * Copyright 2014 vincent Sanders <vince@netsurf-browser.org>
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

/** \file
 * Table operations for files with posix compliant path separator.
 */

#include <PalmOS.h>
#include <VFSMgr.h>

#include "desktop/gui_internal.h"

#include "utils/utils.h"
#include "utils/corestrings.h"
#include "utils/url.h"
#include "utils/nsurl.h"
#include "utils/string.h"
#include "utils/file.h"
#include "utils/dirent.h"

#ifdef nsamiga
#include "frontends/amiga/os3support.h"
#endif

/**
 * Generate a posix path from one or more component elemnts.
 *
 * If a string is allocated it must be freed by the caller.
 *
 * @param[in,out] str pointer to string pointer if this is NULL enough
 *                    storage will be allocated for the complete path.
 * @param[in,out] size The size of the space available if \a str not
 *                     NULL on input and if not NULL set to the total
 *                     output length on output.
 * @param[in] nelm The number of elements.
 * @param[in] ap The elements of the path as string pointers.
 * @return NSERROR_OK and the complete path is written to str
 *         or error code on faliure.
 */
static nserror posix_vmkpath(char **str, size_t *size, size_t nelm, va_list ap)
{
	return vsnstrjoin(str, size, '/', nelm, ap);
}

/**
 * Get the basename of a file using posix path handling.
 *
 * This gets the last element of a path and returns it.
 *
 * @param[in] path The path to extract the name from.
 * @param[in,out] str Pointer to string pointer if this is NULL enough
 *                    storage will be allocated for the path element.
 * @param[in,out] size The size of the space available if \a
 *                     str not NULL on input and set to the total
 *                     output length on output.
 * @return NSERROR_OK and the complete path is written to str
 *         or error code on faliure.
 */
static nserror posix_basename(const char *path, char **str, size_t *size)
{
	const char *leafname;
	char *fname;

	if (path == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	leafname = sys_strrchr(path, '/');
	if (!leafname) {
		leafname = path;
	} else {
		leafname += 1;
	}

	fname = sys_strdup(leafname);
	if (fname == NULL) {
		return NSERROR_NOMEM;
	}

	*str = fname;
	if (size != NULL) {
		*size = sys_strlen(fname);
	}
	return NSERROR_OK;
}

/**
 * Create a path from a nsurl using posix file handling.
 *
 * @param[in] url The url to encode.
 * @param[out] path_out A string containing the result path which should
 *                      be freed by the caller.
 * @return NSERROR_OK and the path is written to \a path or error code
 *         on faliure.
 */
static nserror posix_nsurl_to_path(struct nsurl *url, char **path_out)
{
	lwc_string *urlpath;
	char *path;
	bool match;
	lwc_string *scheme;
	nserror res;

	if ((url == NULL) || (path_out == NULL)) {
		return NSERROR_BAD_PARAMETER;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);

	if (lwc_string_caseless_isequal(scheme, corestring_lwc_file,
					&match) != lwc_error_ok)
	{
		return NSERROR_BAD_PARAMETER;
	}
	lwc_string_unref(scheme);
	if (match == false) {
		return NSERROR_BAD_PARAMETER;
	}

	urlpath = nsurl_get_component(url, NSURL_PATH);
	if (urlpath == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	res = url_unescape(lwc_string_data(urlpath),
			   lwc_string_length(urlpath),
			   NULL,
			   &path);
	lwc_string_unref(urlpath);
	if (res != NSERROR_OK) {
		return res;
	}

	*path_out = path;

	return NSERROR_OK;
}

/**
 * Create a nsurl from a path using posix file handling.
 *
 * Perform the necessary operations on a path to generate a nsurl.
 *
 * @param[in] path The path to convert.
 * @param[out] url_out pointer to recive the nsurl, The returned url
 *                     should be unreferenced by the caller.
 * @return NSERROR_OK and the url is placed in \a url or error code on
 *         faliure.
 */
static nserror posix_path_to_nsurl(const char *path, struct nsurl **url_out)
{
	nserror ret;
	int urllen;
	char *urlstr;
	char *escpath; /* escaped version of the path */
	char *escpaths;

	if ((path == NULL) || (url_out == NULL) || (*path == 0)) {
		return NSERROR_BAD_PARAMETER;
	}

	/* escape the path so it can be placed in a url */
	ret = url_escape(path, false, "/", &escpath);
	if (ret != NSERROR_OK) {
		return ret;
	}
	/* remove unecessary / as file: paths are already absolute */
	escpaths = escpath;
	while (*escpaths == '/') {
		escpaths++;
	}

	/* build url as a string for nsurl constructor */
	urllen = sys_strlen(escpaths) + FILE_SCHEME_PREFIX_LEN + 1;
	urlstr = sys_malloc(urllen);
	if (urlstr == NULL) {
		sys_free(escpath);
		return NSERROR_NOMEM;
	}

	sys_snprintf(urlstr, urllen, "%s%s", FILE_SCHEME_PREFIX, escpaths);
	sys_free(escpath);

	ret = nsurl_create(urlstr, url_out);
	sys_free(urlstr);

	return ret;
}

/**
 * Ensure that all directory elements needed to store a filename exist.
 *
 * @param fname The filename to ensure the path to exists.
 * @return NSERROR_OK on success or error code on failure.
 */
static nserror posix_mkdir_all(const char *fname)
{
	char *dname;
	char *sep;
  UInt32 attributes;

	dname = sys_strdup(fname);

	sep = sys_strrchr(dname, '/');
	if (sep == NULL) {
		/* no directory separator path is just filename so its ok */
		sys_free(dname);
		return NSERROR_OK;
	}

	*sep = 0; /* null terminate directory path */

  if (VFSGetAttributes(1, dname, &attributes) == errNone) {
		sys_free(dname);
    if (attributes & vfsFileAttrDirectory) {
			/* path to file exists and is a directory */
			return NSERROR_OK;
		}
		return NSERROR_NOT_DIRECTORY;
	}
	*sep = '/'; /* restore separator */

	sep = dname;
	while (*sep == '/') {
		sep++;
	}
	while ((sep = sys_strchr(sep, '/')) != NULL) {
		*sep = 0;
    if (VFSGetAttributes(1, dname, &attributes) != errNone) {
			if (VFSDirCreate(1, dname) != 0) {
				/* could not create path element */
				sys_free(dname);
				return NSERROR_NOT_FOUND;
			}
		} else {
      if (!(attributes & vfsFileAttrDirectory)) {
				/* path element not a directory */
				sys_free(dname);
				return NSERROR_NOT_DIRECTORY;
			}
		}
		*sep = '/'; /* restore separator */
		/* skip directory separators */
		while (*sep == '/') {
			sep++;
		}
	}

	sys_free(dname);
	return NSERROR_OK;
}

/**
 * default to using the posix file handling
 */
static struct gui_file_table file_table = {
	.mkpath = posix_vmkpath,
	.basename = posix_basename,
	.nsurl_to_path = posix_nsurl_to_path,
	.path_to_nsurl = posix_path_to_nsurl,
	.mkdir_all = posix_mkdir_all,
};

struct gui_file_table *default_file_table = &file_table;

/* exported interface documented in utils/file.h */
nserror netsurf_mkpath(char **str, size_t *size, size_t nelm, ...)
{
	sys_va_list ap;
	nserror ret;

	sys_va_start(ap, nelm);
	ret = guit->file->mkpath(str, size, nelm, ap);
	sys_va_end(ap);

	return ret;
}

/* exported interface documented in utils/file.h */
nserror netsurf_nsurl_to_path(struct nsurl *url, char **path_out)
{
	return guit->file->nsurl_to_path(url, path_out);
}

/* exported interface documented in utils/file.h */
nserror netsurf_path_to_nsurl(const char *path, struct nsurl **url)
{
	return guit->file->path_to_nsurl(path, url);
}

/* exported interface documented in utils/file.h */
nserror netsurf_mkdir_all(const char *fname)
{
	return guit->file->mkdir_all(fname);
}

static Int16 compareName(void *e1, void *e2, Int32 other) {
  char *s1 = (char *)e1;
  char *s2 = (char *)e2;

  return StrCompare(s1, s2);
}

static int netsurf_scandir(const char *dirp, char ***namelist) {
  UInt32 dirEntryIterator;
  FileInfoType info;
  FileRef f;
  char name[256];
  int total, num = -1;

  if (VFSFileOpen(1, dirp, vfsModeRead, &f) == errNone) {
    total = 64;
    *namelist = sys_calloc(total, sizeof(char *));

    dirEntryIterator = vfsIteratorStart;
    info.nameP = name;
    info.nameBufLen = 256;

    while (VFSDirEntryEnumerate(f, &dirEntryIterator, &info) == errNone) {
      if (num == total) {
        total += 64;
        *namelist = sys_realloc(*namelist, total * sizeof(char *));
      }
      *namelist[num++] = sys_strdup(info.nameP);
    }

    VFSFileClose(f);

    if (num != total) {
      *namelist = sys_realloc(*namelist, num * sizeof(char *));
    }
    SysQSort(*namelist, num, sizeof(char *), compareName, 0);
  }

  return num;
}

/* exported interface documented in utils/file.h */
nserror
netsurf_recursive_rm(const char *path)
{
	char **listing = NULL; /* directory entry listing */
	int nentries, ent;
	nserror ret = NSERROR_OK;
	char *leafpath = NULL;
	const char *leafname;
  UInt32 attributes;

	nentries = netsurf_scandir(path, &listing);

	if (nentries < 0) {
	  return NSERROR_NOT_FOUND;
	}

	for (ent = 0; ent < nentries; ent++) {
		leafname = listing[ent];
		if (sys_strcmp(leafname, ".") == 0 ||
		    sys_strcmp(leafname, "..") == 0)
			continue;
		ret = netsurf_mkpath(&leafpath, NULL, 2, path, leafname);
		if (ret != NSERROR_OK) goto out;
    if (VFSGetAttributes(1, leafpath, &attributes) != errNone) {
			goto out_via_errno;
		}
    if (attributes & vfsFileAttrDirectory) {
			ret = netsurf_recursive_rm(leafpath);
			if (ret != NSERROR_OK) goto out;
		} else {
			if (VFSFileDelete(1, leafpath) != 0) {
				goto out_via_errno;
			}
		}
		sys_free(leafpath);
		leafpath = NULL;
	}

	if (VFSFileDelete(1, path) != 0) {
		goto out_via_errno;
	}

	goto out;

out_via_errno:
  ret = NSERROR_NOT_FOUND;
out:
	if (listing != NULL) {
		for (ent = 0; ent < nentries; ent++) {
			sys_free(listing[ent]);
		}
		sys_free(listing);
	}

	if (leafpath != NULL) {
		sys_free(leafpath);
	}

	return ret;
}
