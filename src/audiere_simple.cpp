#include <windows.h>
#include "audiere_simple.h"

class SimpleAudierePlayer
{
	private:
		AudioDevicePtr device;
		FilePtr memoryfile;
		OutputStreamPtr stream;

	public:
		SimpleAudierePlayer(void* buffer, long size, bool loop, FileFormat format)
		{
			device = OpenDevice();
			if (!device) return;
			memoryfile = CreateMemoryFile(buffer, size, false);
			if (!memoryfile) return;
			stream = OpenSound(device, memoryfile, true, format);
			if (!stream) return;
			stream->setRepeat(loop);
			stream->play();
		}

		SimpleAudierePlayer(char* filename, bool loop)
		{
			device = OpenDevice();
			if (!device) return;
			stream = OpenSound(device, filename, true);
			if (!stream) return;
			stream->setRepeat(loop);
			stream->play();
    }

    SimpleAudierePlayer(wchar_t* filename, bool loop)
    {
      device = OpenDevice();
      if (!device) return;
      stream = OpenSound(device, filename, true);
      if (!stream) return;
      stream->setRepeat(loop);
      stream->play();
		}

		~SimpleAudierePlayer() { }

		BOOL IsPlaying()
		{
			return stream->isPlaying();
		}
};

SimpleAudierePlayer* player;

ADR_FUNCTION(void) AdrSimplePlay(void* buffer, long size, bool loop, FileFormat format)
{
	AdrSimpleStop();
	player = new SimpleAudierePlayer(buffer, size, loop, format);
}

ADR_FUNCTION(void) AdrSimplePlayFile(char* filename, bool loop)
{
	AdrSimpleStop();
	player = new SimpleAudierePlayer(filename, loop);
}

ADR_FUNCTION(void) AdrSimplePlayFileW(wchar_t* filename, bool loop)
{
  AdrSimpleStop();
  player = new SimpleAudierePlayer(filename, loop);
}

ADR_FUNCTION(void) AdrSimpleStop(void)
{
	if (player != NULL)
	{
		delete player;
		player = NULL;
	}
}

ADR_FUNCTION(BOOL) AdrSimpleIsPlaying(void)
{
	return player && player->IsPlaying();
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to the DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_DETACH:
			AdrSimpleStop();
	}

	return TRUE;
}
