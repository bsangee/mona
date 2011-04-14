#include <monapi/messages.h>
#include <servers/screen.h>

#include "Shell.h"

using namespace MonAPI;

int main(int argc, char* argv[])
{
    if (M_OK != monapi_register_to_server("/servers/keyboard")) exit(1);

    /* Server start ok */
    if (monapi_notify_server_start("MONITOR.BIN") != M_OK)
    {
        MONAPI_WARN("MONITOR not found");
        exit(-1);
    }
    MessageInfo msg;
    uint32_t targetID = Message::lookupMainThread("SCREEN.EX5");
    if (targetID == THREAD_UNKNOWN || (Message::sendReceive(&msg, targetID, MSG_SCREEN_GET_STREAM_HANDLE) != M_OK)) {
        MONAPI_WARN("SCREEN.EX5 not found");
        exit(-1);
    }
    uint32_t screenHandle = msg.arg2;

    /* service loop */
    Shell shell(screenHandle);
    shell.run();

    monapi_register_to_server("/servers/keyboard");
    return 0;
}
