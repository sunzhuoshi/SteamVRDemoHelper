#pragma once
#include <sstream>
#include <WinUser.h>
#include "util\l4util.h"

#define VR_DEMO_SUPPRESS_DOWHILE_WARNING()      \
    __pragma (warning (push))                   \
    __pragma (warning (disable:4127))

#define VR_DEMO_RESTORE_DOWHILE_WARNING()       \
    __pragma (warning (pop))


#define VR_DEMO_ALERT_SS(caption, text)                                                             \
    VR_DEMO_SUPPRESS_DOWHILE_WARNING()                                                              \
    do {                                                                                            \
        std::ostringstream msg;                                                                     \
        msg << text;                                                                                \
        MessageBoxA(NULL, msg.str().c_str(), caption, MB_OK);                                       \
    } while (false)                                                                                 \
    VR_DEMO_RESTORE_DOWHILE_WARNING()

#define VR_DEMO_ALERT_IS(captionStringId, text)                                                     \
    VR_DEMO_ALERT_SS(l4util::loadString(captionStringId).c_str(), text)

#define VR_DEMO_ALERT_II(captionStringId, textStringId)                                             \
    VR_DEMO_ALERT_SS(l4util::loadString(captionStringId).c_str(), l4util::loadString(captionStringId).c_str())


