#ifndef _SERVER_CLIPBOARD_
#define _SERVER_CLPIBPARD_

enum
{
    MSG_CLIPBOARD_SET   = 0x12def91e, // CLP:SET
    MSG_CLIPBOARD_GET   = 0x12def73e, // CLP:GET
    MSG_CLIPBOARD_CLEAR = 0x12de25c6  // CLP:CLR
};

#endif
