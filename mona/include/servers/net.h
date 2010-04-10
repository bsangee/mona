#ifndef _SERVER_NET_
#define _SERVER_NET_

enum
{
    MSG_NET_SOCKET_CONN   = 0x69262736,    // NET:CON
    MSG_NET_SOCKET_SOCK   = 0x6927212a,    // NET:SCK
    MSG_NET_SOCKET_RECV   = 0x69271156,    // NET:RCV
    MSG_NET_SOCKET_SEND   = 0x6927268e, // NET:SND
    MSG_NET_SOCKET_CLOSE  = 0x692625ca, // NET:CLS
    MSG_NET_GET_ADDR_INFO = 0x6926600e,    // NET:GAD
    MSG_NET_SOCKET_SET_OPTION =  0x6927273e, // NET:SOP
    MSG_NET_SOCKET_GET_OPTION =  0x6926673e, // NET:GOP
    MSG_NET_SOCKET_SHUTDOWN   = 0x692723ce, // NET:SHT
    MSG_NET_SOCKET_LISTEN   = 0x6926b936, // NET:LSN
    MSG_NET_SOCKET_BIND   = 0x6926168e, // NET:BND
    MSG_NET_SOCKET_ACCEPT   =  0x6926014e, // NET:ACT
    MSG_NET_SELECT   =  0x6927258a, // NET:SLC
    MSG_NET_SOCKET_CONTROL   = 0x692629ae, // NET:CTL
    /* deprecated */
    MSG_NET_TCP_CONNECT   = 0x69262736, // NET:CON
    MSG_NET_TCP_SEND      = 0x6927268e, // NET:SND
    MSG_NET_TCP_RECEIVE   = 0x69271156, // NET:RCV
    MSG_NET_TCP_IS_CLOSED = 0x6926812e, // NET:ICL
    MSG_NET_WEB_GET       = 0x6927634e  // NET:WGT
};

#endif
