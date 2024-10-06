#include <PalmOS.h>

#include "thread.h"
#include "mutex.h"
#include "vfs.h"
#include "bytes.h"
#include "rgb.h"
#include "pumpkin.h"
#include "libretro.h"
#include "libretro_plugin.h"
#include "xalloc.h"
#include "debug.h"

#define XSCALE(x) ((int)((x) * xfactor))
#define YSCALE(y) ((int)((y) * yfactor))

static const char *SYSDIR = "vfs/app_card/PALM/Programs/Libretro";

typedef struct {
  uint8_t from;
  uint8_t mods;
  uint8_t to;
} keymap_t;

static const keymap_t keymap[] = {
  { '\'', WINDOW_MOD_SHIFT, '"' },
  { '1', WINDOW_MOD_SHIFT, '!' },
  { '2', WINDOW_MOD_SHIFT, '@' },
  { '3', WINDOW_MOD_SHIFT, '#' },
  { '4', WINDOW_MOD_SHIFT, '$' },
  { '5', WINDOW_MOD_SHIFT, '%' },
  { '7', WINDOW_MOD_SHIFT, '&' },
  { '8', WINDOW_MOD_SHIFT, '*' },
  { '9', WINDOW_MOD_SHIFT, '(' },
  { '0', WINDOW_MOD_SHIFT, ')' },
  { '-', WINDOW_MOD_SHIFT, '_' },
  { '=', WINDOW_MOD_SHIFT, '+' },
  { '\\', WINDOW_MOD_SHIFT, '|' },
  { '[', WINDOW_MOD_SHIFT, '{' },
  { ']', WINDOW_MOD_SHIFT, '}' },
  { '~', WINDOW_MOD_SHIFT, '^' },
  { '/', WINDOW_MOD_SHIFT, '?' },
  { ';', WINDOW_MOD_SHIFT, ':' },
  { '.', WINDOW_MOD_SHIFT, '>' },
  { ',', WINDOW_MOD_SHIFT, '<' },

  { 'a', WINDOW_MOD_SHIFT, 'A' },
  { 'b', WINDOW_MOD_SHIFT, 'B' },
  { 'c', WINDOW_MOD_SHIFT, 'C' },
  { 'd', WINDOW_MOD_SHIFT, 'D' },
  { 'e', WINDOW_MOD_SHIFT, 'E' },
  { 'f', WINDOW_MOD_SHIFT, 'F' },
  { 'g', WINDOW_MOD_SHIFT, 'G' },
  { 'h', WINDOW_MOD_SHIFT, 'H' },
  { 'i', WINDOW_MOD_SHIFT, 'I' },
  { 'j', WINDOW_MOD_SHIFT, 'J' },
  { 'k', WINDOW_MOD_SHIFT, 'K' },
  { 'l', WINDOW_MOD_SHIFT, 'L' },
  { 'm', WINDOW_MOD_SHIFT, 'M' },
  { 'n', WINDOW_MOD_SHIFT, 'N' },
  { 'o', WINDOW_MOD_SHIFT, 'O' },
  { 'p', WINDOW_MOD_SHIFT, 'P' },
  { 'q', WINDOW_MOD_SHIFT, 'Q' },
  { 'r', WINDOW_MOD_SHIFT, 'R' },
  { 's', WINDOW_MOD_SHIFT, 'S' },
  { 't', WINDOW_MOD_SHIFT, 'T' },
  { 'u', WINDOW_MOD_SHIFT, 'U' },
  { 'v', WINDOW_MOD_SHIFT, 'V' },
  { 'w', WINDOW_MOD_SHIFT, 'W' },
  { 'x', WINDOW_MOD_SHIFT, 'X' },
  { 'y', WINDOW_MOD_SHIFT, 'Y' },
  { 'z', WINDOW_MOD_SHIFT, 'Z' },

  { 'a', WINDOW_MOD_CTRL, 1 },
  { 'b', WINDOW_MOD_CTRL, 2 },
  { 'c', WINDOW_MOD_CTRL, 3 },
  { 'd', WINDOW_MOD_CTRL, 4 },
  { 'e', WINDOW_MOD_CTRL, 5 },
  { 'f', WINDOW_MOD_CTRL, 6 },
  { 'g', WINDOW_MOD_CTRL, 7 },
  { 'h', WINDOW_MOD_CTRL, 8 },
  { 'i', WINDOW_MOD_CTRL, 9 },
  { 'j', WINDOW_MOD_CTRL, 10 },
  { 'k', WINDOW_MOD_CTRL, 11 },
  { 'l', WINDOW_MOD_CTRL, 12 },
  { 'm', WINDOW_MOD_CTRL, 13 },
  { 'n', WINDOW_MOD_CTRL, 14 },
  { 'o', WINDOW_MOD_CTRL, 15 },
  { 'p', WINDOW_MOD_CTRL, 16 },
  { 'q', WINDOW_MOD_CTRL, 17 },
  { 'r', WINDOW_MOD_CTRL, 18 },
  { 's', WINDOW_MOD_CTRL, 19 },
  { 't', WINDOW_MOD_CTRL, 20 },
  { 'u', WINDOW_MOD_CTRL, 21 },
  { 'v', WINDOW_MOD_CTRL, 22 },
  { 'w', WINDOW_MOD_CTRL, 23 },
  { 'x', WINDOW_MOD_CTRL, 24 },
  { 'y', WINDOW_MOD_CTRL, 25 },
  { 'z', WINDOW_MOD_CTRL, 26 },

  { 0, 0, 0 }
};

typedef struct {
  void *lib;
  void (*retro_set_environment)(retro_environment_t env);
  void (*retro_init)(void);
  bool (*retro_load_game)(const struct retro_game_info *game);
  void (*retro_set_video_refresh)(retro_video_refresh_t);
  void (*retro_set_audio_sample)(retro_audio_sample_t);
  void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
  void (*retro_set_input_state)(retro_input_state_t);
  void (*retro_set_controller_port_device)(unsigned port, unsigned device);
  void (*retro_set_input_poll)(retro_input_poll_t);
  unsigned (*retro_api_version)(void);
  void (*retro_get_system_av_info)(struct retro_system_av_info *info);
  void (*retro_get_system_info)(struct retro_system_info *info);
  void (*retro_run)(void);
  void (*retro_unload_game)(void);
  void (*retro_deinit)(void);
  void (*retro_keyboard_event)(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
  retro_proc_address_t (*retro_get_proc_address)(const char *sym);
  struct retro_game_info gameinfo;
  struct retro_system_av_info avinfo;
  struct retro_system_info systeminfo;
  bool no_game;
  char *gamepath;
  int configured;
} liblretro_core_t;

struct retro_vfs_file_handle {
  char *path;
  vfs_file_t *f;
};

struct retro_vfs_dir_handle {
  vfs_dir_t *d;
  char name[FILE_PATH];
  bool hasname, isdir;
};

static mutex_t *mutex;
static UInt16 depth;
static int last_x, last_y;
static int64_t dt;
static int ebuttons;

static vfs_session_t *session;
static liblretro_core_t core;
static int pause, stop;

#define MAX_VARS 256
static struct retro_variable variables[MAX_VARS];
static int num_vars;

// Get path from opaque handle. Returns the exact same path passed to file_open when getting the handle
static const char *liblretro_vfs_get_path(struct retro_vfs_file_handle *stream) {
  return stream ? stream->path : NULL;
}

// Open a file for reading or writing. If path points to a directory, this will fail. Returns the opaque file handle, or NULL for error.
static struct retro_vfs_file_handle *liblretro_vfs_open(const char *path, unsigned mode, unsigned hints) {
  struct retro_vfs_file_handle *stream = NULL;
  int flags;

  if (path) {
    debug(DEBUG_INFO, "LIBRETRO", "retro_vfs_open \"%s\"", path);
    flags = 0;
    if ((mode & RETRO_VFS_FILE_ACCESS_READ))  flags |= VFS_READ;
    if ((mode & RETRO_VFS_FILE_ACCESS_WRITE)) flags |= VFS_WRITE;
    if (!(mode & RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING)) flags |= VFS_TRUNC;

    if ((stream = xcalloc(1, sizeof(struct retro_vfs_file_handle))) != NULL) {
      if ((stream->f = vfs_open(session, (char *)path, flags)) != NULL) {
        stream->path = xstrdup(path);
      } else {
        xfree(stream);
        stream = NULL;
      }
    }
  }

  return stream;
}

static int liblretro_vfs_close(struct retro_vfs_file_handle *stream) {
  int r = -1;

  if (stream) {
    r = vfs_close(stream->f);
    xfree(stream->path);
    xfree(stream);
  }

  return r;
}

// Return the size of the file in bytes, or -1 for error
static int64_t liblretro_vfs_size(struct retro_vfs_file_handle *stream) {
  int64_t r = -1;

  if (stream) {
    r = vfs_seek(stream->f, 0, 1);
    vfs_seek(stream->f, 0, 0);
  }

  return r;
}

static int64_t liblretro_vfs_truncate(struct retro_vfs_file_handle *stream, int64_t length) {
  debug(DEBUG_ERROR, "LIBRETRO", "retro_vfs_truncate not implemented");
  return -1;
}

static int64_t liblretro_vfs_tell(struct retro_vfs_file_handle *stream) {
  int64_t r = -1;

  if (stream) {
    r = vfs_seek(stream->f, 0, -1);
  }

  return r;
}

static int64_t liblretro_vfs_seek(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position) {
  int64_t r = -1;

  if (stream) {
    switch (seek_position) {
      case RETRO_VFS_SEEK_POSITION_START:   r = vfs_seek(stream->f, offset,  0); break;
      case RETRO_VFS_SEEK_POSITION_CURRENT: r = vfs_seek(stream->f, offset, -1); break;
      case RETRO_VFS_SEEK_POSITION_END:     r = vfs_seek(stream->f, offset,  1); break;
    }
  }

  return r;
}

// Read data from a file. Returns the number of bytes read, or -1 for error
static int64_t liblretro_vfs_read(struct retro_vfs_file_handle *stream, void *s, uint64_t len) {
  int64_t r = -1;

  if (stream) {
    r = vfs_read(stream->f, (uint8_t *)s, len);
  }

  return r;
}

static int64_t liblretro_vfs_write(struct retro_vfs_file_handle *stream, const void *s, uint64_t len) {
  int64_t r = -1;

  if (stream) {
    r = vfs_write(stream->f, (uint8_t *)s, len);
  }

  return r;
}

static int liblretro_vfs_flush(struct retro_vfs_file_handle *stream) {
  return 0;
}

static int liblretro_vfs_remove(const char *path) {
  int r = -1;

  if (path) {
    r = vfs_unlink(session, (char *)path);
  }

  return r;
}

static int liblretro_vfs_rename(const char *old_path, const char *new_path) {
  debug(DEBUG_ERROR, "LIBRETRO", "retro_vfs_rename not implemented");
  return -1;
}

// Stat the specified file. Retruns a bitmask of RETRO_VFS_STAT_* flags, none are set if path was not valid.
// Additionally stores file size in given variable, unless NULL is given
// RETRO_VFS_STAT_IS_VALID
// RETRO_VFS_STAT_IS_DIRECTORY
// RETRO_VFS_STAT_IS_CHARACTER_SPECIAL
static int liblretro_vfs_stat(const char *path, int32_t *size) {
  vfs_ent_t ent;
  int r = 0;

  if (path) {
    if (vfs_stat(session, (char *)path, &ent)) {
      r |= RETRO_VFS_STAT_IS_VALID;
      if (ent.type == VFS_DIR) {
        r |= RETRO_VFS_STAT_IS_DIRECTORY;
      }
    }
  }

  return r;
}

static int liblretro_vfs_mkdir(const char *dir) {
  int r = -1;

  if (dir) {
    r = vfs_mkdir(session, (char *)dir);
  }

  return r;
}

static struct retro_vfs_dir_handle *liblretro_vfs_opendir(const char *dir, bool include_hidden) {
  struct retro_vfs_dir_handle *dirstream = NULL;

  if (dir) {
    if ((dirstream = xcalloc(1, sizeof(struct retro_vfs_dir_handle))) != NULL) {
      if ((dirstream->d = vfs_opendir(session, (char *)dir)) == NULL) {
        xfree(dirstream);
        dirstream = NULL;
      }
    }
  }

  return dirstream;
}

// Read the directory entry at the current position, and move the read pointer to the next position.
// Returns true on success, false if already on the last entry.
static bool liblretro_vfs_readdir(struct retro_vfs_dir_handle *dirstream) {
  vfs_ent_t *ent;
  bool r = false;

  if (dirstream) {
    if ((ent = vfs_readdir(dirstream->d)) != NULL) {
      MemMove(dirstream->name, ent->name, FILE_PATH);
      dirstream->hasname = true;
      dirstream->isdir = (ent->type == VFS_DIR);
      r = true;
    } else {
      dirstream->hasname = false;
      dirstream->isdir = false;
    }
  }

  return r;
}

// Get the name of the last entry read. Returns a string on success, or NULL for error.
// The returned string pointer is valid until the next call to readdir or closedir.
static const char *liblretro_vfs_dirent_get_name(struct retro_vfs_dir_handle *dirstream) {
  char *name = NULL;

  if (dirstream && dirstream->hasname) {
    name = dirstream->name;
  }

  return name;
}

// Check if the last entry read was a directory. Returns true if it was, false otherwise (or on error)
static bool liblretro_vfs_dirent_is_dir(struct retro_vfs_dir_handle *dirstream) {
  bool r = false;

  if (dirstream && dirstream->hasname) {
    r =  dirstream->isdir;
  }

  return r;
}

static int liblretro_vfs_closedir(struct retro_vfs_dir_handle *dirstream) {
  int r = -1;

  if (dirstream) {
    r = vfs_closedir(dirstream->d);
    xfree(dirstream);
  }

  return r;
}

static struct retro_vfs_interface vfsiface = {
   /* VFS API v1 */
  liblretro_vfs_get_path,
  liblretro_vfs_open,
  liblretro_vfs_close,
  liblretro_vfs_size,
  liblretro_vfs_tell,
  liblretro_vfs_seek,
  liblretro_vfs_read,
  liblretro_vfs_write,
  liblretro_vfs_flush,
  liblretro_vfs_remove,
  liblretro_vfs_rename,
   /* VFS API v2 */
   liblretro_vfs_truncate,
   /* VFS API v3 */
   liblretro_vfs_stat,
   liblretro_vfs_mkdir,
   liblretro_vfs_opendir,
   liblretro_vfs_readdir,
   liblretro_vfs_dirent_get_name,
   liblretro_vfs_dirent_is_dir,
   liblretro_vfs_closedir
};

void liblretro_log_printf(enum retro_log_level level, const char *fmt, ...) {
  va_list ap;
  int dl;

  switch (level) {
    case RETRO_LOG_DEBUG: dl = DEBUG_TRACE; break;
    case RETRO_LOG_INFO:  dl = DEBUG_INFO; break;
    case RETRO_LOG_WARN:  dl = DEBUG_INFO; break;
    case RETRO_LOG_ERROR: dl = DEBUG_ERROR; break;
    default: dl = DEBUG_ERROR; break;
  }

  va_start(ap, fmt);
  debugva(dl, "LIBRETRO", fmt, ap);
  va_end(ap);
}

static bool liblretro_environment(unsigned cmd, void *data) {
  struct retro_log_callback *logging;
  struct retro_vfs_interface_info *vfsinfo;
  struct retro_variable *var;
  struct retro_game_geometry *geometry;
  struct retro_keyboard_callback *kcb;
  struct retro_get_proc_address_interface *getproc;
  struct retro_input_descriptor *input;
  struct retro_system_av_info *avinfo;
  //struct retro_controller_info *controller;
  //struct retro_core_option_display *optdisplay;
  //struct retro_disk_control_callback *diskcontrol;
  //struct retro_disk_control_ext_callback *diskcontrolext;
  enum retro_pixel_format *pixel_format;
  unsigned *up;
  bool *bp;
  char **sp;
  int i;
  char name[32];
  bool r = false;

  thread_get_name(name, 32);
  if (name[0] == '?') {
    thread_set_name("libretro");
  }

  switch (cmd) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
      logging = (struct retro_log_callback *)data;
      logging->log = liblretro_log_printf;
      r = true;
      break;

    case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
      vfsinfo = (struct retro_vfs_interface_info *)data;
      vfsinfo->iface = &vfsiface;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
      //controller = (struct retro_controller_info *)data;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
      //diskcontrol = (struct retro_disk_control_callback *)data;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
      //diskcontrolext = (struct retro_disk_control_ext_callback *)data;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
      bp = (bool *)data;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
      //optdisplay = (struct retro_core_option_display *)data;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_GEOMETRY:
      geometry = (struct retro_game_geometry *)data;
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_GEOMETRY %d,%d", geometry->base_width, geometry->base_height);
      pumpkin_change_display(geometry->base_width, geometry->base_height);
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
      avinfo = (struct retro_system_av_info *)data;
      MemMove(&core.avinfo, avinfo, sizeof(struct retro_system_av_info));
      debug(DEBUG_INFO, "LIBRETRO", "video fps %.1f", core.avinfo.timing.fps);
      debug(DEBUG_INFO, "LIBRETRO", "video width %d", core.avinfo.geometry.base_width);
      debug(DEBUG_INFO, "LIBRETRO", "video height %d", core.avinfo.geometry.base_height);
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_VARIABLES:
      var = (struct retro_variable *)data;
      for (i = 0; var[i].key != NULL && i < MAX_VARS; i++) {
        variables[i].key = var[i].key;
        variables[i].value = NULL;
        if (var[i].value) {
          char *s = sys_strstr(var[i].value, "; ");
          if (s) {
            char *q = sys_strchr(s + 2, '|');
            if (q) {
              int len = q - (s + 2);
              if (len > 0) {
                variables[i].value = xcalloc(1, len + 1);
                xmemcpy((char *)variables[i].value, s + 2, len);
                debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_VARIABLES %s = %s (%s)", variables[i].key, variables[i].value, var[i].value);
              }
            }
          }
        }
      }
      num_vars = i;
      break;

    // Result is set to true if some variables are updated by
    // frontend since last call to RETRO_ENVIRONMENT_GET_VARIABLE.
    // Variables should be queried with GET_VARIABLE.
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
      bp = (bool *)data;
      *bp = false;
      r = true;
      break;

    case RETRO_ENVIRONMENT_GET_VARIABLE:
      var = (struct retro_variable *)data;
      for (i = 0; i < num_vars; i++) {
        if (!StrCompare(variables[i].key, var->key)) {
          var->value = variables[i].value;
          r = true;
          break;
        }
      }
      break;

    case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
      up = (unsigned *)data;
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL %u", *up);
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
      input = (struct retro_input_descriptor *)data;
      for (i = 0; input[i].description; i++) {
        debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS port=%u, device=%u, index=%u, id=%u, description=\"%s\"",
          input[i].port, input[i].device, input[i].index, input[i].id, input[i].description);
      }
      r = true;
      break;

    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_GET_INPUT_BITMASKS %p", data);
      bp = (bool *)data;
      if (bp) {
        *bp = true;
        r = true;
      }
      break;

    case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
      bp = (bool *)data;
      if (bp) {
        *bp = false;
        r = true;
      }
      break;

    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
      pixel_format = (enum retro_pixel_format *)data;
      switch (*pixel_format) {
        case RETRO_PIXEL_FORMAT_XRGB8888:
          debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_PIXEL_FORMAT RETRO_PIXEL_FORMAT_XRGB8888");
          depth = 32;
          r = true;
          break;
        case RETRO_PIXEL_FORMAT_RGB565:
          debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_PIXEL_FORMAT RETRO_PIXEL_FORMAT_RGB565");
          depth = 16;
          r = true;
          break;
        case RETRO_PIXEL_FORMAT_0RGB1555:
        case RETRO_PIXEL_FORMAT_UNKNOWN:
          debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_PIXEL_FORMAT %d not supported", *pixel_format);
          break;
      }
      break;

    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
      sp = (char **)data;
      *sp = (char *)SYSDIR;
      r = true;
      break;

    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
      bp = (bool *)data;
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME %s", *bp ? "true" : "false");
      core.no_game = *bp;
      break;

    case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
      getproc = (struct retro_get_proc_address_interface *)data;
      core.retro_get_proc_address = getproc->get_proc_address;
      break;

    // Requests the frontend to shutdown.
    // Should only be used if game has a specific
    // way to shutdown the game from a menu item or similar.
    case RETRO_ENVIRONMENT_SHUTDOWN:
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SHUTDOWN");
      stop = 1;
      break;

    // Sets a callback function used to notify core about keyboard events.
    case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK");
      kcb = (struct retro_keyboard_callback *)data;
      core.retro_keyboard_event = kcb->callback;
      r = true;
      break;

    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
      up = (unsigned *)data;
      *up = 0;
      r = true;
      break;

    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
      sp = (char **)data;
      *sp = (char *)SYSDIR;
      r = true;
      break;

    default:
      debug(DEBUG_INFO, "LIBRETRO", "retro_environment %d ignored", cmd);
      break;
  }

  return r;
}

static void fill_rgb565(uint16_t *src, uint16_t *rgb16, uint32_t len) {
  uint32_t i;
  uint16_t w;

  for (i = 0; i < len; i++) {
    get2l(&w, (uint8_t *)&src[i], 0);
    put2b(w, (uint8_t *)&rgb16[i], 0);
  }
}

static void fill_rgba(uint32_t *src, uint16_t *rgb16, uint32_t len) {
  uint16_t red, green, blue, w;
  uint32_t i;

  for (i = 0; i < len; i++) {
    red   = (src[i] >> 16) & 0xff; 
    green = (src[i] >>  8) & 0xff;
    blue  =  src[i]        & 0xff;
    w = rgb565(red, green, blue);
    put2b(w, (uint8_t *)&rgb16[i], 0);
  }
}

static void fill_rgba_le16(uint32_t *src, uint16_t *rgb16, uint32_t len) {
  uint16_t red, green, blue, w;
  uint32_t i;

  for (i = 0; i < len; i++) {
    red   = (src[i] >> 16) & 0xff;
    green = (src[i] >>  8) & 0xff;
    blue  =  src[i]        & 0xff;
    w = rgb565(red, green, blue);
    put2l(w, (uint8_t *)&rgb16[i], 0);
  }
}

static void liblretro_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
  UInt32 sWidth, sHeight;
  WinHandle wh;
  BitmapType *bmp;
  Boolean le16;
  uint16_t *bits;

  if (data) {
    WinScreenGetAttribute(winScreenWidth, &sWidth);
    WinScreenGetAttribute(winScreenHeight, &sHeight);

    if (sWidth == width && sHeight == height) {
      wh = WinGetDisplayWindow();
      bmp = WinGetBitmap(wh);
      bits = (uint16_t *)BmpGetBits(bmp);
      le16 = BmpGetLittleEndian16();

      switch (depth) {
        case 16:
          if (le16) {
            xmemcpy(bits, data, width * height * 2);
          } else {
            fill_rgb565((uint16_t *)data, bits, width * height);
          }
          break;
        case 32:
          if (le16) {
            fill_rgba_le16((uint32_t *)data, bits, width * height);
          } else {
            fill_rgba((uint32_t *)data, bits, width * height);
          }
          break;
      }

      pumpkin_screen_dirty(wh, 0, 0, width, height);
    }
  }
}

static void liblretro_audio_sample(int16_t left, int16_t right) {
}

// One frame is defined as a sample of left and right channels, interleaved.
// I.e. int16_t buf[4] = { l, r, l, r }; would be 2 frames.
static size_t liblretro_audio_sample_batch(const int16_t *data, size_t frames) {
  return 0;
}

static void liblretro_input_poll(void) {
}

static int16_t liblretro_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
  UInt32 state, sWidth, sHeight;
  Int16 x, y;
  Boolean penDown, penRight;
  int r = 0;

  //debug(DEBUG_INFO, "LIBRETRO", "retro_input_state port %u, device %u, index %u, id %u", port, device, index, id);

  switch (device) {
    case RETRO_DEVICE_ANALOG:
/*
      if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) {
        switch (id) {
          case RETRO_DEVICE_ID_ANALOG_X:
            r = (pen_x * 65536) / screen_width - 32768;
            break;
          case RETRO_DEVICE_ID_ANALOG_Y:
            r = (pen_y * 65536) / screen_height - 32768;
            break;
          default:
            debug(DEBUG_INFO, "LIBRETRO", "retro_input_state analog index %u, id %u ignored", index, id);
            break;
        }
      } else {
        //debug(DEBUG_INFO, "LIBRETRO", "retro_input_state analog index %u, id %u ignored", index, id);
      }
*/
      break;

    case RETRO_DEVICE_JOYPAD:
      state = KeyCurrentState();
      switch (id) {
        case RETRO_DEVICE_ID_JOYPAD_SELECT:
          r =  0; // XXX
          break;
        case RETRO_DEVICE_ID_JOYPAD_START:
          r = (state & keyBitHard4) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_JOYPAD_B:
          r = (state & keyBitHard3) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_JOYPAD_UP:
          r = (state & keyBitPageUp) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_JOYPAD_DOWN:
          r = (state & keyBitPageDown) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_JOYPAD_LEFT:
          r = (state & keyBitHard1) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_JOYPAD_RIGHT:
          r = (state & keyBitHard2) ? 1 : 0;
          break;
      }
      break;

    case RETRO_DEVICE_MOUSE:
      EvtGetPenEx(&x, &y, &penDown, &penRight);

      switch (id) {
        case RETRO_DEVICE_ID_MOUSE_X:
          r = x - last_x;
          last_x = x;
          break;
        case RETRO_DEVICE_ID_MOUSE_Y:
          r = y - last_y;
          last_y = y;
          break;
        case RETRO_DEVICE_ID_MOUSE_LEFT:
          r = (penDown && !penRight) ? 1 : 0;
          break;
        case RETRO_DEVICE_ID_MOUSE_MIDDLE:
          break;
        case RETRO_DEVICE_ID_MOUSE_RIGHT:
          r = (penDown && penRight) ? 1 : 0;
          break;
        default:
          debug(DEBUG_INFO, "LIBRETRO", "retro_input_state mouse index %u, id %u ignored", index, id);
          break;
      }
      break;

    case RETRO_DEVICE_POINTER:
      EvtGetPen(&x, &y, &penDown);
      x *= 2;
      y *= 2;

      switch (id) {
        case RETRO_DEVICE_ID_POINTER_X:
          WinScreenGetAttribute(winScreenWidth, &sWidth);
          r = ((int)x * 65536) / sWidth - 32768;
          break;
        case RETRO_DEVICE_ID_POINTER_Y:
          WinScreenGetAttribute(winScreenHeight, &sHeight);
          r = ((int)y * 65536) / sHeight - 32768;
          break;
        case RETRO_DEVICE_ID_POINTER_PRESSED:
          r = penDown;
/*
          if (pen_status && !pen_down && (sys_get_clock() - pen_t) > 15000) {
            pen_status = 0;
          }
*/
          break;
      }
      break;

    default:
      debug(DEBUG_INFO, "LIBRETRO", "retro_input_state port %u, device %u, index %u, id %u ignored", port, device, index, id);
      break;
  }

  return r;
}

static int liblretro_core_load(liblretro_core_t *core, char *corepath, char *gamepath) {
  int first_load, r = -1;

  if (core && corepath) {
    if ((core->lib = vfs_loadlib(session, corepath, &first_load)) != NULL) {
      core->retro_set_environment = sys_lib_defsymbol(core->lib, "retro_set_environment", 1);
      core->retro_init = sys_lib_defsymbol(core->lib, "retro_init", 1);
      core->retro_load_game = sys_lib_defsymbol(core->lib, "retro_load_game", 1);
      core->retro_set_video_refresh = sys_lib_defsymbol(core->lib, "retro_set_video_refresh", 1);
      core->retro_set_audio_sample = sys_lib_defsymbol(core->lib, "retro_set_audio_sample", 1);
      core->retro_set_audio_sample_batch = sys_lib_defsymbol(core->lib, "retro_set_audio_sample_batch", 1);
      core->retro_set_input_state = sys_lib_defsymbol(core->lib, "retro_set_input_state", 1);
      core->retro_api_version = sys_lib_defsymbol(core->lib, "retro_api_version", 1);
      core->retro_set_controller_port_device = sys_lib_defsymbol(core->lib, "retro_set_controller_port_device", 1);
      core->retro_set_input_poll = sys_lib_defsymbol(core->lib, "retro_set_input_poll", 1);
      core->retro_get_system_av_info = sys_lib_defsymbol(core->lib, "retro_get_system_av_info", 1);
      core->retro_get_system_info = sys_lib_defsymbol(core->lib, "retro_get_system_info", 1);
      core->retro_run = sys_lib_defsymbol(core->lib, "retro_run", 1);
      core->retro_unload_game = sys_lib_defsymbol(core->lib, "retro_unload_game", 1);
      core->retro_deinit = sys_lib_defsymbol(core->lib, "retro_deinit", 1);
      core->retro_keyboard_event = NULL;
      core->gamepath = gamepath;
      core->configured = 0;
      debug(DEBUG_INFO, "LIBRETRO", "liblretro_core_load loaded \"%s\"", corepath);
      r = 0;
    } else {
      debug(DEBUG_ERROR, "LIBRETRO", "liblretro_core_load error loading \"%s\"", corepath);
    }
  }

  return r;
}

static int liblretro_core_unload(liblretro_core_t *core) {
  int r = -1;

  if (core) {
    if (core->lib) {
      sys_lib_close(core->lib);
      core->lib = NULL;
    }
    core->retro_set_environment = NULL;
    core->retro_init = NULL;
    core->retro_load_game = NULL;
    core->retro_set_video_refresh = NULL;
    core->retro_set_audio_sample = NULL;
    core->retro_set_audio_sample_batch = NULL;
    core->retro_set_input_state = NULL;
    core->retro_api_version = NULL;
    core->retro_set_input_poll = NULL;
    core->retro_get_system_av_info = NULL;
    core->retro_get_system_info = NULL;
    core->configured = 0;
    r = 0;
  }

  return r;
}

static int liblretro_core_configure(liblretro_core_t *core) {
  struct retro_game_info *gameinfo;
  vfs_file_t *f;
  int api, ok, r = -1;

  debug(DEBUG_INFO, "LIBRETRO", "liblretro_core_configure begin");

  if (core) {
    num_vars = 0;
    debug(DEBUG_INFO, "LIBRETRO", "retro_set_environment");
    core->retro_set_environment(liblretro_environment);
    debug(DEBUG_INFO, "LIBRETRO", "retro_init");
    core->retro_init();

    debug(DEBUG_INFO, "LIBRETRO", "retro_api_version");
    if ((api = core->retro_api_version()) == RETRO_API_VERSION) {
      xmemset(&core->systeminfo, 0, sizeof(struct retro_system_info));
      debug(DEBUG_INFO, "LIBRETRO", "retro_get_system_info");
      core->retro_get_system_info(&core->systeminfo);
      debug(DEBUG_INFO, "LIBRETRO", "library name %s", core->systeminfo.library_name);
      debug(DEBUG_INFO, "LIBRETRO", "library version %s", core->systeminfo.library_version);
      debug(DEBUG_INFO, "LIBRETRO", "needs full path %s", core->systeminfo.need_fullpath ? "true" : "false");
      debug(DEBUG_INFO, "LIBRETRO", "valid extensions %s", core->systeminfo.valid_extensions);

      /*
        If need_fullpath is true and retro_load_game() is called:
           - retro_game_info::path is guaranteed to have a valid path
           - retro_game_info::data and retro_game_info::size are invalid
    
        If need_fullpath is false and retro_load_game() is called:
           - retro_game_info::path may be NULL
           - retro_game_info::data and retro_game_info::size are guaranteed to be valid
      */

      xmemset(&core->gameinfo, 0, sizeof(struct retro_game_info));
      gameinfo = NULL;
      ok = 0;

      if (core->gamepath) {
        gameinfo = &core->gameinfo;
        if (core->systeminfo.need_fullpath) {
          debug(DEBUG_INFO, "LIBRETRO", "core needs game fullpath \"%s\"", core->gamepath);
          gameinfo->path = core->gamepath;
          ok = 1;
        } else {
          debug(DEBUG_INFO, "LIBRETRO", "core does not need game fullpath \"%s\"", core->gamepath);
          gameinfo->path = core->gamepath;
          f = vfs_open(session, (char *)gameinfo->path, VFS_READ);
          gameinfo->data = xcalloc(1, 65536);
          gameinfo->size = vfs_read(f, (uint8_t *)gameinfo->data, 65536);
          vfs_close(f);
          ok = 1;
        }
      } else {
        if (core->no_game) {
          debug(DEBUG_INFO, "LIBRETRO", "core does not need a game");
          ok = 1;
        } else {
          debug(DEBUG_ERROR, "LIBRETRO", "core requires a game but gamepath is null");
        }
      }

      if (ok) {
        debug(DEBUG_INFO, "LIBRETRO", "retro_load_game");
        if (core->retro_load_game(gameinfo)) {
          core->configured = 1;
          r = 0;

        } else {
          debug(DEBUG_ERROR, "LIBRETRO", "retro_load_game failed");
        }
      }

      xmemset(&core->avinfo, 0, sizeof(struct retro_system_av_info));
      debug(DEBUG_INFO, "LIBRETRO", "retro_get_system_av_info");
      core->retro_get_system_av_info(&core->avinfo);
      debug(DEBUG_INFO, "LIBRETRO", "video fps %.1f", core->avinfo.timing.fps);
      debug(DEBUG_INFO, "LIBRETRO", "video width %d", core->avinfo.geometry.base_width);
      debug(DEBUG_INFO, "LIBRETRO", "video height %d", core->avinfo.geometry.base_height);

      debug(DEBUG_INFO, "LIBRETRO", "retro_set_video_refresh");
      core->retro_set_video_refresh(liblretro_video_refresh);
      debug(DEBUG_INFO, "LIBRETRO", "retro_set_audio_sample");
      core->retro_set_audio_sample(liblretro_audio_sample);
      debug(DEBUG_INFO, "LIBRETRO", "retro_set_audio_sample_batch");
      core->retro_set_audio_sample_batch(liblretro_audio_sample_batch);
      debug(DEBUG_INFO, "LIBRETRO", "retro_set_input_state");
      core->retro_set_controller_port_device(0, RETRO_DEVICE_MOUSE);
      core->retro_set_input_state(liblretro_input_state);
      debug(DEBUG_INFO, "LIBRETRO", "retro_set_input_poll");
      core->retro_set_input_poll(liblretro_input_poll);

    } else {
      debug(DEBUG_ERROR, "LIBRETRO", "wrong api version %d", api);
    }
  }

  debug(DEBUG_INFO, "LIBRETRO", "liblretro_core_configure end %d", r);
  return r;
}

static int liblretro_core_finish(liblretro_core_t *core) {
  int r = -1;

  if (core) {
    core->retro_unload_game();
    core->retro_deinit();
    r = 0;
  }

  return r;
}

static int liblretro_core_start(liblretro_core_t *core) {
  debug(DEBUG_INFO, "LIBRETRO", "liblretro_core_start");

  if (!core->configured) {
    if (liblretro_core_configure(core) != 0) return -1;
  }

  last_x = last_y = 0;
  ebuttons = 0;

  dt = 1000000 / core->avinfo.timing.fps;
  debug(DEBUG_INFO, "LIBRETRO", "fps=%.1f dt=%ld", core->avinfo.timing.fps, dt);
  stop = 0;

  return 0;
}

static int liblretro_core_stop(liblretro_core_t *core) {
  debug(DEBUG_INFO, "LIBRETRO", "liblretro_core_stop");
  if (stop) {
    liblretro_core_finish(core);
    core->configured = 0;
  }

  dt = 0;

  return 0;
}

static Err LibretroInit(char *corepath, char *gamepath) {
  int r = -1;

  xmemset(&core, 0, sizeof(liblretro_core_t));

  if (liblretro_core_load(&core, corepath, gamepath) == 0) {
    if (liblretro_core_configure(&core) == 0) {
      r = 0;
    }
  }

  return r;
}

static void LibretroFinish(void) {
  liblretro_core_finish(&core);
  liblretro_core_unload(&core);
}

static void resize(void) {
  WinHandle wh;
  RectangleType rect;
  UInt32 swidth, sheight;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = WinGetDisplayWindow();
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect);
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  UInt32 key, keycode, modifiers, i;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case winDisplayChangedEvent:
      resize();
      handled = true;
      break;

    case modKeyDownEvent:
    case modKeyUpEvent:
      if (event->data.keyDown.modifiers & shiftKeyMask) key = RETROK_LSHIFT;
      else if (event->data.keyDown.modifiers & controlKeyMask) key = RETROK_LCTRL;
      else if (event->data.keyDown.modifiers & optionKeyMask)  key = RETROK_LALT;
      else key = 0;

      if (key) {
        core.retro_keyboard_event(event->eType == modKeyDownEvent, key, key, 0);
      }
      break;

    case keyDownEvent:
      if (dt && !pause) {
        key = event->data.keyDown.chr;

        if ((event->data.keyDown.modifiers & commandKeyMask)) {
          switch (key) {
            case vchrHard1:  key = RETROK_F1; break;
            case vchrHard2:  key = RETROK_F2; break;
            case vchrHard3:  key = RETROK_F3; break;
            case vchrHard4:  key = RETROK_F4; break;
            case vchrHard5:  key = RETROK_F5; break;
            case vchrHard6:  key = RETROK_F6; break;
            case vchrHard7:  key = RETROK_F7; break;
            case vchrHard8:  key = RETROK_F8; break;
            case vchrHard9:  key = RETROK_F9; break;
            case vchrHard10: key = RETROK_F10; break;
            case vchrNativeInsert: key = RETROK_INSERT; break;
            case vchrNativeDelete: key = RETROK_DELETE; break;
            case vchrNativeHome:   key = RETROK_HOME; break;
            case vchrNativeEnd:    key = RETROK_END; break;
            case vchrNativePgUp:   key = RETROK_PAGEUP; break;
            case vchrNativePgDown: key = RETROK_PAGEDOWN; break;
            default: key = 0; break;
          }

          if (key) {
            modifiers = 0;
            if (event->data.keyDown.modifiers & shiftKeyMask)   modifiers |= RETROKMOD_SHIFT;
            if (event->data.keyDown.modifiers & controlKeyMask) modifiers |= RETROKMOD_CTRL;
            if (event->data.keyDown.modifiers & optionKeyMask)  modifiers |= RETROKMOD_ALT;
            core.retro_keyboard_event(true,  key, key, modifiers);
            core.retro_keyboard_event(false, key, key, modifiers);
          }

        } else {
          switch (key) {
            case 10:              key = 13; break;
            case vchrPageUp:      key = RETROK_UP; break;
            case vchrPageDown:    key = RETROK_DOWN; break;
            case vchrRockerLeft:  key = RETROK_LEFT; break;
            case vchrRockerRight: key = RETROK_RIGHT; break;
          }
          keycode = key;
          modifiers = 0;

          if (event->data.keyDown.modifiers & shiftKeyMask) {
            modifiers |= RETROKMOD_SHIFT;
            core.retro_keyboard_event(true,  RETROK_LSHIFT, 0, 0);
            for (i = 0; keymap[i].to; i++) {
              if (keymap[i].to == key && keymap[i].mods == WINDOW_MOD_SHIFT) {
                key = keycode = keymap[i].from;
                break;
              }
            }
          } else if (event->data.keyDown.modifiers & controlKeyMask) {
            modifiers |= RETROKMOD_CTRL;
            core.retro_keyboard_event(true,  RETROK_LCTRL, 0, 0);
            for (i = 0; keymap[i].to; i++) {
              if (keymap[i].to == key && keymap[i].mods == WINDOW_MOD_CTRL) {
                key = keycode = keymap[i].from;
                break;
              }
            }
          } else if (event->data.keyDown.modifiers & optionKeyMask) {
            modifiers |= RETROKMOD_ALT;
            core.retro_keyboard_event(true,  RETROK_LALT, 0, 0);
          }

          core.retro_keyboard_event(true,  keycode, key, modifiers);
          core.retro_keyboard_event(false, keycode, key, modifiers);

          if (event->data.keyDown.modifiers & shiftKeyMask) {
            core.retro_keyboard_event(false, RETROK_LSHIFT, 0, 0);
          } else if (event->data.keyDown.modifiers & controlKeyMask) {
            core.retro_keyboard_event(false, RETROK_LCTRL, 0, 0);
          } else if (event->data.keyDown.modifiers & optionKeyMask) {
            core.retro_keyboard_event(false, RETROK_LALT, 0, 0);
          }
          handled = true;
        }
      }
      break;

    default:
      break;
  }

  return handled;
}

static void EventLoop(void) {
  int64_t real_dt;
  EventType event;
  Int32 wait;
  Err err;

  do {
    wait = 500000;
    if (dt && !pause) {
      real_dt = sys_get_clock();
      core.retro_run();
      real_dt = sys_get_clock() - real_dt;
      if (real_dt < dt) {
        wait = dt - real_dt;
        if (wait > 5000) wait = 5000;
      } else {
        wait = 0;
      }
      debug(DEBUG_TRACE, "LIBRETRO", "dt=%ld wait=%d", real_dt, wait);
    }

    EvtGetEventUs(&event, wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;

  } while (event.eType != appStopEvent && !stop);
}

static Err StartApplication(char *corepath, char *gamepath) {
  Err err = -1;

  dt = 0;
  pause = 0;
  stop = 0;

  if (LibretroInit(corepath, gamepath) == 0) {
    session = vfs_open_session();
    liblretro_core_start(&core);
    err = errNone;
  }

  return err;
}

static void StopApplication(void) {
  int i;

  if (dt) {
    liblretro_core_stop(&core);
    LibretroFinish();
  }
  vfs_close_session(session);

  for (i = 0; i < num_vars; i++) {
    if (variables[i].value) {
      xfree((char *)variables[i].value);
    }
  }
}

static void *PluginMain(void *p) {
  libretro_plugin_t *lp = (libretro_plugin_t *)p;

  if (mutex_lock(mutex) == 0) {
    if (lp && StartApplication(lp->corepath, lp->gamepath) == errNone) {
      pumpkin_set_native_keys(1);
      pumpkin_set_cursor(0);
      EventLoop();
      pumpkin_set_cursor(1);
      pumpkin_set_native_keys(0);
      StopApplication();
    }
    mutex_unlock(mutex);
  }

  return NULL;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  mutex = mutex_create("libretro");

  *type = emulationPluginType;
  *id = libretroPluginId;

  return PluginMain;
}
