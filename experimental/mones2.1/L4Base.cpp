//$Id$
#include <pci/Pci.h>
#include "Net.h"
#include "NE2000.h"
#include "MonAMDpcn.h"
#include "LoopBack.h"
#include "Dispatch.h"

using namespace mones;
using namespace MonAPI;

void L4Base::Close()
{
    dispatcher->RemoveInfo(this,0);
}

word L4Base::checksum(byte *data,word size)
{
    dword sum=0;
    for(int i=0;i<=size-2;i+=2){
        sum+=bswap(*(word*)(data+i));
    }
    if(size%2==1){
        sum+=bswap(*(data+size-1)&0xFF);
    }
    return ~(((sum>>16)+sum)&0xFFFF);
}

void L4Base::CreateIPHeader(Ether* frame,word length,byte protocol)
{
    IP* ip=frame->IPHeader;
    ip->verhead=0x45;       //version & headersize
    ip->tos=0x00;           //tos
    ip->len=bswap(length);  //totallength PING size windows 60 solaris 84
    ip->id=bswap(0xabcd);   //made from PID?
    ip->frag=bswap(0x0000); //flag
    ip->ttl=0x80;           //TTL
    ip->prot=protocol;      
    ip->chksum=0x0;         //for calculating checksum, it should be zero.  
    ip->chksum=bswap(checksum((byte*)ip,(ip->verhead&0x0F)<<2));
}

void L4Base::Read(MessageInfo* m)
{
    memcpy(&msg,(byte*)m,sizeof(MessageInfo));
}

void L4Base::Read_bottom_half(Ether* frame)
{
    byte* data;
    monapi_cmemoryinfo* mi = monapi_cmemoryinfo_new();  
    if (mi != NULL){
        if( frame != NULL ){
            int size=Strip(frame, &data);
            if( size >0 ){
                monapi_cmemoryinfo_create(mi,size, true);
                if( mi != NULL ){
                    memcpy(mi->Data,data,mi->Size);    
                    Message::reply(&msg, mi->Handle, mi->Size);
                    memset(&msg,'\0',sizeof(MessageInfo));
                }
            }
            //dispose
        }
        monapi_cmemoryinfo_delete(mi);
    }else{
        Message::reply(&msg);
    }
}

void L4Base::Write(MessageInfo* m)
{ 
    monapi_cmemoryinfo* ret = monapi_cmemoryinfo_new();
    if( ret != NULL){
        ret->Handle = m->arg2;
        ret->Owner  = m->from;
        ret->Size   = m->arg3;
        monapi_cmemoryinfo_map(ret);
        if( netdsc == m->arg1 ){
            dispatcher->Send(ret->Data,ret->Size,this);           
            memcpy(&msg,(byte*)m,sizeof(MessageInfo)); //Register msg.
        }
        monapi_cmemoryinfo_delete(ret);
    }
    Message::reply(m);
}