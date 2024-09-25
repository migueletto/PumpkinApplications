#include "deviceinfo.h"
#include <PalmOS.h>

Boolean deviceinfo_colorSupported() {
    Boolean supportsColor = false;
    WinScreenMode(winScreenModeGetSupportsColor, (UInt32 *)NULL, (UInt32 *)NULL, (UInt32 *)NULL, &supportsColor);
    return supportsColor;
}

Int32 deviceinfo_currentDepth() {
    UInt32 depth = 0;
    WinScreenMode(winScreenModeGet, (UInt32 *)NULL, (UInt32 *)NULL, &depth, (Boolean *)NULL);
    return depth;
}

Int32 deviceinfo_maxDepth() {
    UInt32 supportedDepths = 0;
    WinScreenMode(winScreenModeGetSupportedDepths, (UInt32 *)NULL, (UInt32 *)NULL, &supportedDepths, (Boolean *)NULL);
    if (supportedDepths & 0x80) {
        return 8;
    } else if (supportedDepths & 0x0B) {
        return 4;
    } else {
        return -1;
    }
}
