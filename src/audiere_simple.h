#include "audiere.h"

using namespace audiere;

ADR_FUNCTION(void) AdrSimplePlay(void* buffer, long size, bool loop, FileFormat format = FF_AUTODETECT);
ADR_FUNCTION(void) AdrSimplePlayFile(char* filename, bool loop);
ADR_FUNCTION(void) AdrSimplePlayFileW(wchar_t* filename, bool loop);
ADR_FUNCTION(void) AdrSimpleStop(void);
ADR_FUNCTION(BOOL) AdrSimpleIsPlaying(void);
