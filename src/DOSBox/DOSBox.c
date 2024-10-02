#include <PalmOS.h>

#include "pumpkin.h"
#include "libretro_plugin.h"
#include "dosbox.h"

static const char *COREPATH = "/app_card/" DOSBOX_LIBRETRO "/cores/dosbox_libretro" SOEXT;
static const char *GAMEPATH = "vfs/app_card/" DOSBOX_HOME "/dosbox.conf";

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  pumpkin_plugin_t *plugin;
  libretro_plugin_t lp;

  if (cmd == sysAppLaunchCmdNormalLaunch || cmd == dosboxLaunchCmd) {
    if ((plugin = pumpkin_get_plugin(emulationPluginType, libretroPluginId)) != NULL) {
      lp.corepath = (char *)COREPATH;
      lp.gamepath = (char *)GAMEPATH;
      plugin->pluginMain(&lp);
    }
  }

  return 0;
}
