#include <monapi.h>
#include <monalibc.h>
#include <servers/audio.h>
#include "../debug.h"

using namespace MonAPI;

int main(int argc, char *argv[])
{
	uint32_t audioServerID;
	uint32_t commanderID;
	int32_t ch;
	char str[128];
	MessageInfo msg;

	audioServerID = Message::lookupMainThread("AUDIO.EX5");
	if( audioServerID == THREAD_UNKNOWN )
		exit(printf("Couldn't find an audio server."));
	dprintf("AUDIO.EX5 : %x\n", audioServerID);

	Message::sendReceive(&msg, audioServerID, MSG_AUDIO_SERVER_COMMAND, GetThreadID, COMMAND_THREAD);
	commanderID = msg.arg2;
	dprintf("AUDIO.EX5 : %x\n", commanderID);

#if 1
	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, GetServerVersion);
	_printf("Server version: %x:%x\n", msg.arg2, msg.arg3);
	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, AllocateChannel);
	ch = msg.arg2;
	_printf("Channel: %d\n", ch);
	struct audio_server_channel_info ci;
	ci.channel = ch;
	ci.samplerate = 44100;
	ci.bitspersample = 16;
	ci.channels = 2;
	memcpy(str, &ci, sizeof(ci));
	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, PrepareChannel, 0, 0, str);
	_printf("Result: %d\n", msg.arg2);

	struct audio_server_buffer_info bi;
	monapi_cmemoryinfo *mi;
	mi = monapi_cmemoryinfo_new();
	if( !monapi_cmemoryinfo_create(mi, 44100, MONAPI_TRUE) )
	{
		monapi_cmemoryinfo_delete(mi); return -1;
	}
	for( int i = 0 ; i < 44100/2 ; i++ )
	{
		if( i % 20 < 10 ) ((short*)mi->Data)[i] = 200;
		else ((short*)mi->Data)[i] = -200;
	}
	bi.handle = mi->Handle;
	bi.size = mi->Size;
	memcpy(str, &bi, sizeof(bi));
	_printf("%x\n", bi.handle);
	printf("I will set a buffer...\n");
	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, SetBuffer, ch, 0, str);

	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, StartChannel, ch);
	while(1);

	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, StopChannel, ch);

	Message::sendReceive(&msg, commanderID, MSG_AUDIO_SERVER_COMMAND, ReleaseChannel, ch);
	_printf("Channel was released.\n");

	monapi_cmemoryinfo_dispose(mi);
	monapi_cmemoryinfo_delete(mi);

#endif

	return 0;
}
